/** @file
 * DRTM API
 *
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
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

#include "common/include/val_interface.h"
#include "common/include/acs_common.h"
#include "common/include/acs_val.h"
#include "common/include/acs_pe.h"
#include "common/include/pal_interface.h"
#include "common/include/acs_std_smc.h"
#include "common/include/acs_memory.h"

#include "../include/drtm_val_interface.h"
#include "../include/drtm_pal_interface.h"

/* g_drtm_features structure is a global structure */
DRTM_ACS_FEATURES g_drtm_features;

/* DL simulation */
DRTM_ACS_DL_SAVED_STATE *g_drtm_acs_dl_saved_state;
DRTM_ACS_DL_RESULT      *g_drtm_acs_dl_result;

/* Assembly code coresponding to below opcode is added as part
 * of comments, last 4 32-Bytes are reserved to save the address
 * of g_drtm_acs_dl_result & g_drtm_acs_dl_saved_state
 * This will be used to store x0 & x1 in DLME Image
*/
uint32_t                g_drtm_acs_dlme[] = {
    0x58000489,  // ldr x9,  =g_drtm_acs_dl_result
    0xF9000120,  // str x0,  [x9]
    0xF9000521,  // str x1,  [x9, #8]
    0x58000469,  // ldr x9,  =g_drtm_acs_dl_saved_state
    0xD50C871F,  // tlbi alle2
    0xD5033B9F,  // dsb ish
    0xD5033FDF,  // isb
    0xD53C1000,  // mrs x0, sctlr_el2
    0xB2400000,  // orr x0, x0, 0x1
    0xB27E0000,  // orr x0, x0, 0x4
    0xB2740000,  // orr x0, x0, 0x1000
    0xD51C1000,  // msr sctlr_el2, x0
    0xD5033FDF,  // isb
    0xF9400133,  // ldr x19, [x9]
    0xF9400534,  // ldr x20, [x9, #8]
    0xF9400935,  // ldr x21, [x9, #16]
    0xF9400D36,  // ldr x22, [x9, #24]
    0xF9401137,  // ldr x23, [x9, #32]
    0xF9401538,  // ldr x24, [x9, #40]
    0xF9401939,  // ldr x25, [x9, #48]
    0xF9401D3A,  // ldr x26, [x9, #56]
    0xF940213B,  // ldr x27, [x9, #64]
    0xF940253C,  // ldr x28, [x9, #72]
    0xF940293D,  // ldr x29, [x9, #80]
    0xF9402D3E,  // ldr x30, [x9, #88]
    0xF9403520,  // ldr x0,  [x9, #104]
    0xD5184100,  // msr sp_el0,  x0
    0xF9403920,  // ldr x0,  [x9, #112]
    0xD51C1000,  // msr sctlr_el2,  x0
    0xD5033F9F,  // dsb sy
    0xD5033FDF,  // isb
    0xF9403129,  // ldr x9,  [x9, #96]
    0x9100013F,  // mov sp,  x9
    0xAA1F03E0,  // mov x0,  xzr
    0xD5033F9F,  // dsb sy
    0xD65F03C0,  // ret
    0xFFFFFFFF,  // lower 32 bits of &g_drtm_acs_dl_result
    0xFFFFFFFF,  // upper 32 bits of &g_drtm_acs_dl_result
    0xFFFFFFFF,  // lower 32 bits of &g_drtm_acs_dl_saved_state
    0xFFFFFFFF   // upper 32 bits of &g_drtm_acs_dl_saved_state
};
uint64_t g_drtm_acs_dlme_size = sizeof(g_drtm_acs_dlme);

