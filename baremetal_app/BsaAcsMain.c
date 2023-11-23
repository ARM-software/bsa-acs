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

#include "val/include/val_interface.h"
#include "val/include/bsa_acs_pe.h"
#include "val/include/bsa_acs_val.h"

#include "val/include/bsa_acs_memory.h"
#include "BsaAcs.h"

uint32_t  g_print_level;
uint32_t  g_execute_nist;
uint32_t  g_print_mmio;
uint32_t  g_curr_module;
uint32_t  g_enable_module;
uint32_t  g_bsa_tests_total;
uint32_t  g_bsa_tests_pass;
uint32_t  g_bsa_tests_fail;
uint64_t  g_stack_pointer;
uint64_t  g_exception_ret_addr;
uint64_t  g_ret_addr;
uint32_t  g_wakeup_timeout;

uint32_t  g_sw_view[3] = {1, 1, 1}; //Operating System, Hypervisor, Platform Security
uint32_t  *g_skip_test_num;
uint32_t  *g_execute_tests;
uint32_t  *g_execute_modules;
uint32_t  g_build_sbsa = 0;

extern uint32_t g_skip_array[];
extern uint32_t g_num_skip;
extern uint32_t g_test_array[];
extern uint32_t g_num_tests;
extern uint32_t g_module_array[];
extern uint32_t g_num_modules;

uint32_t
createPeInfoTable(
)
{

  uint32_t Status;
  uint64_t *PeInfoTable;

  PeInfoTable = val_aligned_alloc(SIZE_4K, sizeof(PE_INFO_TABLE) +
                                (PLATFORM_OVERRIDE_PE_CNT * sizeof(PE_INFO_ENTRY)));

  Status = val_pe_create_info_table(PeInfoTable);

  return Status;

}

uint32_t
createGicInfoTable(
)
{
  uint32_t Status;
  uint64_t *GicInfoTable;
  uint32_t gic_info_end_index = 1; //Additional index for mem alloc to store the end value(0xff)

  GicInfoTable = val_aligned_alloc(SIZE_4K, sizeof(GIC_INFO_TABLE)
                  + ((PLATFORM_OVERRIDE_GICITS_COUNT
                  + PLATFORM_OVERRIDE_GICC_GICRD_COUNT + PLATFORM_OVERRIDE_GICR_GICRD_COUNT
                  + PLATFORM_OVERRIDE_GICC_COUNT + PLATFORM_OVERRIDE_GICD_COUNT
                  + gic_info_end_index) * sizeof(GIC_INFO_ENTRY)));

  Status = val_gic_create_info_table(GicInfoTable);

  return Status;

}

void
createTimerInfoTable(
)
{
  uint64_t   *TimerInfoTable;

  TimerInfoTable = val_aligned_alloc(SIZE_4K, sizeof(TIMER_INFO_TABLE)
                   + (PLATFORM_OVERRIDE_TIMER_COUNT * sizeof(TIMER_INFO_GTBLOCK)));

  val_timer_create_info_table(TimerInfoTable);
}

void
createWatchdogInfoTable(
)
{
  uint64_t *WdInfoTable;

  WdInfoTable = val_aligned_alloc(SIZE_4K, sizeof(WD_INFO_TABLE)
                + (PLATFORM_OVERRIDE_WD_TIMER_COUNT * sizeof(WD_INFO_BLOCK)));

  val_wd_create_info_table(WdInfoTable);
}


void
createPcieVirtInfoTable(
)
{
  uint64_t   *PcieInfoTable;
  uint64_t   *IoVirtInfoTable;

  PcieInfoTable = val_aligned_alloc(SIZE_4K, (sizeof(PCIE_INFO_TABLE)
                  + (PLATFORM_OVERRIDE_NUM_ECAM * sizeof(PCIE_INFO_BLOCK))));
  val_pcie_create_info_table(PcieInfoTable);

  IoVirtInfoTable = val_aligned_alloc(SIZE_4K, (sizeof(IOVIRT_INFO_TABLE)
                    + ((IOVIRT_ITS_COUNT + IOVIRT_SMMUV3_COUNT + IOVIRT_RC_COUNT
                    + IOVIRT_SMMUV2_COUNT + IOVIRT_NAMED_COMPONENT_COUNT + IOVIRT_PMCG_COUNT)
                    * sizeof(IOVIRT_BLOCK)) + (IOVIRT_MAX_NUM_MAP * sizeof(ID_MAP))));
  val_iovirt_create_info_table(IoVirtInfoTable);
}

void
createPeripheralInfoTable(
)
{
  uint64_t   *PeripheralInfoTable;
  uint64_t   *MemoryInfoTable;

  PeripheralInfoTable = val_aligned_alloc(SIZE_4K, sizeof(PERIPHERAL_INFO_TABLE)
                        + (PLATFORM_OVERRIDE_PERIPHERAL_COUNT * sizeof(PERIPHERAL_INFO_BLOCK)));
  val_peripheral_create_info_table(PeripheralInfoTable);

  MemoryInfoTable = val_aligned_alloc(SIZE_4K, sizeof(MEMORY_INFO_TABLE)
                    + (PLATFORM_OVERRIDE_MEMORY_ENTRY_COUNT * sizeof(MEM_INFO_BLOCK)));
  val_memory_create_info_table(MemoryInfoTable);
}

