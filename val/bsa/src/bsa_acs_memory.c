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
#include "common/include/acs_peripherals.h"
#include "common/include/acs_memory.h"
#include "common/include/acs_common.h"
#include "common/include/acs_mmu.h"
#include "common/include/val_interface.h"
#include "bsa/include/bsa_pal_interface.h"
#include "bsa/include/bsa_val_interface.h"
#include "bsa/include/bsa_acs_memory.h"

extern MEMORY_INFO_TABLE *g_memory_info_table;

/**
  @brief   Returns a random address whose attributes match the input
           memory type
           1. Caller       - Test Suite
           2. Prerequisite - val_memory_create_info_table
  @param   type     - type of memory being requested
  @param   instance - instance is '0' based and incremented to get different ranges
  @param   attr     - For cacheability etc. Not used for now.

  @return  64-bit address matching the input criteria
**/
addr_t
val_memory_get_addr(MEMORY_INFO_e mem_type, uint32_t instance, uint64_t *attr)
{
  uint32_t i;

  if (g_memory_info_table == NULL)
      return 0;

  switch (mem_type) {
  case MEM_TYPE_DEVICE:
      i = val_memory_get_entry_index(MEMORY_TYPE_DEVICE, instance);
      break;
  case MEM_TYPE_NORMAL:
      i = val_memory_get_entry_index(MEMORY_TYPE_NORMAL, instance);
      break;
  default:
      i = 0xFF;
      break;
  }
  if (i != 0xFF) {
      *attr = g_memory_info_table->info[i].flags;
      return g_memory_info_table->info[i].phy_addr;
  }

  val_print(ACS_PRINT_INFO, "\n       Instance 0x%x not found ", instance);
  val_print(ACS_PRINT_INFO, "for memory type 0x%x", mem_type);
  return 0;
}

/**
  @brief   Returns the maximum memory address from memeory info table
           1. Caller       - Test Suite
           2. Prerequisite - val_memory_create_info_table

  @param   None

  @return  maximum memory address
**/
uint64_t
val_get_max_memory()
{

  uint32_t index = 0;
  uint64_t addr = 0;

  while (g_memory_info_table->info[index].type != MEMORY_TYPE_LAST_ENTRY) {
      if ((g_memory_info_table->info[index].phy_addr) > addr)
              addr = (g_memory_info_table->info[index].phy_addr +
                      g_memory_info_table->info[index].size);
      index++;
  }
  return addr;

}

/**
  @brief  Allocates requested buffer size in bytes in a cacheable memory
          and returns the base address of the range.

  @param  Size         allocation size in bytes

  @return pointer to allocated memory
**/
void *
val_memory_alloc_cacheable(uint32_t bdf, uint32_t size, void **pa)
{
  return pal_mem_alloc_cacheable(bdf, size, pa);
}

/**
  @brief  Free Allocated buffer size by val_memory_alloc_cacheable.

  @param  bdf   BDF Value
  @param  size  Size
  @param  *va   pointer to virtual address space
  @param  *pa   pointer to physical address space

  @return None
**/
void
val_memory_free_cacheable(uint32_t bdf, uint32_t size, void *va, void *pa)
{
  pal_mem_free_cacheable(bdf, size, va, pa);
}

/**
  @brief  Return the address of unpopulated memory of requested
          instance.

  @param  addr      - Address of the unpopulated memory
          instance  - Instance of memory

  @return unpopulated Address
**/
uint64_t
val_memory_get_unpopulated_addr(addr_t *addr, uint32_t instance)
{
  return pal_memory_get_unpopulated_addr(addr, instance);
}
