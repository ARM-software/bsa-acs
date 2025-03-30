/** @file
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
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

#include "val/common/include/acs_val.h"
#include "val/common/include/acs_pe.h"
#include "val/common/include/acs_memory.h"
#include "val/common/include/acs_mpam.h"
#include "val/common/include/acs_mpam_reg.h"
#include "val/mpam/include/mpam_val_interface.h"

#define TEST_NUM   ACS_MPAM_REGISTER_TEST_NUM_BASE + 1
#define TEST_RULE  ""
#define TEST_DESC  "Check MPAM Version EXT Bit Check      "

static void payload(void)
{

    uint32_t version;
    uint32_t pe_index;
    uint32_t msc_index;
    uint32_t total_nodes;
    uint32_t ext;

    pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

    total_nodes = val_mpam_get_msc_count();

    for (msc_index = 0; msc_index < total_nodes; msc_index++) {
        version = val_mpam_msc_get_version(msc_index);
        val_print(ACS_PRINT_INFO, "\n       MSC Node Index : %d", msc_index);
        val_print(ACS_PRINT_INFO, ", Version : 0x%x", version);

        if (version == MPAM_VERSION_1_0) {
            /* if MPAMv1.0 Check MPAMF_IDR.EXT = 0 */
            ext = BITFIELD_READ(IDR_EXT, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));
            if (ext != 0) {
              /* Fail the test */
              val_print(ACS_PRINT_ERR, "\n       MPAMF_IDR.EXT value is not 0", 0);
              val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
              return;
            }
        }
        else if (version == MPAM_VERSION_1_1) {
            /* if MPAMv1.1 Check MPAMF_IDR.EXT = 1 */
            ext = BITFIELD_READ(IDR_EXT, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));
            if (ext != 1) {
              /* Fail the test */
              val_print(ACS_PRINT_ERR, "\n       MPAMF_IDR.EXT value is not 1", 0);
              val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));
              return;
            }
        }
        else {
            /* Invalid */
            /* TODO : Check for v0.1 */
            val_print(ACS_PRINT_ERR, "\n       MSC Version not valid", 0);
            val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 03));
            return;
        }
    }

    val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));
    return;
}

uint32_t reg001_entry(void)
{

    uint32_t status = ACS_STATUS_FAIL;
    uint32_t num_pe = 1;

    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

    if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

    val_report_status(0, ACS_END(TEST_NUM), NULL);

    return status;
}
