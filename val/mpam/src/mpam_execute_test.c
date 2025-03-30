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

#include "../../common/include/val_interface.h"
#include "../../common/include/acs_common.h"
#include "../../common/include/pal_interface.h"
#include "../../common/include/acs_std_smc.h"

#include "../include/mpam_val_interface.h"

/**
  @brief   This API will execute all MPAM Error tests
           1. Caller       -  Application layer.
           2. Prerequisite
  @param   num_pe - the number of PE to run these tests on.
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_mpam_execute_error_tests(uint32_t num_pe)
{
  uint32_t status, i;

  /* Making num_pe as 1 */
  num_pe = 1;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_MPAM_ERROR_TEST_NUM_BASE) {
        val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all ERROR tests\n", 0);
        return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_MPAM_ERROR_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all ERROR tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  status = ACS_STATUS_PASS;

  val_print_test_start("ERROR");
  g_curr_module = 1 << ERROR_MODULE;

  status = error001_entry();
  status |= error002_entry();
  status |= error003_entry();
  status |= error004_entry();
  status |= error005_entry();
  status |= error006_entry();
  status |= error007_entry();
  status |= error008_entry();
  status |= error009_entry();
  status |= error010_entry();
  status |= error011_entry();
  status |= intr001_entry();
  status |= intr002_entry();
  status |= intr003_entry();

  val_print_test_end(status, "ERROR");

  return status;

}

/**
  @brief   This API will execute all MPAM Memory Bandwidth tests
           1. Caller       -  Application layer.
           2. Prerequisite
  @param   num_pe - the number of PE to run these tests on.
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_mpam_execute_membw_tests(uint32_t num_pe)
{
  uint32_t status, i;

  /* Making num_pe as 1 */
  num_pe = 1;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_MPAM_MEMORY_TEST_NUM_BASE) {
        val_print(ACS_PRINT_INFO,
                             "\n       USER Override - Skipping all Memory Bandwidth tests\n", 0);
        return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_MPAM_MEMORY_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO,
                             "\n       USER Override - Skipping all Memory Bandwidth tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  status = ACS_STATUS_PASS;

  val_print_test_start("MEMORY BANDWIDTH");
  g_curr_module = 1 << MEMORY_MODULE;

  status = mem001_entry();
  status |= mem002_entry();
  status |= mem003_entry();

  val_print_test_end(status, "MEMORY BANDWIDTH");

  return status;

}

/**
  @brief   This API will execute all MPAM Register tests
           1. Caller       -  Application layer.
           2. Prerequisite
  @param   num_pe - the number of PE to run these tests on.
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_mpam_execute_register_tests(uint32_t num_pe)
{
  uint32_t status, i;

  /* Making num_pe as 1 */
  num_pe = 1;
  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_MPAM_REGISTER_TEST_NUM_BASE) {
        val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Register tests\n", 0);
        return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_MPAM_REGISTER_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Register tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  status = ACS_STATUS_PASS;

  val_print_test_start("REGISTER");
  g_curr_module = 1 << REGISTER_MODULE;

  status |= reg001_entry();
  status |= reg002_entry();
  status |= reg003_entry();

  val_print_test_end(status, "REGISTER");

  return status;
}


/**
  @brief   This API will execute all MPAM Cache tests
           1. Caller       -  Application layer.
           2. Prerequisite
  @param   num_pe - the number of PE to run these tests on.
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_mpam_execute_cache_tests(uint32_t num_pe)
{
  uint32_t status, i;

  /* Making num_pe as 1 */
  num_pe = 1;
  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_MPAM_CACHE_TEST_NUM_BASE) {
        val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all CACHE tests\n", 0);
        return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_MPAM_CACHE_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all CACHE tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  status = ACS_STATUS_PASS;

  val_print_test_start("CACHE");
  g_curr_module = 1 << CACHE_MODULE;

  status |= partition001_entry();
  status |= partition002_entry();
  status |= partition003_entry();

  status |= monitor001_entry();
  status |= monitor002_entry();
  status |= monitor003_entry();
  status |= monitor004_entry();

  val_print_test_end(status, "CACHE");

  return status;
}
