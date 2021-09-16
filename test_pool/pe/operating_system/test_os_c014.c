/** @file
 * Copyright (c) 2016-2018, 2020-2021 Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
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
#include "val/include/bsa_acs_pe.h"
#include "val/include/val_interface.h"

#define TEST_NUM   (ACS_PE_TEST_NUM_BASE  +  14)
#define TEST_RULE  "B_PE_15"
#define TEST_DESC  "Check PAuth if implementation         "

static void check_pauth_algorithm(uint32_t index, uint64_t data)
{
    /* Read ID_AA64ISAR1_EL1[11:4] for address authentication and
     * ID_AA64ISAR1_EL1[31:24] for generic authentication
     * defined by Arm architecture
     */
    if ((VAL_EXTRACT_BITS(data, 4, 7) == 0) && (VAL_EXTRACT_BITS(data, 24, 27) == 0)) {
        if ((VAL_EXTRACT_BITS(data, 8, 11) != 0) && (VAL_EXTRACT_BITS(data, 28, 31) != 0))
            val_set_status(index, RESULT_PASS(TEST_NUM, 1));
        else
            val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    }
    else
        val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

static
void
payload()
{
    uint64_t data = val_pe_reg_read(ID_AA64ISAR1_EL1);
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());


    /* Pointer signing is optional, Check if Pointer signing is implemented */
    if ((VAL_EXTRACT_BITS(data, 4, 7) == 0) && (VAL_EXTRACT_BITS(data, 8, 11) == 0) &&
        (VAL_EXTRACT_BITS(data, 24, 27) == 0) && (VAL_EXTRACT_BITS(data, 28, 31) == 0)) {
        /* Pointer signing not implemented, Skip the test */
        val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
        return;
    }

    /* Implemented, Check for pointer authentication using standard algorithm */
    check_pauth_algorithm(index, data);

}

uint32_t
os_c014_entry(uint32_t num_pe)
{
    uint32_t status = ACS_STATUS_FAIL;

    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
    /* This check is when user is forcing us to skip this test */
    if (status != ACS_STATUS_SKIP)
        val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);
    val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

    return status;
}
