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
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "Include/IndustryStandard/Acpi61.h"
#include <Protocol/AcpiTable.h>
#include <Protocol/Cpu.h>

#include "common/include/pal_uefi.h"
#include "sbsa/include/pal_sbsa_uefi.h"

UINT64 pal_get_pptt_ptr(void);
#define ADD_PTR(t, p, l) ((t*)((UINT8*)p + l))
#define PPTT_PE_PRIV_RES_OFFSET 0x14
#define PPTT_STRUCT_OFFSET 0x24

/**
  @brief  This API prints cache info table and cache entry indices for each pe.
  @param  CacheTable Pointer to cache info table.
  @param  PeTable Pointer to pe info table.
  @return None
**/
VOID
pal_cache_dump_info_table(CACHE_INFO_TABLE *CacheTable, PE_INFO_TABLE *PeTable)
{
  UINT32 i, j;
  CACHE_INFO_ENTRY *curr_entry;
  PE_INFO_ENTRY *pe_entry;
  curr_entry = CacheTable->cache_info;
  pe_entry = PeTable->pe_info;

  /*Iterate cache info table and print cache info entries*/
  for (i = 0 ; i < CacheTable->num_of_cache ; i++) {
    acs_print(ACS_PRINT_INFO, L"\nCache info * Index %d *", i);
    acs_print(ACS_PRINT_INFO, L"\n  Offset:                  0x%llx", curr_entry->my_offset);
    acs_print(ACS_PRINT_INFO, L"\n  Type:                    0x%llx", curr_entry->cache_type);
    acs_print(ACS_PRINT_INFO, L"\n  Cache ID:                0x%llx", curr_entry->cache_id);
    acs_print(ACS_PRINT_INFO, L"\n  Size:                    0x%llx", curr_entry->size);
    acs_print(ACS_PRINT_INFO, L"\n  Next level index:        %d", curr_entry->next_level_index);
    acs_print(ACS_PRINT_INFO, L"\n  Private flag:            0x%llx\n", curr_entry->is_private);
    curr_entry++;
  }

  acs_print(ACS_PRINT_INFO, L"\nPE level one cache index info");
  /*Iterate PE info table and print level one cache index info*/
  for (i = 0 ; i < PeTable->header.num_of_pe; i++) {
    acs_print(ACS_PRINT_INFO, L"\nPE Index * %d *", i);
    acs_print(ACS_PRINT_INFO, L"\n  Level 1 Cache index(s) :");

    for (j = 0; pe_entry->level_1_res[j] != DEFAULT_CACHE_IDX && j < MAX_L1_CACHE_RES; j++) {
      acs_print(ACS_PRINT_INFO, L" %d,", pe_entry->level_1_res[j]);
    }
    acs_print(ACS_PRINT_INFO, L"\n");
    pe_entry++;
  }
}

/**
  @brief  This function parses and stores cache info in cache info table
          Caller - pal_cache_create_info_table
  @param  CacheTable Pointer to cache info table.
  @param  cache_type_struct Pointer to PPTT cache structure that needs to be parsed.
  @param  offset Offset of the cache structure in PPTT ACPI table.
  @param  is_private Flag indicating whether the cache is private.
  @return Index to the cache info entry where parsed info is stored.
**/

UINT32
pal_cache_store_info(CACHE_INFO_TABLE *CacheTable,
                     EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE *cache_type_struct,
                     UINT32 offset, UINT32 is_private)
{
  CACHE_INFO_ENTRY *curr_entry;
  curr_entry = &(CacheTable->cache_info[CacheTable->num_of_cache]);
  CacheTable->num_of_cache++;

  curr_entry->my_offset = offset;
  curr_entry->flags.size_property_valid = cache_type_struct->Flags.SizePropertyValid;
  curr_entry->flags.cache_type_valid = cache_type_struct->Flags.CacheTypeValid;
  curr_entry->flags.cache_id_valid = cache_type_struct->Flags.CacheIdValid;
  curr_entry->size = cache_type_struct->Size;
  curr_entry->cache_type = cache_type_struct->Attributes.CacheType;
  curr_entry->cache_id = cache_type_struct->CacheId;
  curr_entry->is_private = is_private;

  /* set default next level index to invalid */
  curr_entry->next_level_index = CACHE_INVALID_NEXT_LVL_IDX;

  return CacheTable->num_of_cache - 1;
}

