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

#define TEST_NUM   ACS_MPAM_ERROR_TEST_NUM_BASE  +  11
#define TEST_DESC  "Check RIS no monitor error            "
#define TEST_RULE  ""

#define PSUEDO_REG_VALUE 0x0FF0
static uint32_t msc_index;

static
uint32_t
check_for_raz_wi(uint32_t reg_offset)
{
    /* With value programmed to unimplemented partitioning registers, access to those should
    be RAZ/WI (Read As Zero/ Write Ignored) */

    uint32_t reg_value;

    reg_value = val_mpam_mmr_read(msc_index, reg_offset);
    /* Check if register returns RAZ */
    if (reg_value != 0x00)
        return ACS_STATUS_FAIL;

    /* Check if register ignores write */
    val_mpam_mmr_write(msc_index, reg_offset, PSUEDO_REG_VALUE);
    val_mpam_mmr_read(msc_index, reg_offset);
    if (reg_value != 0x00)
        return ACS_STATUS_FAIL;

    return ACS_STATUS_PASS;
}

static
uint32_t
configure_monitoring_reg(uint32_t reg_offset)
{
    uint32_t esr_errcode;
    uint32_t status;

    /* Write 0xFFFF to the unimplemented monitoring register of the MSC's resource */
    val_mpam_mmr_write(msc_index, reg_offset, PSUEDO_REG_VALUE);

    /* Access the register back */
    val_mpam_mmr_read(msc_index, reg_offset);

    /* Wait for some time for the error to be reflected in MPAMF_ESR */
    val_time_delay_ms(100 * ONE_MILLISECOND);

    /* Read Error Status Register and check if the error code is recorded */
    esr_errcode = val_mpam_msc_get_errcode(msc_index);
    val_print(ACS_PRINT_DEBUG, "\n       Error code read is %llx", esr_errcode);

    if (esr_errcode != ESR_ERRCODE_RIS_NO_MON)
    {
        val_print(ACS_PRINT_ERR, "\n       Expected errcode: %d", ESR_ERRCODE_RIS_NO_MON);
        val_print(ACS_PRINT_ERR, "\n       Actual errcode: %d", esr_errcode);

        /* Check for RAZ/ WI*/
        status = check_for_raz_wi(reg_offset);
    }

    return status;
}

static
uint32_t
check_for_rsc_mon_controls(void)
{
    uint32_t status;

    if (!val_mpam_supports_csumon(msc_index)) {
        /* If CSU Monitoring is not implemented by the Resource, access MSMON_CFG_CSU_*
           to trigger the error */
           status = configure_monitoring_reg(REG_MSMON_CFG_CSU_CTL);
           status |= configure_monitoring_reg(REG_MSMON_CFG_CSU_FLT);
           return status;
    }
    else
    if (!val_mpam_msc_supports_mbwumon(msc_index)) {
        /* If MBWU Monitoring is not implemented by the Resource, access MSMON_CFG_MBWU_*
        to trigger the error */
        status = configure_monitoring_reg(REG_MSMON_CFG_MBWU_CTL);
        status |= configure_monitoring_reg(REG_MSMON_CFG_MBWU_FLT);
        return status;
    }

    return ACS_STATUS_SKIP;
}

static
void payload(void)
{
    uint32_t status;
    uint32_t pe_index;
    uint32_t total_nodes;
    uint32_t rsc_count;
    uint32_t rsc_index;
    uint32_t mpamf_ecr;
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

        /* Get number of resources implemented within an MSC */
        rsc_count = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);

        for (rsc_index = 0; rsc_index < rsc_count; rsc_index++) {

            /* Read MPAMF_ECR before generating error. This will be used to
            restore to default later */
            mpamf_ecr = val_mpam_mmr_read(msc_index, REG_MPAMF_ECR);
            status    = val_mpam_msc_reset_errcode(msc_index);

            if (!status) {
                val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
                return;
            }

            /* Configure MSMON_CFG_MON_SEL.RIS to select the current RIS Index */
            val_mpam_memory_configure_ris_sel(msc_index, rsc_index);

            /* Check if the resource of the MSC implements monitoring and generate the
            error by writing into unimplemented monitoring registers of the MSC resource */
            status = check_for_rsc_mon_controls();

            if (status == ACS_STATUS_SKIP) {
                /* If the resource implements all the monitoring, skip the rsc.
                Can't generate the error */
                continue;
            }
            else if (status != ACS_STATUS_PASS) {
                /* If the resource does not implement some monitoring and are
                not adhered to MPAM specification, fail the test */
                test_skip = 0;
                test_fail++;
            }

            test_skip = 0;
            /* Restore Error Control Register original settings */
            val_mpam_mmr_write(msc_index, REG_MPAMF_ECR, mpamf_ecr);
        }
    }

    if (test_skip)
        val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 01));
    else if (test_fail)
        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));
    else
        val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));
    return;
}

uint32_t error011_entry(void)
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
