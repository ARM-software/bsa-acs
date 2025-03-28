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
#include "val/common/include/acs_memory.h"
#include "val/common/include/acs_mpam_reg.h"
#include "val/mpam/include/mpam_val_interface.h"

#define TEST_NUM   ACS_MPAM_MEMORY_TEST_NUM_BASE  +  2
#define TEST_DESC  "Check MBWMIN Partitioning             "
#define TEST_RULE  ""

#define MEMCPY_BUF_SIZE 10 * SIZE_1G  // Default buffer size
#define BW1_PERCENTAGE  25            // Min BW limit partitioning scenario 1
#define BW2_PERCENTAGE  10            // Min BW limit partitioning scenario 2
#define NUM_PE_CONT     04            // Number of PEs used to create BW contention
#define MBWMIN_SCENARIO_MAX 2

static uint8_t contend_flag;
static uint32_t num_pe_cont;
static void *branch_to_test;

static
void
esr(uint64_t interrupt_type, void *context)
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  /* Update the ELR to point to next instrcution */
  val_pe_update_elr(context, (uint64_t)branch_to_test);

  val_print(ACS_PRINT_ERR, "\n       Received Exception of type %d", interrupt_type);
  val_set_status(index, RESULT_FAIL(TEST_NUM, 01));
}


static int wait_for_secondary_off(uint32_t primary_pe_index)
{

    uint32_t pe_index;
    uint32_t payload_status;
    uint64_t timeout;

    timeout = (num_pe_cont * TIMEOUT_LARGE);

    /* Wait for all pe OFF or timeout, whichever is first */
    do {
        payload_status = 0;

        for (pe_index = 0; pe_index < num_pe_cont; pe_index++) {

            if (pe_index != primary_pe_index) {
                payload_status |= IS_RESULT_PENDING(val_get_status(pe_index));
            }
        }
    } while (payload_status && (--timeout));

    /* Print all pending pe indices to console at timeout */
    if (!timeout) {

        for (pe_index = 0; pe_index < num_pe_cont; pe_index++) {

            if ((pe_index != primary_pe_index) && IS_RESULT_PENDING(val_get_status(pe_index))) {
                val_print(ACS_PRINT_ERR, " Secondary PE %x OFF time-out \n", pe_index);
            }
        }
        return 1;
    }

    return 0;
}

static void config_mpam_params(uint32_t mpam2_el2)
{

    uint32_t msc_index;
    uint32_t total_node;
    uint32_t rsrc_node_cnt;
    uint32_t rsrc_index;
    uint16_t minmax_partid;

    minmax_partid = DEFAULT_PARTID_MAX;
    total_node = val_mpam_get_msc_count();

    /* Compute the min partition id supported among all MPAM nodes */
    for (msc_index = 0; msc_index < total_node; msc_index++) {
        minmax_partid = GET_MIN_VALUE(minmax_partid,
                             val_mpam_get_max_partid(msc_index));
    }

    /* Disable all types of partitioning for all cache nodes */
    for (msc_index = 0; msc_index < total_node; msc_index++) {

        rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);
        for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

            if (val_mpam_msc_supports_ris(msc_index))
                val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

            if (val_mpam_supports_cpor(msc_index)) {
                /* Disable CPOR partitioning for min(max(PARTID)) */
                val_mpam_configure_cpor(msc_index, minmax_partid, 100);
            }

            if (val_mpam_supports_ccap(msc_index)) {
                /* Disable CCAP partitioning for min(max(PARTID)) */
                val_mpam_configure_ccap(msc_index, minmax_partid, 0, 100);
            }
        }
    }

    /* Clear the PARTID_D & PMG_D bits in mpam2_el2 before writing to them */
    mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2, MPAMn_ELx_PARTID_D_SHIFT+15, MPAMn_ELx_PARTID_D_SHIFT);
    mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2, MPAMn_ELx_PMG_D_SHIFT+7, MPAMn_ELx_PMG_D_SHIFT);

    /* Write MINMAX_PARTID & DEFAULT PMG to mpam2_el2 to generate PE traffic */
    mpam2_el2 |= (((uint64_t)DEFAULT_PMG << MPAMn_ELx_PMG_D_SHIFT) |
                  ((uint64_t)minmax_partid << MPAMn_ELx_PARTID_D_SHIFT));

    val_mpam_reg_write(MPAM2_EL2, mpam2_el2);

    return;
}

