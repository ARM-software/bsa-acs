/** @file
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "common/sys_arch_src/gic/acs_exception.h"
#include "common/include/val_interface.h"
#include "sbsa/include/sbsa_pal_interface.h"
#include "sbsa/include/sbsa_val_interface.h"

/**
  @brief   Pointer to the memory location of the PE Information table
**/
extern PE_INFO_TABLE *g_pe_info_table;

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
  pal_mem_free_aligned((void *)g_cache_info_table);
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
