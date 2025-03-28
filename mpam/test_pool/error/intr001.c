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

#define TEST_NUM   ACS_MPAM_ERROR_TEST_NUM_BASE  +  12
#define TEST_DESC  "Check Level-Sensitive Error Interrupt "
#define TEST_RULE  ""

static uint32_t msc_index;
static uint32_t intr_num;

static void *branch_to_test;

static
void
esr(uint64_t exception_type, void *context)
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  /* Update the ELR to point to next instrcution */
  val_pe_update_elr(context, (uint64_t)branch_to_test);

  val_print(ACS_PRINT_WARN, "\n       Received Exception of type %d", exception_type);
  val_set_status(index, RESULT_FAIL(TEST_NUM, 04));
}

static void intr_handler(void)
{
    uint32_t pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

    val_print(ACS_PRINT_DEBUG, "\n       Received MSC Err interrupt %d", intr_num);
    val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));

    /* Write 0b0000 into MPAMF_ESR.ERRCODE to clear the interrupt */
    val_mpam_msc_reset_errcode(msc_index);

    /* Send EOI to the CPU Interface */
    val_gic_end_of_interrupt(intr_num);
    return;
}

static
void payload(void)
{

    uint8_t intr_type;
    uint32_t intr_flags;
    uint32_t pe_index;
    uint32_t status;
    uint32_t total_nodes;
    uint32_t timeout;
    uint32_t mpamf_ecr;
    uint32_t intr_count = 0;

    pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
    total_nodes = val_mpam_get_msc_count();

    val_pe_install_esr(EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS, esr);
    val_pe_install_esr(EXCEPT_AARCH64_SERROR, esr);

    branch_to_test = &&exception_taken;

    for (msc_index = 0; msc_index < total_nodes; msc_index++) {

        if (!val_mpam_msc_supports_esr(msc_index)) {
            val_print(ACS_PRINT_DEBUG, "\n       MSC index %d does not support ESR", msc_index);
            continue;
        }

        intr_num = val_mpam_get_info(MPAM_MSC_ERR_INTR, msc_index, 0);
        intr_flags = val_mpam_get_info(MPAM_MSC_ERR_INTR_FLAGS, msc_index, 0);

        val_print(ACS_PRINT_DEBUG, "\n       Error interrupt flags - 0x%llx", intr_flags);
        intr_type = intr_flags & MPAM_ACPI_ERR_INTR_TYPE_MASK;

        /* Read MPAMF_ECR before generating error. This will be used to restore to default later */
        mpamf_ecr = val_mpam_mmr_read(msc_index, REG_MPAMF_ECR);
        status    = val_mpam_msc_reset_errcode(msc_index);

        if (!status) {
            val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
            return;
        }

        /*
         * Skip this MSC if it doesn't implement error interrupt
         * or if its error interrupt is not of type level-trigger
         */
        if ((intr_num == 0) || (intr_type != MPAM_ACPI_ERR_INTR_TYPE_LEVEL)) {
            val_print(ACS_PRINT_DEBUG,
                "\n       MSC does not implement level triggered Error intr. Skipping MSC", 0);
            continue;
        } else {
            intr_count++;
        }

        /* Register the interrupt handler */
        if (val_gic_install_isr(intr_num, intr_handler)) {
            val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));
            return;
        }

        /* Enable affinity routing to receive intr_num on primary PE */
        val_gic_route_interrupt_to_pe(intr_num, val_pe_get_mpid_index(pe_index));

        /*
         * Set the interrupt enable bit in MPAMF_ECR & raise
         * an interrupt by writing non-zero to MPAMF_ESR.ERRCODE
         */
        val_print(ACS_PRINT_DEBUG, "\n       Triggering MSC Error interrupt %d", intr_num);
        val_mpam_msc_trigger_intr(msc_index);

        /* PE busy polls to check the completion of interrupt service routine */
        timeout = TIMEOUT_LARGE;
        while ((--timeout > 0) && (IS_RESULT_PENDING(val_get_status(pe_index))));

        /* Restore Error Control Register original settings */
        val_mpam_mmr_write(msc_index, REG_MPAMF_ECR, mpamf_ecr);

        if (timeout == 0) {
            val_print(ACS_PRINT_ERR, "\n       MSC Err Interrupt not received on %d", intr_num);
            val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 03));
            return;
        }
    }

    /* Set the test status to Skip as none of the MPAM nodes implemented error interrupts */
    if (intr_count == 0) {
        val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 01));
        return;
    }

    val_set_status(pe_index, RESULT_PASS(TEST_NUM, 02));
    return;

exception_taken:
    return;
}

uint32_t intr001_entry(void)
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
