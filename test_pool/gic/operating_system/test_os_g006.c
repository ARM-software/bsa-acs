/** @file
 * Copyright (c) 2016-2021, 2023-2025, Arm Limited or its affiliates. All rights reserved.
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

#include "val/common/include/acs_gic.h"
#include "val/common/include/acs_pe.h"

#define TEST_NUM   (ACS_GIC_TEST_NUM_BASE + 6)
#define TEST_RULE  "B_PPI_00"
#define TEST_DESC  "Check EL1-Phy timer PPI assignment    "


static uint32_t intid;

static
void
isr_phy()
{
  val_timer_set_phy_el1(0);
  val_print(ACS_PRINT_INFO, "\n       Received phy el1 interrupt   ", 0);
  val_set_status(0, RESULT_PASS(TEST_NUM, 1));
  val_gic_end_of_interrupt(intid);
}

static
void
payload()
{
  /* Check non-secure physical timer Private Peripheral Interrupt (PPI) assignment */
  uint32_t timeout = TIMEOUT_LARGE;
  uint32_t timer_expire_val = 100;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  val_set_status(0, RESULT_PENDING(TEST_NUM));
  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);

  if ((intid < 16 || intid > 31) && (!val_gic_is_valid_eppi(intid)))
      val_print(ACS_PRINT_WARN,
          "\n       EL0-Phy timer not mapped to PPI recommended range, INTID: %d   ", intid);

  if (val_gic_install_isr(intid, isr_phy)) {
      val_print(ACS_PRINT_ERR, "\n       GIC Install Handler Failed...", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
      return;
  }

  val_timer_set_phy_el1(timer_expire_val);

  while ((--timeout > 0) && (IS_RESULT_PENDING(val_get_status(index)))) {
        ;
  }

  if (timeout == 0) {
    val_print(ACS_PRINT_ERR,
        "\n       EL0-Phy timer interrupt not received on INTID: %d   ", intid);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
    return;
  }

}

uint32_t
os_g006_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  /* This GIC test is run on single processor */

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
