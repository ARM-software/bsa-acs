/** @file
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "val/drtm/include/drtm_val_interface.h"
#define TEST_NUM   (ACS_DRTM_INTERFACE_TEST_NUM_BASE + 2)
#define TEST_RULE  ""
#define TEST_DESC  "DRTM Invalid Function ID Test         "

#define FEAT_ID_INVALID_RSVD ((uint32_t)0x1 << 8)
#define FUNC_ID_INVALID_RSVD ((uint64_t)0x1 << 32)

static void payload(void)
{
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
    int64_t  status;
    uint64_t feat1, feat2;
    uint64_t invalid_fid = 0xC4000999;

    status = val_drtm_features(invalid_fid, &feat1, &feat2);
    if (status != DRTM_ACS_NOT_SUPPORTED) {
        val_print(ACS_PRINT_ERR, "\n       Invalid function ID test failed, status=%d", status);
        val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
        return;
    }

    /* Pass Feature ID Such That Reserved Bits [62:8] != 0 */
    invalid_fid = DRTM_1_0_DRTM_FEATURES_TPM | FEAT_ID_INVALID_RSVD;
    status = val_drtm_features(invalid_fid, &feat1, &feat2);
    if (status != DRTM_ACS_NOT_SUPPORTED) {
        val_print(ACS_PRINT_WARN,
                    "\n       Feature ID Rsvd Bits:[62:8] not zero, status=%d", status);
    }

    /* Pass Function ID Such That Reserved Bits [62:32] != 0 */
    invalid_fid = DRTM_1_0_FN_DRTM_VERSION | FUNC_ID_INVALID_RSVD;
    status = val_drtm_features(invalid_fid, &feat1, &feat2);
    if (status != DRTM_ACS_NOT_SUPPORTED) {
        val_print(ACS_PRINT_WARN,
                    "\n       Function ID Rsvd Bits:[62:32] not zero, status=%d", status);
    }

    val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t interface002_entry(uint32_t num_pe)
{
    uint32_t status = ACS_STATUS_FAIL;

    status = val_initialize_test(TEST_NUM, TEST_DESC, 0);

    if (status != ACS_STATUS_SKIP)
        payload();

    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);
    val_report_status(0, ACS_END(TEST_NUM), NULL);

    return status;
}
