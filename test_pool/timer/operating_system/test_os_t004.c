/** @file
 * Copyright (c) 2016-2018,2021, 2023, Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/bsa_acs_val.h"
#include "val/include/val_interface.h"

#include "val/include/bsa_acs_timer.h"
#include "val/include/bsa_acs_wakeup.h"


#define TEST_NUM   (ACS_TIMER_TEST_NUM_BASE + 4)
#define TEST_RULE  "B_TIME_08"
#define TEST_DESC  "Generate Mem Mapped SYS Timer Intr    "

static uint32_t intid;
static uint64_t cnt_base_n;

static
void
isr()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  val_print(ACS_PRINT_INFO, "\n       Received sys timer interrupt   ", 0);
  val_timer_disable_system_timer((addr_t)cnt_base_n);
  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
  val_gic_end_of_interrupt(intid);
}


static
void
payload()
{

  volatile uint32_t timeout;
  uint32_t timer_expire_val = TIMEOUT_MEDIUM;
  uint32_t status, ns_timer = 0;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint64_t timer_num = val_timer_get_info(TIMER_INFO_NUM_PLATFORM_TIMERS, 0);

  if (!timer_num) {
      val_print(ACS_PRINT_DEBUG, "\n       No System timers are defined  ", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
      return;
  }

  while (timer_num) {
      timer_num--;  //array index starts from 0, so subtract 1 from count

      if (val_timer_get_info(TIMER_INFO_IS_PLATFORM_TIMER_SECURE, timer_num))
          continue;    //Skip Secure Timer

      ns_timer++;
      timeout = TIMEOUT_LARGE;
      val_set_status(index, RESULT_PENDING(TEST_NUM));     // Set the initial result to pending

      //Read CNTACR to determine whether access permission from NS state is permitted
      status = val_timer_skip_if_cntbase_access_not_allowed(timer_num);
      if (status == ACS_STATUS_SKIP) {
          val_print(ACS_PRINT_WARN,
                    "\n       Security doesn't allow access to timer registers      ", 0);
          val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
          return;
      }

      cnt_base_n = val_timer_get_info(TIMER_INFO_SYS_CNT_BASE_N, timer_num);
      if (cnt_base_n == 0) {
          val_print(ACS_PRINT_WARN, "\n       CNT_BASE_N is zero                 ", 0);
          val_set_status(index, RESULT_SKIP(TEST_NUM, 3));
          return;
      }

      /* Install ISR */
      intid = val_timer_get_info(TIMER_INFO_SYS_INTID, timer_num);
      if (val_gic_install_isr(intid, isr)) {
          val_print(ACS_PRINT_ERR, "\n       GIC Install Handler Failed...", 0);
          val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
          return;
      }

      /* enable System timer */
      val_timer_set_system_timer((addr_t)cnt_base_n, timer_expire_val);

      while ((--timeout > 0) && (IS_RESULT_PENDING(val_get_status(index))))
      ;

      if (timeout == 0) {
          val_print(ACS_PRINT_ERR, "\n       Sys timer interrupt not received on %d   ", intid);
          val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
          return;
      }
  }

  if (!ns_timer) {
      val_print(ACS_PRINT_WARN, "\n       No non-secure systimer implemented", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 5));
      return;
  }

}

uint32_t
os_t004_entry(uint32_t num_pe)
{
  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This Timer test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
