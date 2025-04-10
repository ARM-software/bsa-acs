/** @file
 * Copyright (c) 2016-2025, Arm Limited or its affiliates. All rights reserved.
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


/**
  @brief   Pointer to the memory location of the PE Information table
**/
extern PE_INFO_TABLE *g_pe_info_table;
/**
  @brief   global structure to pass and retrieve arguments for the SMC call
**/
extern ARM_SMC_ARGS g_smc_args;

/**
  @brief   Pointer to the memory location of the cache Information table
**/
CACHE_INFO_TABLE *g_cache_info_table;

/**
  @brief   This API will call PAL layer to fill in the PPTT ACPI table information
           into the g_cache_info_table pointer.
           1. Caller       -  Application layer.
           2. Prerequisite -  Memory allocated and passed as argument.
  @param   cache_info_table  pre-allocated memory pointer for cache info.
  @return  Error if Input parameter is NULL
**/
void
val_cache_create_info_table(uint64_t *cache_info_table)
{
  if (cache_info_table == NULL) {
      val_print(ACS_PRINT_ERR, "\n   Pre-allocated memory pointer is NULL\n", 0);
      return;
  }

  g_cache_info_table = (CACHE_INFO_TABLE *)cache_info_table;
#ifndef TARGET_LINUX
  pal_cache_create_info_table(g_cache_info_table, g_pe_info_table);

  if (g_cache_info_table->num_of_cache != 0) {
      val_print(ACS_PRINT_TEST,
                " CACHE_INFO: Number of cache nodes    : %4d\n",
                g_cache_info_table->num_of_cache);
  }

#endif
}

/**
  @brief   This API frees the memory allocated for cache info table.
  @param   None
  @return  None
**/

void
val_cache_free_info_table(void)
{
    if (g_cache_info_table != NULL) {
        pal_mem_free_aligned((void *)g_cache_info_table);
        g_cache_info_table = NULL;
    }
    else {
      val_print(ACS_PRINT_ERR,
                  "\n WARNING: g_cache_info_table pointer is already NULL",
        0);
    }
}