//MMU Off
uint32_t                g_drtm_acs_dlme_mmu_off[] = {
    0x58000349,  // ldr x9,  =g_drtm_acs_dl_result
    0xF9000120,  // str x0,  [x9]
    0xF9000521,  // str x1,  [x9, #8]
    0x58000329,  // ldr x9,  =g_drtm_acs_dl_saved_state
    0xF9400133,  // ldr x19, [x9]
    0xF9400534,  // ldr x20, [x9, #8]
    0xF9400935,  // ldr x21, [x9, #16]
    0xF9400D36,  // ldr x22, [x9, #24]
    0xF9401137,  // ldr x23, [x9, #32]
    0xF9401538,  // ldr x24, [x9, #40]
    0xF9401939,  // ldr x25, [x9, #48]
    0xF9401D3A,  // ldr x26, [x9, #56]
    0xF940213B,  // ldr x27, [x9, #64]
    0xF940253C,  // ldr x28, [x9, #72]
    0xF940293D,  // ldr x29, [x9, #80]
    0xF9402D3E,  // ldr x30, [x9, #88]
    0xF9403520,  // ldr x0,  [x9, #104]
    0xD5184100,  // msr sp_el0,  x0
    0xF9403920,  // ldr x0,  [x9, #112]
    0xD51C1000,  // msr sctlr_el2,  x0
    0xF9403129,  // ldr x9,  [x9, #96]
    0x9100013F,  // mov sp,  x9
    0xAA1F03E0,  // mov x0,  xzr
    0xD5033F9F,  // dsb sy
    0xD5033FDF,  // isb
    0xD65F03C0,  // ret
    0xFFFFFFFF,  // lower 32 bits of &g_drtm_acs_dl_result
    0xFFFFFFFF,  // upper 32 bits of &g_drtm_acs_dl_result
    0xFFFFFFFF,  // lower 32 bits of &g_drtm_acs_dl_saved_state
    0xFFFFFFFF   // upper 32 bits of &g_drtm_acs_dl_saved_state
};

uint64_t g_drtm_acs_dlme_mmu_off_size = sizeof(g_drtm_acs_dlme_mmu_off);

int64_t val_invoke_drtm_fn(unsigned long function_id, unsigned long arg1,
              unsigned long arg2, unsigned long arg3,
              unsigned long arg4, unsigned long arg5,
              unsigned long *ret1, unsigned long *ret2,
              unsigned long *ret3)
{
    int64_t status;
    ARM_SMC_ARGS args;

    args.Arg0 = function_id;
    args.Arg1 = arg1;
    args.Arg2 = arg2;
    args.Arg3 = arg3;
    args.Arg4 = arg4;
    args.Arg5 = arg5;

    pal_pe_call_smc(&args, CONDUIT_SMC);
    status = args.Arg0;

    if (ret1)
      *ret1 = args.Arg1;
    if (ret2)
      *ret2 = args.Arg2;
    if (ret3)
      *ret3 = args.Arg3;

    return status;
}

/**
 *  @brief   This function returns the version of the DRTM implementation.
 *
 *  @return  status
 */
uint32_t val_drtm_get_version(void)
{
    return val_invoke_drtm_fn(DRTM_1_0_FN_DRTM_VERSION, 0, 0, 0, 0, 0,
                              NULL, NULL, NULL);
}

/**
 *  @brief   Query the DRTM features.
 *
 *  @return  status and feature details
 */
int64_t val_drtm_features(uint64_t fid, uint64_t *feat1, uint64_t *feat2)
{
    int64_t status;
    unsigned long _feat1;
    unsigned long _feat2;

    status = val_invoke_drtm_fn(DRTM_1_0_FN_DRTM_FEATURES, fid, 0, 0, 0, 0,
                                &_feat1, &_feat2, NULL);

    if (feat1)
        *feat1 = (uint64_t)_feat1;
    if (feat2)
        *feat2 = (uint64_t)_feat2;
    return status;
}

/**
 *  @brief   Initiates a DRTM dynamic launch
 *
 *  @return  status
 */
int64_t val_drtm_dynamic_launch(DRTM_PARAMETERS *drtm_params)
{
    return val_drtm_simulate_dl(drtm_params);
}

/**
 *  @brief   Closes a dynamic locality in the TPM
 *
 *  @return  status
 */
int64_t val_drtm_close_locality(uint32_t locality)
{
    return val_invoke_drtm_fn(DRTM_1_0_FN_DRTM_CLOSE_LOCALITY, locality, 0, 0, 0, 0,
                              NULL, NULL, NULL);
}

