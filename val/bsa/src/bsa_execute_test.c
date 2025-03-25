/** @file
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
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

#include "common/include/acs_val.h"
#include "bsa/include/bsa_acs_pe.h"
#include "bsa/include/bsa_acs_gic.h"
#include "bsa/include/bsa_acs_pcie.h"
#include "bsa/include/bsa_acs_exerciser.h"
#include "bsa/include/bsa_acs_memory.h"
#include "bsa/include/bsa_acs_peripherals.h"
#include "bsa/include/bsa_acs_smmu.h"
#include "bsa/include/bsa_acs_timer.h"
#include "bsa/include/bsa_acs_wd.h"
#include "common/include/acs_gic_support.h"
#include "common/include/acs_common.h"
#include "common/include/acs_pcie.h"
#include "common/include/acs_smmu.h"
#include "common/include/acs_exerciser.h"
#include "common/include/acs_std_smc.h"
#include "common/sys_arch_src/gic/gic.h"
#include "bsa/include/bsa_acs_wakeup.h"
#include "bsa/include/bsa_val_interface.h"
#include "bsa/include/bsa_pal_interface.h"

extern PCIE_INFO_TABLE *g_pcie_info_table;
extern pcie_device_bdf_table *g_pcie_bdf_table;
extern uint32_t pcie_bdf_table_list_flag;
extern uint32_t g_pcie_integrated_devices;


#define OPERATING_SYSTEM    0
#define HYPERVISOR          1
#define PLATFORM_SECURITY   2

#define MODULE_END          3

void view_print_info(uint32_t view)
{
  static uint32_t view_check[3] = {0, 0, 0};

  if (view == MODULE_END) {
      //memset(view_check, 0, sizeof(view_check)); //Need to include string.h
      view_check[OPERATING_SYSTEM]  = 0;
      view_check[HYPERVISOR]        = 0;
      view_check[PLATFORM_SECURITY] = 0;
      return;
  }

  if (view_check[view] == 1)
      return;
  else
      view_check[view] = 1;

  switch (view) {

  case OPERATING_SYSTEM:
      val_print(ACS_PRINT_ERR, "\nOperating System View:\n", 0);
      break;
  case HYPERVISOR:
      val_print(ACS_PRINT_ERR, "\nHypervisor View:\n", 0);
      break;
  case PLATFORM_SECURITY:
      val_print(ACS_PRINT_ERR, "\nPlatform Security View:\n", 0);
      break;
  }

}

#ifndef TARGET_LINUX

/**
  @brief   This API will execute all PE tests designated for a given compliance level
           1. Caller       -  Application layer.
           2. Prerequisite -  val_pe_create_info_table, val_allocate_shared_mem
  @param   num_pe - the number of PE to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/


uint32_t
val_bsa_pe_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{
  uint32_t status, i;

  if (!(g_bsa_level >= 1 || g_bsa_only_level == 1 || g_bsa_only_level == 2))
      return ACS_STATUS_SKIP;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_PE_TEST_NUM_BASE) {
        val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all PE tests\n", 0);
        return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_PE_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all PE tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  status = ACS_STATUS_PASS;

  val_print_test_start("PE");
  g_curr_module = 1 << PE_MODULE;

  if (g_sw_view[G_SW_OS]) {

      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
          view_print_info(OPERATING_SYSTEM);
          status |= os_c001_entry(num_pe);
          status |= os_c002_entry(num_pe);
          status |= os_c003_entry(num_pe);
          status |= os_c004_entry(num_pe);
          status |= os_c006_entry(num_pe);
          status |= os_c007_entry(num_pe);
          status |= os_c008_entry(num_pe);
          status |= os_c009_entry(num_pe);
          status |= os_c010_entry(num_pe);
          status |= os_c011_entry(num_pe);
          status |= os_c012_entry(num_pe);
          status |= os_c013_entry(num_pe);
          if (!g_build_sbsa) { /* B_PE_15 is only in BSA checklist */
              status |= os_c014_entry(num_pe);
          }

          status |= os_c016_entry(num_pe);
      }

      if (g_bsa_level > 1 || g_bsa_only_level == 2) {
          view_print_info(OPERATING_SYSTEM);
          status |= os_c015_entry(num_pe);
      }
  }

  if (g_sw_view[G_SW_HYP]) {

      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
          view_print_info(HYPERVISOR);
          status |= hyp_c001_entry(num_pe);
          status |= hyp_c002_entry(num_pe);
          status |= hyp_c003_entry(num_pe);
          status |= hyp_c004_entry(num_pe);
          status |= hyp_c005_entry(num_pe);
      }
  }

  if (g_sw_view[G_SW_PS]) {

      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
          view_print_info(PLATFORM_SECURITY);
          status |= ps_c001_entry(num_pe);
      }
  }

  view_print_info(MODULE_END);
  val_print_test_end(status, "PE");

  return status;

}

