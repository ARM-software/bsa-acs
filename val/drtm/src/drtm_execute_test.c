/** @file
 * DRTM API
 *
 * Copyright (c) 2024, 2025, Arm Limited or its affiliates. All rights reserved.
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

#include "../include/drtm_val_interface.h"
#include "../include/drtm_pal_interface.h"

/**
  @brief   This API will execute all DRTM Interface tests
           1. Caller       -  Application layer.
           2. Prerequisite
  @param   num_pe - the number of PE to run these tests on.
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_drtm_execute_interface_tests(uint32_t num_pe)
{
  uint32_t status, i;

  /* Making num_pe as 1 */
  num_pe = 1;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_DRTM_INTERFACE_TEST_NUM_BASE) {
        val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Interface tests\n", 0);
        return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_DRTM_INTERFACE_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all Interface tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  status = ACS_STATUS_PASS;

  val_print_test_start("INTERFACE");
  g_curr_module = 1 << INTERFACE_MODULE;

  status  = interface001_entry(num_pe);
  status |= interface002_entry(num_pe);
  status |= interface003_entry(num_pe);
  status |= interface004_entry(num_pe);
  status |= interface005_entry(num_pe);
  status |= interface006_entry(num_pe);
  status |= interface007_entry(num_pe);
  status |= interface008_entry(num_pe);
  status |= interface009_entry(num_pe);
  status |= interface010_entry(num_pe);
  status |= interface011_entry(num_pe);
  status |= interface012_entry(num_pe);
  status |= interface013_entry(num_pe);
  status |= interface014_entry(num_pe);
  status |= interface015_entry(num_pe);

  val_print_test_end(status, "INTERFACE");

  return status;

}

/**
  @brief   This API will execute all DRTM Dynamic Launch tests
           1. Caller       -  Application layer.
           2. Prerequisite
  @param   num_pe - the number of PE to run these tests on.
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_drtm_execute_dl_tests(uint32_t num_pe)
{
  uint32_t status, i;

  /* Making num_pe as 1 */
  num_pe = 1;
  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_DRTM_DL_TEST_NUM_BASE) {
        val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all DL tests\n", 0);
        return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_DRTM_DL_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all DL tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  /* Check Dynamic Launch should be supported to run this test using drtm features command
   * Less than zero are error case
   */
  if (g_drtm_features.dynamic_launch < DRTM_ACS_SUCCESS) {
    val_print(ACS_PRINT_ERR,
                  "\n       DRTM query Dynamic Launch feature failed err=%d", status);
    val_print(ACS_PRINT_WARN, "\n     *** Skipping remaining DL tests ***\n", 0);
    return ACS_STATUS_FAIL;
  }

  /* Check Min Memory req supported to run this test using drtm features command
   * Less than or equal to zero are error case */
  if (g_drtm_features.min_memory_req.status <= DRTM_ACS_SUCCESS) {
    val_print(ACS_PRINT_ERR,
                  "\n       DRTM query Min memory req feature failed err=%d", status);
      val_print(ACS_PRINT_WARN, "\n     *** Skipping remaining DL tests ***\n", 0);
    return ACS_STATUS_FAIL;
  }

  status = ACS_STATUS_PASS;

  val_print_test_start("Dynamic Launch");
  g_curr_module = 1 << DYNAMIC_LAUNCH_MODULE;

  status  = dl001_entry(num_pe);
  status |= dl002_entry(num_pe);
  status |= dl003_entry(num_pe);
  status |= dl004_entry(num_pe);
  status |= dl005_entry(num_pe);
  status |= dl006_entry(num_pe);
  status |= dl007_entry(num_pe);
  status |= dl008_entry(num_pe);
  status |= dl009_entry(num_pe);
  status |= dl010_entry(num_pe);
  status |= dl011_entry(num_pe);

  val_print_test_end(status, "Dynamic Launch");

  return status;
}
