/** @file
 * Copyright (c) 2020-2023, Arm Limited or its affiliates. All rights reserved.
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

#include "pal_common_support.h"
#include "platform_override_struct.h"

/*
    To run a specific modules:
      - Give the module base numbers in the g_module_array.
      - All module base numbers can be found in val/include/bsa_acs_common.h
      - Example - if g_module_array = {0}, only PE tests will be run while skipping other modules

    To run a specific tests:
      - Give the test numbers in the g_test_array.
      - Test numbers can be found in test_pool/<module>/test.c.
      - For example, if g_test_array = {801}, only first test in PCIe will be run.
        All other tests in all other modules will be skipped.
      - If g_single_module is also given, then single test + tests under single module will be run.

    To skip tests/modules:
      - Give test numbers or module test bases as the entries of g_skip_array.
      - This will only skip the tests or modules given in the array and runs all other tests.

    Tests run = g_test_array + g_module_array - Tests under skip array
*/
uint32_t  g_skip_array[]   = {10000, 10000, 10000, 10000};
uint32_t  g_test_array[]   = {};
uint32_t  g_module_array[] = {};

uint32_t  g_num_skip         = sizeof(g_skip_array)/sizeof(g_skip_array[0]);
uint32_t  g_num_tests        = sizeof(g_test_array)/sizeof(g_test_array[0]);
uint32_t  g_num_modules      = sizeof(g_module_array)/sizeof(g_module_array[0]);

/* VE systems run acs at EL1 and in some systems crash is observed during acess
   of EL1 phy and virt timer, Below command line option is added only for debug
   purpose to complete BSA run on these systems */
uint32_t  g_el1physkip       = FALSE;

PE_INFO_TABLE platform_pe_cfg = {

    .header.num_of_pe = PLATFORM_OVERRIDE_PE_CNT,

    .pe_info[0].pe_num      = PLATFORM_OVERRIDE_PE0_INDEX,
    .pe_info[0].mpidr       = PLATFORM_OVERRIDE_PE0_MPIDR,
    .pe_info[0].pmu_gsiv    = PLATFORM_OVERRIDE_PE0_PMU_GSIV,
    .pe_info[0].gmain_gsiv  = PLATFORM_OVERRIDE_PE0_GMAIN_GSIV,

    .pe_info[1].pe_num      = PLATFORM_OVERRIDE_PE1_INDEX,
    .pe_info[1].mpidr       = PLATFORM_OVERRIDE_PE1_MPIDR,
    .pe_info[1].pmu_gsiv    = PLATFORM_OVERRIDE_PE1_PMU_GSIV,
    .pe_info[1].gmain_gsiv  = PLATFORM_OVERRIDE_PE1_GMAIN_GSIV,

    .pe_info[2].pe_num      = PLATFORM_OVERRIDE_PE2_INDEX,
    .pe_info[2].mpidr       = PLATFORM_OVERRIDE_PE2_MPIDR,
    .pe_info[2].pmu_gsiv    = PLATFORM_OVERRIDE_PE2_PMU_GSIV,
    .pe_info[2].gmain_gsiv  = PLATFORM_OVERRIDE_PE2_GMAIN_GSIV,

    .pe_info[3].pe_num      = PLATFORM_OVERRIDE_PE3_INDEX,
    .pe_info[3].mpidr       = PLATFORM_OVERRIDE_PE3_MPIDR,
    .pe_info[3].pmu_gsiv    = PLATFORM_OVERRIDE_PE3_PMU_GSIV,
    .pe_info[3].gmain_gsiv  = PLATFORM_OVERRIDE_PE3_GMAIN_GSIV,

    .pe_info[4].pe_num      = PLATFORM_OVERRIDE_PE4_INDEX,
    .pe_info[4].mpidr       = PLATFORM_OVERRIDE_PE4_MPIDR,
    .pe_info[4].pmu_gsiv    = PLATFORM_OVERRIDE_PE4_PMU_GSIV,
    .pe_info[4].gmain_gsiv  = PLATFORM_OVERRIDE_PE4_GMAIN_GSIV,

    .pe_info[5].pe_num      = PLATFORM_OVERRIDE_PE5_INDEX,
    .pe_info[5].mpidr       = PLATFORM_OVERRIDE_PE5_MPIDR,
    .pe_info[5].pmu_gsiv    = PLATFORM_OVERRIDE_PE5_PMU_GSIV,
    .pe_info[5].gmain_gsiv  = PLATFORM_OVERRIDE_PE5_GMAIN_GSIV,

    .pe_info[6].pe_num      = PLATFORM_OVERRIDE_PE6_INDEX,
    .pe_info[6].mpidr       = PLATFORM_OVERRIDE_PE6_MPIDR,
    .pe_info[6].pmu_gsiv    = PLATFORM_OVERRIDE_PE6_PMU_GSIV,
    .pe_info[6].gmain_gsiv  = PLATFORM_OVERRIDE_PE6_GMAIN_GSIV,

    .pe_info[7].pe_num      = PLATFORM_OVERRIDE_PE7_INDEX,
    .pe_info[7].mpidr       = PLATFORM_OVERRIDE_PE7_MPIDR,
    .pe_info[7].pmu_gsiv    = PLATFORM_OVERRIDE_PE7_PMU_GSIV,
    .pe_info[7].gmain_gsiv  = PLATFORM_OVERRIDE_PE7_GMAIN_GSIV,

    .pe_info[8].pe_num      = PLATFORM_OVERRIDE_PE8_INDEX,
    .pe_info[8].mpidr       = PLATFORM_OVERRIDE_PE8_MPIDR,
    .pe_info[8].pmu_gsiv    = PLATFORM_OVERRIDE_PE8_PMU_GSIV,
    .pe_info[8].gmain_gsiv  = PLATFORM_OVERRIDE_PE8_GMAIN_GSIV,

    .pe_info[9].pe_num      = PLATFORM_OVERRIDE_PE9_INDEX,
    .pe_info[9].mpidr       = PLATFORM_OVERRIDE_PE9_MPIDR,
    .pe_info[9].pmu_gsiv    = PLATFORM_OVERRIDE_PE9_PMU_GSIV,
    .pe_info[9].gmain_gsiv  = PLATFORM_OVERRIDE_PE9_GMAIN_GSIV,

    .pe_info[10].pe_num     = PLATFORM_OVERRIDE_PE10_INDEX,
    .pe_info[10].mpidr      = PLATFORM_OVERRIDE_PE10_MPIDR,
    .pe_info[10].pmu_gsiv   = PLATFORM_OVERRIDE_PE10_PMU_GSIV,
    .pe_info[10].gmain_gsiv = PLATFORM_OVERRIDE_PE10_GMAIN_GSIV,

    .pe_info[11].pe_num     = PLATFORM_OVERRIDE_PE11_INDEX,
    .pe_info[11].mpidr      = PLATFORM_OVERRIDE_PE11_MPIDR,
    .pe_info[11].pmu_gsiv   = PLATFORM_OVERRIDE_PE11_PMU_GSIV,
    .pe_info[11].gmain_gsiv = PLATFORM_OVERRIDE_PE11_GMAIN_GSIV,

    .pe_info[12].pe_num     = PLATFORM_OVERRIDE_PE12_INDEX,
    .pe_info[12].mpidr      = PLATFORM_OVERRIDE_PE12_MPIDR,
    .pe_info[12].pmu_gsiv   = PLATFORM_OVERRIDE_PE12_PMU_GSIV,
    .pe_info[12].gmain_gsiv = PLATFORM_OVERRIDE_PE12_GMAIN_GSIV,

    .pe_info[13].pe_num     = PLATFORM_OVERRIDE_PE13_INDEX,
    .pe_info[13].mpidr      = PLATFORM_OVERRIDE_PE13_MPIDR,
    .pe_info[13].pmu_gsiv   = PLATFORM_OVERRIDE_PE13_PMU_GSIV,
    .pe_info[13].gmain_gsiv = PLATFORM_OVERRIDE_PE13_GMAIN_GSIV,

    .pe_info[14].pe_num     = PLATFORM_OVERRIDE_PE14_INDEX,
    .pe_info[14].mpidr      = PLATFORM_OVERRIDE_PE14_MPIDR,
    .pe_info[14].pmu_gsiv   = PLATFORM_OVERRIDE_PE14_PMU_GSIV,
    .pe_info[14].gmain_gsiv = PLATFORM_OVERRIDE_PE14_GMAIN_GSIV,

    .pe_info[15].pe_num     = PLATFORM_OVERRIDE_PE15_INDEX,
    .pe_info[15].mpidr      = PLATFORM_OVERRIDE_PE15_MPIDR,
    .pe_info[15].pmu_gsiv   = PLATFORM_OVERRIDE_PE15_PMU_GSIV,
    .pe_info[15].gmain_gsiv = PLATFORM_OVERRIDE_PE15_GMAIN_GSIV

};


