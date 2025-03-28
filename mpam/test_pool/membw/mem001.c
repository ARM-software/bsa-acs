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

#define TEST_NUM   ACS_MPAM_MEMORY_TEST_NUM_BASE  +  1
#define TEST_DESC  "Check MBWPBM Partitioning             "
#define TEST_RULE  ""

#define MBWPBM_SCENARIO_MAX 10
static uint64_t mpam2_el2_temp;

typedef struct {
    char8_t description[64];
    uint8_t partition_percent;  //as function of max mem bandwidth size
    uint64_t memcopy_size;
} mbwpbm_config_t;

mbwpbm_config_t mbwpbm_config_data[] = {
    {"MBWPBM latency check for 20% bw portion size", 20, 30 * SIZE_1G},
    {"MBWPBM latency check for 05% bw portion size", 05, 30 * SIZE_1G},
};

static
uint64_t
get_buffer_size(uint32_t msc_index, uint32_t rsrc_index, uint64_t addr_len, uint32_t index)
{
    uint64_t membw;
    uint64_t limited_bw;
    uint64_t buffer_size;
    uint64_t offset = 5 * SIZE_1G;

    membw = val_mpam_msc_get_mscbw(msc_index, rsrc_index);
    if (membw == HMAT_INVALID_INFO) {
        val_print(ACS_PRINT_WARN, "\n       Using default buffer sizes for memcpy", 0);
        buffer_size = mbwpbm_config_data[index].memcopy_size;
        val_print(ACS_PRINT_WARN, "  buffer_size: %llx", buffer_size);
        return buffer_size;
    }

    val_print(ACS_PRINT_DEBUG, "\n       Channel BW - %d MB/s", membw);
    limited_bw = membw / (mbwpbm_config_data[index].partition_percent);
    val_print(ACS_PRINT_DEBUG, "\n       Restricting Channel BW to %d MB/s", limited_bw);

    /* Calculate required buffer to index 0 and use the same for other variations of
           partition percentage */
    if (index == 0)
        buffer_size = (limited_bw / 2) + offset;
    else
        buffer_size = mbwpbm_config_data[0].memcopy_size;

    val_print(ACS_PRINT_DEBUG, "\n       Buffer Size - 0x%llx", buffer_size);

    /* Check if this buffer is possible to allocate */
    if (addr_len < (2 * buffer_size)) {
        val_print(ACS_PRINT_WARN, "\n       Insufficient buffer size to validate the test", 0);
        return ACS_STATUS_SKIP;
    }

    return buffer_size;
}


