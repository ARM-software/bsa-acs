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

#define TEST_NUM   ACS_MPAM_ERROR_TEST_NUM_BASE  +  14
#define TEST_DESC  "Check MBWU Mon Overflow interrupt     "
#define TEST_RULE  ""

static uint32_t msc_index;
static uint32_t intr_num;
static uint64_t mpam2_el2_temp;

static
void intr_handler(void)
{
    uint32_t pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

    val_print(ACS_PRINT_DEBUG, "\n       Received Oflow error interrupt %d     ", intr_num);
    val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));

    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MBWU_CTL,
                                                ((1 << MBWU_CTL_OFLOW_STATUS_BIT_SHIFT) |
                                                (1 << MBWU_CTL_OFLOW_STATUS_L_SHIFT)));

    /* Write 0b0000 into MPAMF_ESR.ERRCODE to clear the interrupt */
    val_mpam_msc_reset_errcode(msc_index);

    /* Send EOI to the CPU Interface */
    val_gic_end_of_interrupt(intr_num);
    return;
}

static
void payload(void)
{

    uint16_t mon_count;
    uint32_t pe_index;
    uint32_t total_nodes;
    uint32_t rsrc_node_cnt;
    uint32_t rsrc_index;
    uint64_t mpam2_el2;
    uint64_t timeout;
    uint32_t status;
    uint64_t buf_size;
    uint64_t base;
    uint32_t data;
    uint64_t nrdy_timeout;
    void *src_buf = 0;
    void *dest_buf = 0;
    uint32_t intr_count = 0;

    pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
    total_nodes = val_mpam_get_msc_count();

    mpam2_el2 = val_mpam_reg_read(MPAM2_EL2);
    mpam2_el2_temp = mpam2_el2;

    /* Clear the PARTID_D & PMG_D bits in mpam2_el2 before writing to them */
    mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2, MPAMn_ELx_PARTID_D_SHIFT+15, MPAMn_ELx_PARTID_D_SHIFT);
    mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2, MPAMn_ELx_PMG_D_SHIFT+7, MPAMn_ELx_PMG_D_SHIFT);

    /* Write default partid and default pmg to mpam2_el2 to generate PE traffic */
    mpam2_el2 |= (((uint64_t)DEFAULT_PMG << MPAMn_ELx_PMG_D_SHIFT) |
                  ((uint64_t)DEFAULT_PARTID << MPAMn_ELx_PARTID_D_SHIFT));

    val_mpam_reg_write(MPAM2_EL2, mpam2_el2);

    for (msc_index = 0; msc_index < total_nodes; msc_index++) {

        intr_num = val_mpam_get_info(MPAM_MSC_OF_INTR, msc_index, 0);

        rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);
        for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

            if (val_mpam_get_info(MPAM_MSC_RSRC_TYPE, msc_index, rsrc_index)
                                                                        != MPAM_RSRC_TYPE_MEMORY) {
                val_print(ACS_PRINT_TEST, "\n       MSC %d not a memory node. Skipping MSC",
                            msc_index);
                continue;
            }

            /* Select resource instance if RIS feature implemented */
            if (val_mpam_msc_supports_ris(msc_index))
                val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

            /* Read the number of monitors implemented in this MSC */
            if (val_mpam_msc_supports_mon(msc_index)) {
                mon_count = val_mpam_msc_supports_mbwumon(msc_index) ?
                            val_mpam_get_mbwumon_count(msc_index) : 0;
            }

            val_print(ACS_PRINT_DEBUG, "\n       MSC Index: %d", msc_index);
            val_print(ACS_PRINT_DEBUG, "\n       MBWU Monitors: %d", mon_count);
            val_print(ACS_PRINT_DEBUG, "\n       Overflow Interrupt: %d", intr_num);

            /*
            * Skip this MSC if it doesn't implement overflow interrupt
            * support (or) if it doesn't implement any monitors
            */
            if ((intr_num == 0) || (mon_count == 0))
                continue;

            /* Register the interrupt handler */
            status = val_gic_install_isr(intr_num, intr_handler);
            if (status) {
                val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
                val_mpam_reg_write(MPAM2_EL2, mpam2_el2_temp);
                return;
            }

            /* Enable affinity routing to receive intr_num on primary PE */
            val_gic_route_interrupt_to_pe(intr_num, val_pe_get_mpid_index(pe_index));

            intr_count++;
            /* Generate MBWU monitor overflow error for this memory node */
            val_mpam_msc_generate_msmon_oflow_error(msc_index, mon_count);

            /* Create 1 MB buffers sufficient to cretae overflow for this memory channel */
            buf_size = 1 * SIZE_1M;
            base = val_mpam_memory_get_base(msc_index, rsrc_index);
            src_buf = (void *)val_mem_alloc_at_address(base, buf_size);
            dest_buf = (void *)val_mem_alloc_at_address(base + buf_size, buf_size);

            if ((src_buf == NULL) || (dest_buf == NULL)) {
                val_print(ACS_PRINT_ERR, "\n       Mem allocation for buffers failed", 0x0);
                val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));
                return;
            }

            data = val_mpam_mmr_read(msc_index, REG_MSMON_CFG_MBWU_CTL);
            data = data | (1 << MBWU_CTL_ENABLE_SHIFT);
            val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MBWU_CTL, data);
            val_print(ACS_PRINT_DEBUG, "\n       MSMON_CFG_MBWU_CTL is %llx",
                val_mpam_mmr_read(msc_index, REG_MSMON_CFG_MBWU_CTL));
            val_print(ACS_PRINT_DEBUG, "\n       Monitor count is %llx",
                                                    val_mpam_memory_mbwumon_read_count(msc_index));

            /* wait for MAX_NRDY_USEC after msc config change */
            nrdy_timeout = val_mpam_get_info(MPAM_MSC_NRDY, msc_index, 0);
            while (nrdy_timeout) {
                --nrdy_timeout;
            };

            /* Start mem copy to cause the overflow interrupt */
            val_memcpy(src_buf, dest_buf, buf_size);

            /* PE busy polls to check the completion of interrupt service routine */
            timeout = TIMEOUT_LARGE;
            while ((--timeout > 0) && (IS_RESULT_PENDING(val_get_status(pe_index))));

            val_print(ACS_PRINT_DEBUG, "\n       MSMON_CFG_MBWU_CTL is %llx",
                                            val_mpam_mmr_read(msc_index, REG_MSMON_CFG_MBWU_CTL));
            val_print(ACS_PRINT_DEBUG, "\n       Monitor count is %llx",
                                                val_mpam_memory_mbwumon_read_count(msc_index));
            if (timeout == 0) {
                val_print(ACS_PRINT_ERR,
                    "\n       MSC MSMON Oflow Err Interrupt not received on %d", intr_num);
                val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 03));

                /* Free the buffers after return from overflow interrupt */
                val_mem_free_at_address(base, buf_size);
                val_mem_free_at_address(base + buf_size, buf_size);

                /* disable and reset the MBWU monitor */
                val_mpam_memory_mbwumon_disable(msc_index);
                val_mpam_memory_mbwumon_reset(msc_index);
                val_mpam_reg_write(MPAM2_EL2, mpam2_el2_temp);
                return;
            }
        }
    }

    /* Restore MPAM2_EL2 settings */
    val_mpam_reg_write(MPAM2_EL2, mpam2_el2_temp);
    /* Free the buffers after return from overflow interrupt */
    val_mem_free_at_address(base, buf_size);
    val_mem_free_at_address(base + buf_size, buf_size);

    /* Set the test status to Skip if none of the MPAM nodes implement error interrupts */
    if (intr_count == 0) {
        val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 01));
        return;
    }

    val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));
    return;
}

uint32_t intr003_entry(void)
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