/**
  @brief  This function checks whether the cache info for a particular cache already stored.
          Caller - pal_cache_create_info_table
  @param  CacheTable Pointer to cache info table.
  @param  offset Offset of the cache structure in PPTT ACPI table.
  @param  found_index  pointer to a variable, to return index if cache info already present.
  @return 0 if cache info not present, 1 otherwise
**/
UINT32
pal_cache_find(CACHE_INFO_TABLE *CacheTable, UINT32 offset, UINT32 *found_index)
{
  CACHE_INFO_ENTRY *curr_entry;
  UINT32 i;

  curr_entry = CacheTable->cache_info;
  for (i = 0 ; i < CacheTable->num_of_cache ; i++) {
    /* match cache offset of the entry with input offset*/
    if(curr_entry->my_offset == offset) {
      *found_index = i;
      return 1;
    }
    curr_entry++;
  }
  return 0;
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
VOID
pal_cache_store_pe_res(PE_INFO_TABLE *PeTable, UINT32 acpi_uid,
                       UINT32 cache_index, UINT32 res_index)
{
  PE_INFO_ENTRY *entry;
  entry = PeTable->pe_info;
  UINT32 i;

  if (res_index < MAX_L1_CACHE_RES) {
    for (i = 0 ; i < PeTable->header.num_of_pe; i++) {
      if (entry->acpi_proc_uid == acpi_uid) {
        entry->level_1_res[res_index] = cache_index;
      }
      entry++;
    }
  }
  else
    acs_print(ACS_PRINT_ERR,
      L"\n  The input resource index is greater than supported value %d", MAX_L1_CACHE_RES);
}


/**
  @brief  Parses ACPI PPTT table and populates the local cache info table.
          Prerequisite - pal_pe_create_info_table
  @param  CacheTable Pointer to pre-allocated memory for cache info table.
  @return None
**/

VOID
pal_cache_create_info_table(CACHE_INFO_TABLE *CacheTable, PE_INFO_TABLE *PeTable)
{
  EFI_ACPI_6_4_PROCESSOR_PROPERTIES_TOPOLOGY_TABLE_HEADER *PpttHdr;
  UINT32 TableLength = 0;
  EFI_ACPI_6_4_PPTT_STRUCTURE_HEADER *pptt_struct, *pptt_end ;
  EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR *pe_type_struct, *temp_pe_struct;
  EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE *cache_type_struct;
  UINT32 i, j, status=0;
  UINT32 offset;
  UINT32 index;
  UINT32 next_index;

  if (CacheTable == NULL) {
    acs_print(ACS_PRINT_ERR, L" Unable to create cache info table, input pointer is NULL\n");
    return;
  }

  /* initialize cache info table entries */
  CacheTable->num_of_cache = 0;

  PpttHdr = (EFI_ACPI_6_4_PROCESSOR_PROPERTIES_TOPOLOGY_TABLE_HEADER *) pal_get_pptt_ptr();
  if (PpttHdr == NULL) {
    acs_print(ACS_PRINT_ERR, L" PPTT Table not found\n");
    return;
  }
  else {
    TableLength = PpttHdr->Header.Length;
    acs_print(ACS_PRINT_INFO, L"PPTT table found at 0x%llx with length 0x%x\n",
               PpttHdr, TableLength);
  }

/* Pointer to first PPTT structure in PPTT ACPI table */
  pptt_struct = ADD_PTR(EFI_ACPI_6_4_PPTT_STRUCTURE_HEADER, PpttHdr, PPTT_STRUCT_OFFSET);

/* PPTT end boundary */
  pptt_end =  ADD_PTR(EFI_ACPI_6_4_PPTT_STRUCTURE_HEADER, PpttHdr, TableLength);

/* iterate PPTT structs in PPTT ACPI Table */
  while (pptt_struct < pptt_end) {
    if (pptt_struct->Type == EFI_ACPI_6_4_PPTT_TYPE_PROCESSOR) {
      pe_type_struct = (EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR *) pptt_struct;
      /* check whether the PPTT PE structure corresponds to a actual PE and not a group */
      if (pe_type_struct->Flags.NodeIsALeaf == 1) {
        /* Parse PE private cache resources*/
        for (i = 0 ; i < pe_type_struct->NumberOfPrivateResources ; i++) {
          offset = *(ADD_PTR(UINT32, pe_type_struct, PPTT_PE_PRIV_RES_OFFSET + i*4));
          cache_type_struct =  ADD_PTR(EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE, PpttHdr, offset);
          index = pal_cache_store_info(CacheTable, cache_type_struct, offset, CACHE_TYPE_PRIVATE);
          pal_cache_store_pe_res(PeTable, pe_type_struct->AcpiProcessorId, index, i);
          /* parse next level(s) of current private PE cache  */
          while(cache_type_struct->NextLevelOfCache != 0) {
            offset = cache_type_struct->NextLevelOfCache;
            cache_type_struct =  ADD_PTR(EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE, PpttHdr, offset);
            /* check if cache PPTT struct is already parsed*/
            status = pal_cache_find(CacheTable, offset, &next_index);
            /* if cache structure is already parsed update the previous cache info with index
               of found cache entry in cache_info_table, else parse the cache structure*/
            if (status) {
              CacheTable->cache_info[index].next_level_index = next_index;
              break;
            }
            else {
              CacheTable->cache_info[index].next_level_index = CacheTable->num_of_cache;
              index = pal_cache_store_info(CacheTable, cache_type_struct,
                                           offset, CACHE_TYPE_PRIVATE);
            }
          }

          /* if a cache entry is already present in info table, then it means next level cache(s)
             for that cache is already parsed in past iteration, else parse parent PE group */
          if (status) continue;
          temp_pe_struct = pe_type_struct;

          /* Keep on parsing PPTT PE group structures until root */
          while (temp_pe_struct->Parent != 0 ) {
            temp_pe_struct = ADD_PTR(EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR, PpttHdr,
                                     temp_pe_struct->Parent);
            /* If a group has cache resources parse it */
            for (j = 0 ; j < temp_pe_struct->NumberOfPrivateResources;j++) {
              offset = *(ADD_PTR(UINT32, temp_pe_struct, PPTT_PE_PRIV_RES_OFFSET + j*4));
              cache_type_struct =  ADD_PTR(EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE, PpttHdr, offset);
              /* Next level cache type should unified type(0x2 or 0x3) or same as previous type*/
              if (cache_type_struct->Attributes.CacheType > 0x1 ||
                  cache_type_struct->Attributes.CacheType ==
                  CacheTable->cache_info[index].cache_type ) {
                status = pal_cache_find(CacheTable, offset, &next_index);
                /* if cache structure is already parsed update the previous cache info with index
                   of found cache entry in cache_info_table, else parse the cache structure */
                if (status) {
                  CacheTable->cache_info[index].next_level_index = next_index;
                  break;
                }
                else {
                  CacheTable->cache_info[index].next_level_index = CacheTable->num_of_cache;
                  index = pal_cache_store_info(CacheTable, cache_type_struct, offset,
                                               CACHE_TYPE_SHARED);
                }
              }
            }
            /* If cache entry already found in info table, then it means next level cache(s)
               for that cache is already parsed in past iteration, else parse parent PE group
               of current group */
            if(status) break;
          }
        }
      }
    }
    pptt_struct = ADD_PTR(EFI_ACPI_6_4_PPTT_STRUCTURE_HEADER, pptt_struct, pptt_struct->Length);
  }
  pal_cache_dump_info_table(CacheTable, PeTable);
}
