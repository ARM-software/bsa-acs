/** @file
 * Copyright (c) 2016-2018,2021 Arm Limited or its affiliates. All rights reserved.
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

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 66)
#define TEST_RULE  "PCI_MM_04"
#define TEST_DESC  "NP type-1 PCIe supp 32-bit only       "

#define BAR0    0x10

static
void
payload(void)
{
    uint32_t index;
    uint32_t tbl_index;
    uint32_t status = 0;
    uint32_t ret;
    uint32_t bar_data;
    uint32_t data;
    uint32_t dev_type;
    uint64_t dev_bdf;
    pcie_device_bdf_table *bdf_tbl_ptr;

    bdf_tbl_ptr = val_pcie_bdf_table_ptr();
    tbl_index = 0;

    while (tbl_index < bdf_tbl_ptr->num_entries)
    {
        dev_bdf = bdf_tbl_ptr->device[tbl_index++].bdf;

        dev_type = val_pcie_get_device_type(dev_bdf);
        /* Allow only type-1 headers and skip others */
        if (dev_type != 3)
            continue;

        ret = val_pcie_read_cfg(dev_bdf, BAR0, &bar_data);
        if (bar_data) {
            /* Extract pref type */
            data = VAL_EXTRACT_BITS(bar_data, 3, 3);
            if (data == 0) {
                status = 1;
                /* Extract mem type */
                data = VAL_EXTRACT_BITS(bar_data, 1, 2);
                if (data != 0) {
                    val_print(ACS_PRINT_ERR, "\n       NP type-1 pcie is not 32-bit mem type", 0);
                    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
                    status = 2;
                    break;
                }
                /* Scan the all PCIe bridge devices and check memory type */
                ret = val_pcie_scan_bridge_devices_and_check_memtype(dev_bdf);
                if (ret) {
                    val_print(ACS_PRINT_ERR, "\n       NP type-1 pcie bridge end device"
                                                                 "is not 32-bit mem type", 0);
                    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
                    status = 2;
                    break;
                }
            }
        }

        else {
            /* bar_data is zero, so considered as 32 bit BAR */
            status = 1;
        }
    }

    if (!status)
        val_set_status(index, RESULT_SKIP (TEST_NUM, 3));
    else if (status == 1)
        val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_p066_entry(uint32_t num_pe)
{
    uint32_t status = ACS_STATUS_FAIL;

    /* This test is run on single processor */
    num_pe = 1;
    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
    if (status != ACS_STATUS_SKIP)
        val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);
    val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

    return status;
}