/**
 *  @brief   Calls DRTM Unprotect Memory
 *
 *  @return  status
 */
int64_t val_drtm_unprotect_memory(void)
{
    return val_invoke_drtm_fn(DRTM_1_0_FN_DRTM_UNPROTECT_MEMORY, 0, 0, 0, 0, 0,
                              NULL, NULL, NULL);
}

/**
 *  @brief   Calls DRTM Get Error
 *  @param   feat1 Error code from previous launch
 *  @return  status
 */
int64_t val_drtm_get_error(uint64_t *feat1)
{
    int64_t status;
    unsigned long _feat1;

    status = val_invoke_drtm_fn(DRTM_1_0_FN_DRTM_GET_ERROR, 0, 0, 0, 0, 0,
                              &_feat1, NULL, NULL);
    if (feat1)
        *feat1 = (uint64_t)_feat1;
    return status;
}

/**
 *  @brief   Calls DRTM Set TCB Hash function
 *  @param   tcb_hash_table_addr phy address of tcb hash table
 *  @return  status
 */
int64_t val_drtm_set_tcb_hash(uint64_t tcb_hash_table_addr)
{
    return val_invoke_drtm_fn(DRTM_1_0_FN_DRTM_SET_TCB_HASH, tcb_hash_table_addr, 0, 0, 0, 0,
                              NULL, NULL, NULL);
}

/**
 *  @brief   Calls DRTM Lock TCB Hashes function
 *
 *  @return  status
 */
int64_t val_drtm_lock_tcb_hashes(void)
{
    return val_invoke_drtm_fn(DRTM_1_0_FN_DRTM_LOCK_TCB_HASH, 0, 0, 0, 0, 0,
                              NULL, NULL, NULL);
}

/**
  @brief   This function checks if reserved_bits received are zero
  @param   reserved_bits reserved bits value received
  @return  status
**/
uint32_t val_drtm_reserved_bits_check_is_zero(uint32_t reserved_bits)
{
    if (reserved_bits != VAL_DRTM_RESERVED_BYTE_ZERO) {
        val_print(ACS_PRINT_ERR, "\n       CHECK RSVD BITS: FAILED [0x%08x]", reserved_bits);
        return ACS_STATUS_FAIL;
    } else
        val_print(ACS_PRINT_DEBUG, "\n       CHECK RSVD BITS: PASSED", 0);
    return ACS_STATUS_PASS;
}

/**
  @brief  This API is used to get PSCI features
  @return PSCI version
 **/
uint32_t val_drtm_get_psci_ver(void)
{
    ARM_SMC_ARGS smc_args;
    smc_args.Arg0 = ARM_SMC_ID_PSCI_VERSION;

    pal_pe_call_smc(&smc_args, CONDUIT_SMC);

    val_print(ACS_PRINT_DEBUG, "\n       PSCI VERSION = %X", smc_args.Arg0);

    return smc_args.Arg0;
}

/**
  @brief  This API is used to get SMCCC features
  @return SMCCC version
 **/
uint32_t val_drtm_get_smccc_ver(void)
{
    ARM_SMC_ARGS smc_args;
    smc_args.Arg0 = ARM_SMC_ID_PSCI_SMCCC_VERSION;

    pal_pe_call_smc(&smc_args, CONDUIT_SMC);

    val_print(ACS_PRINT_DEBUG, "\n       SMCCC VERSION = %X", smc_args.Arg0);

    return smc_args.Arg0;
}

/**
  @brief  This API is used to initialize the dl params
  @return status
 **/