PLATFORM_OVERRIDE_GIC_INFO_TABLE platform_gic_cfg = {

    .gic_version   = PLATFORM_OVERRIDE_GIC_VERSION,
    .num_gicc      = PLATFORM_OVERRIDE_GICC_COUNT,
    .num_gicd      = PLATFORM_OVERRIDE_GICD_COUNT,
    .num_gicc_rd   = PLATFORM_OVERRIDE_GICC_GICRD_COUNT,
    .num_gicr_rd   = PLATFORM_OVERRIDE_GICR_GICRD_COUNT,
    .num_gicits    = PLATFORM_OVERRIDE_GICITS_COUNT,
    .num_gich      = PLATFORM_OVERRIDE_GICH_COUNT,
    .num_msiframes = PLATFORM_OVERRIDE_GICMSIFRAME_COUNT,

    .gicc_rd_length = PLATFORM_OVERRIDE_GICCIRD_LENGTH,
    .gicr_rd_length = PLATFORM_OVERRIDE_GICRIRD_LENGTH,

    .gicc_base[0]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[1]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[2]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[3]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[4]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[5]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[6]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[7]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[8]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[9]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[10]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[11]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[12]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[13]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[14]   = PLATFORM_OVERRIDE_GICC_BASE,
    .gicc_base[15]   = PLATFORM_OVERRIDE_GICC_BASE,

    .gicd_base[0]   = PLATFORM_OVERRIDE_GICD_BASE,
    .gicr_rd_base[0]= PLATFORM_OVERRIDE_GICR_GICRD_BASE,
    .gicits_base[0] = PLATFORM_OVERRIDE_GICITS0_BASE,
    .gicits_id[0]   = PLATFORM_OVERRIDE_GICITS0_ID,
    .gicits_base[1] = PLATFORM_OVERRIDE_GICITS1_BASE,
    .gicits_id[1]   = PLATFORM_OVERRIDE_GICITS1_ID,
    .gicits_base[2] = PLATFORM_OVERRIDE_GICITS2_BASE,
    .gicits_id[2]   = PLATFORM_OVERRIDE_GICITS2_ID,
    .gicits_base[3] = PLATFORM_OVERRIDE_GICITS3_BASE,
    .gicits_id[3]   = PLATFORM_OVERRIDE_GICITS3_ID,
    .gicits_base[4] = PLATFORM_OVERRIDE_GICITS4_BASE,
    .gicits_id[4]   = PLATFORM_OVERRIDE_GICITS4_ID,
    .gicits_base[5] = PLATFORM_OVERRIDE_GICITS5_BASE,
    .gicits_id[5]   = PLATFORM_OVERRIDE_GICITS5_ID,
    .gich_base[0]   = PLATFORM_OVERRIDE_GICH_BASE

};

PLATFORM_OVERRIDE_TIMER_INFO_TABLE platform_timer_cfg = {

    .header.s_el1_timer_flags   = PLATFORM_OVERRIDE_S_EL1_TIMER_FLAGS,
    .header.ns_el1_timer_flags  = PLATFORM_OVERRIDE_NS_EL1_TIMER_FLAGS,
    .header.el2_timer_flags     = PLATFORM_OVERRIDE_NS_EL2_TIMER_FLAGS,
    .header.s_el1_timer_gsiv    = PLATFORM_OVERRIDE_S_EL1_TIMER_GSIV,
    .header.ns_el1_timer_gsiv   = PLATFORM_OVERRIDE_NS_EL1_TIMER_GSIV,
    .header.el2_timer_gsiv      = PLATFORM_OVERRIDE_NS_EL2_TIMER_GSIV,
    .header.virtual_timer_flags = PLATFORM_OVERRIDE_VIRTUAL_TIMER_FLAGS,
    .header.virtual_timer_gsiv  = PLATFORM_OVERRIDE_VIRTUAL_TIMER_GSIV,
    .header.el2_virt_timer_gsiv = PLATFORM_OVERRIDE_EL2_VIR_TIMER_GSIV,
    .header.num_platform_timer  = PLATFORM_OVERRIDE_PLATFORM_TIMER_COUNT,

    .gt_info.type               = PLATFORM_OVERRIDE_TIMER_TYPE,
    .gt_info.timer_count        = PLATFORM_OVERRIDE_TIMER_COUNT,
    .gt_info.block_cntl_base    = PLATFORM_OVERRIDE_TIMER_CNTCTL_BASE,
    .gt_info.frame_num[0]       = PLATFORM_OVERRIDE_TIMER_FRAME_NUM_0,
    .gt_info.frame_num[1]       = PLATFORM_OVERRIDE_TIMER_FRAME_NUM_1,
    .gt_info.GtCntBase[0]       = PLATFORM_OVERRIDE_TIMER_CNTBASE_0,
    .gt_info.GtCntBase[1]       = PLATFORM_OVERRIDE_TIMER_CNTBASE_1,
    .gt_info.GtCntEl0Base[0]    = PLATFORM_OVERRIDE_TIMER_CNTEL0BASE_0,
    .gt_info.GtCntEl0Base[1]    = PLATFORM_OVERRIDE_TIMER_CNTEL0BASE_1,
    .gt_info.gsiv[0]            = PLATFORM_OVERRIDE_TIMER_GSIV_0,
    .gt_info.gsiv[1]            = PLATFORM_OVERRIDE_TIMER_GSIV_1,
    .gt_info.virt_gsiv[0]       = PLATFORM_OVERRIDE_TIMER_VIRT_GSIV_0,
    .gt_info.virt_gsiv[1]       = PLATFORM_OVERRIDE_TIMER_VIRT_GSIV_1,
    .gt_info.flags[0]           = PLATFORM_OVERRIDE_TIMER_FLAGS_0,
    .gt_info.flags[1]           = PLATFORM_OVERRIDE_TIMER_FLAGS_1

};

WD_INFO_TABLE platform_wd_cfg = {
    .header.num_wd              = PLATFORM_OVERRIDE_WD_TIMER_COUNT,
    .wd_info[0].wd_ctrl_base    = PLATFORM_OVERRIDE_WD_CTRL_BASE,
    .wd_info[0].wd_refresh_base = PLATFORM_OVERRIDE_WD_REFRESH_BASE,
    .wd_info[0].wd_gsiv         = PLATFORM_OVERRIDE_WD_GSIV_0,
    .wd_info[0].wd_flags        = PLATFORM_OVERRIDE_WD_FLAGS_0,
    .wd_info[1].wd_ctrl_base    = PLATFORM_OVERRIDE_WD_CTRL_BASE,
    .wd_info[1].wd_refresh_base = PLATFORM_OVERRIDE_WD_REFRESH_BASE,
    .wd_info[1].wd_gsiv         = PLATFORM_OVERRIDE_WD_GSIV_1,
    .wd_info[1].wd_flags        = PLATFORM_OVERRIDE_WD_FLAGS_1

};

