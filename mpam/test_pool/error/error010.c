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
#include "val/common/include/acs_mpam.h"
#include "val/common/include/acs_mpam_reg.h"
#include "val/mpam/include/mpam_val_interface.h"

#define TEST_NUM   ACS_MPAM_ERROR_TEST_NUM_BASE  +  10
#define TEST_DESC  "Check Undef RIS MSMON_CFG_MON_SEL Err "
#define TEST_RULE  ""

static uint32_t max_ris_index;

static
uint32_t
check_for_raz_wi(uint32_t msc_index, uint32_t reg_offset)
{
    /* With out-of-range RIS value programmed to MSMON_CFG_MON_SEL.RIS, access to MSMON_CFG* should
       be RAZ/WI (Read As Zero/ Write Ignored) */

    uint32_t reg_value;
    uint32_t data;
    reg_value = val_mpam_mmr_read(msc_index, reg_offset);
    /* Check if register returns RAZ */
    if (reg_value != 0x00)
        return ACS_STATUS_FAIL;

    /* Check if register ignores write */
    data = BITFIELD_SET(MON_SEL_MON_SEL, 0x10) |
           BITFIELD_SET(MON_SEL_RIS, (max_ris_index + 1));
    val_mpam_mmr_write(msc_index, reg_offset, data);
    val_mpam_mmr_read(msc_index, reg_offset);
    if (reg_value != 0x00)
        return ACS_STATUS_FAIL;

    return ACS_STATUS_PASS;
}

static
void payload(void)
{

    uint32_t msc_index;
    uint32_t status;
    uint32_t pe_index;
    uint32_t total_nodes;
    uint32_t mpamf_ecr;
    uint32_t esr_errcode;
    uint32_t data;
    uint32_t test_fail = 0;
    uint32_t test_skip = 1;

    pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
    total_nodes = val_mpam_get_msc_count();

    for (msc_index = 0; msc_index < total_nodes; msc_index++) {

        if (!val_mpam_msc_supports_esr(msc_index)) {
            val_print(ACS_PRINT_DEBUG, "\n       MSC index %d does not support ESR", msc_index);
            continue;
        }

        if (!val_mpam_msc_supports_ris(msc_index)) {
            val_print(ACS_PRINT_DEBUG,
                        "\n       MSC index %d does not support RIS", msc_index);
            continue;
        }

        /* Error cannot be generated if MSMON_CFG_MON_SEL register is not implemented */
        if (!val_mpam_msc_supports_mon(msc_index)) {
            val_print(ACS_PRINT_DEBUG,
                          "\n       MSC index %d does not implement MSMON", msc_index);
            continue;
        }

        /* Read MPAMF_ECR before generating error. This will be used to restore to default later */
        mpamf_ecr = val_mpam_mmr_read(msc_index, REG_MPAMF_ECR);
        status    = val_mpam_msc_reset_errcode(msc_index);

        if (!status) {
            val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
            return;
        }

        test_skip = 0;

        /* Program MSMON_CFG_MON_SEL with out-of-range RIS index */
        max_ris_index = val_mpam_get_max_ris_count(msc_index);

        data = BITFIELD_SET(MON_SEL_MON_SEL, 0) |
            BITFIELD_SET(MON_SEL_RIS, (max_ris_index + 1));

        val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MON_SEL, data);
        val_print(ACS_PRINT_DEBUG, "\n       Value written to MSMON_CFG_MON_SEL is %llx", data);

        /* Access MSMON_CFG_* register to cause the error */
        val_mpam_mmr_read(msc_index, REG_MSMON_CFG_MON_SEL);

        /* Wait for some time for the error to be reflected in MPAMF_ESR */
        val_time_delay_ms(100 * ONE_MILLISECOND);

        /* Read Error Status Register and check if the error code is recorded */
        esr_errcode = val_mpam_msc_get_errcode(msc_index);
        val_print(ACS_PRINT_DEBUG, "\n       Error code read is %llx", esr_errcode);

        if (esr_errcode != ESR_ERRCODE_UNDEF_RIS_MON_SEL)
        {
            val_print(ACS_PRINT_ERR, "\n       Expected errcode: %d",
                                                                ESR_ERRCODE_UNDEF_RIS_MON_SEL);
            val_print(ACS_PRINT_ERR, "\n       Actual errcode: %d", esr_errcode);
            status = check_for_raz_wi(msc_index, REG_MSMON_CFG_MON_SEL);
            if (status == ACS_STATUS_FAIL) {
                val_print(ACS_PRINT_ERR,
                    "\n       MSMON_CFG_MON_SEL is not RAZ/WI with out-of-range RIS programmed", 0);
                test_fail++;
            }
        }

        /* Restore Error Control Register original settings */
        val_mpam_mmr_write(msc_index, REG_MPAMF_ECR, mpamf_ecr);
    }

    if (test_skip)
        val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 01));
    else if (test_fail)
        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));
    else
        val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));
    return;
}

uint32_t error010_entry(void)
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
