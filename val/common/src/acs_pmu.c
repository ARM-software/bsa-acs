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

#include "common/include/acs_val.h"
#include "common/include/acs_pe.h"
#include "common/include/acs_pmu.h"
#include "common/include/acs_common.h"

static uint64_t ControlRegBackup;
static uint64_t CcFiltRegBackup;

/**
 * @brief   This API provides VAL interface to PMU register reads
 *
 * @param   RegId   - the pmu register index for which data is returned
 * @return  the value read from the pmu register.
 */
uint64_t
val_pmu_reg_read (
  uint32_t RegId
  )
{

    switch (RegId) {
    case PMCR_EL0:
        return AA64ReadPmcr();
    case PMCCNTR_EL0:
        return AA64ReadPmccntr();
    case PMCCFILTR_EL0:
        return AA64ReadPmccfiltr();
    case PMCNTENSET_EL0:
        return AA64ReadPmcntenset();
    default:
        val_print(ACS_PRINT_ERR, "\n FATAL - Unsupported PMU register read \n", 0);
    }

    return 0x0;
}

/**
 * @brief   This API provides VAL interface to PMU register writes
 *
 * @param   reg_id  - the pmu register index for which data is written
 * @param   write_data - the 64-bit data to write to the pmu register
 * @return  None
 */
void
val_pmu_reg_write (
  uint32_t RegId,
  uint64_t WriteData
  )
{

    switch (RegId) {
    case PMCR_EL0:
        AA64WritePmcr(WriteData);
        break;
    case PMCCNTR_EL0:
        AA64WritePmccntr(WriteData);
        break;
    case PMCCFILTR_EL0:
        AA64WritePmccfiltr(WriteData);
        break;
    case PMCNTENSET_EL0:
        AA64WritePmcntenset(WriteData);
        break;
    default:
        val_print(ACS_PRINT_ERR, "\n FATAL - Unsupported PMU register read \n", 0);
    }
}

/**
 * @brief   Configures necessary PMU registers & starts the Cycle Counter
 *
 * @param   None
 * @return  None
 */
void
val_pmu_cycle_counter_start (
  )
{

    uint64_t ControlReg;
    uint64_t CcFiltReg;
    uint64_t CcEnableSetReg;

    /* Save PMU Filter Register settings to a temp storage */
    CcFiltReg = val_pmu_reg_read(PMCCFILTR_EL0);
    CcFiltRegBackup = CcFiltReg;

    /* Save PMU Control Register settings to a temp storage */
    ControlReg = val_pmu_reg_read(PMCR_EL0);
    ControlRegBackup = ControlReg;

    /* Enable the PMU filter to count cycles in EL2 mode */
    CcFiltReg |= (1 << PMCCFILTR_NSH_EN_BIT);
    val_pmu_reg_write(PMCCFILTR_EL0, CcFiltReg);
    CcFiltReg = val_pmu_reg_read(PMCCFILTR_EL0);

    /* Enable the PMU Cycle Count Register */
    CcEnableSetReg = val_pmu_reg_read(PMCNTENSET_EL0);
    CcEnableSetReg |= (1 << PMCNTENSET_C_EN_BIT);
    val_pmu_reg_write(PMCNTENSET_EL0, CcEnableSetReg);
    CcEnableSetReg = val_pmu_reg_read(PMCNTENSET_EL0);

    /* Enable the PMU Long Cycle Counter, reset the Cycle Counter and set Global Enable */
    ControlReg |= (1 << PMCR_LC_EN_BIT) | (1 << PMCR_C_RESET_BIT) | (1 << PMCR_EN_BIT);
    val_pmu_reg_write(PMCR_EL0, ControlReg);
    ControlReg = val_pmu_reg_read(PMCR_EL0);

}

/**
 * @brief   Disables the Cycle Counter and restores the PMU reg values
 *
 * @param   None
 * @return  None
 */
void
val_pmu_cycle_counter_stop (
  )
{

    uint64_t ControlReg;
    uint64_t CcFiltReg;

    /* Disable Cycle Counter & restore PMU Control Reg settings */
    ControlReg = ControlRegBackup;
    val_pmu_reg_write(PMCR_EL0, ControlReg);

    /* Restore PMU Filter Register settings */
    CcFiltReg = CcFiltRegBackup;
    val_pmu_reg_write(PMCCFILTR_EL0, CcFiltReg);
}