int64_t val_drtm_init_drtm_params(DRTM_PARAMETERS *drtm_params)
{

    int64_t  status = ACS_STATUS_PASS;
    uint64_t dlme_data_region_size;
    uint64_t dlme_region_size;
    uint64_t dlme_image_size;
    uint64_t free_space_1_size = DRTM_SIZE_4K;
    uint64_t free_space_2_size = DRTM_SIZE_4K;
    uint64_t dlme_base_addr;
    uint64_t dlme_image_addr;
    uint64_t mmu_on = (val_pe_reg_read(SCTLR_EL2) & 0x1);
    uint32_t last_index;

    /*Status grater than zero indicates availability of feature bits in return value*/
    dlme_data_region_size =
        DRTM_SIZE_4K * VAL_EXTRACT_BITS(g_drtm_features.min_memory_req.value, 0, 31);

    /* Assign size based on MMU on/off */
    dlme_image_size = (mmu_on) ? ROUND_UP_TO_4K(g_drtm_acs_dlme_size)
                               : ROUND_UP_TO_4K(g_drtm_acs_dlme_mmu_off_size);

    /* Compile DRTM_PARAMETERS for Dynamic Launch */

    /* -------|--free space--|--DLME Image--|--free space--|--DLME Data Region--|-------*/
    /* -------^--------------^--------------^--------------^----------------------------*/
    /* -------|-----------------------DLME Region-------------------------------|-------*/

    dlme_region_size = free_space_1_size + dlme_image_size +
        free_space_2_size + dlme_data_region_size;

    dlme_base_addr = (uint64_t)val_aligned_alloc(DRTM_SIZE_4K, dlme_region_size);
    if (!dlme_base_addr) {
        val_print(ACS_PRINT_ERR, "\n    Failed to allocate memory for DLME region", 0);
        return ACS_STATUS_FAIL;
    }

    status = val_memory_set_wb_executable((void *)(dlme_base_addr + free_space_1_size),
                                          dlme_image_size);
    if (status) {
        val_print(ACS_PRINT_ERR, "\n    Failed to Set executable memory for DLME Image", 0);
        return ACS_STATUS_FAIL;
    }

    val_memory_set((void *)dlme_base_addr, dlme_region_size, 0);

    drtm_params->revision                = VAL_DRTM_PARAMETERS_REVISION;
    drtm_params->reserved                = 0;
    drtm_params->launch_features         = 0;
    drtm_params->dlme_region_address     = (uint64_t) dlme_base_addr;
    drtm_params->dlme_region_size        = dlme_region_size;
    drtm_params->dlme_image_start        = free_space_1_size;
    drtm_params->dlme_entry_point_offset = 0;
    drtm_params->dlme_image_size         = dlme_image_size;
    drtm_params->dlme_data_offset        = free_space_1_size + dlme_image_size + free_space_2_size;
    drtm_params->nw_dce_region_address   = 0;
    drtm_params->nw_dce_region_size      = 0;
    drtm_params->mem_prot_table_address  = 0;
    drtm_params->mem_prot_table_size     = 0;

    /* We have free space at drtm_params->dlme_region_address of 4KB */
    g_drtm_acs_dl_saved_state = (DRTM_ACS_DL_SAVED_STATE *)drtm_params->dlme_region_address;
    g_drtm_acs_dl_result = (DRTM_ACS_DL_RESULT *)(drtm_params->dlme_region_address
                            + sizeof(DRTM_ACS_DL_SAVED_STATE));

    /* Update the DLME Image Last Location with address of g_drtm_acs_dl* structures */
    if (mmu_on) {
      last_index = g_drtm_acs_dlme_size / sizeof(uint32_t) - 1;
      g_drtm_acs_dlme[last_index - 3] = ((uint64_t) g_drtm_acs_dl_result) & 0xFFFFFFFF;
      g_drtm_acs_dlme[last_index - 2] = ((uint64_t) g_drtm_acs_dl_result) >> 32;
      g_drtm_acs_dlme[last_index - 1] = ((uint64_t) g_drtm_acs_dl_saved_state) & 0xFFFFFFFF;
      g_drtm_acs_dlme[last_index]     = ((uint64_t) g_drtm_acs_dl_saved_state) >> 32;
    } else {
      last_index = g_drtm_acs_dlme_mmu_off_size / sizeof(uint32_t) - 1;
      g_drtm_acs_dlme_mmu_off[last_index - 3] = ((uint64_t) g_drtm_acs_dl_result) & 0xFFFFFFFF;
      g_drtm_acs_dlme_mmu_off[last_index - 2] = ((uint64_t) g_drtm_acs_dl_result) >> 32;
      g_drtm_acs_dlme_mmu_off[last_index - 1] = ((uint64_t) g_drtm_acs_dl_saved_state) & 0xFFFFFFFF;
      g_drtm_acs_dlme_mmu_off[last_index]     = ((uint64_t) g_drtm_acs_dl_saved_state) >> 32;
    }

    val_pe_cache_clean_invalidate_range((uint64_t)drtm_params, sizeof(DRTM_PARAMETERS));

    /* DLME Image address is sum of region start and DLME image offset */
    dlme_image_addr = drtm_params->dlme_region_address + drtm_params->dlme_image_start;
    if (mmu_on) {
      val_memcpy((void *)dlme_image_addr, (void *)g_drtm_acs_dlme, g_drtm_acs_dlme_size);
      val_pe_cache_clean_invalidate_range((uint64_t)dlme_image_addr,
                                          (uint64_t)g_drtm_acs_dlme_size);
    } else {
      val_memcpy((void *)dlme_image_addr, (void *)g_drtm_acs_dlme_mmu_off,
                                          g_drtm_acs_dlme_mmu_off_size);
      val_pe_cache_clean_invalidate_range((uint64_t)dlme_image_addr,
                                          (uint64_t)g_drtm_acs_dlme_mmu_off_size);
    }

    return status;
}

