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

#define TEST_NUM  (ACS_WAKEUP_TEST_NUM_BASE + 4)
#define TEST_RULE "B_WAK_01, B_WAK_02, B_WAK_03, B_WAK_04, B_WAK_05 \
                    \n       B_WAK_06, B_WAK_07, B_WAK_10, B_WAK_11"
#define TEST_DESC "Wake from Watchdog WS0 Int            "

static uint64_t wd_num;
static uint32_t g_wd_int_received;
extern uint32_t g_wakeup_timeout;
static uint32_t g_failsafe_int_received;

static
void
isr_failsafe()
{
  uint32_t intid;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  val_timer_set_phy_el1(0);
  val_print(ACS_PRINT_ERR, "       Received Failsafe interrupt\n", 0);
  g_failsafe_int_received = 1;
  val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  val_gic_end_of_interrupt(intid);
}

static
void
isr4()
{
  uint32_t intid;
  val_wd_set_ws0(wd_num, 0);
  val_print(ACS_PRINT_INFO, "       Received WS0 interrupt\n", 0);
  g_wd_int_received = 1;
  intid = val_wd_get_info(wd_num, WD_INFO_GSIV);
  val_gic_end_of_interrupt(intid);
}

static
void
wakeup_set_failsafe()
{
  uint32_t intid;
  uint64_t timer_expire_val = (val_get_counter_frequency() * 3 * g_wakeup_timeout) / 2;

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
payload4()
{
  uint32_t status;
  uint32_t ns_wdg = 0;
  uint32_t intid;
  uint32_t delay_loop;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint64_t timer_expire_val = 1 * g_wakeup_timeout;

  wd_num = val_wd_get_info(0, WD_INFO_COUNT);

  // Assume a test passes until something causes a failure.
  val_set_status(index, RESULT_PASS(TEST_NUM, 1));

  if (!wd_num) {
      val_print(ACS_PRINT_DEBUG, "\n       No watchdog implemented      ", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
      return;
  }

  while (wd_num--) {
      if (val_wd_get_info(wd_num, WD_INFO_ISSECURE))
          continue;

      ns_wdg++;
      intid = val_wd_get_info(wd_num, WD_INFO_GSIV);
      status = val_gic_install_isr(intid, isr4);
      if (status == 0) {

          /* Set Interrupt Type Edge/Level Trigger */
          if (val_wd_get_info(wd_num, WD_INFO_IS_EDGE))
              val_gic_set_intr_trigger(intid, INTR_TRIGGER_INFO_EDGE_RISING);
          else
              val_gic_set_intr_trigger(intid, INTR_TRIGGER_INFO_LEVEL_HIGH);

          g_wd_int_received = 0;
          g_failsafe_int_received = 0;
	  wakeup_set_failsafe();
	  status = val_wd_set_ws0(wd_num, timer_expire_val);
          if (status) {
              wakeup_clear_failsafe();
    	      val_print(ACS_PRINT_ERR, "\n       Setting watchdog timeout failed", 0);
              val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
              return;
          }
          val_power_enter_semantic(BSA_POWER_SEM_B);

          /* Add a delay loop after WFI called in case PE needs some time to enter WFI state
           * exit in case test or failsafe int is received
          */
          delay_loop = val_get_counter_frequency() * g_wakeup_timeout;
	  while (delay_loop && (g_wd_int_received == 0) && (g_failsafe_int_received == 0)) {
              delay_loop--;
          }

          /* We are here means
           * 1. test interrupt has come (PASS) isr4
           * 2. failsafe int recvd before test int (FAIL) isr_failsafe
           * 3. some other system interrupt or event has wakeup the PE (SKIP)
           * 4. PE didn't enter WFI mode, treating as (SKIP), as finding 3rd,4th case not feasible
           * 5. Hang, if PE didn't exit WFI (FAIL)
          */
	  wakeup_clear_failsafe();
          if (!(g_wd_int_received || g_failsafe_int_received)) {
              intid = val_wd_get_info(wd_num, WD_INFO_GSIV);
	      val_gic_clear_interrupt(intid);
              val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
    	      val_print(ACS_PRINT_DEBUG,
                        "\n       PE wakeup by some other events/int or didn't enter WFI", 0);
	  }
	  val_print(ACS_PRINT_DEBUG, "\n       delay loop remainig value %d", delay_loop);
      } else {
          val_print(ACS_PRINT_WARN, "\n       GIC Install Handler Failed...", 0);
          val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
      }

      /* Disable watchdog so it doesn't trigger after this test. */
      val_wd_set_ws0(wd_num, 0);
  }

  if (!ns_wdg) {
      val_print(ACS_PRINT_DEBUG, "       No non-secure watchdog implemented\n", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
      return;
  }

}

uint32_t
os_u004_entry(uint32_t num_pe)
{
  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This Timer test is run on single processor

  /* Watchdog */
  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload4, 0);

  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
