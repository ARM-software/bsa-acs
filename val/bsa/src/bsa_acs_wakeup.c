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

#include "common/include/acs_val.h"
#include "common/include/acs_pe.h"
#include "common/include/acs_common.h"
#include "common/include/acs_std_smc.h"

#include "bsa/include/bsa_acs_wakeup.h"
#include "bsa/include/bsa_val_interface.h"

extern int32_t gPsciConduit;

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

  switch (semantic) {
  case BSA_POWER_SEM_B:
      ArmCallWFI();
      break;
  default:
      break;
  }

  return 0;
}