static
void payload(void)
{

    uint8_t enabled_scenarios = 0;
    uint16_t minmax_partid;
    uint16_t index;
    uint32_t msc_index;
    uint32_t rsrc_index;
    uint32_t rsrc_node_cnt;
    uint32_t mbwpbm_node_cnt = 0;
    void *src_buf = 0;
    void *dest_buf = 0;
    uint64_t buf_size;
    uint64_t start_count;
    uint64_t end_count;
    uint64_t addr_base, addr_len;
    uint64_t  nrdy_timeout;
    uint64_t mpam2_el2 = 0;
    uint32_t pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
    uint32_t total_nodes = val_mpam_get_msc_count();
    uint64_t counter[MBWPBM_SCENARIO_MAX][total_nodes][10];

    minmax_partid = DEFAULT_PARTID_MAX;

    /*
     * Compute the number of MBWPBM supported nodes,
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

            /* Select Resource within MSC and read MBWPBM info */
            if (val_mpam_msc_supports_ris(msc_index))
                val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

            if (val_mpam_msc_supports_mbwpbm(msc_index)) {
                mbwpbm_node_cnt++;
            }
        }

        minmax_partid = GET_MIN_VALUE(minmax_partid, val_mpam_get_max_partid(msc_index));
    }

    val_print(ACS_PRINT_TEST,
        "\n       %d MSC Memory Nodes support MBW Portion Partitioning", mbwpbm_node_cnt);

    /* Skip this test if no MBWPBM supported MSC present in the system */
    if (mbwpbm_node_cnt == 0) {
        val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 01));
        return;
    }

    val_print(ACS_PRINT_DEBUG, "\n       MinMax PARTID = %d\n", minmax_partid);

    /* Disable all types of partitioning for all other nodes */
    for (msc_index = 0; msc_index < total_nodes; msc_index++) {

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

    mpam2_el2 = val_mpam_reg_read(MPAM2_EL2);
    mpam2_el2_temp = mpam2_el2;

    /* Clear the PARTID_D & PMG_D bits in mpam2_el2 before writing to them */
    mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2, MPAMn_ELx_PARTID_D_SHIFT + 15,
                                                                MPAMn_ELx_PARTID_D_SHIFT);
    mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2, MPAMn_ELx_PMG_D_SHIFT + 7,
                                                                MPAMn_ELx_PMG_D_SHIFT);

    /* Write MINMAX_PARTID & DEFAULT PMG to mpam2_el2 to generate PE traffic */
    mpam2_el2 |= (((uint64_t)DEFAULT_PMG << MPAMn_ELx_PMG_D_SHIFT) |
                  ((uint64_t)minmax_partid << MPAMn_ELx_PARTID_D_SHIFT));

    val_mpam_reg_write(MPAM2_EL2, mpam2_el2);

    /* Set the memory of coutner arr to 0 */
    val_memory_set(counter, sizeof(counter), 0);

    /* Iterate through various partition settings and gather MBWU monitor count for each */
    for (index = 0; index < sizeof(mbwpbm_config_data)/sizeof(mbwpbm_config_t); index++) {

        val_print(ACS_PRINT_DEBUG,
            "\n       Programming MSC with %d percent of MBW Portion partitioning",
                                    mbwpbm_config_data[index].partition_percent);

        for (msc_index = 0; msc_index < total_nodes; msc_index++) {

            rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);
            for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

                if (val_mpam_get_info(MPAM_MSC_RSRC_TYPE, msc_index, rsrc_index)
                                                                        != MPAM_RSRC_TYPE_MEMORY) {
                    val_print(ACS_PRINT_WARN, "\n       MSC %d not a memory node. Skipping MSC",
                                msc_index);
                    continue;
                }

                val_print(ACS_PRINT_DEBUG, "\n       MSC Index: %d", msc_index);
                val_print(ACS_PRINT_DEBUG, "  RIS Index: %d", rsrc_index);

                if (val_mpam_msc_supports_ris(msc_index))
                    val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

                if (!val_mpam_msc_supports_mbwpbm(msc_index))
                    continue;

                /* Configure MBWPBM partition properties for this MSC */
                val_mpam_configure_mbwpbm(msc_index, minmax_partid,
                                          mbwpbm_config_data[index].partition_percent);

                /* Disable MBWMIN partitioning for the current memory msc_index */
                if (val_mpam_msc_supports_mbw_min(msc_index)) {
                    val_mpam_msc_configure_mbwmin(msc_index, minmax_partid, 0);
                }

                /* Disable MBWMAX partitioning for the current memory msc_index */
                if (val_mpam_msc_supports_mbw_max(msc_index)) {
                    val_mpam_msc_configure_mbwmax(msc_index, minmax_partid, 0, 100);
                }

                /* Allocate source and destination memory buffers*/
                addr_base = val_mpam_memory_get_base(msc_index, rsrc_index);
                addr_len  = val_mpam_memory_get_size(msc_index, rsrc_index);

                val_print(ACS_PRINT_DEBUG, "\n       addr_base is %llx", addr_base);
                val_print(ACS_PRINT_DEBUG, "\n       addr_len is %llx", addr_len);

                buf_size  = get_buffer_size(msc_index, rsrc_index, addr_len, index);
                if (buf_size == ACS_STATUS_SKIP) {
                    val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 02));
                    return;
                }

                if ((addr_base == SRAT_INVALID_INFO) || (addr_len == SRAT_INVALID_INFO) ||
                    (addr_len <= 2 * buf_size)) { /* src and dst buffer size */
                    val_print(ACS_PRINT_ERR, "\n       No SRAT mem range info found", 0);
                    val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));

                    /* Restore MPAM2_EL2 settings */
                    val_mpam_reg_write(MPAM2_EL2, mpam2_el2_temp);
                    return;
                }

                src_buf = (void *)val_mem_alloc_at_address(addr_base, buf_size);
                dest_buf = (void *)val_mem_alloc_at_address(addr_base + buf_size, buf_size);

                if ((src_buf == NULL) || (dest_buf == NULL)) {
                    val_print(ACS_PRINT_ERR, "\n       Memory allocation of buffers failed", 0);
                    val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));

                    /* Restore MPAM2_EL2 settings */
                    val_mpam_reg_write(MPAM2_EL2, mpam2_el2_temp);
                    return;
                }

                if (!val_mpam_get_mbwumon_count(msc_index)) {
                    val_print(ACS_PRINT_TEST,
                          "\n       No MBWU Monitor found to validate the test. Skipping test", 0);
                          val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 03));
                          return;
                }

                val_print(ACS_PRINT_DEBUG,
                          "\n       Using MBWU monitor to measure MBW count during buffer copy", 0);
                /* configure MBWU Monitor for this memory resource node */
                val_mpam_memory_configure_mbwumon(msc_index);

                /* enable and reset MBWU monitoring */
                val_mpam_memory_mbwumon_enable(msc_index);
                val_mpam_memory_mbwumon_reset(msc_index);

                /* wait for MAX_NRDY_USEC after msc config change */
                nrdy_timeout = val_mpam_get_info(MPAM_MSC_NRDY, msc_index, 0);
                while (nrdy_timeout) {
                    --nrdy_timeout;
                };

                start_count = val_mpam_memory_mbwumon_read_count(msc_index);
                val_print(ACS_PRINT_TEST, "\n        Start count is %llx", start_count);

                /* perform memory operation */
                val_memcpy(src_buf, dest_buf, buf_size);

                while (nrdy_timeout) {
                    --nrdy_timeout;
                };

                end_count = val_mpam_memory_mbwumon_read_count(msc_index);
                val_print(ACS_PRINT_TEST, "\n        End count is %llx", end_count);

                /* read the memory bandwidth usage monitor */
                counter[enabled_scenarios++][msc_index][rsrc_index] =
                                                    end_count - start_count;

                /* disable and reset the MBWU monitor */
                val_mpam_memory_mbwumon_disable(msc_index);
                val_mpam_memory_mbwumon_reset(msc_index);

                val_print(ACS_PRINT_TEST,
                    "\n       byte_count = 0x%llx bytes", end_count - start_count);

                /* Free the buffers to the heap manager */
                val_mem_free_at_address((uint64_t)src_buf, buf_size);
                val_mem_free_at_address((uint64_t)dest_buf, buf_size);
            }
        }
    }

    /* Restore MPAM2_EL2 settings */
    val_mpam_reg_write(MPAM2_EL2, mpam2_el2_temp);

    /* Compare the stream copy latencies for all the scenarios */
    for (index = 1; index < enabled_scenarios; index++) {

        for (msc_index = 0; msc_index < total_nodes; msc_index++) {
            rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);

            for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

                if (val_mpam_msc_supports_ris(msc_index))
                    val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

                if (val_mpam_msc_supports_mbwpbm(msc_index))  {
                    if (counter[index][msc_index][rsrc_index]
                                                    > counter[index-1][msc_index][rsrc_index]) {
                        val_print(ACS_PRINT_ERR, "\n       Failed for msc_index : %d", msc_index);
                        val_print(ACS_PRINT_ERR, "\n       cfg_index : %d", index);
                        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
                        return;
                    }
                }
            }
        }
    }

    val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));

    return;
}

uint32_t mem001_entry(void)
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
