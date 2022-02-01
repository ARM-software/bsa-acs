/** @file
 * Copyright (c) 2021-2022, Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/bsa_acs_memory.h"

#define TEST_NUM   (ACS_GIC_ITS_TEST_NUM_BASE + 3)
#define TEST_RULE  "ITS_DEV_2"
#define TEST_DESC  "Check uniqueness of StreamID          "

static
void
payload()
{
  uint32_t bdf;
  uint32_t status;
  uint32_t pe_index;
  uint32_t tbl_index;
  uint32_t dev_index = 0;
  uint32_t device_id, req_id;
  uint32_t stream_id, its_id;
  uint32_t cap_base;
  uint32_t test_skip = 1;
  pcie_device_bdf_table *bdf_tbl_ptr;
  int32_t prev_its_id = -1, i, j;
  uint32_t *streamID = NULL;

  pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
  bdf_tbl_ptr = val_pcie_bdf_table_ptr();

  if (!bdf_tbl_ptr) {
      val_print(ACS_PRINT_DEBUG, "\n       No BDF table created", 0);
      val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 1));
      return;
  }

  if (!bdf_tbl_ptr->num_entries) {
      val_print(ACS_PRINT_DEBUG, "\n       No entries in BDF table", 0);
      val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 2));
      return;
  }

  /* Allocate memory to store stream ID */
  streamID = val_memory_alloc(bdf_tbl_ptr->num_entries * sizeof(uint32_t));
  if (!streamID) {
      val_print(ACS_PRINT_DEBUG, "\n       Stream ID memory allocation failed", 0);
      val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 1));
      return;
  }

  /* Check for all the function present in bdf table */
  for (tbl_index = 0; tbl_index < bdf_tbl_ptr->num_entries; tbl_index++)
  {
    bdf = bdf_tbl_ptr->device[tbl_index].bdf;

    /* If MSI or MSI-X not supported, Skip current device */
    if ((val_pcie_find_capability(bdf, PCIE_CAP, CID_MSI, &cap_base) == PCIE_CAP_NOT_FOUND) &&
        (val_pcie_find_capability(bdf, PCIE_CAP, CID_MSIX, &cap_base) == PCIE_CAP_NOT_FOUND))
      continue;

    /* If test runs for atleast an endpoint */
    test_skip = 0;

    /* If MSI Supported then Check for Valid DeviceID */
    req_id = GET_DEVICE_ID(PCIE_EXTRACT_BDF_BUS(bdf),
                           PCIE_EXTRACT_BDF_DEV(bdf),
                           PCIE_EXTRACT_BDF_FUNC(bdf));

    status = val_iovirt_get_device_info(req_id, PCIE_EXTRACT_BDF_SEG(bdf), &device_id,
                                        &stream_id, &its_id);
    if (status) {
        val_print(ACS_PRINT_DEBUG,
            "\n       Could not get device info for BDF : 0x%x", bdf);
        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 2));
        /* Free allocated memory before return*/
        val_memory_free(streamID);
        return;
    }

    /* Update ITS id for first group */
    if (prev_its_id == -1)
      prev_its_id = its_id;

    /* Update streamID & index for the last device before checking streamID uniqueness*/
    if (tbl_index == (bdf_tbl_ptr->num_entries - 1)) {
      streamID[dev_index] = stream_id;
      dev_index++;
    }

    /* Check uniqueness of streamID if ITS group changed or for the last group*/
    if ((prev_its_id != its_id) || (tbl_index == (bdf_tbl_ptr->num_entries - 1))) {
      for (i = 0; i < (dev_index - 1); i++) {
          for (j = (i + 1); j < dev_index; j++) {
            if (streamID[i] == streamID[j]) {
                val_print(ACS_PRINT_DEBUG,
                        "\n       Stream ID is not unique for bdf : 0x%x", bdf);
                val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 3));
                /* Free allocated memory before return*/
                val_memory_free(streamID);
                return;
            }
          }
      }
      prev_its_id = its_id;
      dev_index = 0;
    }
    /* Skip executing remaining steps for last device to avoid buffer overflow*/
    if (tbl_index == (bdf_tbl_ptr->num_entries - 1))
      break;

    /* Update streamID & index */
    streamID[dev_index] = stream_id;
    dev_index++;
  }

  /* Free allocated memory before return*/
  val_memory_free(streamID);

  if (test_skip == 1)
      val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 3));
  else
      val_set_status(pe_index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_its003_entry(uint32_t num_pe)
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