/**
  @brief  This API is used to check dl result
  @return status
 **/
int64_t val_drtm_check_dl_result(uint64_t dlme_base_addr, uint64_t dlme_data_offset)
{
    int64_t status = ACS_STATUS_PASS;

    /* in the dlme x0 should have dlme_base_addr */
    /* in the dlme x1 should have dlme_data_offset */
    /* If both condition are met then PASS */
    if (g_drtm_acs_dl_result->x0 != dlme_base_addr) {
        val_print(ACS_PRINT_ERR, "\n    Invalid x0 after dynamic launch", 0);
        val_print(ACS_PRINT_ERR, "\n    Expected 0x%lx,", dlme_base_addr);
        val_print(ACS_PRINT_ERR, " got 0x%lx", g_drtm_acs_dl_result->x0);
        status = ACS_STATUS_FAIL;
    }

    if (g_drtm_acs_dl_result->x1 != dlme_data_offset) {
        val_print(ACS_PRINT_ERR, "\n    Invalid x1 after dynamic launch", 0);
        val_print(ACS_PRINT_ERR, "\n    Expected 0x%lx,", dlme_data_offset);
        val_print(ACS_PRINT_ERR, " got 0x%lx", g_drtm_acs_dl_result->x1);
        status = ACS_STATUS_FAIL;
    }
    return status;
}

