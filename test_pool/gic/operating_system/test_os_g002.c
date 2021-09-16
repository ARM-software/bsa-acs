/** @file
 * Copyright (c) 2016-2021, Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/bsa_acs_pcie.h"

#define TEST_NUM   (ACS_GIC_TEST_NUM_BASE + 2)
#define TEST_RULE  "B_GIC_02"
#define TEST_DESC  "Check GICv2 Valid Configuration       "

static
void
payload()
{
  /* PE Limitations are covered in PE Rules */
  /* GICv3 + No ITS is covered in RB_GIC_03 */

  /* Check If PCIe Support and GicV2 -> Invalid Configuration */

  uint32_t gic_version;
  uint32_t num_msi_frame;
  uint32_t num_ecam = 0;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  num_ecam = val_pcie_get_info(PCIE_INFO_NUM_ECAM, 0);
  gic_version = val_gic_get_info(GIC_INFO_VERSION);
  num_msi_frame = val_gic_get_info(GIC_INFO_NUM_MSI_FRAME);

  val_print(ACS_PRINT_INFO, "\n       Received GIC version = %4d      ", gic_version);

  if (gic_version < 3) {
    if ((num_ecam > 0) && (num_msi_frame == 0)) {
      val_print(ACS_PRINT_ERR, "\n       GICv2 with PCIe : Invalid Configuration", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
      return;
    }
  } else {
    val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
    return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_g002_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This GIC test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