/**
  @brief   This API executes all the GIC tests sequentially
           1. Caller       -  Application layer.
           2. Prerequisite -  val_gic_create_info_table()
  @param   num_pe - the number of PE to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_bsa_gic_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{

  uint32_t status, i;
  uint32_t gic_version, num_msi_frame;

  if (!(g_bsa_level >= 1 || g_bsa_only_level == 1))
      return ACS_STATUS_SKIP;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_GIC_TEST_NUM_BASE) {
          val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all GIC tests\n", 0);
          return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_GIC_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all GIC tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  status = ACS_STATUS_PASS;

  val_print_test_start("GIC");
  g_curr_module = 1 << GIC_MODULE;

  gic_version = val_gic_get_info(GIC_INFO_VERSION);

  if (g_sw_view[G_SW_OS]) {
      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
          view_print_info(OPERATING_SYSTEM);

          /* B_GIC_01 and B_GIC_02 only for BSA */
          if (!g_build_sbsa) {
              status |= os_g001_entry(num_pe);

              /* Run B_GIC_02 test only if system has GICv2 */
              if (gic_version == 2)
                  status |= os_g002_entry(num_pe);
          }
          if (gic_version > 2) {
              status |= os_g003_entry(num_pe);
              status |= os_g004_entry(num_pe);
          }
          status |= os_g005_entry(num_pe);
          if (!g_el1physkip)
              status |= os_g006_entry(num_pe);

          status |= os_g007_entry(num_pe);
      }
  }

  if (g_sw_view[G_SW_HYP]) {

      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
      view_print_info(HYPERVISOR);
      status |= hyp_g001_entry(num_pe);
      status |= hyp_g002_entry(num_pe);
      status |= hyp_g003_entry(num_pe);
      }
  }

  view_print_info(MODULE_END);
  if (g_bsa_level >= 1 || g_bsa_only_level == 1) {

      /* Run GICv2m only if GIC Version is v2m. */
      num_msi_frame = val_gic_get_info(GIC_INFO_NUM_MSI_FRAME);

      if ((gic_version != 2) || (num_msi_frame == 0)) {
          val_print(ACS_PRINT_INFO, "\n       No GICv2m, Skipping all GICv2m tests\n", 0);
          goto its_test;
      }

      if (val_gic_v2m_parse_info()) {
          val_print(ACS_PRINT_INFO,
                    "\n       GICv2m info mismatch, Skipping all GICv2m tests\n", 0);
          goto its_test;
      }

      val_print_test_start("GICv2m");
      if (g_sw_view[G_SW_OS]) {
          view_print_info(OPERATING_SYSTEM);
          status |= os_v2m001_entry(num_pe);
          status |= os_v2m002_entry(num_pe);
          status |= os_v2m003_entry(num_pe);
          status |= os_v2m004_entry(num_pe);
      }
      view_print_info(MODULE_END);

its_test:
      if ((val_gic_get_info(GIC_INFO_NUM_ITS) == 0) || (pal_target_is_dt())) {
          val_print(ACS_PRINT_DEBUG, "\n       No ITS, Skipping all DeviceID & ITS tests\n", 0);
          goto test_done;
      }

      val_print_test_start("DeviceID generation and ITS");
      if (g_sw_view[G_SW_OS]) {
          view_print_info(OPERATING_SYSTEM);
          status |= os_its001_entry(num_pe);
          status |= os_its002_entry(num_pe);
          status |= os_its003_entry(num_pe);
          status |= os_its004_entry(num_pe);
      }
      view_print_info(MODULE_END);
  }

