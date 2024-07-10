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

#define TEST_NUM2  (ACS_WAKEUP_TEST_NUM_BASE + 2)
#define TEST_RULE2 "B_WAK_01, B_WAK_02, B_WAK_03, B_WAK_04, B_WAK_05 \
                    \n       B_WAK_06, B_WAK_07, B_WAK_10, B_WAK_11"
#define TEST_DESC2 "Wake from EL1 VIR Timer Int           "

extern uint32_t g_wakeup_timeout;
static uint32_t g_el1vir_int_received;
static uint32_t g_failsafe_int_rcvd;

static
void
isr_failsafe()
{
  uint32_t intid;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  val_timer_set_phy_el1(0);
  val_print(ACS_PRINT_ERR, "       Received Failsafe interrupt\n", 0);
  g_failsafe_int_rcvd = 1;
  val_set_status(index, RESULT_FAIL(TEST_NUM2, 2));
  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
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
isr2()
{
  uint32_t intid;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  /* We received our interrupt, so disable timer from generating further interrupts */
  val_timer_set_vir_el1(0);
  val_print(ACS_PRINT_INFO, "       Received EL1 VIRT interrupt\n", 0);
  g_el1vir_int_received = 1;
  val_set_status(index, RESULT_PASS(TEST_NUM2, 1));
  intid = val_timer_get_info(TIMER_INFO_VIR_EL1_INTID, 0);
  val_gic_end_of_interrupt(intid);
}

static
void
payload2()
{
  uint32_t intid;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint64_t delay_loop = val_get_counter_frequency() * g_wakeup_timeout;
  uint64_t timer_expire_val = val_get_counter_frequency() * g_wakeup_timeout;

  intid = val_timer_get_info(TIMER_INFO_VIR_EL1_INTID, 0);
  if (val_gic_install_isr(intid, isr2)) {
    val_print(ACS_PRINT_WARN, "\n       GIC Install Handler Failed...", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM2, 1));
    return;
  }

  g_failsafe_int_rcvd = 0;
  g_el1vir_int_received = 0;
  wakeup_set_failsafe();
  val_timer_set_vir_el1(timer_expire_val);
  val_power_enter_semantic(BSA_POWER_SEM_B);

  /* Add a delay loop after WFI called in case PE needs some time to enter WFI state 
   * exit in case test or failsafe int is received
  */
  while (delay_loop && (g_el1vir_int_received == 0) && (g_failsafe_int_rcvd == 0)) {
      delay_loop--;
  }

  /* We are here means
   * 1. test interrupt has come (PASS) isr2
   * 2. failsafe int recvd before test int (FAIL) isr_failsafe
   * 3. some other system interrupt or event has wakeup the PE (SKIP)
   * 4. PE didn't enter WFI mode, treating as (SKIP), as finding 3rd or 4th case not feasible
   * 5. Hang, if PE didn't exit WFI (FAIL)
  */
  wakeup_clear_failsafe();
  if (!(g_el1vir_int_received || g_failsafe_int_rcvd)) {
      val_timer_set_vir_el1(0);
      intid = val_timer_get_info(TIMER_INFO_VIR_EL1_INTID, 0);
      val_gic_end_of_interrupt(intid);
      val_set_status(index, RESULT_SKIP(TEST_NUM2, 1));
      val_print(ACS_PRINT_DEBUG, "\n       PE wakeup by some other events/int or didn't enter WFI", 0);
  }
  val_print(ACS_PRINT_INFO, "\n       delay loop remainig value %d", delay_loop);
  return;
}

uint32_t
os_u002_entry(uint32_t num_pe)
{
  uint32_t status = ACS_STATUS_SKIP;

  num_pe = 1;  //This Timer test is run on single processor

  if (!g_el1physkip) {
      status = val_initialize_test(TEST_NUM2, TEST_DESC2, num_pe);

      if (status != ACS_STATUS_SKIP)
          val_run_test_payload(TEST_NUM2, num_pe, payload2, 0);

      status = val_check_for_error(TEST_NUM2, num_pe, TEST_RULE2);

      val_report_status(0, ACS_END(TEST_NUM2), NULL);
  }
  return status;
}
