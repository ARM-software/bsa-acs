/** @file
 * Copyright (c) 2016-2021, 2023 Arm Limited or its affiliates. All rights reserved.
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

#define TEST_NUM   (ACS_GIC_HYP_TEST_NUM_BASE + 1)
#define TEST_RULE  "B_PPI_02"
#define TEST_DESC  "Check NS EL2-Virt timer PPI Assignment"

static uint32_t intid;

/*Interrupt handler for the hypervisor virtual timer interrupt*/
static
void
isr_vir()
{
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
    /* We received our interrupt, so disable timer from generating further interrupts */
    val_timer_set_vir_el2(0);
    val_print(ACS_PRINT_INFO, "\n       Received vir el2 interrupt    ", 0);
    val_set_status(index, RESULT_PASS(TEST_NUM, 1));
    val_gic_end_of_interrupt(intid);
}

static
void
payload()
{

    /*Check CNTHV interrupt received*/
    uint32_t data;
    uint32_t timeout = TIMEOUT_LARGE;
    uint64_t timer_expire_val = 100;
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

    if (val_pe_reg_read(CurrentEL) == AARCH64_EL1) {
        val_print(ACS_PRINT_DEBUG, "\n       Skipping. Test accesses EL2"
                                    " Registers       ", 0);
        val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
        return;
    }

    /*Check non-secure EL2 virtual timer*/
    val_set_status(index, RESULT_PENDING(TEST_NUM));

    /* This test is run only when ARM v8.1 Virtualized Host Extensions are supported */
    data = val_pe_reg_read(ID_AA64MMFR1_EL1);

    /* Check for EL2 virtual timer interrupt, if PE supports 8.1 or greater.
     * ID_AA64MMFR1_EL1 VH, bits [11:8] must be 0x1 */
    if (!((data >> 8) & 0xF)) {
        val_print(ACS_PRINT_DEBUG, "\n       v8.1 VHE not supported on this PE ", 0);
        val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
        return;
    }

    /*Get EL2 virtual timer interrupt ID*/
    intid = val_timer_get_info(TIMER_INFO_VIR_EL2_INTID, 0);
    /*Recommended EL2 virtual timer interrupt ID is 28 as per SBSA*/
    if (g_build_sbsa) {
        if (intid != 28) {
           val_print(ACS_PRINT_ERR, "\n       NS EL2 virtual timer not mapped to PPI ID 28, id %d",
                                                                    intid);
           val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
           return;
        }
    }
    /*Check if interrupt is in PPI INTID range*/
    if ((intid < 16 || intid > 31) && (!val_gic_is_valid_eppi(intid))) {
        val_print(ACS_PRINT_DEBUG,
                    "\n       NS EL2 virtual timer not mapped to PPI base range, INTID: %d   ",
                    intid);
        val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
        return;
    }

    if (val_gic_install_isr(intid, isr_vir)) {
        val_print(ACS_PRINT_ERR, "\n       GIC Install Handler Failed...", 0);
        val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
        return;
    }

    val_timer_set_vir_el2(timer_expire_val);
    while ((--timeout > 0) && (IS_RESULT_PENDING(val_get_status(index)))) {
        ;
    }

    if (timeout == 0) {
        val_print(ACS_PRINT_ERR,
            "\n       NS EL2 Virtual timer interrupt %d not received", intid);
        val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
    }
    return;
}

uint32_t
hyp_g001_entry(uint32_t num_pe)
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
