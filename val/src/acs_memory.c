/** @file
 * Copyright (c) 2016-2021, Arm Limited or its affiliates. All rights reserved.
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
#include "include/bsa_acs_peripherals.h"
#include "include/bsa_acs_memory.h"
#include "include/bsa_acs_common.h"


MEMORY_INFO_TABLE  *g_memory_info_table;

/**
  @brief   This API will execute all Memory tests designated for a given compliance level
           1. Caller       -  Application layer.
           2. Prerequisite -  val_memory_create_info_table
  @param   num_pe - the number of PE to run these tests on.
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_memory_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{

  uint32_t status, i;

  for (i = 0 ; i < MAX_TEST_SKIP_NUM ; i++) {
      if (g_skip_test_num[i] == ACS_MEMORY_MAP_TEST_BASE) {
        val_print(ACS_PRINT_TEST, "\n      USER Override - Skipping all Memory tests \n", 0);
        return ACS_STATUS_SKIP;
      }
  }

  status = ACS_STATUS_PASS;

  if (g_sw_view[G_SW_OS]) {
      val_print(ACS_PRINT_ERR, "\nOperating System View:\n", 0);
#ifndef TARGET_LINUX
//    status |= os_m001_entry(num_pe);
      status |= os_m002_entry(num_pe);
      status |= os_m003_entry(num_pe);
#else
      status |= os_m004_entry(num_pe);
#endif

  }

  if (status != ACS_STATUS_PASS)
      val_print(ACS_PRINT_TEST, "\n      *** One or more tests have Failed/Skipped.*** \n", 0);
  else
      val_print(ACS_PRINT_TEST, "\n      All Memory tests have passed!! \n", 0);

  return status;
}

#ifndef TARGET_LINUX
/**
  @brief  Free the memory allocated for the Memory Info table
**/
void
val_memory_free_info_table()
{
  pal_mem_free((void *)g_memory_info_table);
}

/**
  @brief   This function will call PAL layer to fill all relevant peripheral
           information into the g_peripheral_info_table pointer.
           1. Caller       - Application layer
           2. Prerequisite - Memory allocated and passed as argument.
  @param   memory_info_table - Address where the memory info table is created

  @return  None
**/

void
val_memory_create_info_table(uint64_t *memory_info_table)
{

  g_memory_info_table = (MEMORY_INFO_TABLE *)memory_info_table;

  pal_memory_create_info_table(g_memory_info_table);

}
#endif

/**
  @brief   Return the Index of the entry in the peripheral info table
           which matches the input type and the input instance number
           Instance number is '0' based
           1. Caller       -  VAL
           2. Prerequisite -
  @param   type     - type of memory being requested
  @param   instance - instance is '0' based and incremented to get different ranges
  @return  index
**/
uint32_t
val_memory_get_entry_index(uint32_t type, uint32_t instance)
{
  uint32_t  i = 0;

  while (g_memory_info_table->info[i].type != MEMORY_TYPE_LAST_ENTRY) {
      if (g_memory_info_table->info[i].type == type) {
          if (instance == 0)
             return i;
          else
             instance--;

      }
      i++;
  }
  return 0xFF;
}

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

  switch(mem_type) {
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

  val_print(ACS_PRINT_INFO, "Instance 0x%x not found ", instance);
  val_print(ACS_PRINT_INFO, "for memory type 0x%x\n", mem_type);
  return 0;
}

/**
  @brief   Returns the type and attributes of a given memory address
           1. Caller       - Test Suite
           2. Prerequisite - val_memory_create_info_table
  @param   addr     - Address whose type and attributes are being requested
  @param   attr     - For cacheability etc. Not used for now.

  @return  type of the memory address
**/
uint64_t
val_memory_get_info(addr_t addr, uint64_t *attr)
{

  uint32_t index = 0;

  while (g_memory_info_table->info[index].type != MEMORY_TYPE_LAST_ENTRY) {
      if ((addr >= g_memory_info_table->info[index].phy_addr) &&
        (addr < (g_memory_info_table->info[index].phy_addr +
        g_memory_info_table->info[index].size))) {
          *attr =  g_memory_info_table->info[index].flags;
          return g_memory_info_table->info[index].type;
       }
       index++;
      // val_print(ACS_PRINT_INFO," .", 0);
  }

  return MEM_TYPE_NOT_POPULATED;

}

/**
  @brief   Returns the maximum memory address from memeory info table
           1. Caller       - Test Suite
           2. Prerequisite - val_memory_create_info_table

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

addr_t
val_memory_ioremap(void *addr, uint32_t size, uint64_t attr)
{
  return (pal_memory_ioremap(addr, size, attr));
}

void
val_memory_unmap(void *ptr)
{
  pal_memory_unmap(ptr);
}

void *
val_memory_alloc(uint32_t size)
{
  return pal_mem_alloc(size);
}

void *
val_memory_alloc_cacheable(uint32_t bdf, uint32_t size, void **pa)
{
  return pal_mem_alloc_cacheable(bdf, size, pa);
}

void
val_memory_free(void *addr)
{
  pal_mem_free(addr);
}

int
val_memory_compare(void *src, void *dest, uint32_t len)
{
  return pal_mem_compare(src, dest, len);
}

void
val_memory_set(void *buf, uint32_t size, uint8_t value)
{
  pal_mem_set(buf, size, value);
}

void
val_memory_free_cacheable(uint32_t bdf, uint32_t size, void *va, void *pa)
{
  pal_mem_free_cacheable(bdf, size, va, pa);
}

void *
val_memory_virt_to_phys(void *va)
{
  return pal_mem_virt_to_phys(va);
}

void *
val_memory_phys_to_virt(uint64_t pa)
{
  return pal_mem_phys_to_virt(pa);
}

uint64_t
val_memory_get_unpopulated_addr(addr_t *addr, uint32_t instance)
{
  return pal_memory_get_unpopulated_addr(addr, instance);
}

uint32_t val_memory_page_size(void)
{
    return pal_mem_page_size();
}

void *
val_memory_alloc_pages(uint32_t num_pages)
{
    return pal_mem_alloc_pages(num_pages);
}

void
val_memory_free_pages(void *addr, uint32_t num_pages)
{
    pal_mem_free_pages(addr, num_pages);
}

/**
  @brief  Allocates memory with the given alignment.

  @param  Alignment   Specifies the alignment.
  @param  Size        Requested memory allocation size.

  @return Pointer to the allocated memory with requested alignment.
**/
void
*val_aligned_alloc(uint32_t alignment, uint32_t size)
{
  void *Mem = NULL;
  void *Aligned_Ptr = NULL;

  /* Generate mask for the Alignment parameter*/
  uint64_t Mask = ~(uint64_t)(alignment - 1);

  /* Allocate memory with extra bytes, so we can return an aligned address*/
  Mem = (void *)pal_mem_alloc(size + alignment);

  if (Mem == NULL)
    return 0;

  /* Add the alignment to allocated memory address and align it to target alignment*/
  Aligned_Ptr = (void *)(((uint64_t) Mem + alignment-1) & Mask);

  return Aligned_Ptr;
}
