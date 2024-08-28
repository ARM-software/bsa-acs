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

#define TEST_NUM  (ACS_WAKEUP_TEST_NUM_BASE + 1)
#define TEST_RULE "B_WAK_01, B_WAK_02, B_WAK_03, B_WAK_04, B_WAK_05 \
                    \n       B_WAK_06, B_WAK_07, B_WAK_10, B_WAK_11"
#define TEST_DESC "Wake from EL1 PHY Timer Int           "

extern uint32_t g_wakeup_timeout;
static uint32_t g_el1phy_int_received;

static
void
isr1()
{
  uint32_t intid;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  val_timer_set_phy_el1(0);
  val_print(ACS_PRINT_INFO, "       Received EL1 PHY interrupt\n", 0);
  g_el1phy_int_received = 1;
  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  val_gic_end_of_interrupt(intid);
}

static
void
payload1()
{
  uint32_t intid;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint64_t delay_loop = val_get_counter_frequency() * g_wakeup_timeout;
  uint64_t timer_expire_val = val_get_counter_frequency() * g_wakeup_timeout;

  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  if (val_gic_install_isr(intid, isr1)) {
    val_print(ACS_PRINT_WARN, "\n       GIC Install Handler Failed...", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
    return;
  }
  g_el1phy_int_received = 0;
  val_timer_set_phy_el1(timer_expire_val);
  val_power_enter_semantic(BSA_POWER_SEM_B);

  /* Add a delay loop after WFI called in case PE needs some time to enter WFI state
   * exit if test int comes
  */
  while (delay_loop && (g_el1phy_int_received == 0)) {
      delay_loop--;
  }

  /* We are here means
   * 1. test interrupt has come (PASS) isr1
   * 2. some other system interrupt or event has wakeup the PE (SKIP)
   * 3. PE didn't enter WFI mode, treating as (SKIP), as finding 2nd or 3rd case not feasible
   * 4. Hang, if PE didn't exit WFI (FAIL)
  */
  if (!g_el1phy_int_received) {
      val_timer_set_phy_el1(0);
      intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
      val_gic_end_of_interrupt(intid);
      val_print(ACS_PRINT_DEBUG, "\n       PE wakeup by some other events/int", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
  }
  val_print(ACS_PRINT_INFO, "\n       delay loop remainig value %d", delay_loop);
  return;
}

uint32_t
os_u001_entry(uint32_t num_pe)
{
  uint32_t status = ACS_STATUS_SKIP;

  num_pe = 1;  //This Timer test is run on single processor

  if (!g_el1physkip) {
      status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

      if (status != ACS_STATUS_SKIP)
          val_run_test_payload(TEST_NUM, num_pe, payload1, 0);

      status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

      val_report_status(0, ACS_END(TEST_NUM), NULL);

  }
  return status;
}
