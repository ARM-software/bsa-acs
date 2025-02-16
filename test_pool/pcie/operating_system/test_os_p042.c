/** @file
 * Copyright (c) 2016-2019, 2022-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "val/common/include/acs_smmu.h"
#include "val/common/include/acs_pcie.h"
#include "val/bsa/include/bsa_acs_pcie.h"

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 42)
#define TEST_RULE  "PCI_PAS_1"
#define TEST_DESC  "PASID support atleast 16 bits         "

#define MIN_PASID_SUPPORT 16

static void payload(void)
{
  int num_smmu = 0, skip = 1;
  uint32_t max_pasids = 0;
  uint32_t status;
  uint32_t bdf;
  uint32_t fail_cnt = 0;
  uint32_t tbl_index;
  pcie_device_bdf_table *bdf_tbl_ptr;
  uint32_t index = val_pe_get_index_mpid (val_pe_get_mpid());

  /* Get pointer to bdf table */
  bdf_tbl_ptr = val_pcie_bdf_table_ptr();

  /* Iterate through bdf info table and check for PASID support */
  for (tbl_index = 0; tbl_index < bdf_tbl_ptr->num_entries; tbl_index++)
  {
      /* Index the bdf table using the iterator */
      bdf = bdf_tbl_ptr->device[tbl_index].bdf;
      /* Get max PASID with from device bdf */
      status = val_pcie_get_max_pasid_width(bdf, &max_pasids);
      /* Skip the device if PASID extended capability not supported */
      if (status == PCIE_CAP_NOT_FOUND)
      {
          val_print(ACS_PRINT_DEBUG, "\n       PASID extended capability not supported.", 0);
          val_print(ACS_PRINT_DEBUG, " Skipping for BDF: 0x%x", bdf);
          continue;
      }
      /* Raise an error if any failure in obtaining the PASID max width */
      else if (status)
      {
          val_print(ACS_PRINT_ERR,
                    "\n       Error in obtaining the PASID max width for BDF: 0x%x", bdf);
          fail_cnt++;
      }
      val_print(ACS_PRINT_DEBUG, "\n       BDF - 0x%x", bdf);
      val_print(ACS_PRINT_DEBUG, "- Max PASID bits - 0x%x", max_pasids);
      if (max_pasids > 0)
      {
          skip = 0;
          if (max_pasids < MIN_PASID_SUPPORT)
          {
              val_print(ACS_PRINT_ERR, "\n       Max PASID support less than 16 bits  ", 0);
              fail_cnt++;
          }
      }
  }

  /* For each SMMUv3 check for PASID support */
  /* If PASID is supported, test the max number of PASIDs supported */
  num_smmu = val_iovirt_get_smmu_info(SMMU_NUM_CTRL, 0);
  for (num_smmu--; num_smmu >= 0; num_smmu--)
  {
      if (val_iovirt_get_smmu_info(SMMU_CTRL_ARCH_MAJOR_REV, num_smmu) == 3)
      {
          max_pasids = val_smmu_max_pasids(num_smmu);
          val_print(ACS_PRINT_DEBUG, "\n       SMMU check- Max PASID bits- 0x%x", max_pasids);
          if (max_pasids > 0)
          {
              skip = 0;
              if (max_pasids < MIN_PASID_SUPPORT)
              {
                  val_print(ACS_PRINT_ERR, "\n      Max PASID support less than 16 bits  ", 0);
                  fail_cnt++;
              }
          }
      }
  }

  /* Report the status of test */
  if (skip)
      val_set_status(index, RESULT_SKIP(TEST_NUM, 0));
  else if (fail_cnt)
      val_set_status(index, RESULT_FAIL(TEST_NUM, 0));
  else
      val_set_status(index, RESULT_PASS(TEST_NUM, 0));
}

uint32_t
os_p042_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
