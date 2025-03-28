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
#include "common/include/acs_common.h"
#include "common/include/acs_mpam.h"
#include "common/include/acs_mpam_reg.h"
#include "common/include/acs_memory.h"
#include "mpam/include/mpam_val_interface.h"

/**
  @brief   This API resets the MPAMF_ESR.errcode bits. This would be used to clear
            error recognized by MSC or to clear the error interrupt.
           1. Caller       -  Error Tests
           2. Prerequisite -  None
  @param   msc_index  - index of the MSC node in the MPAM info table.
  @return  1 if successful ;  0 if error cannot be cleared.
**/
uint32_t val_mpam_msc_reset_errcode(uint32_t msc_index)
{
    uint64_t esr_value;
    uint32_t esr_errcode;

    esr_value = val_mpam_mmr_read(msc_index, REG_MPAMF_ESR);

    /* Create a mask to clear bits 24-27 */
    uint64_t mask = ~(ESR_ERRCODE_MASK << ESR_ERRCODE_SHIFT);

    /* Update ESR and write back to the register */
    esr_value &= mask;
    val_mpam_mmr_write64(msc_index, REG_MPAMF_ESR, esr_value);
    val_mem_issue_dsb();

    /* Wait for sometime before reading back the errcode */
    val_time_delay_ms(100 * ONE_MILLISECOND);
    esr_errcode = val_mpam_msc_get_errcode(msc_index);
    if (esr_errcode != ESR_ERRCODE_NO_ERROR) {
        val_print(ACS_PRINT_ERR,
            "\n       Cannot clear errorcode for MSC %d", msc_index);
        return 0;
    }

    return 1;
}

/**
  @brief   This API reads and returns the MPAMF_ESR.errcode bits
           1. Caller       -  Error Tests
           2. Prerequisite -  None
  @param   msc_index  - index of the MSC node in the MPAM info table.
  @return  MPAMF_ESR.errcode register value
**/
uint32_t val_mpam_msc_get_errcode(uint32_t msc_index)
{
    uint32_t errcode;
    uint64_t esr_value = val_mpam_mmr_read(msc_index, REG_MPAMF_ESR);

    errcode = ((esr_value >> ESR_ERRCODE_SHIFT) & ESR_ERRCODE_MASK);
    return errcode;
}

/**
  @brief   This API generates PARTID selection out-of-range error.
           Program MPAMCFG_PART_SEL with out-of-range PARTID of the MSC to trigger the error.
           1. Caller       -  Error Tests
           2. Prerequisite -  None
  @param   msc_index  - index of the MSC node in the MPAM info table.
  @return  void
**/
void val_mpam_msc_generate_psr_error(uint32_t msc_index)
{

    uint16_t max_partid;

    /* Extract max PARTID supported by this MSC */
    max_partid = val_mpam_get_max_partid(msc_index);
    val_print(ACS_PRINT_DEBUG, "\n       Max PARTID is %d", max_partid);

    /* Write (max_partid+1) to PART_SEL register to generate PSR error */
    val_mpam_mmr_write(msc_index, REG_MPAMCFG_PART_SEL, max_partid + 1);
    val_print(ACS_PRINT_DEBUG, "\n       PARTID written to MPAMCFG_PART_SEL is %d", max_partid + 1);

    val_mem_issue_dsb();
    return;
}

/**
  @brief   This API generates Monitor selection out-of-range error.
           Program MSMON_CFG_MON_SEL with out-of-range monitor index of the MSC to trigger the error
           1. Caller       -  Error Tests
           2. Prerequisite -  None
  @param   msc_index  - index of the MSC node in the MPAM info table.
  @param   msc_index  - Max monitors supported by MSC (MBWU + CSU).
  @return  void
**/
void val_mpam_msc_generate_msr_error(uint32_t msc_index, uint16_t mon_count)
{
    /* Write (mon_count+1) to MON_SEL register to generate MSR error */
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MON_SEL, mon_count + 1);
    val_print(ACS_PRINT_DEBUG, "\n       Value written to MSMON_CFG_MON_SEL is %d", mon_count + 1);

    val_mem_issue_dsb();
    return;
}