PCIE_INFO_TABLE platform_pcie_cfg = {
    .num_entries             = PLATFORM_OVERRIDE_NUM_ECAM,
    .block[0].ecam_base      = PLATFORM_OVERRIDE_PCIE_ECAM_BASE_ADDR_0,
    .block[0].segment_num    = PLATFORM_OVERRIDE_PCIE_SEGMENT_GRP_NUM_0,
    .block[0].start_bus_num  = PLATFORM_OVERRIDE_PCIE_START_BUS_NUM_0,
    .block[0].end_bus_num    = PLATFORM_OVERRIDE_PCIE_END_BUS_NUM_0,

/** Configure more PCIe info details as per specification for more than 1 ECAM
    Refer to platform_override_fvp.h file for an example
    .block[1].ecam_base      = PLATFORM_OVERRIDE_PCIE_ECAM_BASE_ADDR_1,
    .block[1].segment_num    = PLATFORM_OVERRIDE_PCIE_SEGMENT_GRP_NUM_1,
    .block[1].start_bus_num  = PLATFORM_OVERRIDE_PCIE_START_BUS_NUM_1,
    .block[1].end_bus_num    = PLATFORM_OVERRIDE_PCIE_END_BUS_NUM_1

**/
};

PLATFORM_OVERRIDE_IOVIRT_INFO_TABLE platform_iovirt_cfg = {
    .Address              = IOVIRT_ADDRESS,
    .node_count           = IORT_NODE_COUNT,
    .type[0]              = IOVIRT_NODE_ITS_GROUP,
    .type[1]              = IOVIRT_NODE_ITS_GROUP,
    .type[2]              = IOVIRT_NODE_ITS_GROUP,
    .type[3]              = IOVIRT_NODE_ITS_GROUP,
    .type[4]              = IOVIRT_NODE_ITS_GROUP,
    .type[5]              = IOVIRT_NODE_SMMU_V3,
    .type[6]              = IOVIRT_NODE_SMMU_V3,
    .type[7]              = IOVIRT_NODE_SMMU_V3,
    .type[8]              = IOVIRT_NODE_SMMU_V3,
    .type[9]              = IOVIRT_NODE_SMMU_V3,
    .type[10]             = IOVIRT_NODE_PCI_ROOT_COMPLEX,
    .type[11]             = IOVIRT_NODE_NAMED_COMPONENT,
    .type[12]             = IOVIRT_NODE_NAMED_COMPONENT,
    .num_map[5]           = IOVIRT_SMMUV3_0_NUM_MAP,
    .num_map[6]           = IOVIRT_SMMUV3_1_NUM_MAP,
    .num_map[7]           = IOVIRT_SMMUV3_2_NUM_MAP,
    .num_map[8]           = IOVIRT_SMMUV3_3_NUM_MAP,
    .num_map[9]           = IOVIRT_SMMUV3_4_NUM_MAP,
    .num_map[10]          = IOVIRT_RC_NUM_MAP,
    .num_map[11]          = IOVIRT_NAMED_COMP0_NUM_MAP,
    .num_map[12]          = IOVIRT_NAMED_COMP1_NUM_MAP,


    .map[5].input_base[0] = SMMUV3_0_ID_MAP0_INPUT_BASE,
    .map[5].id_count[0]   = SMMUV3_0_ID_MAP0_ID_COUNT,
    .map[5].output_base[0]= SMMUV3_0_ID_MAP0_OUTPUT_BASE,
    .map[5].output_ref[0] = SMMUV3_0_ID_MAP0_OUTPUT_REF,
    .map[5].input_base[1] = SMMUV3_0_ID_MAP1_INPUT_BASE,
    .map[5].id_count[1]   = SMMUV3_0_ID_MAP1_ID_COUNT,
    .map[5].output_base[1]= SMMUV3_0_ID_MAP1_OUTPUT_BASE,
    .map[5].output_ref[1] = SMMUV3_0_ID_MAP1_OUTPUT_REF,

    .map[6].input_base[0] = SMMUV3_1_ID_MAP0_INPUT_BASE,
    .map[6].id_count[0]   = SMMUV3_1_ID_MAP0_ID_COUNT,
    .map[6].output_base[0]= SMMUV3_1_ID_MAP0_OUTPUT_BASE,
    .map[6].output_ref[0] = SMMUV3_1_ID_MAP0_OUTPUT_REF,
    .map[6].input_base[1] = SMMUV3_1_ID_MAP1_INPUT_BASE,
    .map[6].id_count[1]   = SMMUV3_1_ID_MAP1_ID_COUNT,
    .map[6].output_base[1]= SMMUV3_1_ID_MAP1_OUTPUT_BASE,
    .map[6].output_ref[1] = SMMUV3_1_ID_MAP1_OUTPUT_REF,


    .map[7].input_base[0] = SMMUV3_2_ID_MAP0_INPUT_BASE,
    .map[7].id_count[0]   = SMMUV3_2_ID_MAP0_ID_COUNT,
    .map[7].output_base[0]= SMMUV3_2_ID_MAP0_OUTPUT_BASE,
    .map[7].output_ref[0] = SMMUV3_2_ID_MAP0_OUTPUT_REF,
    .map[7].input_base[1] = SMMUV3_2_ID_MAP1_INPUT_BASE,
    .map[7].id_count[1]   = SMMUV3_2_ID_MAP1_ID_COUNT,
    .map[7].output_base[1]= SMMUV3_2_ID_MAP1_OUTPUT_BASE,
    .map[7].output_ref[1] = SMMUV3_2_ID_MAP1_OUTPUT_REF,

    .map[8].input_base[0] = SMMUV3_3_ID_MAP0_INPUT_BASE,
    .map[8].id_count[0]   = SMMUV3_3_ID_MAP0_ID_COUNT,
    .map[8].output_base[0]= SMMUV3_3_ID_MAP0_OUTPUT_BASE,
    .map[8].output_ref[0] = SMMUV3_3_ID_MAP0_OUTPUT_REF,
    .map[8].input_base[1] = SMMUV3_3_ID_MAP1_INPUT_BASE,
    .map[8].id_count[1]   = SMMUV3_3_ID_MAP1_ID_COUNT,
    .map[8].output_base[1]= SMMUV3_3_ID_MAP1_OUTPUT_BASE,
    .map[8].output_ref[1] = SMMUV3_3_ID_MAP1_OUTPUT_REF,

    .map[9].input_base[0] = SMMUV3_4_ID_MAP0_INPUT_BASE,
    .map[9].id_count[0]   = SMMUV3_4_ID_MAP0_ID_COUNT,
    .map[9].output_base[0]= SMMUV3_4_ID_MAP0_OUTPUT_BASE,
    .map[9].output_ref[0] = SMMUV3_4_ID_MAP0_OUTPUT_REF,
    .map[9].input_base[1] = SMMUV3_4_ID_MAP1_INPUT_BASE,
    .map[9].id_count[1]   = SMMUV3_4_ID_MAP1_ID_COUNT,
    .map[9].output_base[1]= SMMUV3_4_ID_MAP1_OUTPUT_BASE,
    .map[9].output_ref[1] = SMMUV3_4_ID_MAP1_OUTPUT_REF,
    .map[9].input_base[2] = SMMUV3_4_ID_MAP2_INPUT_BASE,
    .map[9].id_count[2]   = SMMUV3_4_ID_MAP2_ID_COUNT,
    .map[9].output_base[2]= SMMUV3_4_ID_MAP2_OUTPUT_BASE,
    .map[9].output_ref[2] = SMMUV3_4_ID_MAP2_OUTPUT_REF,

    .map[10].input_base[0] = RC_MAP0_INPUT_BASE,
    .map[10].id_count[0]   = RC_MAP0_ID_COUNT,
    .map[10].output_base[0]= RC_MAP0_OUTPUT_BASE,
    .map[10].output_ref[0] = RC_MAP0_OUTPUT_REF,
    .map[10].input_base[1] = RC_MAP1_INPUT_BASE,
    .map[10].id_count[1]   = RC_MAP1_ID_COUNT,
    .map[10].output_base[1]= RC_MAP1_OUTPUT_BASE,
    .map[10].output_ref[1] = RC_MAP1_OUTPUT_REF,
    .map[10].input_base[2] = RC_MAP2_INPUT_BASE,
    .map[10].id_count[2]   = RC_MAP2_ID_COUNT,
    .map[10].output_base[2]= RC_MAP2_OUTPUT_BASE,
    .map[10].output_ref[2] = RC_MAP2_OUTPUT_REF,
    .map[10].input_base[3] = RC_MAP3_INPUT_BASE,
    .map[10].id_count[3]   = RC_MAP3_ID_COUNT,
    .map[10].output_base[3]= RC_MAP3_OUTPUT_BASE,
    .map[10].output_ref[3] = RC_MAP3_OUTPUT_REF,

    .map[11].input_base[0] = NAMED_COMP0_MAP0_INPUT_BASE,
    .map[11].id_count[0]   = NAMED_COMP0_MAP0_ID_COUNT,
    .map[11].output_base[0]= NAMED_COMP0_MAP0_OUTPUT_BASE,
    .map[11].output_ref[0] = NAMED_COMP0_MAP0_OUTPUT_REF,
    .map[11].input_base[1] = NAMED_COMP0_MAP1_INPUT_BASE,
    .map[11].id_count[1]   = NAMED_COMP0_MAP1_ID_COUNT,
    .map[11].output_base[1]= NAMED_COMP0_MAP1_OUTPUT_BASE,
    .map[11].output_ref[1] = NAMED_COMP0_MAP1_OUTPUT_REF,
    .map[11].input_base[2] = NAMED_COMP0_MAP2_INPUT_BASE,
    .map[11].id_count[2]   = NAMED_COMP0_MAP2_ID_COUNT,
    .map[11].output_base[2]= NAMED_COMP0_MAP2_OUTPUT_BASE,
    .map[11].output_ref[2] = NAMED_COMP0_MAP2_OUTPUT_REF,
    .map[11].input_base[3] = NAMED_COMP0_MAP3_INPUT_BASE,
    .map[11].id_count[3]   = NAMED_COMP0_MAP3_ID_COUNT,
    .map[11].output_base[3]= NAMED_COMP0_MAP3_OUTPUT_BASE,
    .map[11].output_ref[3] = NAMED_COMP0_MAP3_OUTPUT_REF,
    .map[11].input_base[4] = NAMED_COMP0_MAP4_INPUT_BASE,
    .map[11].id_count[4]   = NAMED_COMP0_MAP4_ID_COUNT,
    .map[11].output_base[4]= NAMED_COMP0_MAP4_OUTPUT_BASE,
    .map[11].output_ref[4] = NAMED_COMP0_MAP4_OUTPUT_REF,
    .map[11].input_base[5] = NAMED_COMP0_MAP5_INPUT_BASE,
    .map[11].id_count[5]   = NAMED_COMP0_MAP5_ID_COUNT,
    .map[11].output_base[5]= NAMED_COMP0_MAP5_OUTPUT_BASE,
    .map[11].output_ref[5] = NAMED_COMP0_MAP5_OUTPUT_REF,
    .map[11].input_base[6] = NAMED_COMP0_MAP6_INPUT_BASE,
    .map[11].id_count[6]   = NAMED_COMP0_MAP6_ID_COUNT,
    .map[11].output_base[6]= NAMED_COMP0_MAP6_OUTPUT_BASE,
    .map[11].output_ref[6] = NAMED_COMP0_MAP6_OUTPUT_REF,
    .map[11].input_base[7] = NAMED_COMP0_MAP7_INPUT_BASE,
    .map[11].id_count[7]   = NAMED_COMP0_MAP7_ID_COUNT,
    .map[11].output_base[7]= NAMED_COMP0_MAP7_OUTPUT_BASE,
    .map[11].output_ref[7] = NAMED_COMP0_MAP7_OUTPUT_REF,
    .map[11].input_base[8] = NAMED_COMP0_MAP8_INPUT_BASE,
    .map[11].id_count[8]   = NAMED_COMP0_MAP8_ID_COUNT,
    .map[11].output_base[8]= NAMED_COMP0_MAP8_OUTPUT_BASE,
    .map[11].output_ref[8] = NAMED_COMP0_MAP8_OUTPUT_REF,

    .map[12].input_base[0] = NAMED_COMP1_MAP0_INPUT_BASE,
    .map[12].id_count[0]   = NAMED_COMP1_MAP0_ID_COUNT,
    .map[12].output_base[0]= NAMED_COMP1_MAP0_OUTPUT_BASE,
    .map[12].output_ref[0] = NAMED_COMP1_MAP0_OUTPUT_REF,
    .map[12].input_base[1] = NAMED_COMP1_MAP1_INPUT_BASE,
    .map[12].id_count[1]   = NAMED_COMP1_MAP1_ID_COUNT,
    .map[12].output_base[1]= NAMED_COMP1_MAP1_OUTPUT_BASE,
    .map[12].output_ref[1] = NAMED_COMP1_MAP1_OUTPUT_REF,
    .map[12].input_base[2] = NAMED_COMP1_MAP2_INPUT_BASE,
    .map[12].id_count[2]   = NAMED_COMP1_MAP2_ID_COUNT,
    .map[12].output_base[2]= NAMED_COMP1_MAP2_OUTPUT_BASE,
    .map[12].output_ref[2] = NAMED_COMP1_MAP2_OUTPUT_REF,
    .map[12].input_base[3] = NAMED_COMP1_MAP3_INPUT_BASE,
    .map[12].id_count[3]   = NAMED_COMP1_MAP3_ID_COUNT,
    .map[12].output_base[3]= NAMED_COMP1_MAP3_OUTPUT_BASE,
    .map[12].output_ref[3] = NAMED_COMP1_MAP3_OUTPUT_REF,
    .map[12].input_base[4] = NAMED_COMP1_MAP4_INPUT_BASE,
    .map[12].id_count[4]   = NAMED_COMP1_MAP4_ID_COUNT,
    .map[12].output_base[4]= NAMED_COMP1_MAP4_OUTPUT_BASE,
    .map[12].output_ref[4] = NAMED_COMP1_MAP4_OUTPUT_REF,
    .map[12].input_base[5] = NAMED_COMP1_MAP5_INPUT_BASE,
    .map[12].id_count[5]   = NAMED_COMP1_MAP5_ID_COUNT,
    .map[12].output_base[5]= NAMED_COMP1_MAP5_OUTPUT_BASE,
    .map[12].output_ref[5] = NAMED_COMP1_MAP5_OUTPUT_REF,
    .map[12].input_base[6] = NAMED_COMP1_MAP6_INPUT_BASE,
    .map[12].id_count[6]   = NAMED_COMP1_MAP6_ID_COUNT,
    .map[12].output_base[6]= NAMED_COMP1_MAP6_OUTPUT_BASE,
    .map[12].output_ref[6] = NAMED_COMP1_MAP6_OUTPUT_REF,
    .map[12].input_base[7] = NAMED_COMP1_MAP7_INPUT_BASE,
    .map[12].id_count[7]   = NAMED_COMP1_MAP7_ID_COUNT,
    .map[12].output_base[7]= NAMED_COMP1_MAP7_OUTPUT_BASE,
    .map[12].output_ref[7] = NAMED_COMP1_MAP7_OUTPUT_REF,
    .map[12].input_base[8] = NAMED_COMP1_MAP8_INPUT_BASE,
    .map[12].id_count[8]   = NAMED_COMP1_MAP8_ID_COUNT,
    .map[12].output_base[8]= NAMED_COMP1_MAP8_OUTPUT_BASE,
    .map[12].output_ref[8] = NAMED_COMP1_MAP8_OUTPUT_REF,


};

