/** @file
 * Copyright (c) 2016-2018, 2021, 2023, Arm Limited or its affiliates. All rights reserved.
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

#define TEST_NUM   (ACS_TIMER_TEST_NUM_BASE + 2)
#define TEST_RULE  "B_TIME_06"
#define TEST_DESC  "SYS Timer if PE Timer not ON          "

static
void
payload()
{

  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  if (val_timer_get_info(TIMER_INFO_NUM_PLATFORM_TIMERS, 0) == 0) {
      val_print(ACS_PRINT_INFO, "\n Physical EL1 timer flag = %x",
                val_timer_get_info(TIMER_INFO_PHY_EL1_FLAGS, 0));
      val_print(ACS_PRINT_INFO, "\n Physical EL2 timer flag = %x",
                val_timer_get_info(TIMER_INFO_PHY_EL2_FLAGS, 0));
      val_print(ACS_PRINT_INFO, "\n Virtual EL1 timer flag  = %x",
                val_timer_get_info(TIMER_INFO_VIR_EL1_FLAGS, 0));

      if ((val_timer_get_info(TIMER_INFO_PHY_EL1_FLAGS, 0) & BSA_TIMER_FLAG_ALWAYS_ON) &&
        (val_timer_get_info(TIMER_INFO_PHY_EL2_FLAGS, 0) & BSA_TIMER_FLAG_ALWAYS_ON) &&
        (val_timer_get_info(TIMER_INFO_VIR_EL1_FLAGS, 0) & BSA_TIMER_FLAG_ALWAYS_ON)) {
          val_set_status(index, RESULT_PASS(TEST_NUM, 1));
      } else {
          val_print(ACS_PRINT_ERR, "\n       PE Timers are not always-on\n"
                                   "       And no system wake up timer", 0);
          val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
      }
  } else {
      val_set_status(index, RESULT_PASS(TEST_NUM, 2));
  }

}

uint32_t
os_t002_entry(uint32_t num_pe)
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