test_done:
  val_print_test_end(status, "GIC");

  return status;

}

/**
  @brief   This API executes all the timer tests sequentially
           1. Caller       -  Application layer.
           2. Prerequisite -  val_timer_create_info_table()
  @param   num_pe - the number of PE to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_bsa_timer_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{
  uint32_t status, i;

  if (!(g_bsa_level >= 1 || g_bsa_only_level == 1))
      return ACS_STATUS_SKIP;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_TIMER_TEST_NUM_BASE) {
          val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Timer tests\n", 0);
          return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_TIMER_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Timer tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  val_print_test_start("Timer");
  status = ACS_STATUS_PASS;

  g_curr_module = 1 << TIMER_MODULE;

  if (g_sw_view[G_SW_OS]) {

      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
          view_print_info(OPERATING_SYSTEM);
          status |= os_t001_entry(num_pe);
          status |= os_t002_entry(num_pe);
          status |= os_t003_entry(num_pe);
          status |= os_t004_entry(num_pe);
          status |= os_t005_entry(num_pe);
      }
  }

  view_print_info(MODULE_END);
  val_print_test_end(status, "Timer");

  return status;
}

/**
  @brief   This API executes all the Watchdog tests sequentially
           1. Caller       -  Application layer.
           2. Prerequisite -  val_wd_create_info_table
  @param   num_pe - the number of PE to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_bsa_wd_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{
  uint32_t status, i;

   if (!(g_bsa_level >= 1 || g_bsa_only_level == 1))
      return ACS_STATUS_SKIP;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_WD_TEST_NUM_BASE) {
          val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Watchdog tests\n", 0);
          return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_WD_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Watchdog tests\n", 0);
      return ACS_STATUS_SKIP;
  }

if (!g_build_sbsa) { /* For SBSA compliance WD is mandatory */
  if (val_wd_get_info(0, WD_INFO_COUNT) == 0) {
    val_print(ACS_PRINT_WARN, "\n       No Watchdog Found, Skipping Watchdog "
                                                    "tests...\n", 0);
    return ACS_STATUS_SKIP;
  }
}

  val_print_test_start("Watchdog");
  status = ACS_STATUS_PASS;

  g_curr_module = 1 << WD_MODULE;

  if (g_sw_view[G_SW_OS]) {
      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
          view_print_info(OPERATING_SYSTEM);
          status |= os_w001_entry(num_pe);
          status |= os_w002_entry(num_pe);
      }
  }
  view_print_info(MODULE_END);

  val_print_test_end(status, "Watchdog");

  return status;
}

