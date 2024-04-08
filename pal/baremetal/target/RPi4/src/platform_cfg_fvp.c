/** @file
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
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
   purpose to complete BSA run on these systems
*/
uint32_t  g_el1physkip       = FALSE;

/* B_PE_06 and S_L5PE_05 rules are conditional implementation based on export restrictions
   In case due to export restrictions, cryptography algorithm support is not present, set
   g_crypto_support to FALSE (Default value is TRUE)
*/
uint32_t g_crypto_support    = TRUE;

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
    .gicd_base[0]   = PLATFORM_OVERRIDE_GICD_BASE,

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

    .gt_info.timer_count        = PLATFORM_OVERRIDE_TIMER_COUNT,
/** Configure the Timer info details as per the requirements
    .gt_info.type               = PLATFORM_OVERRIDE_TIMER_TYPE,
    .gt_info.block_cntl_base    = PLATFORM_OVERRIDE_TIMER_CNTCTL_BASE,
**/
};

WD_INFO_TABLE platform_wd_cfg = {
    .header.num_wd              = PLATFORM_OVERRIDE_WD_TIMER_COUNT,

};

PCIE_INFO_TABLE platform_pcie_cfg = {
    .num_entries             = PLATFORM_OVERRIDE_NUM_ECAM,

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
};

PLATFORM_OVERRIDE_NODE_DATA platform_node_type = {
    .its_count                        = IOVIRT_ITS_COUNT,
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
    .info[4].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY4_PHY_ADDR,
    .info[4].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY4_VIRT_ADDR,
    .info[4].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY4_SIZE,
    .info[4].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY4_TYPE,
    .info[5].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY5_PHY_ADDR,
    .info[5].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY5_VIRT_ADDR,
    .info[5].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY5_SIZE,
    .info[5].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY5_TYPE,
    .info[6].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY6_PHY_ADDR,
    .info[6].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY6_VIRT_ADDR,
    .info[6].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY6_SIZE,
    .info[6].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY6_TYPE,
    .info[7].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY7_PHY_ADDR,
    .info[7].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY7_VIRT_ADDR,
    .info[7].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY7_SIZE,
    .info[7].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY7_TYPE,
    .info[8].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY8_PHY_ADDR,
    .info[8].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY8_VIRT_ADDR,
    .info[8].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY8_SIZE,
    .info[8].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY8_TYPE,
    .info[9].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY9_PHY_ADDR,
    .info[9].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY9_VIRT_ADDR,
    .info[9].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY9_SIZE,
    .info[9].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY9_TYPE,
    .info[10].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY10_PHY_ADDR,
    .info[10].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY10_VIRT_ADDR,
    .info[10].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY10_SIZE,
    .info[10].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY10_TYPE,
    .info[11].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY11_PHY_ADDR,
    .info[11].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY11_VIRT_ADDR,
    .info[11].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY11_SIZE,
    .info[11].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY11_TYPE,
    .info[12].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY12_PHY_ADDR,
    .info[12].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY12_VIRT_ADDR,
    .info[12].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY12_SIZE,
    .info[12].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY12_TYPE,
    .info[13].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY13_PHY_ADDR,
    .info[13].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY13_VIRT_ADDR,
    .info[13].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY13_SIZE,
    .info[13].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY13_TYPE,
    .info[14].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY14_PHY_ADDR,
    .info[14].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY14_VIRT_ADDR,
    .info[14].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY14_SIZE,
    .info[14].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY14_TYPE,
    .info[15].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY15_PHY_ADDR,
    .info[15].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY15_VIRT_ADDR,
    .info[15].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY15_SIZE,
    .info[15].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY15_TYPE,
    .info[16].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY16_PHY_ADDR,
    .info[16].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY16_VIRT_ADDR,
    .info[16].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY16_SIZE,
    .info[16].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY16_TYPE,
    .info[17].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY17_PHY_ADDR,
    .info[17].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY17_VIRT_ADDR,
    .info[17].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY17_SIZE,
    .info[17].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY17_TYPE,
    .info[18].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY18_PHY_ADDR,
    .info[18].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY18_VIRT_ADDR,
    .info[18].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY18_SIZE,
    .info[18].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY18_TYPE,
    .info[19].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY19_PHY_ADDR,
    .info[19].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY19_VIRT_ADDR,
    .info[19].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY19_SIZE,
    .info[19].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY19_TYPE,
    .info[20].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY20_PHY_ADDR,
    .info[20].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY20_VIRT_ADDR,
    .info[20].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY20_SIZE,
    .info[20].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY20_TYPE,
    .info[21].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY21_PHY_ADDR,
    .info[21].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY21_VIRT_ADDR,
    .info[21].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY21_SIZE,
    .info[21].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY21_TYPE,
    .info[22].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY22_PHY_ADDR,
    .info[22].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY22_VIRT_ADDR,
    .info[22].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY22_SIZE,
    .info[22].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY22_TYPE,
    .info[23].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY23_PHY_ADDR,
    .info[23].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY23_VIRT_ADDR,
    .info[23].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY23_SIZE,
    .info[23].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY23_TYPE,
    .info[24].phy_addr        = PLATFORM_OVERRIDE_MEMORY_ENTRY24_PHY_ADDR,
    .info[24].virt_addr       = PLATFORM_OVERRIDE_MEMORY_ENTRY24_VIRT_ADDR,
    .info[24].size            = PLATFORM_OVERRIDE_MEMORY_ENTRY24_SIZE,
    .info[24].type            = PLATFORM_OVERRIDE_MEMORY_ENTRY24_TYPE,
};

PCIE_READ_TABLE platform_pcie_device_hierarchy = {
    .num_entries             = PLATFORM_PCIE_NUM_ENTRIES,
};
