/** @file
 * Copyright (c) 2016-2023, Arm Limited or its affiliates. All rights reserved.
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


/**
  @brief   Pointer to the memory location of the PE Information table
**/
extern PE_INFO_TABLE *g_pe_info_table;
/**
  @brief   global structure to pass and retrieve arguments for the SMC call
**/
extern ARM_SMC_ARGS g_smc_args;


/**
  @brief   This API will execute all PE tests designated for a given compliance level
           1. Caller       -  Application layer.
           2. Prerequisite -  val_pe_create_info_table, val_allocate_shared_mem
  @param   num_pe - the number of PE to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_pe_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{
  uint32_t status, i;

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
      val_print(ACS_PRINT_ERR, "\nOperating System View:\n", 0);
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
  }

  if (g_sw_view[G_SW_HYP]) {
      val_print(ACS_PRINT_ERR, "\nHypervisor View:\n", 0);
      status |= hyp_c001_entry(num_pe);
      status |= hyp_c002_entry(num_pe);
      status |= hyp_c003_entry(num_pe);
      status |= hyp_c004_entry(num_pe);
      status |= hyp_c005_entry(num_pe);
  }

  if (g_sw_view[G_SW_PS]) {
      val_print(ACS_PRINT_ERR, "\nPlatform Security View:\n", 0);
      status |= ps_c001_entry(num_pe);
  }

  val_print_test_end(status, "PE");

  return status;

}

/**
  @brief   This API provides a 'C' interface to call System register reads
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   reg_id  - the system register index for which data is returned
  @return  the value read from the system register.
**/
uint64_t
val_pe_reg_read(uint32_t reg_id)
{

  switch(reg_id) {
      case MPIDR_EL1:
          return ArmReadMpidr();
      case ID_AA64PFR0_EL1:
          return ArmReadIdPfr0();
      case ID_AA64PFR1_EL1:
          return ArmReadIdPfr1();
      case ID_AA64MMFR0_EL1:
          return AA64ReadMmfr0();
      case ID_AA64MMFR1_EL1:
          return AA64ReadMmfr1();
      case ID_AA64MMFR2_EL1:
          return AA64ReadMmfr2();
      case CTR_EL0:
          return AA64ReadCtr();
      case ID_AA64ISAR0_EL1:
          return AA64ReadIsar0();
      case ID_AA64ISAR1_EL1:
          return AA64ReadIsar1();
      case SCTLR_EL3:
          return AA64ReadSctlr3();
      case SCTLR_EL2:
          return AA64ReadSctlr2();
      case SCTLR_EL1:
          return AA64ReadSctlr1();
      case PMCR_EL0:
          return AA64ReadPmcr();
      case ID_AA64DFR0_EL1:
          return AA64ReadIdDfr0();
      case ID_AA64DFR1_EL1:
          return AA64ReadIdDfr1();
      case CurrentEL:
          return AA64ReadCurrentEL();
      case MDCR_EL2:
          return AA64ReadMdcr2();
      case VBAR_EL2:
          return AA64ReadVbar2();
      case CCSIDR_EL1:
          return AA64ReadCcsidr();
      case CSSELR_EL1:
          return AA64ReadCsselr();
      case CLIDR_EL1:
          return AA64ReadClidr();
      case ID_DFR0_EL1:
          return ArmReadDfr0();
      case ID_ISAR0_EL1:
          return ArmReadIsar0();
      case ID_ISAR1_EL1:
          return ArmReadIsar1();
      case ID_ISAR2_EL1:
          return ArmReadIsar2();
      case ID_ISAR3_EL1:
          return ArmReadIsar3();
      case ID_ISAR4_EL1:
          return ArmReadIsar4();
      case ID_ISAR5_EL1:
          return ArmReadIsar5();
      case ID_MMFR0_EL1:
          return ArmReadMmfr0();
      case ID_MMFR1_EL1:
          return ArmReadMmfr1();
      case ID_MMFR2_EL1:
          return ArmReadMmfr2();
      case ID_MMFR3_EL1:
          return ArmReadMmfr3();
      case ID_MMFR4_EL1:
          return ArmReadMmfr4();
      case ID_PFR0_EL1:
          return ArmReadPfr0();
      case ID_PFR1_EL1:
          return ArmReadPfr1();
      case MIDR_EL1:
          return ArmReadMidr();
      case MVFR0_EL1:
          return ArmReadMvfr0();
      case MVFR1_EL1:
          return ArmReadMvfr1();
      case MVFR2_EL1:
          return ArmReadMvfr2();
      case PMCEID0_EL0:
          return AA64ReadPmceid0();
      case PMCEID1_EL0:
          return AA64ReadPmceid1();
      case VMPIDR_EL2:
          return AA64ReadVmpidr();
      case VPIDR_EL2:
          return AA64ReadVpidr();
      case PMBIDR_EL1:
          return AA64ReadPmbidr();
      case PMSIDR_EL1:
          return AA64ReadPmsidr();
      case LORID_EL1:
          return AA64ReadLorid();
      case ERRIDR_EL1:
          return AA64ReadErridr();
      case ERR0FR_EL1:
          return AA64ReadErr0fr();
      case ERR1FR_EL1:
          return AA64ReadErr1fr();
      case ERR2FR_EL1:
          return AA64ReadErr2fr();
      case ERR3FR_EL1:
          return AA64ReadErr3fr();
      case ESR_EL2:
          return AA64ReadEsr2();
      case FAR_EL2:
          return AA64ReadFar2();
      case RDVL:
          return ArmRdvl();
      case MAIR_ELx:
          if (AA64ReadCurrentEL() == AARCH64_EL1)
            return AA64ReadMair1();
          if (AA64ReadCurrentEL() == AARCH64_EL2)
            return AA64ReadMair2();
          break;
      case TCR_ELx:
          if (AA64ReadCurrentEL() == AARCH64_EL1)
            return AA64ReadTcr1();
          if (AA64ReadCurrentEL() == AARCH64_EL2)
            return AA64ReadTcr2();
          break;
      case DBGBCR0_EL1:
          return AA64ReadDbgbcr0El1();
      case DBGBCR1_EL1:
          return AA64ReadDbgbcr1El1();
      case DBGBCR2_EL1:
          return AA64ReadDbgbcr2El1();
      case DBGBCR3_EL1:
          return AA64ReadDbgbcr3El1();
      case DBGBCR4_EL1:
          return AA64ReadDbgbcr4El1();
      case DBGBCR5_EL1:
          return AA64ReadDbgbcr5El1();
      case DBGBCR6_EL1:
          return AA64ReadDbgbcr6El1();
      case DBGBCR7_EL1:
          return AA64ReadDbgbcr7El1();
      case DBGBCR8_EL1:
          return AA64ReadDbgbcr8El1();
      case DBGBCR9_EL1:
          return AA64ReadDbgbcr9El1();
      case DBGBCR10_EL1:
          return AA64ReadDbgbcr10El1();
      case DBGBCR11_EL1:
          return AA64ReadDbgbcr11El1();
      case DBGBCR12_EL1:
          return AA64ReadDbgbcr12El1();
      case DBGBCR13_EL1:
          return AA64ReadDbgbcr13El1();
      case DBGBCR14_EL1:
          return AA64ReadDbgbcr14El1();
      case DBGBCR15_EL1:
          return AA64ReadDbgbcr15El1();
      default:
           val_report_status(val_pe_get_index_mpid(val_pe_get_mpid()),
                                                 RESULT_FAIL(0, 0xFF), NULL);
           break;
  }

  return 0x0;
}

