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
#include "common/include/acs_peripherals.h"
#include "common/include/acs_memory.h"
#include "common/include/acs_common.h"
#include "common/include/acs_mmu.h"
#include "common/include/val_interface.h"
#include "bsa/include/bsa_val_interface.h"

MEMORY_INFO_TABLE  *g_memory_info_table;

#define SIZE_4KB   0x00001000

#define ADDR_52BIT_MASK 0xFFFFFFFFFFFFULL

#define CHECK_ADDR_52BIT(addr) (((uint64_t)(addr)) & ~ADDR_52BIT_MASK)


#ifdef TARGET_BM_BOOT
/**
 *   @brief    Add regions assigned to host into its translation table data structure.
 *   @param    void
 *   @return   void
**/
void val_mmu_add_mmap(void)
{
    return pal_mmu_add_mmap();
}

/**
 *   @brief    Get the list of mem regions.
 *   @param    void
 *   @return   Pointer of the list.
**/
void *val_mmu_get_mmap_list(void)
{
    return pal_mmu_get_mmap_list();
}

/**
 *   @brief    Get the total number of mem map regions.
 *   @param    void
 *   @return   Count of mem map regions.
**/
uint32_t val_mmu_get_mapping_count(void)
{
    return pal_mmu_get_mapping_count();
}
#endif  // TARGET_BM_BOOT

#ifndef TARGET_LINUX
/**
  @brief  Free the memory allocated for the Memory Info table

  @param  None

  @return None
**/

void
val_memory_free_info_table(void)
{
    if (g_memory_info_table != NULL) {
        pal_mem_free((void *)g_memory_info_table);
        g_memory_info_table = NULL;
    }
    else {
      val_print(ACS_PRINT_ERR,
                  "\n WARNING: g_memory_info_table pointer is already NULL",
        0);
    }
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
  val_print(ACS_PRINT_INFO, " Creating MEMORY INFO table\n", 0);

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
  @brief   Maps the physical memory to virtual address space
           1. Caller       - Test Suite
           2. Prerequisite - val_memory_create_info_table

  @param   *addr  Physical address
  @param   size   Size
  @param   attr   Attributes

  @return  Mapped Address Starting Pointer
**/
addr_t
val_memory_ioremap(void *addr, uint32_t size, uint32_t attr)
{
  return (pal_memory_ioremap(addr, size, attr));
}

/**
  @brief   Removes the physical memory to virtual address space mapping
           1. Caller       - Test Suite
           2. Prerequisite - val_memory_ioremap

  @param   *ptr   Pointer to mapped space

  @return  None
**/
void
val_memory_unmap(void *ptr)
{
  pal_memory_unmap(ptr);
}

/**
  @brief  Allocates requested buffer size in bytes in a contiguous memory
          and returns the base address of the range.

  @param  Size         allocation size in bytes

  @return pointer to allocated memory
**/
void *
val_memory_alloc(uint32_t size)
{
  return pal_mem_alloc(size);
}

/**
  @brief  Allocates requested zero buffer in bytes in a contiguous memory
          and returns the base address of the range.

  @param  Size         allocation size in bytes

  @return pointer to allocated memory
**/
void *
val_memory_calloc(uint32_t num, uint32_t size)
{
  return pal_mem_calloc(num, size);
}

/**
  @brief  Free Allocated buffer size by val_memory_alloc.

  @param  *addr   pointer to allocated memory

  @return None
**/
void
val_memory_free(void *addr)
{
  pal_mem_free(addr);
}

/**
  @brief  Compare two buffers of length len

  @param  *src  Source Buffer
  @param  *dest Destination Buffer
  @param  len   Length

  @return 0 If contents are same
  @return 1 Otherwise
**/
int
val_memory_compare(void *src, void *dest, uint32_t len)
{
  return pal_mem_compare(src, dest, len);
}

/**
  @brief  Set buffer with value given in arguments

  @param  *buf  Buffer
  @param  size  size
  @param  value value to be written

  @return None
**/
void
val_memory_set(void *buf, uint32_t size, uint8_t value)
{
  pal_mem_set(buf, size, value);
}

/**
  @brief  Returns the physical address for virtual address space.

  @param  *va   pointer to virtual address space

  @return Pointer to Physical address space
**/
void *
val_memory_virt_to_phys(void *va)
{
  return pal_mem_virt_to_phys(va);
}

/**
  @brief  Returns the virtual address for physical address space.

  @param  *pa   pointer to physical address space

  @return Pointer to virtual address space
**/
void *
val_memory_phys_to_virt(uint64_t pa)
{
  return pal_mem_phys_to_virt(pa);
}

/**
  @brief  Return the Memory Page Size.

  @param  None

  @return Size of memory page
**/
uint32_t val_memory_page_size(void)
{
    return pal_mem_page_size();
}

/**
  @brief  Allocates number of pages in the memory.

  @param  num_pages  Number of memory pages needed

  @return Address of the allocated space.
**/
void *
val_memory_alloc_pages(uint32_t num_pages)
{
    return pal_mem_alloc_pages(num_pages);
}

/**
  @brief  Free number of pages in the memory.

  @param  *addr      Address from where we need to free
  @param  num_pages  Number of memory pages needed

  @return None
**/
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

  return pal_aligned_alloc(alignment, size);

}

/**
  @brief  Free Allocated buffer size by val_aligned_alloc.

  @param  *addr   pointer to allocated memory

  @return None
**/
void
val_memory_free_aligned(void *addr)
{
  pal_mem_free_aligned(addr);
}

/**
  @brief  Sets memory page with WB executable.

  @param  addr   Specifies the base address.
  @param  Size   Requested size.

  @return Pointer to the allocated memory with requested alignment.
**/
uint32_t
val_memory_set_wb_executable(void *addr, uint32_t size)
{

  return pal_mem_set_wb_executable(addr, size);

}

/**
 * @brief Checks if uefi memory map has 52 bit addr
 * @param void
 * @return 1 52 bit addr present, else 0
**/
uint32_t val_memory_region_has_52bit_addr(void)
{
  uint32_t index = 0;

  while (g_memory_info_table->info[index].type != MEMORY_TYPE_LAST_ENTRY) {
      val_print(ACS_PRINT_INFO, " \n       Mem Phy Addr %lx",
                                   g_memory_info_table->info[index].phy_addr);
      if (CHECK_ADDR_52BIT(g_memory_info_table->info[index].phy_addr))
          return 1;
      index++;
  }

  return 0;
}

/**
  @brief   This API issues a DSB Memory barrier instruction
  @param   None
  @return  None
**/
void
val_mem_issue_dsb(void)
{
#ifndef TARGET_LINUX
    AA64IssueDSB();
#endif
}
