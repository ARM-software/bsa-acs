/** @file
 * Copyright (c) 2016-2018, 2021-2023, Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_pcie.h"

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 9)
#define TEST_RULE  "PCI_IN_20"
#define TEST_DESC  "Vendor specific data is PCIe compliant"

// Valid PCIe CapID ranges (PCIe 6.0 - v1.0)
#define PCIE_CAP_ID_END     0x15
#define PCIE_ECAP_ID_END    0x34

/*
 Check if vendor specific data are presented as non-PCIe compliant capabilities
 in either the regular or extended PCIe spaces
 returns: 0 - No violations present    1 - One or more violations present
 */
static
uint32_t
check_pcie_cfg_space(uint32_t bdf)
{
    uint32_t reg_value;
    uint32_t next_cap;
    uint32_t err = 0;
    uint32_t cid;

    // Search through regular PCIe cfg space first (start at first implemented cap)
    val_pcie_read_cfg(bdf, TYPE01_CPR, &reg_value);
    // DWORD aligned: next_cap[1:0] are hardwired to '0'
    next_cap = VAL_EXTRACT_BITS(reg_value, 2, 7) << 2;

    while (next_cap) {

        val_pcie_read_cfg(bdf, next_cap, &reg_value);
        next_cap = VAL_EXTRACT_BITS(reg_value, 8, 15);
        cid = VAL_EXTRACT_BITS(reg_value, 0, 7);

        // Check PCIe Cap IDs are in range
        if (cid > PCIE_CAP_ID_END) {
            val_print(ACS_PRINT_ERR,
                "\n       Invalid Cap ID: 0x%x found in regular cfg space", cid);
            err = 1;
        }
    }

    // Search through extended PCIe cfg space next
    next_cap = PCIE_ECAP_START;

    while (next_cap) {

        val_pcie_read_cfg(bdf, next_cap, &reg_value);
        next_cap = VAL_EXTRACT_BITS(reg_value, 20, 31);
        cid = VAL_EXTRACT_BITS(reg_value, 0, 15);

        // Check PCIe Ext Cap IDs are in range
        if (cid > PCIE_ECAP_ID_END) {
            val_print(ACS_PRINT_ERR,
                "\n       Invalid Cap ID: 0x%x found in extended cfg space", cid);
            err = 1;
        }
    }

    return err;
}

static
void
payload(void)
{
    uint32_t pe_index;
    uint32_t tbl_index;
    uint32_t bdf;
    uint32_t dp_type;
    uint32_t test_skip = 1;
    uint32_t test_fail = 0;

    pcie_device_bdf_table *bdf_tbl_ptr;

    pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
    bdf_tbl_ptr = val_pcie_bdf_table_ptr();

    tbl_index = 0;

    while (tbl_index < bdf_tbl_ptr->num_entries) {

        bdf = bdf_tbl_ptr->device[tbl_index++].bdf;
        dp_type = val_pcie_device_port_type(bdf);

        // Only check for Root Ports
        if (dp_type == RP) {
            test_skip = 0;
            val_print(ACS_PRINT_DEBUG, "\n       BDF - 0x%x", bdf);
            if (check_pcie_cfg_space(bdf)) {
                val_print(ACS_PRINT_ERR,
                    "\n       Invalid PCIe capability found on dev: %d", tbl_index);
                test_fail++;
            }
        }
    }

    if (test_skip)
        val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 1));
    else if (test_fail)
        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, test_fail));
    else
        val_set_status(pe_index, RESULT_PASS(TEST_NUM, 1));

    return;
}

uint32_t
os_p009_entry(uint32_t num_pe)
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
