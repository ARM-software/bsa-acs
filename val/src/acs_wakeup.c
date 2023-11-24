/** @file
 * Copyright (c) 2016-2018, 2021-2023, Arm Limited or its affiliates. All rights reserved.
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

#include "include/bsa_acs_val.h"
#include "include/bsa_acs_pe.h"
#include "include/bsa_acs_common.h"
#include "include/bsa_std_smc.h"

#include "include/bsa_acs_wakeup.h"

extern int32_t gPsciConduit;

/**
  @brief   This API executes all the wakeup tests sequentially
           1. Caller       -  Application layer.
           2. Prerequisite -  None
  @param   num_pe - the number of PE to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_wakeup_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{
  uint32_t status, i;

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
      val_print(ACS_PRINT_ERR, "\nOperating System View:\n", 0);
      status |= os_u001_entry(num_pe);

 /*B_WAK_09 is required only for SBSA complaince
if (g_build_sbsa) {
      // Test needs multi-PE interrupt handling support
     // status |= os_u002_entry(num_pe);
}
*/
  }

  val_print_test_end(status, "Wakeup");

  return status;

}

/**
  @brief  This API is used to get PSCI Version

  @param  None

  @return PSCI Version
**/
static uint32_t val_get_psci_ver(void)
{
  ARM_SMC_ARGS smc_args;

  smc_args.Arg0 = ARM_SMC_ID_PSCI_VERSION;

  pal_pe_call_smc(&smc_args, gPsciConduit);

  val_print(ACS_PRINT_DEBUG, "\n       PSCI VERSION = %X", smc_args.Arg0);

  return smc_args.Arg0;
}

/**
  @brief  This API is used to get PSCI features

  @param  psci_func_id  Function ID

  @return PSCI features
**/
static uint32_t val_get_psci_features(uint64_t psci_func_id)
{
  ARM_SMC_ARGS smc_args;

  smc_args.Arg0 = ARM_SMC_ID_PSCI_FEATURES;
  smc_args.Arg1 = psci_func_id;

  pal_pe_call_smc(&smc_args, gPsciConduit);

  val_print(ACS_PRINT_DEBUG, "\n       PSCI FEATURS = %d", smc_args.Arg0);

  return smc_args.Arg0;
}

/**
  @brief  This API initiates a Power state Suspend sequence by calling SUSPEND PSCI call

  @entry              - See PSCI specification
  @context_id         - See PSCI specification
  @return               status of PSCI call.
**/
int
val_suspend_pe(uint64_t entry, uint32_t context_id)
{
  ARM_SMC_ARGS smc_args;
  int psci_major_ver, pwr_state_fmt;
  uint32_t power_state;

  psci_major_ver = (val_get_psci_ver() >> 16);
  val_print(ACS_PRINT_DEBUG, "\n       PSCI MAJOR VERSION = %X", psci_major_ver);
  if (psci_major_ver < 1)
    power_state = 0;
  else {
      pwr_state_fmt = (val_get_psci_features(ARM_SMC_ID_PSCI_CPU_SUSPEND_AARCH64) >> 1);
      val_print(ACS_PRINT_DEBUG, "\n       PSCI PWR_STATE_FMT = %d                ",
                                                                pwr_state_fmt);
      if (pwr_state_fmt == ARM_SMC_ID_PSCI_POWER_STATE_FMT_ORIGINAL)
        power_state = 0;
      else
        power_state = 1;
  }

  smc_args.Arg0 = ARM_SMC_ID_PSCI_CPU_SUSPEND_AARCH64;
  smc_args.Arg1 = power_state;
  smc_args.Arg2 = entry;
  smc_args.Arg3 = context_id;
  pal_pe_call_smc(&smc_args, gPsciConduit);

  return smc_args.Arg0;
}


/**
  @brief   Common API to initiate any Low power state entry
  @param   semantic  - See BSA specification

  @return  always 0 - not used for now
**/
uint32_t
val_power_enter_semantic(BSA_POWER_SEM_e semantic)
{

  switch(semantic) {
      case BSA_POWER_SEM_B:
          ArmCallWFI();
          break;
      default:
          break;
  }

  return 0;
}