/**
  @brief   This API provides a 'C' interface to call System register writes
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   reg_id  - the system register index for which data is written
  @param   write_data - the 64-bit data to write to the system register
  @return  None
**/
void
val_pe_reg_write(uint32_t reg_id, uint64_t write_data)
{

  switch(reg_id) {
      case CSSELR_EL1:
          AA64WriteCsselr(write_data);
          break;
      case PMCR_EL0:
          AA64WritePmcr(write_data);
          break;
      case PMOVSSET_EL0:
          AA64WritePmovsset(write_data);
          break;
      case PMOVSCLR_EL0:
          AA64WritePmovsclr(write_data);
          break;
      case PMINTENSET_EL1:
          AA64WritePmintenset(write_data);
          break;
      case PMINTENCLR_EL1:
          AA64WritePmintenclr(write_data);
          break;
      case MDCR_EL2:
          AA64WriteMdcr2(write_data);
          break;
      case VBAR_EL2:
          AA64WriteVbar2(write_data);
          break;
      case PMSIRR_EL1:
          AA64WritePmsirr(write_data);
          break;
      case PMSCR_EL2:
          AA64WritePmscr2(write_data);
          break;
      case PMSFCR_EL1:
          AA64WritePmsfcr(write_data);
          break;
      case PMBPTR_EL1:
          AA64WritePmbptr(write_data);
          break;
      case PMBLIMITR_EL1:
          AA64WritePmblimitr(write_data);
          break;
      default:
           val_report_status(val_pe_get_index_mpid(val_pe_get_mpid()),
                                                  RESULT_FAIL(0, 0xFF), NULL);
  }

}