PLATFORM_OVERRIDE_NODE_DATA platform_node_type = {
    .its_count                        = IOVIRT_ITS_COUNT,
    .smmu[0].base                     = IOVIRT_SMMUV3_0_BASE_ADDRESS,
    .smmu[1].base                     = IOVIRT_SMMUV3_1_BASE_ADDRESS,
    .smmu[2].base                     = IOVIRT_SMMUV3_2_BASE_ADDRESS,
    .smmu[3].base                     = IOVIRT_SMMUV3_3_BASE_ADDRESS,
    .smmu[4].base                     = IOVIRT_SMMUV3_4_BASE_ADDRESS,
    .smmu[0].context_interrupt_offset = IOVIRT_SMMU_CTX_INT_OFFSET,
    .smmu[0].context_interrupt_count  = IOVIRT_SMMU_CTX_INT_CNT,
    .rc.segment                       = IOVIRT_RC_PCI_SEG_NUM,
    .rc.cca                           = IOVIRT_RC_MEMORY_PROPERTIES,
    .rc.ats_attr                      = IOVIRT_RC_ATS_ATTRIBUTE

};


PLATFORM_OVERRIDE_UART_INFO_TABLE platform_uart_cfg = {
    .Address               = UART_ADDRESS,
    .BaseAddress.Address   = BASE_ADDRESS_ADDRESS,
    .InterfaceType         = INTERFACE_TYPE,
    .GlobalSystemInterrupt = UART_GLOBAL_SYSTEM_INTERRUPT,
    .BaudRate              = UART_BAUD_RATE,
    .PciDeviceId           = UART_PCI_DEVICE_ID,
    .PciVendorId           = UART_PCI_VENDOR_ID,
    .PciBusNumber          = UART_PCI_BUS_NUMBER,
    .PciDeviceNumber       = UART_PCI_DEV_NUMBER,
    .PciFunctionNumber     = UART_PCI_FUNC_NUMBER,
    .PciFlags              = UART_PCI_FLAGS,
    .PciSegment            = UART_PCI_SEGMENT
};