/**
  @brief   This API generates Requester PARTID out-of-range error
           Program MPAM2_EL2 with out-of-range PARTID of the MSC to trigger the error.
           1. Caller       -  Error Tests
           2. Prerequisite -  None
  @param   msc_index  - index of the MSC node in the MPAM info table.
  @return  SKIP if PE Max PARTID <= MSC Max PARTID
**/
uint32_t val_mpam_msc_generate_por_error(uint32_t msc_index)
{

    uint16_t msc_max_partid;
    uint16_t pe_max_partid;
    uint64_t mpamidr;
    uint64_t mpam2_el2 = 0;
    void *src_buf = 0;
    void *dest_buf = 0;

    /* Extract max PARTID supported by this MSC */
    msc_max_partid = val_mpam_get_max_partid(msc_index);
    val_print(ACS_PRINT_DEBUG, "\n       MSC Max PARTID is %d", msc_max_partid);

    /* Extract max PARTID supported by MPAMIDR_EL1 */
    mpamidr = AA64ReadMpamidr();
    pe_max_partid = (mpamidr >> MPAMIDR_PARTID_MAX_SHIFT) & MPAMIDR_PARTID_MAX_MASK;
    val_print(ACS_PRINT_DEBUG, "\n       PE Max PARTID is %d", pe_max_partid);

    /* Check if MSC PARTID support exceeds PE PARTID generation range */
    if (msc_max_partid >= pe_max_partid) {
        val_print(ACS_PRINT_WARN, "\n       msc_index is %d", msc_index);
        val_print(ACS_PRINT_WARN, "\n       Skipping MSC as MSC PARTID >= PE PARTID", 0);
        return ACS_STATUS_SKIP;
    }

    /* Write MSC out-of-range PARTID (msc_max_partid+1) to MPAM2_EL2 */
    mpam2_el2 = val_mpam_reg_read(MPAM2_EL2);

    /* Clear the PARTID_D bits in mpam2_el2 before writing to them */
    mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2, MPAMn_ELx_PARTID_D_SHIFT+15, MPAMn_ELx_PARTID_D_SHIFT);
    mpam2_el2 |= ((msc_max_partid + 1) << MPAMn_ELx_PARTID_D_SHIFT);
    val_mpam_reg_write(MPAM2_EL2, mpam2_el2);
    val_print(ACS_PRINT_DEBUG, "\n       Value written to MPAM2_EL2 Reg: %llx", mpam2_el2);

    /* Create buffers to perform memcopy with out-of-range PARTID */
    src_buf = (void *)val_aligned_alloc(MEM_ALIGN_4K, SIZE_1K);
    dest_buf = (void *)val_aligned_alloc(MEM_ALIGN_4K, SIZE_1K);

    /* Start mem copy transaction to generate POR error interrupt */
    val_memcpy(src_buf, dest_buf, SIZE_1K);

    return ACS_STATUS_PASS;
}

/**
  @brief   This API generates Requester PMG out-of-range error
           Program MPAM2_EL2 with out-of-range PMG of the MSC to trigger the error.
           1. Caller       -  Error Tests
           2. Prerequisite -  None
  @param   msc_index  - index of the MSC node in the MPAM info table.
  @return  SKIP if PE Max PMG <= MSC Max PMG
**/
uint32_t val_mpam_msc_generate_pmgor_error(uint32_t msc_index)
{

    uint16_t msc_max_pmg;
    uint16_t pe_max_pmg;
    uint64_t mpamidr;
    uint64_t mpam2_el2 = 0;
    void *src_buf = 0;
    void *dest_buf = 0;

    /* Extract max PMG supported by this MSC */
    msc_max_pmg = val_mpam_get_max_pmg(msc_index);
    val_print(ACS_PRINT_DEBUG, "\n       MSC Max PMG is %d", msc_max_pmg);

    /* Extract max PMG supported by MPAMIDR_EL1 */
    mpamidr = val_mpam_reg_read(MPAMIDR_EL1);
    pe_max_pmg = (mpamidr >> MPAMIDR_PMG_MAX_SHIFT) & MPAMIDR_PMG_MAX_MASK;
    val_print(ACS_PRINT_DEBUG, "\n       PE Max PMG is %d", pe_max_pmg);

    /* Check if MSC PMG support exceeds PE PMG generation range */
    if (msc_max_pmg >= pe_max_pmg) {
        val_print(ACS_PRINT_WARN, "\n       msc_index is %d", msc_index);
        val_print(ACS_PRINT_WARN, "\n       Skipping MSC as MSC pmg >= PE pmg", 0);
        return ACS_STATUS_SKIP;
    }

    mpam2_el2 = val_mpam_reg_read(MPAM2_EL2);

    /* Clear the PARTID_D & PMG_D bits in mpam2_el2 before writing to them */
    mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2, MPAMn_ELx_PARTID_D_SHIFT+15, MPAMn_ELx_PARTID_D_SHIFT);
    mpam2_el2 = CLEAR_BITS_M_TO_N(mpam2_el2, MPAMn_ELx_PMG_D_SHIFT+7, MPAMn_ELx_PMG_D_SHIFT);

    /* Write DEFAULT_PARTID & (MSC_MAX_PMG + 1) to MPAM2_EL2 and generate PE traffic */
    mpam2_el2 |= (((uint64_t)(msc_max_pmg+1) << MPAMn_ELx_PMG_D_SHIFT) |
                  ((uint64_t)DEFAULT_PARTID << MPAMn_ELx_PARTID_D_SHIFT));

    /* Write MSC out-of-range PMG (msc_max_pmg+1) to MPAM2_EL2 */
    val_mpam_reg_write(MPAM2_EL2, mpam2_el2);
    val_print(ACS_PRINT_DEBUG, "\n       Value written to MPAM2_EL2 Reg: %llx", mpam2_el2);

    /* Create buffers to perform memcopy with out-of-range PMG */
    src_buf = (void *)val_aligned_alloc(MEM_ALIGN_4K, SIZE_1K);
    dest_buf = (void *)val_aligned_alloc(MEM_ALIGN_4K, SIZE_1K);

    /* Start mem copy transaction to generate PMGOR error interrupt */
    val_memcpy(src_buf, dest_buf, SIZE_1K);
    return ACS_STATUS_PASS;
}