/**
  @brief   This API indicates the presence of exception level 3
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   None
  @return  1 if EL3 is present, 0 if EL3 is not implemented
**/
uint8_t
val_is_el3_enabled()
{
  uint64_t data;
  data = val_pe_reg_read(ID_AA64PFR0_EL1);
  return ((data >> 12) & 0xF);

}

/**
  @brief   This API indicates the presence of exception level 2
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   None
  @return  1 if EL2 is present, 0 if EL2 is not implemented
**/
uint8_t
val_is_el2_enabled()
{

  uint64_t data;
  data = val_pe_reg_read(ID_AA64PFR0_EL1);
  return ((data >> 8) & 0xF);

}


/**
  @brief   This API returns the PMU Overflow Signal Interrupt ID for a given PE index
           1. Caller       -  Test Suite, VAL
           2. Prerequisite -  val_create_peinfo_table
  @param   index - the index of PE whose PMU interrupt ID is returned.
  @return  PMU interrupt id
**/
uint32_t
val_pe_get_pmu_gsiv(uint32_t index)
{

  PE_INFO_ENTRY *entry;

  if (index > g_pe_info_table->header.num_of_pe) {
        val_report_status(index, RESULT_FAIL(0, 0xFF), NULL);
        return 0xFFFFFF;
  }

  entry = g_pe_info_table->pe_info;

  return entry[index].pmu_gsiv;

}

/**
  @brief   This API returns the GIC Maintenance Interrupt ID for a given PE index
           1. Caller       -  Test Suite
           2. Prerequisite -  val_create_peinfo_table
  @param   index - the index of PE whose GIC Maintenance interrupt ID is to be returned.
  @return  GIC Maintenance interrupt id
**/
uint32_t
val_pe_get_gmain_gsiv(uint32_t index)
{

  PE_INFO_ENTRY *entry;

  if (index > g_pe_info_table->header.num_of_pe) {
        val_report_status(index, RESULT_FAIL(0, 0xFF), NULL);
        return 0xFFFFFF;
  }

  entry = g_pe_info_table->pe_info;

  return entry[index].gmain_gsiv;

}

/**
  @brief   This API will call an assembly sequence with interval
           as argument over which an SPE event is exected to be generated.
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   interval - The interval after completion of which SPE event
                      would be generated
  @param   address  - Address on which to trigger the SPE
  @return  None.
**/
void
val_pe_spe_program_under_profiling(uint64_t interval, addr_t address)
{
  SpeProgramUnderProfiling(interval, address);
}

/**
  @brief   This API disables the SPE interrupt generation logic.
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   None
  @return  None
**/
void
val_pe_spe_disable(void)
{
  DisableSpe();
}

