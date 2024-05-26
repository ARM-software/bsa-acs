/** @file
 * Copyright (c) 2016-2019, 2021-2024, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#include "val/common/include/acs_val.h"
#include "val/common/include/acs_pe.h"
#include "val/bsa/include/bsa_val_interface.h"

#include "val/bsa/include/bsa_acs_wakeup.h"

#define TEST_NUM5  (ACS_WAKEUP_TEST_NUM_BASE + 5)
#define TEST_RULE5 "B_WAK_01, B_WAK_02, B_WAK_03, B_WAK_04, B_WAK_05 \
                    \n       B_WAK_06, B_WAK_07, B_WAK_10, B_WAK_11"
#define TEST_DESC5 "Wake from System Timer Int            "

static uint64_t timer_num;
extern uint32_t g_wakeup_timeout;
static uint32_t g_failsafe_int_rcvd;
static uint32_t g_timer_int_rcvd;

static
void
isr_failsafe()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t intid;

  val_timer_set_phy_el1(0);
  val_print(ACS_PRINT_ERR, "       Received Failsafe interrupt\n", 0);
  g_failsafe_int_rcvd = 1;
  val_set_status(index, RESULT_FAIL(TEST_NUM5, 1));
  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  val_gic_end_of_interrupt(intid);
}

static
void
isr5()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t intid;
  uint64_t cnt_base_n = val_timer_get_info(TIMER_INFO_SYS_CNT_BASE_N, timer_num);

  val_timer_disable_system_timer((addr_t)cnt_base_n);
  val_print(ACS_PRINT_INFO, "       Received Sys timer interrupt\n", 0);
  g_timer_int_rcvd = 1;
  val_set_status(index, RESULT_PASS(TEST_NUM5, 1));
  intid = val_timer_get_info(TIMER_INFO_SYS_INTID, timer_num);
  val_gic_end_of_interrupt(intid);
}

static
void
wakeup_set_failsafe()
{
  uint32_t intid;
  uint64_t timer_expire_val = val_get_counter_frequency() * (g_wakeup_timeout + 1);

  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  val_gic_install_isr(intid, isr_failsafe);
  val_timer_set_phy_el1(timer_expire_val);
}

static
void
wakeup_clear_failsafe()
{
  val_timer_set_phy_el1(0);
}

static
void
payload5()
{
  uint32_t status;
  uint32_t ns_timer = 0;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t intid;
  uint64_t delay_loop;
  uint64_t cnt_base_n;
  uint64_t timer_expire_val = val_get_counter_frequency() * g_wakeup_timeout;

  timer_num = val_timer_get_info(TIMER_INFO_NUM_PLATFORM_TIMERS, 0);
  if(!timer_num){
      val_print(ACS_PRINT_DEBUG, "\n       No system timers implemented", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM5, 1));
      return;
  }

  while(timer_num--) {
      if(val_timer_get_info(TIMER_INFO_IS_PLATFORM_TIMER_SECURE, timer_num))
          continue;
      ns_timer++;

      //Read CNTACR to determine whether access permission from NS state is permitted
      status = val_timer_skip_if_cntbase_access_not_allowed(timer_num);
      if(status == ACS_STATUS_SKIP){
          val_print(ACS_PRINT_DEBUG, "       Timer cntbase can't accessed\n", 0);
          val_set_status(index, RESULT_SKIP(TEST_NUM5, 2));
          return;
      }

      cnt_base_n = val_timer_get_info(TIMER_INFO_SYS_CNT_BASE_N, timer_num);
      if(cnt_base_n == 0){
          val_print(ACS_PRINT_DEBUG, "       Timer cntbase is invalid\n", 0);
          val_set_status(index, RESULT_SKIP(TEST_NUM5, 3));
          return;
      }

      intid = val_timer_get_info(TIMER_INFO_SYS_INTID, timer_num);
      status = val_gic_install_isr(intid, isr5);
      if(status == 0) {
          wakeup_set_failsafe();
          /* enable System timer */
          val_timer_set_system_timer((addr_t)cnt_base_n, timer_expire_val);
	  val_power_enter_semantic(BSA_POWER_SEM_B);

          /* Add a delay loop after WFI called in case PE needs some time to enter WFI state
           * exit in case test or failsafe int is received
          */
          delay_loop = 100;
          while (delay_loop && (g_timer_int_rcvd == 0) && (g_failsafe_int_rcvd == 0)) {
              delay_loop--;
          }

          /* We are here means
           * 1. test interrupt has come (PASS) isr5
           * 2. failsafe int recvd before test int (FAIL) isr_failsafe
           * 3. some other system interrupt or event has wakeup the PE (SKIP)
           * 4. PE didn't enter WFI mode, treating as (SKIP), as finding 3rd and 4th case not feasible
           * 5. Hang, if PE didn't exit WFI (FAIL)
          */
          wakeup_clear_failsafe();
          if (!(g_timer_int_rcvd || g_failsafe_int_rcvd)) {
              intid = val_timer_get_info(TIMER_INFO_SYS_INTID, timer_num);
	      val_gic_clear_interrupt(intid);
              val_set_status(index, RESULT_SKIP(TEST_NUM5, 4));
              val_print(ACS_PRINT_DEBUG, "\n       PE wakeup by some other events/int or didn't enter WFI", 0);
          }
          val_print(ACS_PRINT_DEBUG, "\n       delay loop remainig value %d", delay_loop);

      } else{
          val_print(ACS_PRINT_WARN, "\n       GIC Install Handler Failed...", 0);
          val_set_status(index, RESULT_FAIL(TEST_NUM5, 2));
          return;
      }
  }

  if(!ns_timer){
      val_print(ACS_PRINT_WARN, "       No non-secure systimer implemented\n", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM5, 5));
      return;
  }
}

uint32_t
os_u005_entry(uint32_t num_pe)
{
  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This Timer test is run on single processor

  /* System Timer */
  status = val_initialize_test(TEST_NUM5, TEST_DESC5, num_pe);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM5, num_pe, payload5, 0);

  status = val_check_for_error(TEST_NUM5, num_pe, TEST_RULE5);

  val_report_status(0, ACS_END(TEST_NUM5), NULL);

  return status;
}