/**
  @brief   This API generates MSC monitor selection range error
           Program MSMON_CFG_* registers with out-of-range PARTID.
           1. Caller       -  Error Tests
           2. Prerequisite -  None
  @param   msc_index  - index of the MSC node in the MPAM info table.
  @return  None
**/
void val_mpam_msc_generate_msmon_config_error(uint32_t msc_index, uint16_t mon_count)
{

    uint16_t max_partid;

    /* Extract max PARTID supported by this MSC */
    max_partid = val_mpam_get_max_partid(msc_index);
    val_print(ACS_PRINT_DEBUG, "\n       MSC Max PARTID is %d", max_partid);

    /* Write (mon_count-1) to MON_SEL register to configure the last monitor */
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MON_SEL, mon_count - 1);

    if (val_mpam_supports_csumon(msc_index)) {
        /* Write (max_partid+1) to CSU monitor filter reg to raise monitor config error */
        val_mpam_mmr_write(msc_index, REG_MSMON_CFG_CSU_FLT, max_partid + 1);
        return;
    }

    if (val_mpam_msc_supports_mbwumon(msc_index)) {
        /* Write (max_partid+1) to MBW monitor filter reg to raise monitor config error */
        val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MBWU_FLT, max_partid + 1);
        return;
    }

    val_mem_issue_dsb();
    return;
}

/**
  @brief   This API generates MBWU Monitor overflow.
           1. Caller       -  Error Tests
           2. Prerequisite -  None
  @param   msc_index  - index of the MSC node in the MPAM info table.
  @param   mon_count  - monitors supported by MSC
  @return  None
**/
void val_mpam_msc_generate_msmon_oflow_error(uint32_t msc_index, uint16_t mon_count)
{

    uint64_t max_count = 0;
    uint64_t nrdy_timeout;

    /* Set interrupt enable bit in MPAMF_ECR */
    val_mpam_mmr_write(msc_index, REG_MPAMF_ECR, (1 << ECR_ENABLE_INTEN_SHIFT));

    /* Write mon_count to MON_SEL register to configure the last monitor */
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MON_SEL, mon_count);

    /* Configure monitor filter reg for default partid and default pmg */
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MBWU_FLT,
                                    ((DEFAULT_PMG << MBWU_FLT_PMG_SHIFT) | DEFAULT_PARTID));

    /* Configure monitor ctrl reg for default partid and pmg to generate a oflow interrupt */
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MBWU_CTL, (
                                                    (1 << MBWU_CTL_OFLOW_INTR_L_SHIFT) |
                                                    (1 << MBWU_CTL_ENABLE_MATCH_PARTID_SHIFT) |
                                                    (1 << MBWU_CTL_ENABLE_MATCH_PMG_SHIFT) |
                                                    (1 << MBWU_CTL_ENABLE_OFLOW_INTR_SHIFT)
                                                   ));

    /* Write the maximum memory bandwidth usage value to MSMON_MBWU register */
    /*if MSMON_MBWU_L is implemented*/
    if (val_mpam_mbwu_supports_long(msc_index)) {
        if (val_mpam_mbwu_supports_lwd(msc_index)) {
            max_count = MSMON_COUNT_63BIT;  // (63 bits)
            val_mpam_mmr_write64(msc_index, REG_MSMON_MBWU_L, ((0 << MBWU_NRDY_SHIFT) | max_count));
        }
        else
        {
            max_count = MSMON_COUNT_44BIT;  // (44 bits)
            val_mpam_mmr_write64(msc_index, REG_MSMON_MBWU_L, ((0 << MBWU_NRDY_SHIFT) | max_count));
        }
    }
    else {
            max_count = MSMON_COUNT_31BIT;  // (31 bits)
            val_mpam_mmr_write(msc_index, REG_MSMON_MBWU, ((0 << MBWU_NRDY_SHIFT) | max_count));
    }

    val_mem_issue_dsb();

    nrdy_timeout = val_mpam_get_info(MPAM_MSC_NRDY, msc_index, 0);
    while (nrdy_timeout) {
        --nrdy_timeout;
    };

    return;
}

/**
  @brief   This API writes non-zero values to MPAMF_ESR.errcode
           1. Caller       -  Error Tests
           2. Prerequisite -  None
  @param   msc_index  - index of the MSC node in the MPAM info table.
  @return  None
**/
void val_mpam_msc_trigger_intr(uint32_t msc_index)
{
    /* Set interrupt enable bit in MPAMF_ECR */
    val_mpam_mmr_write(msc_index, REG_MPAMF_ECR, (1 << ECR_ENABLE_INTEN_SHIFT));

    /* Trigger error interrupt by writing non-zero to MPAMF_ESR.ERRCODE */
    val_mpam_mmr_write(msc_index, REG_MPAMF_ESR, (1 << ESR_ERRCODE_SHIFT));

    val_mem_issue_dsb();
    return;
}