void
freeBsaAcsMem()
{

  val_pe_free_info_table();
  val_gic_free_info_table();
  val_timer_free_info_table();
  val_wd_free_info_table();
  val_pcie_free_info_table();
  val_iovirt_free_info_table();
  val_peripheral_free_info_table();
  val_free_shared_mem();
}

/***
  BSA Compliance Suite Entry Point.

  Call the Entry points of individual modules.

  @retval  0         The application exited normally.
  @retval  Other     An error occurred.
***/
int32_t
ShellAppMainbsa(
  )
{

  uint32_t             Status;
  void                 *branch_label;

  g_print_level = PLATFORM_OVERRIDE_PRINT_LEVEL;

  if (g_print_level < ACS_PRINT_INFO)
  {
      val_print(ACS_PRINT_ERR, "Print Level %d is not supported.\n", g_print_level);
      val_print(ACS_PRINT_ERR, "Setting Print level to %d\n", ACS_PRINT_INFO);
      g_print_level = ACS_PRINT_INFO;
  } else if (g_print_level > ACS_PRINT_ERR) {
      val_print(ACS_PRINT_ERR, "Print Level %d is not supported.\n", g_print_level);
      val_print(ACS_PRINT_ERR, "Setting Print level to %d\n", ACS_PRINT_ERR);
      g_print_level = ACS_PRINT_ERR;
  }

#ifdef TARGET_BM_BOOT
  /* Write page tables */
  if (val_setup_mmu())
      return ACS_STATUS_FAIL;

  /* Enable Stage-1 MMU */
  if (val_enable_mmu())
      return ACS_STATUS_FAIL;
#endif

  g_print_mmio = FALSE;
  g_wakeup_timeout = 1;

  //
  // Initialize global counters
  //
  g_bsa_tests_total = 0;
  g_bsa_tests_pass  = 0;
  g_bsa_tests_fail  = 0;

  val_print(ACS_PRINT_TEST, "\n\n BSA Architecture Compliance Suite\n", 0);
  val_print(ACS_PRINT_TEST, "\n          Version %d.", BSA_ACS_MAJOR_VER);
  val_print(ACS_PRINT_TEST, "%d.", BSA_ACS_MINOR_VER);
  val_print(ACS_PRINT_TEST, "%d\n", BSA_ACS_SUBMINOR_VER);

  val_print(ACS_PRINT_TEST, " Starting tests with print level : %2d\n\n", ACS_PRINT_TEST);

  g_skip_test_num = &g_skip_array[0];

  /* Check if there is a user override to run specific tests*/
  if (g_num_tests) {
      g_execute_tests   = &g_test_array[0];
  }

  /* Check if there is a user override to run specific modules*/
  if (g_num_modules) {
      g_execute_modules = &g_module_array[0];
  }

  val_print(ACS_PRINT_TEST, " Creating Platform Information Tables\n", 0);
  Status = createPeInfoTable();
  if (Status)
    return Status;

  Status = createGicInfoTable();
  if (Status)
    return Status;

  createTimerInfoTable();
  createWatchdogInfoTable();
  createPcieVirtInfoTable();
  createPeripheralInfoTable();

  val_allocate_shared_mem();

  /* Initialise exception vector, so any unexpected exception gets handled
   *  by default BSA exception handler.
   */
  branch_label = &&print_test_status;
  val_pe_context_save(AA64ReadSp(), (uint64_t)branch_label);
  val_pe_initialize_default_exception_handler(val_pe_default_esr);

  /***  Starting PE tests             ***/
  Status = val_pe_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting Memory Map tests     ***/
  Status |= val_memory_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting GIC tests            ***/
  Status |= val_gic_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting System MMU tests     ***/
  Status |= val_smmu_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting Timer tests          ***/
  Status |= val_timer_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting Wakeup semantic tests ***/
  Status |= val_wakeup_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting Peripheral tests     ***/
  Status |= val_peripheral_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting Watchdog tests       ***/
  Status |= val_wd_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting PCIe tests           ***/
  Status |= val_pcie_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting PCIe Exerciser tests ***/
  Status |= val_exerciser_execute_tests(g_sw_view);

print_test_status:
  val_print(ACS_PRINT_TEST, "\n     -------------------------------------------------------\n", 0);
  val_print(ACS_PRINT_TEST, "     Total Tests run  = %4d", g_bsa_tests_total);
  val_print(ACS_PRINT_TEST, "  Tests Passed  = %4d", g_bsa_tests_pass);
  val_print(ACS_PRINT_TEST, "  Tests Failed = %4d\n", g_bsa_tests_fail);
  val_print(ACS_PRINT_TEST, "     -------------------------------------------------------\n", 0);

  freeBsaAcsMem();

  val_print(ACS_PRINT_TEST, "\n      *** BSA tests complete. Reset the system. ***\n\n", 0);

  val_pe_context_restore(AA64WriteSp(g_stack_pointer));
  while (1);
  return 0;
}
