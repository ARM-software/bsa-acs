/** @file
 * Copyright (c) 2016-2018, 2021, 2023-2025, Arm Limited or its affiliates. All rights reserved.
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

#include "val/common/include/acs_pcie.h"
#include "val/bsa/include/bsa_acs_pcie.h"
#include "val/bsa/include/bsa_acs_memory.h"

#define TEST_NUM   (ACS_MEMORY_MAP_TEST_NUM_BASE + 4)
#define TEST_RULE  "B_MEM_03, B_MEM_04, B_MEM_06"
#define TEST_DESC  "Addressability                        "

static
void
payload (void)
{
  /* This test checks for the Addressability of Non-Secure Masters */
  uint32_t index;
  uint32_t count;
  uint32_t data;
  uint32_t dev_type;
  uint32_t dev_bdf;
  uint32_t p_cap, cid_offset;
  uint32_t test_run = 0;

  index = val_pe_get_index_mpid (val_pe_get_mpid());
  count = val_peripheral_get_info (NUM_ALL, 0);

  if (!count) {
     val_print(ACS_PRINT_DEBUG, "\n       Skip as No peripherals detected   ", 0);
     val_set_status(index, RESULT_SKIP (TEST_NUM, 1));
     return;
  }

  val_print(ACS_PRINT_DEBUG, "\n PE index: %d", index);
  val_print(ACS_PRINT_DEBUG, "\n Peripherals count: : %d", count);


  while (count) {
      count--;
      dev_bdf = (uint32_t)val_peripheral_get_info (ANY_BDF, count);

      if (dev_bdf == 0)
          continue;

      dev_type = val_pcie_get_device_type(dev_bdf);
      // 1: Normal PCIe device, 2: PCIe Host bridge, 3: PCIe bridge device, else: INVALID

      val_print(ACS_PRINT_INFO, "\n Dev bdf 0x%x", dev_bdf);

      if ((!dev_type) || (dev_type > 1)) {
          //Skip this device, if we either got pdev as NULL or if it is a bridge
          continue;
      }

      if (!val_pcie_device_driver_present(dev_bdf)) {
          val_print(ACS_PRINT_DEBUG, "\n Driver not present for bdf 0x%x", dev_bdf);
          continue;
      }

      /* Skip if the device is a PCI legacy device */
      p_cap = val_pcie_find_capability(dev_bdf, PCIE_CAP, CID_PCIECS, &cid_offset);
      if (p_cap != PCIE_SUCCESS) {
        val_print(ACS_PRINT_DEBUG, " \nBDF 0x%x PCI Express capability not present...Skipping\n",
                                                                             dev_bdf);
        continue;
      }

      test_run = 1;

      data = val_pcie_is_devicedma_64bit(dev_bdf);
      if (data == 0) {
          if (!val_pcie_is_device_behind_smmu(dev_bdf)) {
              val_print(ACS_PRINT_ERR, "\n   WARNING:The device with bdf=0x%x", dev_bdf);
              val_print(ACS_PRINT_ERR, "\n   doesn't support 64 bit addressing and is not", 0);
              val_print(ACS_PRINT_ERR, "\n   behind smmu. device is of type = %d", dev_type);
              val_set_status(index, RESULT_FAIL (TEST_NUM, 1));
              return;
          }
       }
  }

  if (test_run)
      val_set_status (index, RESULT_PASS (TEST_NUM, 01));
  else
      val_set_status (index, RESULT_SKIP (TEST_NUM, 02));
}

uint32_t
os_m004_entry (uint32_t num_pe)
{
  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test (TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP) {
      val_run_test_payload (TEST_NUM, num_pe, payload, 0);
  }

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