#endif
/**
  @brief   This API executes all the PCIe tests sequentially
           1. Caller       -  Application layer.
           2. Prerequisite -  val_pcie_create_info_table()
  @param   num_pe      - the number of PE to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run

  @return  Consolidated status of all the tests run.
**/
uint32_t
val_bsa_pcie_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{
  uint32_t status, i;
  uint32_t num_ecam = 0;

  if (!(g_bsa_level >= 1 || g_bsa_only_level == 1))
      return ACS_STATUS_SKIP;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_PCIE_TEST_NUM_BASE) {
          val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all PCIe tests\n", 0);
          return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_PCIE_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all PCIe tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  if (pcie_bdf_table_list_flag == 1) {
      val_print(ACS_PRINT_WARN, "\n     *** Created device list with valid bdf doesn't match \
                    with the platform pcie device hierarchy, Skipping PCIE tests ***\n", 0);
      return ACS_STATUS_SKIP;
  }

  num_ecam = (uint32_t)val_pcie_get_info(PCIE_INFO_NUM_ECAM, 0);
  if (!num_ecam) {
      val_print(ACS_PRINT_WARN, "\n      *** No ECAM region found, Skipping PCIE tests ***\n", 0);
      return ACS_STATUS_SKIP;
  }

  val_print_test_start("PCIe");
  status = ACS_STATUS_PASS;

  g_curr_module = 1 << PCIE_MODULE;

  if (g_sw_view[G_SW_OS]  && (g_bsa_level >= 1 || g_bsa_only_level == 1)) {
      view_print_info(OPERATING_SYSTEM);

      status |= os_p001_entry(num_pe);
      if (status == ACS_STATUS_FAIL) {
        val_print(ACS_PRINT_WARN, "\n      *** Skipping remaining PCIE tests ***\n", 0);
        return status;
      }
  }

  if (g_pcie_bdf_table->num_entries == 0) {
      val_print(ACS_PRINT_WARN,
                "\n       No PCIe Devices Found, Skipping PCIe tests...\n",
                0);
      return ACS_STATUS_SKIP;
  }

  if (g_pcie_bdf_table->num_entries == g_pcie_integrated_devices) {
      val_print(ACS_PRINT_WARN,
                "\n       Only integrated PCIe Devices Found, Skipping PCIe tests...\n",
                0);
      return ACS_STATUS_SKIP;
  }

  if (g_sw_view[G_SW_OS]) {

      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
#if defined(TARGET_LINUX) || defined(ENABLE_OOB) || defined(TARGET_EMULATION)
          status |= os_p061_entry(num_pe);
          status |= os_p062_entry(num_pe);
          status |= os_p063_entry(num_pe);
          status |= os_p064_entry(num_pe);
#endif
#ifndef TARGET_LINUX
          status |= os_p002_entry(num_pe);
          status |= os_p003_entry(num_pe);
#if defined(ENABLE_OOB) || defined(TARGET_EMULATION)
          status |= os_p004_entry(num_pe);
          status |= os_p005_entry(num_pe);
#endif
          status |= os_p006_entry(num_pe);
          status |= os_p008_entry(num_pe);
          status |= os_p009_entry(num_pe);
          status |= os_p011_entry(num_pe);

          status |= os_p017_entry(num_pe);
          status |= os_p018_entry(num_pe);
          status |= os_p019_entry(num_pe);
          status |= os_p020_entry(num_pe);
          status |= os_p021_entry(num_pe);
          status |= os_p022_entry(num_pe);
          status |= os_p024_entry(num_pe);
          status |= os_p025_entry(num_pe);
          status |= os_p026_entry(num_pe);
          status |= os_p030_entry(num_pe);
          status |= os_p031_entry(num_pe);
          status |= os_p032_entry(num_pe);
          status |= os_p033_entry(num_pe);
          status |= os_p035_entry(num_pe);
          status |= os_p036_entry(num_pe);
          status |= os_p037_entry(num_pe);
          status |= os_p038_entry(num_pe);
          status |= os_p039_entry(num_pe);
          status |= os_p042_entry(num_pe);
#endif
      }

  }

  view_print_info(MODULE_END);
  val_print_test_end(status, "PCIe");

  return status;
}

/**
  @brief  Sequentially execute all the peripheral tests
          1. Caller       - Application
          2. Prerequisite - val_peripheral_create_info_table
  @param  num_pe - number of PEs to run this test on
  @param  g_sw_view - Keeps the information about which view tests to be run

  @result  consolidated status of all the tests
**/
uint32_t
val_bsa_peripheral_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{

  uint32_t status, i;

  if (!(g_bsa_level >= 1 || g_bsa_only_level == 1))
      return ACS_STATUS_SKIP;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_PER_TEST_NUM_BASE) {
          val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Peripheral tests\n", 0);
          return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_PER_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Peripheral tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  val_print_test_start("Peripheral");
  status = ACS_STATUS_PASS;

  g_curr_module = 1 << PERIPHERAL_MODULE;

  if (g_sw_view[G_SW_OS]) {

      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
          view_print_info(OPERATING_SYSTEM);
#ifndef TARGET_LINUX
          status |= os_d001_entry(num_pe);
          status |= os_d002_entry(num_pe);
          status |= os_d003_entry(num_pe);
          status |= os_d005_entry(num_pe);
#endif
#if defined(TARGET_LINUX) || defined(ENABLE_OOB) || defined(TARGET_EMULATION)
          status |= os_d004_entry(num_pe);
#endif
      }
  }

  view_print_info(MODULE_END);
  val_print_test_end(status, "Peripheral");

  return status;
}