DMA_INFO_TABLE platform_dma_cfg = {
    .num_dma_ctrls = PLATFORM_OVERRIDE_DMA_CNT

    /** Place holder
    .info[0].target = TARGET,
    .info[0].port = PORT,
    .info[0].host = HOST,
    .info[0].flags = FLAGS,
    .info[0].type = TYPE**/

};

PLATFORM_OVERRIDE_MEMORY_INFO_TABLE platform_mem_cfg = {
    .count                   = PLATFORM_OVERRIDE_MEMORY_ENTRY_COUNT,
    .info[0].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY0_PHY_ADDR,
    .info[0].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY0_VIRT_ADDR,
    .info[0].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY0_SIZE,
    .info[0].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY0_TYPE,
    .info[1].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY1_PHY_ADDR,
    .info[1].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY1_VIRT_ADDR,
    .info[1].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY1_SIZE,
    .info[1].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY1_TYPE,
    .info[2].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY2_PHY_ADDR,
    .info[2].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY2_VIRT_ADDR,
    .info[2].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY2_SIZE,
    .info[2].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY2_TYPE,
    .info[3].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY3_PHY_ADDR,
    .info[3].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY3_VIRT_ADDR,
    .info[3].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY3_SIZE,
    .info[3].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY3_TYPE,
};

