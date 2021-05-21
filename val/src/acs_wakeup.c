/** @file
 * Copyright (c) 2016-2018,2021 Arm Limited or its affiliates. All rights reserved.
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

/**
  @brief   This API executes all the wakeup tests sequentially
           1. Caller       -  Application layer.
           2. Prerequisite -  None
  @param   num_pe - the number of PE to run these tests on.
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_wakeup_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{
  uint32_t status, i;

  status = ACS_STATUS_PASS;

  for (i=0 ; i<MAX_TEST_SKIP_NUM ; i++){
      if (g_skip_test_num[i] == ACS_WAKEUP_TEST_NUM_BASE) {
          val_print(ACS_PRINT_TEST, "\n      USER Override - Skipping all Wakeup tests \n", 0);
          return ACS_STATUS_SKIP;
      }
  }

  if (g_sw_view[G_SW_OS]) {
      val_print(ACS_PRINT_ERR, "\nOperating System:\n", 0);
      status |= os_u001_entry(num_pe);
     // Test needs multi-PE interrupt handling support
     // status |= os_u002_entry(num_pe);
  }

  if (status != ACS_STATUS_PASS)
    val_print(ACS_PRINT_TEST, "\n      *** One or more tests have Failed/Skipped.*** \n", 0);
  else
    val_print(ACS_PRINT_TEST, "\n      All Wakeup tests passed!! \n", 0);

  return status;

}


/**
  @brief  This API initiates a Power state Suspend sequence by calling SUSPEND PSCI call

  @param power_state  - See PSCI specification
  @entry              - See PSCI specification
  @context_id         - See PSCI specification
**/
void
val_suspend_pe(uint32_t power_state, uint64_t entry, uint32_t context_id)
{
  ARM_SMC_ARGS smc_args;

  smc_args.Arg0 = ARM_SMC_ID_PSCI_CPU_SUSPEND_AARCH64;
  smc_args.Arg1 = power_state;
  smc_args.Arg2 = entry;
  smc_args.Arg3 = context_id;
  pal_pe_call_smc(&smc_args);
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