/**
  @brief   This API reads the TCR register and fills info to structure.

  @param   ttbr1  If ttbr1 is used
  @param   *tcr   To fill the TCR information

  @return  Status 0 if Success
**/
uint32_t val_pe_reg_read_tcr(uint32_t ttbr1, PE_TCR_BF *tcr)
{
    uint64_t val = val_pe_reg_read(TCR_ELx);
    uint32_t el = AA64ReadCurrentEL() & AARCH64_EL_MASK;
    uint8_t tg_ttbr0[3] = {12 /*4KB*/, 16 /*64KB*/, 14 /*16KB*/};
    uint8_t tg_ttbr1[4] = {0 /* N/A */, 14 /*16KB*/, 12 /*4KB*/, 16 /* 64KB*/};
    uint64_t e2h = 0;

    if ((tcr == NULL) ||
        (el != AARCH64_EL1 && el != AARCH64_EL2))
        return ACS_STATUS_ERR;

    if (el == AARCH64_EL2)
        e2h = ArmReadHcr() & AARCH64_HCR_E2H_MASK;

    if (el == AARCH64_EL1 || (el == AARCH64_EL2 && e2h))
    {
        tcr->ps = (val & BSA_TCR_IPS_MASK) >> BSA_TCR_IPS_SHIFT;
        if (ttbr1) {
            tcr->tg = (val & BSA_TCR_TG1_MASK) >> BSA_TCR_TG1_SHIFT;
            if (tcr->tg == 0 || tcr->tg > 3)
                return ACS_STATUS_ERR;
            tcr->tg_size_log2 = tg_ttbr1[tcr->tg];
            tcr->sh = (val & BSA_TCR_SH1_MASK) >> BSA_TCR_SH1_SHIFT;
            tcr->orgn = (val & BSA_TCR_ORGN1_MASK) >> BSA_TCR_ORGN1_SHIFT;
            tcr->irgn = (val & BSA_TCR_IRGN1_MASK) >> BSA_TCR_IRGN1_SHIFT;
            tcr->tsz = (val & BSA_TCR_T1SZ_MASK) >> BSA_TCR_T1SZ_SHIFT;
            return 0;
        }
    }
    else if (!ttbr1)
        tcr->ps = (val & BSA_TCR_PS_MASK) >> BSA_TCR_PS_SHIFT;
    else
        return ACS_STATUS_ERR;

    tcr->tg = (val & BSA_TCR_TG0_MASK) >> BSA_TCR_TG0_SHIFT;
    if (tcr->tg > 2)
        return ACS_STATUS_ERR;
    tcr->tg_size_log2 = tg_ttbr0[tcr->tg];
    tcr->sh = (val & BSA_TCR_SH0_MASK) >> BSA_TCR_SH0_SHIFT;
    tcr->orgn = (val & BSA_TCR_ORGN0_MASK) >> BSA_TCR_ORGN0_SHIFT;
    tcr->irgn = (val & BSA_TCR_IRGN0_MASK) >> BSA_TCR_IRGN0_SHIFT;
    tcr->tsz = (val & BSA_TCR_T0SZ_MASK) >> BSA_TCR_T0SZ_SHIFT;
    return 0;
}

/**
  @brief   This API reads the TTBR register and fills info to structure.

  @param   ttbr1       If ttbr1 is used
  @param   *ttbr_ptr   To fill the TTBR information

  @return  Status 0 if Success
**/
uint32_t val_pe_reg_read_ttbr(uint32_t ttbr1, uint64_t *ttbr_ptr)
{
    uint32_t el = AA64ReadCurrentEL() & AARCH64_EL_MASK;
    typedef uint64_t (*ReadTtbr_t)(void);
    ReadTtbr_t ReadTtbr[2][2] = {{AA64ReadTtbr0El1, AA64ReadTtbr0El2},
                                  {AA64ReadTtbr1El1, AA64ReadTtbr1El2}};

    if ((ttbr_ptr == NULL) ||
        (el != AARCH64_EL1 && el != AARCH64_EL2) ||
        ttbr1 > 1)
        return ACS_STATUS_ERR;

    *ttbr_ptr = ReadTtbr[ttbr1][(el >> 2) - 1]();
    return 0;
}
