/** @file
 * Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_gic.h"
#include "val/include/bsa_acs_pe.h"

#define TEST_NUM   (ACS_GIC_TEST_NUM_BASE + 7)
#define TEST_RULE  "B_PPI_01"
#define TEST_DESC  "Check EL1-Virt timer PPI assignment   "

static uint32_t intid;

static
void
isr_vir()
{
  val_timer_set_vir_el1(0);
  val_print(ACS_PRINT_INFO, "\n       Received virt el1 interrupt   ", 0);
  val_set_status(0, RESULT_PASS(TEST_NUM, 1));
  val_gic_end_of_interrupt(intid);
}

static
void
payload()
{
  /* Check CTIIRQ interrupt received    (x)    -- not feasible */
  /* Check PMU interrupt received - Checked in PE test         */
  /* Check COMMIRQ interrupt received   (x)    -- not feasible */
  /* Check PMBIRQ interrupt received    (x)    -- requires access to secure monitor */

  uint32_t timeout = TIMEOUT_LARGE;
  uint32_t timer_expire_val = 100;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  /* Check non-secure virtual timer Private Peripheral Interrupt (PPI) assignment */
  val_set_status(0, RESULT_PENDING(TEST_NUM));
  intid = val_timer_get_info(TIMER_INFO_VIR_EL1_INTID, 0);
  if (g_build_sbsa) {
      if (intid != 27) {
          val_print(ACS_PRINT_ERR,
              "\n       EL0-Virtual timer not mapped to PPI ID 27, INTID: %d   ", intid);
          val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
          return;
      }
  }

  if ((intid < 16 || intid > 31) && (!val_gic_is_valid_eppi(intid)))
      val_print(ACS_PRINT_WARN,
          "\n       EL0-Virtual timer not mapped to PPI recommended range, INTID: %d   ", intid);

  val_gic_install_isr(intid, isr_vir);
  val_timer_set_vir_el1(timer_expire_val);

  while ((--timeout > 0) && (IS_RESULT_PENDING(val_get_status(index)))) {
        ;
  }

  if (timeout == 0) {
    val_print(ACS_PRINT_ERR,
        "\n       EL0-Virtual timer interrupt not received on INTID: %d   ", intid);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
    return;
  }

}

uint32_t
os_g007_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  /* This GIC test is run on single processor */

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
