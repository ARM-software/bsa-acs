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

#define TEST_NUM   ACS_MPAM_REGISTER_TEST_NUM_BASE + 2
#define TEST_RULE  ""
#define TEST_DESC  "Check Expansion of MPAMF_ESR          "

static void payload(void)
{

    uint32_t pe_index;
    uint32_t msc_index;
    uint32_t total_nodes;
    uint32_t idr_value;

    pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

    total_nodes = val_mpam_get_msc_count();

    for (msc_index = 0; msc_index < total_nodes; msc_index++) {
      idr_value = val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR);

      /* if MPAMF_IDR.{HAS_ESR, HAS_RIS} are 1 then MPAMF_IDR.{EXT, HAS_EXTD_ESR} are 1.*/
      if (BITFIELD_READ(IDR_HAS_ESR, idr_value) &&
          BITFIELD_READ(IDR_HAS_RIS, idr_value) &&
          BITFIELD_READ(IDR_EXT, idr_value)) {

        if (BITFIELD_READ(IDR_HAS_EXTD_ESR, idr_value) == 0) {
          /* Fail The Test */
          val_print(ACS_PRINT_ERR, "\n       MPAMF_IDR.HAS_EXTD_ESR value is 0", 0);
          val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
          return;
        }
      }
    }

    val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));
    return;
}

uint32_t reg002_entry(void)
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