/**
  @brief   This API will execute all Memory tests
           1. Caller       -  Application layer.
           2. Prerequisite -  val_memory_create_info_table
  @param   num_pe - the number of PE to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_bsa_memory_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{

  uint32_t status, i;

 if (!(g_bsa_level >= 1 || g_bsa_only_level == 1))
      return ACS_STATUS_SKIP;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_MEMORY_MAP_TEST_NUM_BASE) {
        val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Memory tests\n", 0);
        return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_MEMORY_MAP_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Memory tests\n", 0);
      return ACS_STATUS_SKIP;
  }


  status = ACS_STATUS_PASS;

  val_print_test_start("Memory Map");
  g_curr_module = 1 << MEM_MAP_MODULE;

  if (g_sw_view[G_SW_OS]) {

      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
          view_print_info(OPERATING_SYSTEM);
#if defined(ENABLE_OOB) || defined(TARGET_EMULATION)
          status |= os_m001_entry(num_pe);
#endif
#ifndef TARGET_LINUX
          status |= os_m002_entry(num_pe);
          status |= os_m003_entry(num_pe);
#endif
#if defined(TARGET_LINUX) || defined(ENABLE_OOB) || defined(TARGET_EMULATION)
          status |= os_m004_entry(num_pe);
#endif
      }
  }

  view_print_info(MODULE_END);
  val_print_test_end(status, "Memory");

  return status;
}

#ifndef TARGET_LINUX

/**
  @brief   This API executes all the wakeup tests sequentially
           1. Caller       -  Application layer.
           2. Prerequisite -  None
  @param   num_pe - the number of PE to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_bsa_wakeup_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{
  uint32_t status, i;

  if (!(g_bsa_level >= 1 || g_bsa_only_level == 1))
      return ACS_STATUS_SKIP;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_WAKEUP_TEST_NUM_BASE) {
          val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Wakeup tests\n", 0);
          return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_WAKEUP_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Wakeup tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  val_print_test_start("Wakeup semantic");
  status = ACS_STATUS_PASS;

  g_curr_module = 1 << WAKEUP_MODULE;

  if (g_sw_view[G_SW_OS]) {

      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
          view_print_info(OPERATING_SYSTEM);
          status |= os_u001_entry(num_pe);
          status |= os_u002_entry(num_pe);
          status |= os_u003_entry(num_pe);
          status |= os_u004_entry(num_pe);
          status |= os_u005_entry(num_pe);
      }

 /*B_WAK_09 is required only for SBSA complaince
if (g_build_sbsa) {
      // Test needs multi-PE interrupt handling support
     // status |= os_u006_entry(num_pe);
}
*/
  }

  view_print_info(MODULE_END);
  val_print_test_end(status, "Wakeup");

  return status;

}