PCIE_READ_TABLE platform_pcie_device_hierarchy = {
    .num_entries             = PLATFORM_PCIE_NUM_ENTRIES,

    .device[0].class_code    = PLATFORM_PCIE_DEV0_CLASSCODE,
    .device[0].vendor_id     = PLATFORM_PCIE_DEV0_VENDOR_ID,
    .device[0].device_id     = PLATFORM_PCIE_DEV0_DEV_ID,
    .device[0].bus           = PLATFORM_PCIE_DEV0_BUS_NUM,
    .device[0].dev           = PLATFORM_PCIE_DEV0_DEV_NUM,
    .device[0].func          = PLATFORM_PCIE_DEV0_FUNC_NUM,
    .device[0].seg           = PLATFORM_PCIE_DEV0_SEG_NUM,
    .device[0].dma_support   = PLATFORM_PCIE_DEV0_DMA_SUPPORT,
    .device[0].dma_coherent  = PLATFORM_PCIE_DEV0_DMA_COHERENT,
    .device[0].p2p_support   = PLATFORM_PCIE_DEV0_P2P_SUPPORT,
    .device[0].dma_64bit     = PLATFORM_PCIE_DEV0_DMA_64BIT,
    .device[0].behind_smmu   = PLATFORM_PCIE_DEV0_BEHIND_SMMU,
    .device[0].atc_present   = PLATFORM_PCIE_DEV0_ATC_SUPPORT,

    .device[1].class_code    = PLATFORM_PCIE_DEV1_CLASSCODE,
    .device[1].vendor_id     = PLATFORM_PCIE_DEV1_VENDOR_ID,
    .device[1].device_id     = PLATFORM_PCIE_DEV1_DEV_ID,
    .device[1].bus           = PLATFORM_PCIE_DEV1_BUS_NUM,
    .device[1].dev           = PLATFORM_PCIE_DEV1_DEV_NUM,
    .device[1].func          = PLATFORM_PCIE_DEV1_FUNC_NUM,
    .device[1].seg           = PLATFORM_PCIE_DEV1_SEG_NUM,
    .device[1].dma_support   = PLATFORM_PCIE_DEV1_DMA_SUPPORT,
    .device[1].dma_coherent  = PLATFORM_PCIE_DEV1_DMA_COHERENT,
    .device[1].p2p_support   = PLATFORM_PCIE_DEV1_P2P_SUPPORT,
    .device[1].dma_64bit     = PLATFORM_PCIE_DEV1_DMA_64BIT,
    .device[1].behind_smmu   = PLATFORM_PCIE_DEV1_BEHIND_SMMU,
    .device[1].atc_present   = PLATFORM_PCIE_DEV1_ATC_SUPPORT,

    .device[2].class_code    = PLATFORM_PCIE_DEV2_CLASSCODE,
    .device[2].vendor_id     = PLATFORM_PCIE_DEV2_VENDOR_ID,
    .device[2].device_id     = PLATFORM_PCIE_DEV2_DEV_ID,
    .device[2].bus           = PLATFORM_PCIE_DEV2_BUS_NUM,
    .device[2].dev           = PLATFORM_PCIE_DEV2_DEV_NUM,
    .device[2].func          = PLATFORM_PCIE_DEV2_FUNC_NUM,
    .device[2].seg           = PLATFORM_PCIE_DEV2_SEG_NUM,
    .device[2].dma_support   = PLATFORM_PCIE_DEV2_DMA_SUPPORT,
    .device[2].dma_coherent  = PLATFORM_PCIE_DEV2_DMA_COHERENT,
    .device[2].p2p_support   = PLATFORM_PCIE_DEV2_P2P_SUPPORT,
    .device[2].dma_64bit     = PLATFORM_PCIE_DEV2_DMA_64BIT,
    .device[2].behind_smmu   = PLATFORM_PCIE_DEV2_BEHIND_SMMU,
    .device[2].atc_present   = PLATFORM_PCIE_DEV2_ATC_SUPPORT,

    .device[3].class_code    = PLATFORM_PCIE_DEV3_CLASSCODE,
    .device[3].vendor_id     = PLATFORM_PCIE_DEV3_VENDOR_ID,
    .device[3].device_id     = PLATFORM_PCIE_DEV3_DEV_ID,
    .device[3].bus           = PLATFORM_PCIE_DEV3_BUS_NUM,
    .device[3].dev           = PLATFORM_PCIE_DEV3_DEV_NUM,
    .device[3].func          = PLATFORM_PCIE_DEV3_FUNC_NUM,
    .device[3].seg           = PLATFORM_PCIE_DEV3_SEG_NUM,
    .device[3].dma_support   = PLATFORM_PCIE_DEV3_DMA_SUPPORT,
    .device[3].dma_coherent  = PLATFORM_PCIE_DEV3_DMA_COHERENT,
    .device[3].p2p_support   = PLATFORM_PCIE_DEV3_P2P_SUPPORT,
    .device[3].dma_64bit     = PLATFORM_PCIE_DEV3_DMA_64BIT,
    .device[3].behind_smmu   = PLATFORM_PCIE_DEV3_BEHIND_SMMU,
    .device[3].atc_present   = PLATFORM_PCIE_DEV3_ATC_SUPPORT,
    /* IRQ list of interrupt pin INTC# */
    .device[3].irq_map.legacy_irq_map[2].irq_count = 1,
    .device[3].irq_map.legacy_irq_map[2].irq_list[0] = 200,

    .device[4].class_code    = PLATFORM_PCIE_DEV4_CLASSCODE,
    .device[4].vendor_id     = PLATFORM_PCIE_DEV4_VENDOR_ID,
    .device[4].device_id     = PLATFORM_PCIE_DEV4_DEV_ID,
    .device[4].bus           = PLATFORM_PCIE_DEV4_BUS_NUM,
    .device[4].dev           = PLATFORM_PCIE_DEV4_DEV_NUM,
    .device[4].func          = PLATFORM_PCIE_DEV4_FUNC_NUM,
    .device[4].seg           = PLATFORM_PCIE_DEV4_SEG_NUM,
    .device[4].dma_support   = PLATFORM_PCIE_DEV4_DMA_SUPPORT,
    .device[4].dma_coherent  = PLATFORM_PCIE_DEV4_DMA_COHERENT,
    .device[4].p2p_support   = PLATFORM_PCIE_DEV4_P2P_SUPPORT,
    .device[4].dma_64bit     = PLATFORM_PCIE_DEV4_DMA_64BIT,
    .device[4].behind_smmu   = PLATFORM_PCIE_DEV4_BEHIND_SMMU,
    .device[4].atc_present   = PLATFORM_PCIE_DEV4_ATC_SUPPORT,
    /* IRQ list of interrupt pin INTC# */
    .device[4].irq_map.legacy_irq_map[0].irq_count = 1,
    .device[4].irq_map.legacy_irq_map[0].irq_list[0] = 200,

    .device[5].class_code    = PLATFORM_PCIE_DEV5_CLASSCODE,
    .device[5].vendor_id     = PLATFORM_PCIE_DEV5_VENDOR_ID,
    .device[5].device_id     = PLATFORM_PCIE_DEV5_DEV_ID,
    .device[5].bus           = PLATFORM_PCIE_DEV5_BUS_NUM,
    .device[5].dev           = PLATFORM_PCIE_DEV5_DEV_NUM,
    .device[5].func          = PLATFORM_PCIE_DEV5_FUNC_NUM,
    .device[5].seg           = PLATFORM_PCIE_DEV5_SEG_NUM,
    .device[5].dma_support   = PLATFORM_PCIE_DEV5_DMA_SUPPORT,
    .device[5].dma_coherent  = PLATFORM_PCIE_DEV5_DMA_COHERENT,
    .device[5].p2p_support   = PLATFORM_PCIE_DEV5_P2P_SUPPORT,
    .device[5].dma_64bit     = PLATFORM_PCIE_DEV5_DMA_64BIT,
    .device[5].behind_smmu   = PLATFORM_PCIE_DEV5_BEHIND_SMMU,
    .device[5].atc_present   = PLATFORM_PCIE_DEV5_ATC_SUPPORT,
    /* IRQ list of interrupt pin INTC# */
    .device[5].irq_map.legacy_irq_map[2].irq_count = 1,
    .device[5].irq_map.legacy_irq_map[2].irq_list[0] = 200,

    .device[6].class_code    = PLATFORM_PCIE_DEV6_CLASSCODE,
    .device[6].vendor_id     = PLATFORM_PCIE_DEV6_VENDOR_ID,
    .device[6].device_id     = PLATFORM_PCIE_DEV6_DEV_ID,
    .device[6].bus           = PLATFORM_PCIE_DEV6_BUS_NUM,
    .device[6].dev           = PLATFORM_PCIE_DEV6_DEV_NUM,
    .device[6].func          = PLATFORM_PCIE_DEV6_FUNC_NUM,
    .device[6].seg           = PLATFORM_PCIE_DEV6_SEG_NUM,
    .device[6].dma_support   = PLATFORM_PCIE_DEV6_DMA_SUPPORT,
    .device[6].dma_coherent  = PLATFORM_PCIE_DEV6_DMA_COHERENT,
    .device[6].p2p_support   = PLATFORM_PCIE_DEV6_P2P_SUPPORT,
    .device[6].dma_64bit     = PLATFORM_PCIE_DEV6_DMA_64BIT,
    .device[6].behind_smmu   = PLATFORM_PCIE_DEV6_BEHIND_SMMU,
    .device[6].atc_present   = PLATFORM_PCIE_DEV6_ATC_SUPPORT,
    /* IRQ list of interrupt pin INTC# */
    .device[6].irq_map.legacy_irq_map[2].irq_count = 1,
    .device[6].irq_map.legacy_irq_map[2].irq_list[0] = 200,

    .device[7].class_code    = PLATFORM_PCIE_DEV7_CLASSCODE,
    .device[7].vendor_id     = PLATFORM_PCIE_DEV7_VENDOR_ID,
    .device[7].device_id     = PLATFORM_PCIE_DEV7_DEV_ID,
    .device[7].bus           = PLATFORM_PCIE_DEV7_BUS_NUM,
    .device[7].dev           = PLATFORM_PCIE_DEV7_DEV_NUM,
    .device[7].func          = PLATFORM_PCIE_DEV7_FUNC_NUM,
    .device[7].seg           = PLATFORM_PCIE_DEV7_SEG_NUM,
    .device[7].dma_support   = PLATFORM_PCIE_DEV7_DMA_SUPPORT,
    .device[7].dma_coherent  = PLATFORM_PCIE_DEV7_DMA_COHERENT,
    .device[7].p2p_support   = PLATFORM_PCIE_DEV7_P2P_SUPPORT,
    .device[7].dma_64bit     = PLATFORM_PCIE_DEV7_DMA_64BIT,
    .device[7].behind_smmu   = PLATFORM_PCIE_DEV7_BEHIND_SMMU,
    .device[7].atc_present   = PLATFORM_PCIE_DEV7_ATC_SUPPORT,
    /* IRQ list of interrupt pin INTC# */
    .device[7].irq_map.legacy_irq_map[3].irq_count = 1,
    .device[7].irq_map.legacy_irq_map[3].irq_list[0] = 200,

    .device[8].class_code    = PLATFORM_PCIE_DEV8_CLASSCODE,
    .device[8].vendor_id     = PLATFORM_PCIE_DEV8_VENDOR_ID,
    .device[8].device_id     = PLATFORM_PCIE_DEV8_DEV_ID,
    .device[8].bus           = PLATFORM_PCIE_DEV8_BUS_NUM,
    .device[8].dev           = PLATFORM_PCIE_DEV8_DEV_NUM,
    .device[8].func          = PLATFORM_PCIE_DEV8_FUNC_NUM,
    .device[8].seg           = PLATFORM_PCIE_DEV8_SEG_NUM,
    .device[8].dma_support   = PLATFORM_PCIE_DEV8_DMA_SUPPORT,
    .device[8].dma_coherent  = PLATFORM_PCIE_DEV8_DMA_COHERENT,
    .device[8].p2p_support   = PLATFORM_PCIE_DEV8_P2P_SUPPORT,
    .device[8].dma_64bit     = PLATFORM_PCIE_DEV8_DMA_64BIT,
    .device[8].behind_smmu   = PLATFORM_PCIE_DEV8_BEHIND_SMMU,
    .device[8].atc_present   = PLATFORM_PCIE_DEV8_ATC_SUPPORT,
    /* IRQ list of interrupt pin INTC# */
    .device[8].irq_map.legacy_irq_map[2].irq_count = 1,
    .device[8].irq_map.legacy_irq_map[2].irq_list[0] = 200,

    .device[9].class_code    = PLATFORM_PCIE_DEV9_CLASSCODE,
    .device[9].vendor_id     = PLATFORM_PCIE_DEV9_VENDOR_ID,
    .device[9].device_id     = PLATFORM_PCIE_DEV9_DEV_ID,
    .device[9].bus           = PLATFORM_PCIE_DEV9_BUS_NUM,
    .device[9].dev           = PLATFORM_PCIE_DEV9_DEV_NUM,
    .device[9].func          = PLATFORM_PCIE_DEV9_FUNC_NUM,
    .device[9].seg           = PLATFORM_PCIE_DEV9_SEG_NUM,
    .device[9].dma_support   = PLATFORM_PCIE_DEV9_DMA_SUPPORT,
    .device[9].dma_coherent  = PLATFORM_PCIE_DEV9_DMA_COHERENT,
    .device[9].p2p_support   = PLATFORM_PCIE_DEV9_P2P_SUPPORT,
    .device[9].dma_64bit     = PLATFORM_PCIE_DEV9_DMA_64BIT,
    .device[9].behind_smmu   = PLATFORM_PCIE_DEV9_BEHIND_SMMU,
    .device[9].atc_present   = PLATFORM_PCIE_DEV9_ATC_SUPPORT,

    .device[10].class_code    = PLATFORM_PCIE_DEV10_CLASSCODE,
    .device[10].vendor_id     = PLATFORM_PCIE_DEV10_VENDOR_ID,
    .device[10].device_id     = PLATFORM_PCIE_DEV10_DEV_ID,
    .device[10].bus           = PLATFORM_PCIE_DEV10_BUS_NUM,
    .device[10].dev           = PLATFORM_PCIE_DEV10_DEV_NUM,
    .device[10].func          = PLATFORM_PCIE_DEV10_FUNC_NUM,
    .device[10].seg           = PLATFORM_PCIE_DEV10_SEG_NUM,
    .device[10].dma_support   = PLATFORM_PCIE_DEV10_DMA_SUPPORT,
    .device[10].dma_coherent  = PLATFORM_PCIE_DEV10_DMA_COHERENT,
    .device[10].p2p_support   = PLATFORM_PCIE_DEV10_P2P_SUPPORT,
    .device[10].dma_64bit     = PLATFORM_PCIE_DEV10_DMA_64BIT,
    .device[10].behind_smmu   = PLATFORM_PCIE_DEV10_BEHIND_SMMU,
    .device[10].atc_present   = PLATFORM_PCIE_DEV10_ATC_SUPPORT,

    .device[11].class_code    = PLATFORM_PCIE_DEV11_CLASSCODE,
    .device[11].vendor_id     = PLATFORM_PCIE_DEV11_VENDOR_ID,
    .device[11].device_id     = PLATFORM_PCIE_DEV11_DEV_ID,
    .device[11].bus           = PLATFORM_PCIE_DEV11_BUS_NUM,
    .device[11].dev           = PLATFORM_PCIE_DEV11_DEV_NUM,
    .device[11].func          = PLATFORM_PCIE_DEV11_FUNC_NUM,
    .device[11].seg           = PLATFORM_PCIE_DEV11_SEG_NUM,
    .device[11].dma_support   = PLATFORM_PCIE_DEV11_DMA_SUPPORT,
    .device[11].dma_coherent  = PLATFORM_PCIE_DEV11_DMA_COHERENT,
    .device[11].p2p_support   = PLATFORM_PCIE_DEV11_P2P_SUPPORT,
    .device[11].dma_64bit     = PLATFORM_PCIE_DEV11_DMA_64BIT,
    .device[11].behind_smmu   = PLATFORM_PCIE_DEV11_BEHIND_SMMU,
    .device[11].atc_present   = PLATFORM_PCIE_DEV11_ATC_SUPPORT,
    /* IRQ list of interrupt pin INTC# */
    .device[11].irq_map.legacy_irq_map[0].irq_count = 1,
    .device[11].irq_map.legacy_irq_map[0].irq_list[0] = 200,

    .device[12].class_code    = PLATFORM_PCIE_DEV12_CLASSCODE,
    .device[12].vendor_id     = PLATFORM_PCIE_DEV12_VENDOR_ID,
    .device[12].device_id     = PLATFORM_PCIE_DEV12_DEV_ID,
    .device[12].bus           = PLATFORM_PCIE_DEV12_BUS_NUM,
    .device[12].dev           = PLATFORM_PCIE_DEV12_DEV_NUM,
    .device[12].func          = PLATFORM_PCIE_DEV12_FUNC_NUM,
    .device[12].seg           = PLATFORM_PCIE_DEV12_SEG_NUM,
    .device[12].dma_support   = PLATFORM_PCIE_DEV12_DMA_SUPPORT,
    .device[12].dma_coherent  = PLATFORM_PCIE_DEV12_DMA_COHERENT,
    .device[12].p2p_support   = PLATFORM_PCIE_DEV12_P2P_SUPPORT,
    .device[12].dma_64bit     = PLATFORM_PCIE_DEV12_DMA_64BIT,
    .device[12].behind_smmu   = PLATFORM_PCIE_DEV12_BEHIND_SMMU,
    .device[12].atc_present   = PLATFORM_PCIE_DEV12_ATC_SUPPORT,

    .device[13].class_code    = PLATFORM_PCIE_DEV13_CLASSCODE,
    .device[13].vendor_id     = PLATFORM_PCIE_DEV13_VENDOR_ID,
    .device[13].device_id     = PLATFORM_PCIE_DEV13_DEV_ID,
    .device[13].bus           = PLATFORM_PCIE_DEV13_BUS_NUM,
    .device[13].dev           = PLATFORM_PCIE_DEV13_DEV_NUM,
    .device[13].func          = PLATFORM_PCIE_DEV13_FUNC_NUM,
    .device[13].seg           = PLATFORM_PCIE_DEV13_SEG_NUM,
    .device[13].dma_support   = PLATFORM_PCIE_DEV13_DMA_SUPPORT,
    .device[13].dma_coherent  = PLATFORM_PCIE_DEV13_DMA_COHERENT,
    .device[13].p2p_support   = PLATFORM_PCIE_DEV13_P2P_SUPPORT,
    .device[13].dma_64bit     = PLATFORM_PCIE_DEV13_DMA_64BIT,
    .device[13].behind_smmu   = PLATFORM_PCIE_DEV13_BEHIND_SMMU,
    .device[13].atc_present   = PLATFORM_PCIE_DEV13_ATC_SUPPORT,

    .device[14].class_code    = PLATFORM_PCIE_DEV14_CLASSCODE,
    .device[14].vendor_id     = PLATFORM_PCIE_DEV14_VENDOR_ID,
    .device[14].device_id     = PLATFORM_PCIE_DEV14_DEV_ID,
    .device[14].bus           = PLATFORM_PCIE_DEV14_BUS_NUM,
    .device[14].dev           = PLATFORM_PCIE_DEV14_DEV_NUM,
    .device[14].func          = PLATFORM_PCIE_DEV14_FUNC_NUM,
    .device[14].seg           = PLATFORM_PCIE_DEV14_SEG_NUM,
    .device[14].dma_support   = PLATFORM_PCIE_DEV14_DMA_SUPPORT,
    .device[14].dma_coherent  = PLATFORM_PCIE_DEV14_DMA_COHERENT,
    .device[14].p2p_support   = PLATFORM_PCIE_DEV14_P2P_SUPPORT,
    .device[14].dma_64bit     = PLATFORM_PCIE_DEV14_DMA_64BIT,
    .device[14].behind_smmu   = PLATFORM_PCIE_DEV14_BEHIND_SMMU,
    .device[14].atc_present   = PLATFORM_PCIE_DEV14_ATC_SUPPORT,

    .device[15].class_code    = PLATFORM_PCIE_DEV15_CLASSCODE,
    .device[15].vendor_id     = PLATFORM_PCIE_DEV15_VENDOR_ID,
    .device[15].device_id     = PLATFORM_PCIE_DEV15_DEV_ID,
    .device[15].bus           = PLATFORM_PCIE_DEV15_BUS_NUM,
    .device[15].dev           = PLATFORM_PCIE_DEV15_DEV_NUM,
    .device[15].func          = PLATFORM_PCIE_DEV15_FUNC_NUM,
    .device[15].seg           = PLATFORM_PCIE_DEV15_SEG_NUM,
    .device[15].dma_support   = PLATFORM_PCIE_DEV15_DMA_SUPPORT,
    .device[15].dma_coherent  = PLATFORM_PCIE_DEV15_DMA_COHERENT,
    .device[15].p2p_support   = PLATFORM_PCIE_DEV15_P2P_SUPPORT,
    .device[15].dma_64bit     = PLATFORM_PCIE_DEV15_DMA_64BIT,
    .device[15].behind_smmu   = PLATFORM_PCIE_DEV15_BEHIND_SMMU,
    .device[15].atc_present   = PLATFORM_PCIE_DEV15_ATC_SUPPORT,

    .device[16].class_code    = PLATFORM_PCIE_DEV16_CLASSCODE,
    .device[16].vendor_id     = PLATFORM_PCIE_DEV16_VENDOR_ID,
    .device[16].device_id     = PLATFORM_PCIE_DEV16_DEV_ID,
    .device[16].bus           = PLATFORM_PCIE_DEV16_BUS_NUM,
    .device[16].dev           = PLATFORM_PCIE_DEV16_DEV_NUM,
    .device[16].func          = PLATFORM_PCIE_DEV16_FUNC_NUM,
    .device[16].seg           = PLATFORM_PCIE_DEV16_SEG_NUM,
    .device[16].dma_support   = PLATFORM_PCIE_DEV16_DMA_SUPPORT,
    .device[16].dma_coherent  = PLATFORM_PCIE_DEV16_DMA_COHERENT,
    .device[16].p2p_support   = PLATFORM_PCIE_DEV16_P2P_SUPPORT,
    .device[16].dma_64bit     = PLATFORM_PCIE_DEV16_DMA_64BIT,
    .device[16].behind_smmu   = PLATFORM_PCIE_DEV16_BEHIND_SMMU,
    .device[16].atc_present   = PLATFORM_PCIE_DEV16_ATC_SUPPORT,

    .device[17].class_code    = PLATFORM_PCIE_DEV17_CLASSCODE,
    .device[17].vendor_id     = PLATFORM_PCIE_DEV17_VENDOR_ID,
    .device[17].device_id     = PLATFORM_PCIE_DEV17_DEV_ID,
    .device[17].bus           = PLATFORM_PCIE_DEV17_BUS_NUM,
    .device[17].dev           = PLATFORM_PCIE_DEV17_DEV_NUM,
    .device[17].func          = PLATFORM_PCIE_DEV17_FUNC_NUM,
    .device[17].seg           = PLATFORM_PCIE_DEV17_SEG_NUM,
    .device[17].dma_support   = PLATFORM_PCIE_DEV17_DMA_SUPPORT,
    .device[17].dma_coherent  = PLATFORM_PCIE_DEV17_DMA_COHERENT,
    .device[17].p2p_support   = PLATFORM_PCIE_DEV17_P2P_SUPPORT,
    .device[17].dma_64bit     = PLATFORM_PCIE_DEV17_DMA_64BIT,
    .device[17].behind_smmu   = PLATFORM_PCIE_DEV17_BEHIND_SMMU,
    .device[17].atc_present   = PLATFORM_PCIE_DEV17_ATC_SUPPORT,

    .device[18].class_code    = PLATFORM_PCIE_DEV18_CLASSCODE,
    .device[18].vendor_id     = PLATFORM_PCIE_DEV18_VENDOR_ID,
    .device[18].device_id     = PLATFORM_PCIE_DEV18_DEV_ID,
    .device[18].bus           = PLATFORM_PCIE_DEV18_BUS_NUM,
    .device[18].dev           = PLATFORM_PCIE_DEV18_DEV_NUM,
    .device[18].func          = PLATFORM_PCIE_DEV18_FUNC_NUM,
    .device[18].seg           = PLATFORM_PCIE_DEV18_SEG_NUM,
    .device[18].dma_support   = PLATFORM_PCIE_DEV18_DMA_SUPPORT,
    .device[18].dma_coherent  = PLATFORM_PCIE_DEV18_DMA_COHERENT,
    .device[18].p2p_support   = PLATFORM_PCIE_DEV18_P2P_SUPPORT,
    .device[18].dma_64bit     = PLATFORM_PCIE_DEV18_DMA_64BIT,
    .device[18].behind_smmu   = PLATFORM_PCIE_DEV18_BEHIND_SMMU,
    .device[18].atc_present   = PLATFORM_PCIE_DEV18_ATC_SUPPORT,
    /* IRQ list of interrupt pin INTC# */
    .device[18].irq_map.legacy_irq_map[3].irq_count = 1,
    .device[18].irq_map.legacy_irq_map[3].irq_list[0] = 200,

    .device[19].class_code    = PLATFORM_PCIE_DEV19_CLASSCODE,
    .device[19].vendor_id     = PLATFORM_PCIE_DEV19_VENDOR_ID,
    .device[19].device_id     = PLATFORM_PCIE_DEV19_DEV_ID,
    .device[19].bus           = PLATFORM_PCIE_DEV19_BUS_NUM,
    .device[19].dev           = PLATFORM_PCIE_DEV19_DEV_NUM,
    .device[19].func          = PLATFORM_PCIE_DEV19_FUNC_NUM,
    .device[19].seg           = PLATFORM_PCIE_DEV19_SEG_NUM,
    .device[19].dma_support   = PLATFORM_PCIE_DEV19_DMA_SUPPORT,
    .device[19].dma_coherent  = PLATFORM_PCIE_DEV19_DMA_COHERENT,
    .device[19].p2p_support   = PLATFORM_PCIE_DEV19_P2P_SUPPORT,
    .device[19].dma_64bit     = PLATFORM_PCIE_DEV19_DMA_64BIT,
    .device[19].behind_smmu   = PLATFORM_PCIE_DEV19_BEHIND_SMMU,
    .device[19].atc_present   = PLATFORM_PCIE_DEV19_ATC_SUPPORT,

    .device[20].class_code    = PLATFORM_PCIE_DEV20_CLASSCODE,
    .device[20].vendor_id     = PLATFORM_PCIE_DEV20_VENDOR_ID,
    .device[20].device_id     = PLATFORM_PCIE_DEV20_DEV_ID,
    .device[20].bus           = PLATFORM_PCIE_DEV20_BUS_NUM,
    .device[20].dev           = PLATFORM_PCIE_DEV20_DEV_NUM,
    .device[20].func          = PLATFORM_PCIE_DEV20_FUNC_NUM,
    .device[20].seg           = PLATFORM_PCIE_DEV20_SEG_NUM,
    .device[20].dma_support   = PLATFORM_PCIE_DEV20_DMA_SUPPORT,
    .device[20].dma_coherent  = PLATFORM_PCIE_DEV20_DMA_COHERENT,
    .device[20].p2p_support   = PLATFORM_PCIE_DEV20_P2P_SUPPORT,
    .device[20].dma_64bit     = PLATFORM_PCIE_DEV20_DMA_64BIT,
    .device[20].behind_smmu   = PLATFORM_PCIE_DEV20_BEHIND_SMMU,
    .device[20].atc_present   = PLATFORM_PCIE_DEV20_ATC_SUPPORT,
    /* IRQ list of interrupt pin INTC# */
    .device[20].irq_map.legacy_irq_map[3].irq_count = 1,
    .device[20].irq_map.legacy_irq_map[3].irq_list[0] = 200,

};
