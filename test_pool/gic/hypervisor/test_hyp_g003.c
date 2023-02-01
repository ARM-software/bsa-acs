/** @file
 * Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/bsa_acs_common.h"
#include "val/include/val_interface.h"

#include "val/include/bsa_acs_pe.h"
#include "val/include/bsa_acs_gic.h"
#include "val/include/bsa_acs_gic_support.h"

#define TEST_NUM   (ACS_GIC_HYP_TEST_NUM_BASE + 3)
#define TEST_RULE  "B_PPI_02"
#define TEST_DESC  "Check GIC Maintenance PPI Assignment  "

static uint32_t intid;

/* Interrupt handler for the GIC Maintenance interrupt*/
static
void
isr_mnt()
{
    uint32_t data;
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

    /* We received our interrupt, so disable Maintenance
     *interrupt from generating further interrupts */
    data = val_gic_reg_read(ICH_HCR_EL2);
    /*unset ICH_HCR_EL2 bits [2:0] to disable the interrupt*/
    data &= ~0x7;
    val_gic_reg_write(ICH_HCR_EL2, data);

    val_set_status(index, RESULT_PASS(TEST_NUM, 1));
    val_print(ACS_PRINT_INFO, "\n       Received GIC maintenance interrupt ", 0);
    val_gic_end_of_interrupt(intid);
}

static
void
payload()
{

    /*Check GIC Maintenance interrupt received*/
    uint32_t data;
    uint32_t timeout = TIMEOUT_LARGE;
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

    if (val_pe_reg_read(CurrentEL) == AARCH64_EL1) {
        val_print(ACS_PRINT_DEBUG, "\n       Skipping. Test accesses EL2"
                                    " Registers       ", 0);
        val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
        return;
    }

    /*Check GIC maintenance interrupt*/
    val_set_status(index, RESULT_PENDING(TEST_NUM));
    /*Get GIC maintenance interrupt ID*/
    intid = val_pe_get_gmain_gsiv(index);
    /*Recommended GIC maintenance interrupt ID is 25 as per SBSA*/
    if (g_build_sbsa) {
        if (intid != 25) {
           val_print(ACS_PRINT_ERR,
                     "\n       GIC Maintenance interrupt not mapped to PPI ID 25, id %d", intid);
           val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
           return;
        }
    }
    /*Check if interrupt is in PPI INTID range*/
    if ((intid < 16 || intid > 31) && (!val_gic_is_valid_eppi(intid))) {
        val_print(ACS_PRINT_DEBUG,
            "\n       GIC Maintenance interrupt not mapped to PPI base range,"
            "\n       INTID: %d   ", intid);
        val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
        return;
    }

    if (val_gic_install_isr(intid, isr_mnt)) {
        val_print(ACS_PRINT_ERR, "\n       GIC Install Handler Failed...", 0);
        val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
        return;
    }

    /*Write to GIC registers which will generate Maintenance interrupt*/
    data = val_gic_reg_read(ICH_HCR_EL2);
    /*set ICH_HCR_EL2 bits [2:0] to enable the interrupt*/
    data |= 0x7;
    val_gic_reg_write(ICH_HCR_EL2, data);

    while ((--timeout > 0) && (IS_RESULT_PENDING(val_get_status(index)))) {
        ;
    }

    if (timeout == 0) {
        val_print(ACS_PRINT_ERR, "\n       Interrupt not received within timeout", 0);
        val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
        return;
    }
    return;
}

uint32_t
hyp_g003_entry(uint32_t num_pe)
{

    uint32_t status = ACS_STATUS_FAIL;
    /*This GIC test is run on single processor*/
    num_pe = 1;

    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
    if (status != ACS_STATUS_SKIP)
        val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

    val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

    return status;
}
