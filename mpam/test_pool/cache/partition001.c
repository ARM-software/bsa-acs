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


#define TEST_NUM   ACS_MPAM_CACHE_TEST_NUM_BASE + 1
#define TEST_RULE  ""
#define TEST_DESC  "Check CPOR Partitioning               "

#define CPOR_SCENARIO_MAX 10
static uint64_t mpam2_el2_temp;

typedef struct {
    char8_t description[64];
    uint8_t partition_percent;  //as function of max cpor cache size
    uint8_t cache_percent;      //as function of max cpor cache size
    uint8_t config_enable;
} cpor_config_t;

static cpor_config_t cpor_config_data[] = {
    {"CPOR check for 75% partition size", 75, 75, TRUE},
    {"CPOR check for 25% partition size", 25, 75, TRUE}
};

static void payload(void)
{
    uint8_t enabled_scenarios = 0;
    uint32_t cpor_nodes = 0;
    uint32_t cfg_index = 0;
    uint32_t start_count = 0;
    uint32_t end_count = 0;
    void *src_buf = 0;
    void *dest_buf = 0;
    uint64_t buf_size;
    uint64_t nrdy_timeout;
    uint64_t **counter;
    uint64_t it;
    uint64_t mpam2_el2 = 0;
    uint32_t msc_node_cnt = val_mpam_get_msc_count();
    uint32_t rsrc_node_cnt;
    uint32_t msc_index;
    uint32_t rsrc_index;
    uint32_t llc_index;
    uint64_t cache_identifier;
    uint32_t cache_maxsize;
    uint32_t test_partid = 1; //Selecting distinct partid
    uint32_t csumon_count = 0;
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
    uint32_t test_fail = 0;

   /* Get the Index for LLC */
    llc_index = val_cache_get_llc_index();
    if (llc_index == CACHE_TABLE_EMPTY) {
      val_print(ACS_PRINT_ERR, "\n       Cache info table empty", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 01));
      return;
    }

    /* Get the cache identifier for LLC */
    cache_identifier = val_cache_get_info(CACHE_ID, llc_index);
    if (cache_identifier == INVALID_CACHE_INFO) {
      val_print(ACS_PRINT_ERR, "\n       LLC invalid in PPTT", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 02));
      return;
    }

    /* Get MPAM related information for LLC */
    for (msc_index = 0; msc_index < msc_node_cnt; msc_index++) {

      rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);
      for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

        if (val_mpam_get_info(MPAM_MSC_RSRC_TYPE, msc_index, rsrc_index) != MPAM_RSRC_TYPE_PE_CACHE)
          continue;

        if (val_mpam_get_info(MPAM_MSC_RSRC_DESC1, msc_index, rsrc_index) != cache_identifier)
          continue;

        if (val_mpam_supports_cpor(msc_index)) {
          cache_maxsize = GET_MAX_VALUE(cache_maxsize,
                              val_cache_get_info(CACHE_SIZE, llc_index));

          if (val_mpam_supports_csumon(msc_index))
              csumon_count = val_mpam_get_csumon_count(msc_index);

          cpor_nodes++;
        }
        test_partid = GET_MIN_VALUE(test_partid, val_mpam_get_max_partid(msc_index));
      }
    }

    val_print(ACS_PRINT_DEBUG, "\n       CPOR Nodes = %d", cpor_nodes);
    val_print(ACS_PRINT_DEBUG, "\n       Test PARTID = %d", test_partid);
    val_print(ACS_PRINT_DEBUG, "\n       Cache Max Size = 0x%x", cache_maxsize);
    val_print(ACS_PRINT_DEBUG, "\n       Number of CSU Monitors = %d", csumon_count);

    /* Skip the test if CSU monitors/ nodes supporting Cache Portion Partitoning are 0 */
    if ((cpor_nodes == 0) || (csumon_count == 0)) {
        val_set_status(index, RESULT_SKIP(TEST_NUM, 03));
        return;
    }

    /* Dynamically create the CSU Mon Counter buffer
     * uint64_t counter[CPOR_SCENARIO_MAX][cpor_nodes]
     */
    counter = (uint64_t **) val_memory_alloc(CPOR_SCENARIO_MAX * sizeof (uint64_t *));
    for (it = 0; it < CPOR_SCENARIO_MAX; it++) {
        counter[it] = val_memory_alloc(msc_node_cnt * sizeof (uint64_t));
    }

    mpam2_el2 = val_mpam_reg_read(MPAM2_EL2);
    mpam2_el2_temp = mpam2_el2;

    for (cfg_index = 0; cfg_index < sizeof(cpor_config_data)/sizeof(cpor_config_t); cfg_index++) {

      test_partid = test_partid + cfg_index;
      /* Clear the PARTID_D & PMG_D bits in mpam2_el2 before writing to them */
      mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2,
                                    MPAMn_ELx_PARTID_D_SHIFT+15, MPAMn_ELx_PARTID_D_SHIFT);
      mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2,
                                    MPAMn_ELx_PMG_D_SHIFT+7, MPAMn_ELx_PMG_D_SHIFT);

      /* Write test_partid & DEFAULT PMG to mpam2_el2 to generate PE traffic */
      mpam2_el2 |= (((uint64_t)DEFAULT_PMG << MPAMn_ELx_PMG_D_SHIFT) |
                    ((uint64_t)test_partid << MPAMn_ELx_PARTID_D_SHIFT));

      val_mpam_reg_write(MPAM2_EL2, mpam2_el2);

      if (cpor_config_data[cfg_index].config_enable) {
        /* Configure CPOR settings for nodes supporting CPOR */
        for (msc_index = 0; msc_index < msc_node_cnt; msc_index++) {
          val_print(ACS_PRINT_DEBUG, "\n       Running for cfg_index = %d", cfg_index);
          val_print(ACS_PRINT_DEBUG, ", msc_index = %d", msc_index);

          rsrc_node_cnt = val_mpam_get_info(MPAM_MSC_RSRC_COUNT, msc_index, 0);
          for (rsrc_index = 0; rsrc_index < rsrc_node_cnt; rsrc_index++) {

            if (val_mpam_get_info(MPAM_MSC_RSRC_TYPE, msc_index, rsrc_index) !=
                                                                MPAM_RSRC_TYPE_PE_CACHE)
              continue;

            if (val_mpam_get_info(MPAM_MSC_RSRC_DESC1, msc_index, rsrc_index) != cache_identifier)
              continue;

            start_count = 0;
            end_count = 0;
            /* Select resource instance if RIS feature implemented */
            if (val_mpam_msc_supports_ris(msc_index))
              val_mpam_memory_configure_ris_sel(msc_index, rsrc_index);

            if (val_mpam_supports_cpor(msc_index)) {
              val_mpam_configure_cpor(msc_index, test_partid,
                            cpor_config_data[cfg_index].partition_percent);
            }

            if (val_mpam_supports_ccap(msc_index))
              val_mpam_configure_ccap(msc_index, test_partid, 0, 100);

            buf_size = cache_maxsize * cpor_config_data[cfg_index].cache_percent / 100 / 2;

            /*Allocate memory for source and destination buffers */
            src_buf = (void *)val_aligned_alloc(MEM_ALIGN_4K, buf_size);
            dest_buf = (void *)val_aligned_alloc(MEM_ALIGN_4K, buf_size);

            if ((src_buf == NULL) || (dest_buf == NULL)) {
                val_print(ACS_PRINT_ERR, "\n       Mem allocation failed", 0);
                val_set_status(index, RESULT_FAIL(TEST_NUM, 01));
            }

            /* Configure CSU Monitor */
            val_mpam_configure_csu_mon(msc_index, test_partid, DEFAULT_PMG, 0);

            /* Enable monitoring */
            val_mpam_csumon_enable(msc_index);

            /* wait for MAX_NRDY_USEC after msc config change */
            nrdy_timeout = val_mpam_get_info(MPAM_MSC_NRDY, msc_index, 0);
            while (nrdy_timeout) {
                --nrdy_timeout;
            };

            start_count = val_mpam_read_csumon(msc_index);
            val_print(ACS_PRINT_DEBUG, "\n       Start Count = 0x%lx", start_count);

            /* Start mem copy */
            val_memcpy(src_buf, dest_buf, buf_size);

            end_count = val_mpam_read_csumon(msc_index);
            val_print(ACS_PRINT_DEBUG, "\n       End Count = 0x%lx", end_count);

            /* Disable CSU MON */
            val_mpam_csumon_disable(msc_index);

            /* Read CSU MON */
            counter[enabled_scenarios++][msc_index] = end_count - start_count;

            val_print(ACS_PRINT_DEBUG, "\n       Count Difference = 0x%lx",
                                      end_count - start_count);

            /* Free the buffers to the heap manager */
            val_mem_free_at_address((uint64_t)src_buf, buf_size);
            val_mem_free_at_address((uint64_t)dest_buf, buf_size);
          }
        }
      }
    }

    /* Restore MPAM2_EL2 settings */
    val_mpam_reg_write(MPAM2_EL2, mpam2_el2_temp);

    /* Compare the stream copy latencies for all the scenarios */
    for (msc_index = 0; msc_index < msc_node_cnt; msc_index++) {
      for (cfg_index = 1; cfg_index < enabled_scenarios; cfg_index++) {
        if (counter[cfg_index][msc_index] > counter[cfg_index-1][msc_index]) {
          val_print(ACS_PRINT_ERR, "\n       Failed for msc_index : %d", msc_index);
          val_print(ACS_PRINT_ERR, "\n       cfg_index : %d", cfg_index-1);
          val_print(ACS_PRINT_ERR, ", CSU Mon Count :0x%lx", counter[cfg_index-1][msc_index]);
          val_print(ACS_PRINT_ERR, "\n       cfg_index : %d", cfg_index);
          val_print(ACS_PRINT_ERR, ", CSU Mon Count :0x%lx", counter[cfg_index][msc_index]);
          test_fail++;
        }
      }
    }

    if (test_fail)
      val_set_status(index, RESULT_FAIL(TEST_NUM, 02));
    else
      val_set_status(index, RESULT_PASS(TEST_NUM, 01));

    return;
}

uint32_t partition001_entry(void)
{
    uint32_t status = ACS_STATUS_FAIL;
    uint32_t num_pe = 1;

    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

    /* This check is when user is forcing us to skip this test */
    if (status != ACS_STATUS_SKIP)
        val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);
    val_report_status(0, ACS_END(TEST_NUM), TEST_RULE);

    return status;
}