/**
  @brief   This API executes all the SMMU tests sequentially
           1. Caller       -  Application layer.
           2. Prerequisite -  val_smmu_create_info_table()
  @param   num_pe - the number of PE to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_bsa_smmu_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{
  uint32_t status, i;
  uint32_t num_smmu;
  uint32_t ver_smmu;

  if (!(g_bsa_level >= 1 || g_bsa_only_level == 1))
      return ACS_STATUS_SKIP;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_SMMU_TEST_NUM_BASE) {
          val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all SMMU tests\n", 0);
          return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_SMMU_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all SMMU tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  num_smmu = val_iovirt_get_smmu_info(SMMU_NUM_CTRL, 0);
  if (num_smmu == 0) {
    val_print(ACS_PRINT_WARN, "\n       No SMMU Controller Found, Skipping SMMU tests...\n", 0);
    return ACS_STATUS_SKIP;
  }

  val_print_test_start("SMMU");
  status = ACS_STATUS_PASS;

  g_curr_module = 1 << SMMU_MODULE;

  ver_smmu = val_smmu_get_info(SMMU_CTRL_ARCH_MAJOR_REV, 0);
  if (g_sw_view[G_SW_OS]) {

       if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
           view_print_info(OPERATING_SYSTEM);
           status |= os_i001_entry(num_pe);
           status |= os_i002_entry(num_pe);
           status |= os_i003_entry(num_pe);
           status |= os_i004_entry(num_pe);
       }
  }

  if (g_sw_view[G_SW_HYP]) {

      if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
          view_print_info(HYPERVISOR);
          status |= hyp_i002_entry(num_pe);
          if (ver_smmu == 2)
              status |= hyp_i003_entry(num_pe);
          status |= hyp_i004_entry(num_pe);
      }
  }

  view_print_info(MODULE_END);
  val_print_test_end(status, "SMMU");

  return status;
}

/**
  @brief   This API executes all the Exerciser tests sequentially
           1. Caller       -  Application layer.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_bsa_exerciser_execute_tests(uint32_t *g_sw_view)
{
  uint32_t status, i;
  uint32_t num_instances;
  uint32_t instance, num_smmu;

  if (!(g_bsa_level >= 1 || g_bsa_only_level == 1))
      return ACS_STATUS_SKIP;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_EXERCISER_TEST_NUM_BASE) {
          val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Exerciser tests\n", 0);
          return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_EXERCISER_TEST_NUM_BASE);
  if (status) {
    val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Exerciser tests\n", 0);
    return ACS_STATUS_SKIP;
  }

  /* Create the list of valid Pcie Device Functions */
  if (val_exerciser_create_info_table()) {
      val_print(ACS_PRINT_WARN, "\n     Create BDF Table Failed, Skipping Exerciser tests...\n", 0);
      return ACS_STATUS_SKIP;
  }

  num_instances = val_exerciser_get_info(EXERCISER_NUM_CARDS);

  if (num_instances == 0) {
      val_print(ACS_PRINT_WARN, "\n       No Exerciser Devices Found, "
                                    "Skipping tests...\n", 0);
      return ACS_STATUS_SKIP;
  }

  val_print(ACS_PRINT_INFO, "  Initializing SMMU\n", 0);

  num_smmu = (uint32_t)val_iovirt_get_smmu_info(SMMU_NUM_CTRL, 0);
  val_smmu_init();

  /* Disable All SMMU's */
  for (instance = 0; instance < num_smmu; ++instance)
      val_smmu_disable(instance);

  val_print(ACS_PRINT_INFO, "  Initializing ITS\n", 0);
  val_gic_its_configure();

  val_print_test_start("Exerciser");
  status = ACS_STATUS_PASS;

  g_curr_module = 1 << EXERCISER_MODULE;

  if (g_sw_view[G_SW_OS]) {

     if (g_bsa_level >= 1 || g_bsa_only_level == 1) {
         view_print_info(OPERATING_SYSTEM);
         status |= os_e001_entry();
         status |= os_e002_entry();
         status |= os_e003_entry();
         status |= os_e004_entry();
         status |= os_e005_entry();
         status |= os_e006_entry();
         status |= os_e007_entry();
         status |= os_e008_entry();
         status |= os_e010_entry();

         if (!pal_target_is_dt()) {
             status |= os_e011_entry();
             status |= os_e012_entry();
             status |= os_e013_entry();
         }

         status |= os_e014_entry();
         status |= os_e015_entry();
         status |= os_e016_entry();
         status |= os_e017_entry();
     }
  }

  val_smmu_stop();

  view_print_info(MODULE_END);
  val_print_test_end(status, "Exerciser");

  return status;
}

#endif