void static payload_secondary()
{

    uint32_t pe_index;
    uint8_t *src_buf = 0;
    uint8_t *dest_buf = 0;
    uint64_t buf_size;
    uint64_t mpam2_el2 = 0;

    pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
    mpam2_el2 = val_mpam_reg_read(MPAM2_EL2);

    /* Make this PE configurations */
    config_mpam_params(mpam2_el2);

    /* Create buffers to perform memcopy (stream copy) */
    buf_size = MEMCPY_BUF_SIZE / 2;
    src_buf = (uint8_t *) val_get_shared_memcpybuf(pe_index);
    dest_buf = src_buf + buf_size;

    if ((src_buf == NULL) || (dest_buf == NULL)) {
        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));

        /* Restore MPAM2_EL2 settings */
        val_mpam_reg_write(MPAM2_EL2, mpam2_el2);
        return;
    }

    /* Generate memory bandwidth contention via PE traffic */
    while (contend_flag) {
        val_memcpy((void *)src_buf, (void *)dest_buf, buf_size);
        val_data_cache_ops_by_va((addr_t)&contend_flag, INVALIDATE);
    }

    /* Restore MPAM2_EL2 settings */
    val_mpam_reg_write(MPAM2_EL2, mpam2_el2);
    val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));

    return;
}

static
uint64_t
get_buffer_size(uint32_t msc_index, uint32_t rsrc_index, uint32_t num_pe_cont)
{
    uint64_t mem_length;
    uint64_t buf_size;

    mem_length = val_mpam_memory_get_size(msc_index, rsrc_index);
    if (mem_length > (NUM_PE_CONT * MEMCPY_BUF_SIZE))
        buf_size =  MEMCPY_BUF_SIZE;
    else
        buf_size =  (mem_length / NUM_PE_CONT);

    val_print(ACS_PRINT_DEBUG, "\n       Chosen Buffer Size is 0x%llx", buf_size);
    return buf_size;
}