/**
  @brief  This API returns info of the cache indexed in cache info table.
  @param type - requested info type.
  @param cache_index - index of the cache in cache info table.
  @return info value in 64-bit unsigned int if success,
          else returns INVALID_CACHE_INFO indicating failure.
**/
uint64_t
val_cache_get_info(CACHE_INFO_e type, uint32_t cache_index)
{
  CACHE_INFO_ENTRY *entry;
  char *cache_info_type[] = {"cache_type", "cache_size", "cache_identifier"};

  if (cache_index >= g_cache_info_table->num_of_cache) {
      val_print(ACS_PRINT_ERR, "\n       invalid cache index: %d", cache_index);
      return 0;
  }
  entry = &(g_cache_info_table->cache_info[cache_index]);
  switch (type) {
  case CACHE_TYPE:
      if (entry->flags.cache_type_valid)
          return entry->cache_type;
      break;
  case CACHE_SIZE:
      if (entry->flags.size_property_valid)
          return entry->size;
      break;
  case CACHE_ID:
      if (entry->flags.cache_id_valid)
          return entry->cache_id;
      break;
  case CACHE_NEXT_LEVEL_IDX:
      return entry->next_level_index;
  case CACHE_PRIVATE_FLAG:
      return entry->is_private;
  default:
      val_print(ACS_PRINT_ERR,
                "\n      cache option not supported %d\n", type);
      return INVALID_CACHE_INFO;
  }

  val_print(ACS_PRINT_ERR,
   "\n       cache %d has invalid ", cache_index);
  val_print(ACS_PRINT_ERR, cache_info_type[type], 0);
  return INVALID_CACHE_INFO;
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
      case ID_AA64ISAR2_EL1:
          return AA64ReadIsar2();
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
      case ID_AA64ZFR0_EL1:
          return AA64ReadZfr0();
      case BRBIDR0_EL1:
          return AA64ReadBrbidr0();
      case TRBIDR_EL1:
          return AA64ReadTrbidr();
      case TRCIDR0:
          return AA64ReadTrcidr0();
       case TRCIDR4:
          return AA64ReadTrcidr4();
       case TRCIDR5:
          return AA64ReadTrcidr5();
       case HCR_EL2:
          return ArmReadHcr();
       case VTCR_EL2:
          return AA64ReadVtcr();
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
  @brief   This API returns the GIC TRBE Interrupt ID for a given PE index
           1. Caller       -  Test Suite
           2. Prerequisite -  val_create_peinfo_table
  @param   index - the index of PE whose GIC TRBE interrupt ID is returned.
  @return  GIC TRBE interrupt ID
**/
uint32_t
val_pe_get_gicc_trbe_interrupt(uint32_t index)
{

  PE_INFO_ENTRY *entry;

  if (index > g_pe_info_table->header.num_of_pe) {
        val_report_status(index, RESULT_FAIL(0, 0xFF), NULL);
        return 0xFFFFFF;
  }

  entry = g_pe_info_table->pe_info;

  return entry[index].trbe_interrupt;

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
  @brief   This API reads the TCR register and fills info to structure.

  @param   ttbr1  If ttbr1 is used
  @param   *tcr   To fill the TCR information

  @return  Status 0 if Success
**/
void
val_pe_spe_disable(void)
{
  DisableSpe();
}

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

/**
  @brief  This API returns index of last-level cache in cache info table
          for the current PE.

  @return index of the last-level cache.
**/
uint32_t
val_cache_get_llc_index(void)
{
  uint32_t curr_cache_idx;
  uint32_t next_lvl_idx;
  uint32_t llc_idx = CACHE_INVALID_IDX;
  if (g_cache_info_table->num_of_cache) {
      /* get first level private cache index for current PE */
      /* setting res_index to 0 since PE should have atleast one L1 cache */
      curr_cache_idx = val_cache_get_pe_l1_cache_res(0);

      /* get to last level cache in the cache info chain */
      while (curr_cache_idx != CACHE_INVALID_NEXT_LVL_IDX) {
        /* check if next level cache is present */
        next_lvl_idx = val_cache_get_info(CACHE_NEXT_LEVEL_IDX, curr_cache_idx);
        if (next_lvl_idx == CACHE_INVALID_NEXT_LVL_IDX) {
            llc_idx = curr_cache_idx;
            break;
        }
        else
            curr_cache_idx = next_lvl_idx;
      }

      return llc_idx;
  }
  else {
      val_print(ACS_PRINT_DEBUG, "\n       CACHE INFO table invalid", 0);
      return CACHE_TABLE_EMPTY;
  }
}

/**
  @brief  This API returns level 1 cache index for resource index requested.
  @param res_index level 1 private resource index.
  @return return index of cache in the cache info table.
**/
uint32_t
val_cache_get_pe_l1_cache_res(uint32_t res_index)
{
  PE_INFO_ENTRY *entry;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  entry = &(g_pe_info_table->pe_info[index]);
  if (res_index < MAX_L1_CACHE_RES)
      return entry->level_1_res[res_index];
  else {
      val_print(ACS_PRINT_ERR,
               "\n   Requested resource index exceeds maximum index value %d\n", MAX_L1_CACHE_RES);
      return DEFAULT_CACHE_IDX;
  }
}

/**
  @brief   This API checks whether the requested PE feature is implemented or not.
  @param   pe_feature - PE feature to be checked.
  @return  ACS_STATUS_PASS if implemented., else ACS_STATUS_FAIL.
**/
uint32_t val_pe_feat_check(PE_FEAT_NAME pe_feature)
{
    uint64_t data;

    switch (pe_feature) {
    case PE_FEAT_MPAM:
        /* ID_AA64PFR0_EL1.MPAM bits[43:40] > 0 or ID_AA64PFR1_EL1.MPAM_frac bits[19:16] > 0
        indicates implementation of MPAM extension */
        if ((VAL_EXTRACT_BITS(val_pe_reg_read(ID_AA64PFR0_EL1), 40, 43) > 0) ||
        (VAL_EXTRACT_BITS(val_pe_reg_read(ID_AA64PFR1_EL1), 16, 19) > 0))
            return ACS_STATUS_PASS;
        else
            return ACS_STATUS_FAIL;
    case PE_FEAT_PMU:
        /* ID_AA64DFR0_EL1.PMUVer, bits [11:8] == 0000 or 1111
           indicate PMU extension not implemented */
        data = VAL_EXTRACT_BITS(val_pe_reg_read(ID_AA64DFR0_EL1), 8, 11);
        if (!(data == 0 || data == 0xF))
            return ACS_STATUS_PASS;
        else
            return ACS_STATUS_FAIL;
    case PE_FEAT_RAS:
        /*  ID_AA64PFR0_EL1 RAS bits [31:28] != 0 indicate RAS extension implemented */
        if ((VAL_EXTRACT_BITS(val_pe_reg_read(ID_AA64PFR0_EL1), 28, 31)) != 0)
            return ACS_STATUS_PASS;
        else
            return ACS_STATUS_FAIL;
    default:
        val_print(ACS_PRINT_ERR, "\nPE_FEAT_CHECK: Invalid PE feature", 0);
        return ACS_STATUS_FAIL;
    }
}
