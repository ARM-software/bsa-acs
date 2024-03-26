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
#include "pal_common_support.h"
#include "pal_sbsa_common_support.h"
#include "pal_pcie_enum.h"
#include "platform_override_struct.h"
#include "platform_override_sbsa_struct.h"

extern PLATFORM_OVERRIDE_CACHE_INFO_TABLE platform_cache_cfg;
extern PLATFORM_OVERRIDE_PPTT_INFO_TABLE platform_pptt_cfg;

/**
  @brief  This API prints cache info table and cache entry indices for each pe.
  @param  CacheTable Pointer to cache info table.
  @param  PeTable Pointer to pe info table.
  @return None
**/
void
pal_cache_dump_info_table(CACHE_INFO_TABLE *CacheTable, PE_INFO_TABLE *PeTable)
{
  uint32_t i, j;
  CACHE_INFO_ENTRY *curr_entry;
  PE_INFO_ENTRY *pe_entry;
  curr_entry = CacheTable->cache_info;
  pe_entry = PeTable->pe_info;

  /*Iterate cache info table and print cache info entries*/
  for (i = 0 ; i < CacheTable->num_of_cache ; i++) {
    print(ACS_PRINT_INFO, "\nCache info * Index %d *", i);
    print(ACS_PRINT_INFO, "\n  Offset:                  0x%llx", curr_entry->my_offset);
    print(ACS_PRINT_INFO, "\n  Type:                    0x%llx", curr_entry->cache_type);
    print(ACS_PRINT_INFO, "\n  Cache ID:                0x%llx", curr_entry->cache_id);
    print(ACS_PRINT_INFO, "\n  Size:                    0x%llx", curr_entry->size);
    print(ACS_PRINT_INFO, "\n  Next level index:        %d", curr_entry->next_level_index);
    print(ACS_PRINT_INFO, "\n  Private flag:            0x%llx\n", curr_entry->is_private);
    curr_entry++;
  }

  print(ACS_PRINT_INFO, "\nPE level one cache index info");
  /*Iterate PE info table and print level one cache index info*/
  for (i = 0 ; i < PeTable->header.num_of_pe; i++) {
    print(ACS_PRINT_INFO, "\nPE Index * %d *", i);
    print(ACS_PRINT_INFO, "\n  Level 1 Cache index(s) :");

    for (j = 0; pe_entry->level_1_res[j] != DEFAULT_CACHE_IDX && j < MAX_L1_CACHE_RES; j++) {
      print(ACS_PRINT_INFO, " %d,", pe_entry->level_1_res[j]);
    }
    print(ACS_PRINT_INFO, "\n");
    pe_entry++;
  }
}


/**
  @brief  This function stores level 1 cache info entry index(s) to pe info table.
          Caller - pal_cache_create_info_table
  @param  PeTable Pointer to pe info table.
  @param  acpi_uid ACPI UID of the pe entry, to which index(s) to be stored.
  @param  cache_index index of the level 1 cache entry.
  @param  res_index private resource index of pe private cache.
  @return None
**/
void
pal_cache_store_pe_res(CACHE_INFO_TABLE *CacheTable, PE_INFO_TABLE *PeTable)
{
  PE_INFO_ENTRY *entry;
  entry = PeTable->pe_info;

  uint32_t i, j, res_index = 0;


    for (i = 0; i < PeTable->header.num_of_pe; i++) {
        for (j = 0 ; j < CacheTable->num_of_cache; j++) {
          if (platform_pptt_cfg.pptt_info[i].cache_id[res_index] == CacheTable->cache_info[j].cache_id) {
            entry->level_1_res[res_index] = j;
            res_index++;
            if(res_index >= 2)
            {
                res_index = 0;
                break;
            }
          }
        }
      entry++;
    }
}


/**
  @brief  Parses ACPI PPTT table and populates the local cache info table.
          Prerequisite - pal_pe_create_info_table
  @param  CacheTable Pointer to pre-allocated memory for cache info table.
  @return None
**/
void
pal_cache_create_info_table(CACHE_INFO_TABLE *CacheTable, PE_INFO_TABLE *PeTable)
{
  uint32_t i;

  if (CacheTable == NULL) {
    print(ACS_PRINT_ERR, " Unable to create cache info table, input pointer is NULL\n");
    return;
  }

  CACHE_INFO_ENTRY *curr_entry;
  curr_entry = CacheTable->cache_info;

  /* initialize cache info table entries */
  CacheTable->num_of_cache =  platform_cache_cfg.num_of_cache;

  for (i = 0; i < CacheTable->num_of_cache; i++)
  {
    curr_entry->my_offset = platform_cache_cfg.cache_info[i].offset;
    curr_entry->flags.size_property_valid = platform_cache_cfg.cache_info[i].flags & SIZE_MASK;
    curr_entry->flags.cache_type_valid = (platform_cache_cfg.cache_info[i].flags & CACHE_TYPE_MASK) >> CACHE_TYPE_SHIFT;
    curr_entry->flags.cache_id_valid = (platform_cache_cfg.cache_info[i].flags & CACHE_ID_MASK) >> CACHE_ID_SHIFT;
    curr_entry->size = platform_cache_cfg.cache_info[i].size;
    curr_entry->cache_type = platform_cache_cfg.cache_info[i].cache_type;
    curr_entry->cache_id = platform_cache_cfg.cache_info[i].cache_id;
    curr_entry->is_private = platform_cache_cfg.cache_info[i].is_private;
    curr_entry->next_level_index = platform_cache_cfg.cache_info[i].next_level_index;
    curr_entry++;
   }

  pal_cache_store_pe_res(CacheTable, PeTable);
  pal_cache_dump_info_table(CacheTable, PeTable);
}
