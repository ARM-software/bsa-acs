/** @file
 * Copyright (c) 2021, 2023-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "val/bsa/include/bsa_val_interface.h"

#include "val/common/include/acs_timer.h"
#include "val/common/include/acs_pe.h"

#define TEST_NUM   (ACS_TIMER_TEST_NUM_BASE + 5)
#define TEST_RULE  "B_TIME_09"
#define TEST_DESC  "Restore PE timer on PE wake up        "

static uint32_t intid;
static uint64_t cnt_base_n;
static int irq_received;
static uint32_t intid_phy;
static
void
isr_sys_timer()
{
  val_timer_disable_system_timer((addr_t)cnt_base_n);
  val_gic_end_of_interrupt(intid);
  irq_received = 1;
  val_print(ACS_PRINT_INFO, "\n       System timer interrupt received", 0);
}

static
void
isr_phy_el1()
{
  val_timer_set_phy_el1(0);
  val_print(ACS_PRINT_INFO, "\n       Received el1_phy interrupt   ", 0);
  val_gic_end_of_interrupt(intid_phy);
}

static
void
payload()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint64_t sys_timer_ticks = val_get_counter_frequency() * 1;
  uint64_t pe_timer_ticks = val_get_counter_frequency() * 2;
  uint32_t ns_timer = 0;
  uint64_t timer_num, timer_cnt;
  int32_t status;

  timer_num = val_timer_get_info(TIMER_INFO_NUM_PLATFORM_TIMERS, 0);

  while (timer_num--) {
      if (val_timer_get_info(TIMER_INFO_IS_PLATFORM_TIMER_SECURE, timer_num))
        continue;
      else{
        ns_timer++;
        break;
      }
  }

  if (!ns_timer) {
      val_print(ACS_PRINT_DEBUG, "\n       No non-secure systimer implemented", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
      return;
  }

  /* Start Sys timer*/
  cnt_base_n = val_timer_get_info(TIMER_INFO_SYS_CNT_BASE_N, timer_num);
  if (cnt_base_n == 0) {
      val_print(ACS_PRINT_WARN, "\n       CNT_BASE_N is zero                 ", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
      return;
  }

  val_timer_set_system_timer((addr_t)cnt_base_n, sys_timer_ticks);

  irq_received = 0;

  intid = val_timer_get_info(TIMER_INFO_SYS_INTID, timer_num);
  val_gic_install_isr(intid, isr_sys_timer);

  intid_phy = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  val_gic_install_isr(intid_phy, isr_phy_el1);

  /* Start EL1 PHY timer */
  val_timer_set_phy_el1(pe_timer_ticks);

  /* Put current PE in to low power mode*/
  status = val_suspend_pe(0, 0);
  if (status) {
      val_print(ACS_PRINT_DEBUG, "\n       Not able to suspend the PE : %d", status);
      val_timer_disable_system_timer((addr_t)cnt_base_n);
      val_gic_clear_interrupt(intid);
      val_timer_set_phy_el1(0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 3));
      return;
  }

  if (irq_received == 0) {
      val_print(ACS_PRINT_ERR, "\n       System timer interrupt not generated", 0);
      val_timer_disable_system_timer((addr_t)cnt_base_n);
      val_gic_clear_interrupt(intid);
      val_timer_set_phy_el1(0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
      return;
  }

  /* PE wake up from sys timer interrupt & start execution here */
  /* Read PE timer*/
  timer_cnt = val_get_phy_el1_timer_count();

  /*Disable PE timer*/
  val_timer_set_phy_el1(0);

  val_print(ACS_PRINT_INFO, "\n       Read back PE timer count :%d", timer_cnt);

  /* Check whether count is moved or not*/
  if ((timer_cnt < ((pe_timer_ticks - sys_timer_ticks) + (sys_timer_ticks/100)))
                                                      && (timer_cnt != 0))
    val_set_status(index, RESULT_PASS(TEST_NUM, 1));
  else
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
}

uint32_t
os_t005_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This Timer test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);
  return status;

}
