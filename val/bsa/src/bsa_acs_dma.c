/** @file
 * Copyright (c) 2016-2018,2021,2023-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "common/include/acs_common.h"
#include "bsa/include/bsa_acs_dma.h"
#include "bsa/include/bsa_pal_interface.h"
#include "bsa/include/bsa_val_interface.h"

DMA_INFO_TABLE  *g_dma_info_table;

/**
  @brief  free memory of DMA info table

  @param  None

  @result None
**/

void
val_dma_free_info_table(void)
{
    if (g_dma_info_table != NULL) {
        pal_mem_free((void *)g_dma_info_table);
        g_dma_info_table = NULL;
    }
    else {
      val_print(ACS_PRINT_ERR,
                  "\n WARNING: g_dma_info_table pointer is already NULL",
        0);
    }
}

/**
  @brief  API to keep all DMA Controller related information.

  @param  dma_info_ptr - DMA Info table pointer

  @return None
**/
void
val_dma_create_info_table(uint64_t *dma_info_ptr)
{

  if (dma_info_ptr == NULL) {
      val_print(ACS_PRINT_ERR, "Input for Create Info table cannot be NULL\n", 0);
      return;
  }
  val_print(ACS_PRINT_INFO, " Creating DMA INFO table\n", 0);

  g_dma_info_table = (DMA_INFO_TABLE *)dma_info_ptr;

  pal_dma_create_info_table(g_dma_info_table);

  val_print(ACS_PRINT_TEST, " DMA_INFO: Number of DMA CTRL in PCIe :    %x\n",
                                                            val_dma_get_info(DMA_NUM_CTRL, 0));
}


/**
  @brief  Single entry point to return all DMA Controller related information.
          1. Caller       - Test Suite
          2. Prerequisite - val_dma_create_info_table
  @param  info_type - Type of DMA controller whose index needs to be returned
  @param  instance  - id (0' based) for which the info has to be returned

  @return  64-bit data of dma info matching type and instance
**/
uint64_t
val_dma_get_info(DMA_INFO_e type, uint32_t index)
{

  if (g_dma_info_table == NULL)
  {
      val_print(ACS_PRINT_DEBUG, "GET_DMA_INFO: DMA info table is not created\n", 0);
      return 0;
  }
  if (index > g_dma_info_table->num_dma_ctrls)
  {
      val_print(ACS_PRINT_ERR, "GET_DMA_INFO: Index (%d) is greater than num of DMA\n", index);
      return 0;
  }

  switch (type)
  {
      case DMA_NUM_CTRL:
          return g_dma_info_table->num_dma_ctrls;

      case DMA_HOST_INFO:
          return (uint64_t)g_dma_info_table->info[index].host;

      case DMA_PORT_INFO:
          return (uint64_t)g_dma_info_table->info[index].port;

      case DMA_TARGET_INFO:
          return (uint64_t)g_dma_info_table->info[index].target;

      case DMA_HOST_COHERENT:
          return ((uint64_t)g_dma_info_table->info[index].flags & (DMA_COHERENT_MASK));

      case DMA_HOST_IOMMU_ATTACHED:
          return ((uint64_t)g_dma_info_table->info[index].flags & (IOMMU_ATTACHED_MASK));

      case DMA_HOST_PCI:
          return ((uint64_t)g_dma_info_table->info[index].flags & (PCI_EP_MASK));

      default:
          val_print(ACS_PRINT_ERR, "This DMA info option not supported %d\n", type);
          break;
  }

  return 0;
}

/**
  @brief  Allocate memory which is to be used for DMA

  @param  *buffer    - Address of the Memory Bufffer to be returned
  @param  size       - Memory size to be allocated
  @param  dev_index  - DMA Controller Index
  @param  flags      - Flags to determine if DMA is Coherent

  @result Start Address of the Allocated memory
**/
addr_t
val_dma_mem_alloc(void **buffer, uint32_t size, uint32_t dev_index, uint32_t flags)
{

  void *ap = NULL;

  ap = (void *)val_dma_get_info(DMA_PORT_INFO, dev_index);
  return pal_dma_mem_alloc(buffer, size, ap, flags);

}

/**
  @brief  free memory which is to used for DMA

  @param  *buffer    - Address of the Memory Bufffer to free
  @param  mem_dma    - DMA handle of the memory to free
  @param  size       - Memory size to be allocated
  @param  dev_index  - DMA Controller Index
  @param  flags      - Flags to determine if DMA is Coherent

  @result None
**/
void
val_dma_mem_free(void *buffer, dma_addr_t mem_dma, uint32_t size, uint32_t dev_index,
                  uint32_t flags)
{

  void *ap = NULL;

  ap = (void *)val_dma_get_info(DMA_PORT_INFO, dev_index);
  pal_dma_mem_free(buffer, mem_dma, size, ap, flags);

}

/**
  @brief  Hook to get the DMA address used by the controller pointed by the index
          1. Caller       - Test Suite
          2. Prerequisite - val_dma_create_info_table
  @param  ctrl_index - Index DMA controller whose index needs to be returned
  @param  dma_addr   - DMA device visible Address returned from the device driver
  @param  cpu_addr   - The CPU visible Address returned from the device driver

  @return None
**/

void
val_dma_device_get_dma_addr(uint32_t ctrl_index, uint64_t *dma_addr, uint32_t *cpu_len)
{

#ifdef TARGET_LINUX
  *dma_addr = g_dma_info_table->info[ctrl_index].dma_sg_address;
  *cpu_len =  g_dma_info_table->info[ctrl_index].dma_sg_length;
#else
 (void)ctrl_index;
 (void)dma_addr;
 (void)cpu_len;
#endif

}

/**
  @brief  Get Memory attributes of the Memory Buffer

  @param  buf - Memory buffer for which we want to get the attributes
  @param  *attr - Memory attributes to return
  @param  *sh - Sharability

  @result Status
**/
int
val_dma_mem_get_attrs(void *buf, uint32_t *attr, uint32_t *sh)
{
  return pal_dma_mem_get_attrs(buf, attr, sh);
}
