/** @file
 * Copyright (c) 2016-2018,2021, 2023 Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_wd.h"

#define TEST_NUM   (ACS_WD_TEST_NUM_BASE + 1)
#define TEST_RULE  "B_WD_01, B_WD_02, S_L3WD_01"
#define TEST_DESC  "Non Secure Watchdog Access            "

static
void
payload()
{

    uint64_t ctrl_base;
    uint64_t refresh_base;
    uint64_t wd_num = val_wd_get_info(0, WD_INFO_COUNT);
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
    uint32_t data, ns_wdg = 0;

    val_print(ACS_PRINT_DEBUG,
            "\n       Found %d watchdogs in table             ",
            wd_num);

    if (wd_num == 0) {
        if (g_build_sbsa)
            val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
        else
            val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
        return;
    }

    do {

        wd_num--;  /*array index starts from 0, so subtract 1 from count*/

        if (val_wd_get_info(wd_num, WD_INFO_ISSECURE))
            continue; /*Skip Secure watchdog*/

        ns_wdg++;
        refresh_base = val_wd_get_info(wd_num, WD_INFO_REFRESH_BASE);
        val_print(ACS_PRINT_INFO, "\n       Watchdog Refresh base is %llx ", refresh_base);
        ctrl_base    = val_wd_get_info(wd_num, WD_INFO_CTRL_BASE);
        val_print(ACS_PRINT_INFO, "\n       Watchdog CTRL base is  %llx      ", ctrl_base);

        data = val_mmio_read(ctrl_base);
        /*Control register bits 31:3 are reserved 0*/
        if (data >> WD_CSR_RSRV_SHIFT) {
            val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
            return;
        }

        data = val_mmio_read(refresh_base);
        /*refresh frame offset 0 must return 0 on reads.*/
        if (data) {
            val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
            return;
        }

        /* WOR.Upper word  [31:16] is reserved & must be zero */
        data = val_mmio_read(ctrl_base + WD_OR_UPPER_WORD_OFFSET);
        if (data >> WD_OR_RSRV_SHIFT) {
            val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
            return;
        }
    } while (wd_num);

    if (!ns_wdg) {
        if (g_build_sbsa) {
            val_print(ACS_PRINT_ERR, "\n       No non-secure Watchdogs reported", 0);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 5));
        } else {
            val_print(ACS_PRINT_WARN, "\n       No non-secure Watchdogs reported", 0);
            val_set_status(index, RESULT_SKIP(TEST_NUM, 3));
        }
        return;
    }

    val_set_status(index, RESULT_PASS(TEST_NUM, 1));

}

uint32_t
os_w001_entry(uint32_t num_pe)
{

    uint32_t status = ACS_STATUS_FAIL;

    num_pe = 1; /*This Timer test is run on single processor*/

    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

    if (status != ACS_STATUS_SKIP)
        val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

    val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);
    return status;

}