static
void
payload_primary(void)
{

    uint32_t pe_index;
    uint32_t msc_index;
    uint32_t status;
    uint32_t primary_pe_index;
    uint32_t rsrc_node_cnt;
    uint32_t rsrc_index;
    uint32_t mbwmin_node_cnt = 0;
    uint16_t minmax_partid;
    uint64_t mpam2_el2 = 0;
    uint8_t alloc_status;
    uint8_t *src_buf = 0;
    uint8_t *dest_buf = 0;
    uint64_t buf_size;
    uint64_t start_count;
    uint64_t end_count;
    uint64_t nrdy_timeout;
    uint32_t scenario_cnt = 0;
    uint32_t num_pe = val_pe_get_num();
    uint32_t total_nodes =  val_mpam_get_msc_count();
    uint64_t counter[total_nodes][10][MBWMIN_SCENARIO_MAX];

    minmax_partid = DEFAULT_PARTID_MAX;
    primary_pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

    /*
     * Compute the number of MIN BW supported MPAM MEMORY nodes,
     * and the min partition id supported among all MPAM nodes
     */
    for (msc_index = 0; msc_index < total_nodes; msc_index++) {

        rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);

        val_print(ACS_PRINT_DEBUG, "\n       msc index  = %d", msc_index);
        val_print(ACS_PRINT_DEBUG, "\n       Resource count = %d ", rsrc_node_cnt);

        for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

            /* Skip the MSC if it is not a memory node */
            if (val_mpam_get_info(MPAM_MSC_RSRC_TYPE, msc_index, rsrc_index)
                                                                    != MPAM_RSRC_TYPE_MEMORY)
                continue;

            if (val_mpam_msc_supports_ris(msc_index))
                val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

            if (val_mpam_msc_supports_mbw_min(msc_index)) {
                mbwmin_node_cnt++;
            }
        }

        minmax_partid = GET_MIN_VALUE(minmax_partid,
                                      val_mpam_get_max_partid(msc_index));
    }

    /* Skip this test if no MIN BW supported MPAM memory node present in the system */
    if (mbwmin_node_cnt == 0) {
        val_print(ACS_PRINT_TEST,
                "\n       %d MSC Memory Nodes support MBW Min Limit Partitioning", mbwmin_node_cnt);
        val_set_status(primary_pe_index, RESULT_SKIP(TEST_NUM, 01));
        return;
    }

    mpam2_el2 = val_mpam_reg_read(MPAM2_EL2);

    /* Set the memory of coutner arr to 0 */
    val_memory_set(counter, sizeof(counter), 0);


      /* Install sync and async handlers to handle exceptions.*/
    status = val_pe_install_esr(EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS, esr);
    status |= val_pe_install_esr(EXCEPT_AARCH64_SERROR, esr);
    branch_to_test = &&exception_return;
    if (status)
    {
        val_print(ACS_PRINT_ERR, "\n       Failed in installing the exception handler", 0);
        val_set_status(primary_pe_index, RESULT_FAIL(TEST_NUM, 03));
        return;
    }

    /* Make this PE configurations */
    config_mpam_params(mpam2_el2);

    for (msc_index = 0; msc_index < total_nodes; msc_index++) {

        rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);
        for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

            if (val_mpam_msc_supports_mbw_min(msc_index)) {

                if (val_mpam_get_info(MPAM_MSC_RSRC_TYPE, msc_index, rsrc_index)
                                                                != MPAM_RSRC_TYPE_MEMORY)
                    continue;

                /* Select Resource within an MSC */
                if (val_mpam_msc_supports_ris(msc_index))
                    val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

                /* Disable MBWPBM partitioning for the current memory msc_index */
                if (val_mpam_msc_supports_mbwpbm(msc_index))
                    val_mpam_configure_mbwpbm(msc_index, minmax_partid, 100);

                /* Disable MBWMAX partitioning for the current memory msc_index */
                if (val_mpam_msc_supports_mbw_max(msc_index))
                    val_mpam_msc_configure_mbwmax(msc_index, minmax_partid, 1, 100);

                /* Create BW contention using 4 other PEs */
                num_pe_cont = num_pe >= NUM_PE_CONT ? NUM_PE_CONT : num_pe;

                buf_size = get_buffer_size(msc_index, rsrc_index, num_pe_cont);
                /* Create a shared memcopy buffer from this memory node */
                alloc_status = val_alloc_shared_memcpybuf(
                                                    val_mpam_memory_get_base(msc_index, rsrc_index),
                                                    buf_size,
                                                    num_pe_cont);

                if (alloc_status == 0) {
                    val_set_status(primary_pe_index, RESULT_FAIL(TEST_NUM, 04));
                    val_mpam_reg_write(MPAM2_EL2, mpam2_el2);
                    return;
                }

                /* Create buffers to perform memcopy (stream copy) */
                src_buf = (uint8_t *) val_get_shared_memcpybuf(primary_pe_index);
                dest_buf = src_buf + buf_size;

                /****************************************************************
                 *                        SCENARIO ONE
                 ***************************************************************/

                scenario_cnt = 0;

                /* Configure the current memory msc_index for MIN BW1 */
                val_mpam_msc_configure_mbwmin(msc_index, minmax_partid, BW1_PERCENTAGE);

                contend_flag = 1;
                val_data_cache_ops_by_va((addr_t)&contend_flag, CLEAN);

                /* Create bandwidth contention on the current memory node */
                for (pe_index = 0; pe_index < num_pe_cont; pe_index++) {

                    if (pe_index != primary_pe_index) {
                        val_set_status(pe_index, RESULT_PENDING(TEST_NUM));
                        val_execute_on_pe(pe_index, payload_secondary, 0);
                    }
                }

                if (!val_mpam_get_mbwumon_count(msc_index)) {
                    val_print(ACS_PRINT_TEST,
                        "\n       No MBWU Monitor found to validate the test. Skipping test", 0);
                        val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 02));
                        contend_flag = 0;
                        return;
                }

                val_print(ACS_PRINT_DEBUG,
                            "\n       Using MBWU monitor to measure MBW during buffer copy", 0);
                /* configure MBWU Monitor for this memory resource node */
                val_mpam_memory_configure_mbwumon(msc_index);

                /* enable MBWU monitoring */
                val_mpam_memory_mbwumon_enable(msc_index);

                /* wait for MAX_NRDY_USEC after msc config change */
                nrdy_timeout = val_mpam_get_info(MPAM_MSC_NRDY, msc_index, 0);
                while (nrdy_timeout) {
                    --nrdy_timeout;
                };

                start_count = val_mpam_memory_mbwumon_read_count(msc_index);
                val_print(ACS_PRINT_TEST, "\n       Start Count = 0x%llx", start_count);
                /* perform memory operation */
                val_memcpy((void *)src_buf, (void *)dest_buf, buf_size);

                end_count = val_mpam_memory_mbwumon_read_count(msc_index);
                val_print(ACS_PRINT_TEST, "\n       End Count = 0x%llx", end_count);
                /* read the memory bandwidth usage monitor */
                counter[msc_index][rsrc_index][scenario_cnt] = end_count - start_count;

                val_print(ACS_PRINT_TEST,
                "\n       Byte count = 0x%llx", end_count - start_count);

                /* disable and reset the MBWU monitor */
                val_mpam_memory_mbwumon_disable(msc_index);
                val_mpam_memory_mbwumon_reset(msc_index);

                contend_flag = 0;
                val_data_cache_ops_by_va((addr_t)&contend_flag, CLEAN);

                /* Return from the test if any secondary pe is timed out */
                if (wait_for_secondary_off(primary_pe_index)) {
                    goto error_secondary_pending;
                }

                /****************************************************************
                 *                        SCENARIO TWO
                 ***************************************************************/

                scenario_cnt++;

                /* Configure the current memory msc_index for MIN BW2 */
                val_mpam_msc_configure_mbwmin(msc_index, minmax_partid, BW2_PERCENTAGE);

                contend_flag = 1;
                val_data_cache_ops_by_va((addr_t)&contend_flag, CLEAN);

                /* Create bandwidth contention on the current memory node */
                for (pe_index = 0; pe_index < num_pe_cont; pe_index++) {
                    if (pe_index != primary_pe_index) {
                        val_set_status(pe_index, RESULT_PENDING(TEST_NUM));
                        val_execute_on_pe(pe_index, payload_secondary, 0);
                    }
                }

                /* enable MBWU monitoring */
                val_mpam_memory_mbwumon_enable(msc_index);

                /* wait for MAX_NRDY_USEC after msc config change */
                nrdy_timeout = val_mpam_get_info(MPAM_MSC_NRDY, msc_index, 0);
                while (nrdy_timeout) {
                    --nrdy_timeout;
                };

                start_count = val_mpam_memory_mbwumon_read_count(msc_index);
                val_print(ACS_PRINT_TEST, "\n       Start Count = 0x%llx", start_count);

                /* perform memory operation */
                val_memcpy((void *)src_buf, (void *)dest_buf, buf_size);
                end_count = val_mpam_memory_mbwumon_read_count(msc_index);
                val_print(ACS_PRINT_TEST, "\n       End Count = 0x%llx", end_count);

                /* read the memory bandwidth usage monitor */
                counter[msc_index][rsrc_index][scenario_cnt] = end_count - start_count;

                val_print(ACS_PRINT_TEST,
                "\n       Byte Count = 0x%llx", end_count - start_count);

                /* disable and reset the MBWU monitor */
                val_mpam_memory_mbwumon_disable(msc_index);
                val_mpam_memory_mbwumon_reset(msc_index);

                contend_flag = 0;
                val_data_cache_ops_by_va((addr_t)&contend_flag, CLEAN);

                /* Return from the test if any secondary is timed out */
                if (wait_for_secondary_off(primary_pe_index)) {
                    goto error_secondary_pending;
                }

                /* Free the copy buffers to the heap manager */
                val_mem_free_shared_memcpybuf(NUM_PE_CONT);
            }
        }
    }

    /* Restore MPAM2_EL2 settings */
    val_mpam_reg_write(MPAM2_EL2, mpam2_el2);

    /* Compare the stream copy MBW counters for all the scenarios */
    for (msc_index = 0; msc_index < total_nodes; msc_index++) {
        rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);

        for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

            if (val_mpam_msc_supports_ris(msc_index))
                val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

            if (val_mpam_msc_supports_mbw_min(msc_index)) {
                scenario_cnt = 0;
                if (counter[msc_index][rsrc_index][scenario_cnt] <
                                        counter[msc_index][rsrc_index][scenario_cnt + 1]) {
                        val_print(ACS_PRINT_ERR, "\n       Failed for msc_index : %d", msc_index);
                        val_set_status(primary_pe_index, RESULT_FAIL(TEST_NUM, 05));
                        return;
                }
            }
        }
    }

    /* Set the test status to pass */
    val_set_status(primary_pe_index, RESULT_PASS(TEST_NUM, 01));

    return;

error_secondary_pending:
    /* Restore MPAM2_EL2 settings */
    val_mpam_reg_write(MPAM2_EL2, mpam2_el2);

    /* Return the copy buffers to the heap manager */
    val_mem_free_shared_memcpybuf(num_pe_cont);

exception_return:
    return;
}

uint32_t
mem002_entry(void)
{

    uint32_t status = ACS_STATUS_FAIL;
    uint32_t num_pe = 1;

    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

    if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload_primary, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

    val_report_status(0, ACS_END(TEST_NUM), NULL);

    return status;
}
