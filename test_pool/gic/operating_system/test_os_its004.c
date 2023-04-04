/** @file
 * Copyright (c) 2021-2023, Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/bsa_acs_iovirt.h"
#include "val/include/bsa_acs_pcie.h"

#define TEST_NUM   (ACS_GIC_ITS_TEST_NUM_BASE + 4)
#define TEST_RULE  "ITS_DEV_7,ITS_DEV_8"
#define TEST_DESC  "Check Device's ReqID-DeviceID-StreamID"

static
void
payload()
{
  uint32_t bdf;
  uint32_t status;
  uint32_t pe_index;
  uint32_t tbl_index;
  uint32_t device_id, req_id;
  uint32_t stream_id, its_id;
  uint32_t smmu_id;
  uint32_t seg_num = 0;
  uint32_t cap_base;
  uint32_t test_skip = 1;
  uint32_t test_fail = 0;
  uint32_t streamid_check = 1;
  uint32_t curr_grp_did_cons, curr_grp_sid_cons;
  uint32_t curr_grp_its_id = -1;
  uint32_t curr_smmu_id = -1;
  uint32_t curr_seg_num = -1;
  pcie_device_bdf_table *bdf_tbl_ptr;

  pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
  bdf_tbl_ptr = val_pcie_bdf_table_ptr();

  /* Check for all the function present in bdf table */
  for (tbl_index = 0; tbl_index < bdf_tbl_ptr->num_entries; tbl_index++)
  {
    bdf = bdf_tbl_ptr->device[tbl_index].bdf;

    /* If MSI or MSI-X not supported, Skip current device */
    if ((val_pcie_find_capability(bdf, PCIE_CAP, CID_MSI, &cap_base) == PCIE_CAP_NOT_FOUND) &&
        (val_pcie_find_capability(bdf, PCIE_CAP, CID_MSIX, &cap_base) == PCIE_CAP_NOT_FOUND))
      continue;

    /* If MSI Supported then Check for Valid DeviceID */
    req_id = PCIE_CREATE_BDF_PACKED(bdf);
    seg_num = PCIE_EXTRACT_BDF_SEG(bdf);

    status = val_iovirt_get_device_info(PCIE_CREATE_BDF_PACKED(bdf),
                                        seg_num, &device_id,
                                        &stream_id, &its_id);
    if (status) {
        val_print(ACS_PRINT_DEBUG, "\n       Could not get device info for BDF : 0x%x", bdf);
        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 1));
        return;
    }

    smmu_id = val_iovirt_get_rc_smmu_index(seg_num, PCIE_CREATE_BDF_PACKED(bdf));
    if (smmu_id == ACS_INVALID_INDEX) {
        val_print(ACS_PRINT_INFO,
            "\n       Skipping StreamID Association check, Bdf : 0x%llx Not Behind an SMMU", bdf);
        streamid_check = 0;
    } else {
        streamid_check = 1;
    }

    /* If test runs for atleast an endpoint */
    test_skip = 0;

    /* Store the Current group SID Constant offset & Dev ID Constant offset using first device */
    if ((its_id != curr_grp_its_id) || (smmu_id != curr_smmu_id) || (seg_num != curr_seg_num)) {
      curr_grp_its_id = its_id;
      curr_smmu_id = smmu_id;
      curr_seg_num = seg_num;

      /* streamid_check is 0 when Root Complex is not behind an SMMU */
      if (streamid_check != 0) {
        /* Device behind SMMU */
        curr_grp_sid_cons = stream_id - req_id;
        curr_grp_did_cons = device_id - stream_id;
      } else {
        /* No SMMU, stream_id check not needed */
        curr_grp_sid_cons = 0;
        curr_grp_did_cons = device_id - req_id;
      }
      continue;
    }

    if (streamid_check == 0) {
      val_print(ACS_PRINT_DEBUG,
                "\n       Checking ReqID-DeviceID Association, Bdf : 0x%llx", bdf);
      /* No SMMU, Check only for device_id */
      if (curr_grp_did_cons != (device_id - req_id)) {
        /* DeviceID Constant Base Failure */
        val_print(ACS_PRINT_ERR,
                  "\n       ReqID-DeviceID Association Fail for Bdf : %x", bdf);
        test_fail++;
      }
    } else {
      val_print(ACS_PRINT_DEBUG,
                "\n       Checking ReqID-StreamID-DeviceID Association, Bdf : %x", bdf);
      /* Check for stream_id & device_id */
      if (curr_grp_sid_cons != (stream_id - req_id)) {
        /* StreamID Constant Base Failure */
        val_print(ACS_PRINT_ERR, "\n       ReqID-StreamID Association Fail for Bdf : %x", bdf);
        test_fail++;
      }

      if (curr_grp_did_cons != (device_id - stream_id)) {
        /* DeviceID Constant Base Failure */
        val_print(ACS_PRINT_ERR,
                  "\n       StreamID-DeviceID Association Fail for Bdf : %x", bdf);
        test_fail++;
      }
    }
  }

  if (test_skip == 1)
      val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 1));
  else if (test_fail)
      val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 2));
  else
      val_set_status(pe_index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_its004_entry(uint32_t num_pe)
{
  uint32_t status = ACS_STATUS_FAIL;
  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);
  return status;
}
