/** @file
 * Copyright (c) 2016-2018, 2020-2021, 2024, Arm Limited or its affiliates. All rights reserved.
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

static void check_pauth_algorithm(uint32_t index, uint64_t data1, uint32_t data2)
{

    /* Read ID_AA64ISAR1_EL1.APA[7:4] and ID_AA64ISAR1_EL1.GPA[27:24]  ! = 0 indicates
     * address and generic authentication support using QARMA5
     *
     * Read ID_AA64ISAR2_EL1.APA3[15:12] and ID_AA64ISAR2_EL1.GPA3[11:8] ! = 0 indicates
     * address and generic authentication support using QARMA3
     */

    if (((VAL_EXTRACT_BITS(data1, 4, 7) != 0) && (VAL_EXTRACT_BITS(data1, 24, 27) != 0)) ||
        ((VAL_EXTRACT_BITS(data2, 8, 11) != 0) && (VAL_EXTRACT_BITS(data2, 12, 15) != 0)))
        val_set_status(index, RESULT_PASS(TEST_NUM, 1));
    else
        val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
}

static
void
payload()
{
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
    uint32_t primary_pe_idx = val_pe_get_primary_index();

    /* Read ID_AA64ISAR1_EL1 and ID_AA64ISAR2_EL1 for PAuth support */
    uint64_t data1 = val_pe_reg_read(ID_AA64ISAR1_EL1);
    uint64_t data2 = val_pe_reg_read(ID_AA64ISAR2_EL1);

    if (index == primary_pe_idx) {
        val_print(ACS_PRINT_DEBUG, "\n       ID_AA64ISAR1_EL1 = %llx", data1);
        val_print(ACS_PRINT_DEBUG, "\n       ID_AA64ISAR2_EL1 = %llx", data2);
    }
     /* PAuth is optional, For PAuth authentication support atleast one of generic or address
      * authentication of any one of the standard algorithm is needed
      *
      * Read ID_AA64ISAR1_EL1.APA[7:4] and ID_AA64ISAR1_EL1.GPA[27:24]  ! = 0 indicates
      * address and generic authentication support using QARMA5
      *
      * Read ID_AA64ISAR2_EL1.APA3[15:12] and ID_AA64ISAR2_EL1.GPA3[11:8] ! = 0 indicates
      * address and generic authentication support using QARMA3
      *
      * Read ID_AA64ISAR1_EL1.API[11:8] and ID_AA64ISAR1_EL1.GPI[31:28] ! = 0 indicates
      * address and generic authentication support using IMPDEF
      */

    if ((VAL_EXTRACT_BITS(data1, 4, 7) == 0) && (VAL_EXTRACT_BITS(data2, 12, 15) == 0) &&
        (VAL_EXTRACT_BITS(data1, 24, 27) == 0) && (VAL_EXTRACT_BITS(data2, 8, 11) == 0) &&
        (VAL_EXTRACT_BITS(data1, 8, 11) == 0) && (VAL_EXTRACT_BITS(data1, 28, 31) == 0)) {

        /* Pointer signing not implemented, Skip the test */
        val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
        return;
    }

    /* Implemented, Check for pointer authentication using standard algorithm */
    check_pauth_algorithm(index, data1, data2);

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