uint32_t val_drtm_create_info_table(void)
{
    int64_t  status;
    uint32_t drtm_version;
    uint64_t feat1, feat2;

    /* Saving interface into in global structure, which will be used in tests */
    /* Save DRTM version */
    drtm_version = val_drtm_get_version();
    if ((int32_t) drtm_version == DRTM_ACS_NOT_SUPPORTED) {
        g_drtm_features.version.status = DRTM_ACS_NOT_SUPPORTED;
    } else {
        g_drtm_features.version.status = DRTM_ACS_SUCCESS;
    }
    g_drtm_features.version.value = drtm_version;

    /* Save TPM features */
    status = val_drtm_features(DRTM_1_0_DRTM_FEATURES_TPM, &feat1, &feat2);
    g_drtm_features.tpm_features.status = status;
    if (status >= DRTM_ACS_SUCCESS) {
        g_drtm_features.tpm_features.value = feat1;
    }

    /* Save minimum memory requirement */
    status = val_drtm_features(DRTM_1_0_DRTM_FEATURES_MEM_REQ, &feat1, &feat2);
    g_drtm_features.min_memory_req.status = status;
    if (status >= DRTM_ACS_SUCCESS) {
        g_drtm_features.min_memory_req.value = feat1;
    }

    /* Save DMA protection features */
    status = val_drtm_features(DRTM_1_0_DRTM_FEATURES_DMA_PROT, &feat1, &feat2);
    g_drtm_features.dma_prot_features.status = status;
    if (status >= DRTM_ACS_SUCCESS) {
        g_drtm_features.dma_prot_features.value = feat1;
    }

    /*
     * Save boot PE affinity.
     * We don't need to query DRTM to get this value, and can
     * instead read it directly from the MPIDR register.
     * This will serve as a good sanity check for the DRTM boot PE
     * ID feature.
     */
    g_drtm_features.boot_pe_aff.status = DRTM_ACS_SUCCESS;
    g_drtm_features.boot_pe_aff.value  = val_pe_get_mpid();

    /* Save TCB hash features */
    status = val_drtm_features(DRTM_1_0_DRTM_FEATURES_TCB_HASHES, &feat1, &feat2);
    g_drtm_features.tcb_hash_features.status = status;
    if (status >= DRTM_ACS_SUCCESS) {
        g_drtm_features.tcb_hash_features.value = feat1;
    }

    /* Save DLME image authentication features */
    status = val_drtm_features(DRTM_1_0_DRTM_FEATURES_DLME_IMG_AUTH, &feat1, &feat2);
    g_drtm_features.dlme_image_authentication.status = status;
    if (status >= DRTM_ACS_SUCCESS) {
        g_drtm_features.dlme_image_authentication.value = feat1;
    }

    /* Save whether DRTM functions are implemented */
    g_drtm_features.dynamic_launch   =
        val_drtm_features(DRTM_1_0_FN_DRTM_DYNAMIC_LAUNCH, &feat1, &feat2);
    g_drtm_features.unprotect_memory =
        val_drtm_features(DRTM_1_0_FN_DRTM_UNPROTECT_MEMORY, &feat1, &feat2);
    g_drtm_features.close_locality   =
        val_drtm_features(DRTM_1_0_FN_DRTM_CLOSE_LOCALITY, &feat1, &feat2);
    g_drtm_features.get_error        =
        val_drtm_features(DRTM_1_0_FN_DRTM_GET_ERROR, &feat1, &feat2);
    g_drtm_features.set_error        =
        val_drtm_features(DRTM_1_0_FN_DRTM_SET_ERROR, &feat1, &feat2);
    g_drtm_features.set_tcb_hash     =
        val_drtm_features(DRTM_1_0_FN_DRTM_SET_TCB_HASH, &feat1, &feat2);
    g_drtm_features.lock_tcb_hashes  =
        val_drtm_features(DRTM_1_0_FN_DRTM_LOCK_TCB_HASH, &feat1, &feat2);

    return 0;
}

/**
  @brief  This API is used to get DRTM features
  @return status
 **/
uint64_t val_drtm_get_feature(uint64_t feature_type)
{
    uint64_t feature;

    switch (feature_type) {
    case DRTM_DRTM_FEATURES_DLME_IMG_AUTH:
        feature = (g_drtm_features.dlme_image_authentication.value &
                            DRTM_GET_FEATURES_MASK_DLME_IMAGE_AUTH);
    break;
    case DRTM_DRTM_FEATURES_DMA_PROTECTION:
        feature = (g_drtm_features.dma_prot_features.value & DRTM_GET_FEATURES_MASK_DMA_PROTECTION);
    break;
    case DRTM_DRTM_FEATURES_PCR_SCHEMA:
        feature = ((g_drtm_features.tpm_features.value >> DRTM_GET_FEATURES_SHIFT_PCR_SCHEMA) &
                            DRTM_GET_FEATURES_MASK_PCR_SCHEMA);
    break;
    case DRTM_DRTM_FEATURES_TPM_BASED_HASHING:
        feature = ((g_drtm_features.tpm_features.value >>
            DRTM_GET_FEATURES_SHIFT_TPM_BASED_HASHING) & DRTM_GET_FEATURES_MASK_TPM_BASED_HASHING);
    break;
    default:
        feature = ACS_STATUS_ERR;
    break;
    }

    return feature;
}
