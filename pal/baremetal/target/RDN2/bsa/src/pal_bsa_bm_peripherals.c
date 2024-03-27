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

#include "pal_pcie_enum.h"
#include "pal_common_support.h"
#include "platform_override_struct.h"

extern PLATFORM_OVERRIDE_MEMORY_INFO_TABLE  platform_mem_cfg;

/**
  @brief  Return the address of unpopulated memory of requested
          instance from the GCD memory map.

  @param  addr      - Address of the unpopulated memory
          instance  - Instance of memory

  @return 0 - SUCCESS
          1 - No unpopulated memory present
          2 - FAILURE
**/
uint64_t
pal_memory_get_unpopulated_addr(uint64_t *addr, uint32_t instance)
{
  uint32_t index = 0;
  uint32_t memory_instance = 0;

  for (index = 0; index < platform_mem_cfg.count; index++)
  {
      if (platform_mem_cfg.info[index].type == MEMORY_TYPE_NOT_POPULATED)
      {
          if (memory_instance == instance)
          {
              *addr =  platform_mem_cfg.info[index].virt_addr;
              print(ACS_PRINT_INFO, "Unpopulated region with base address 0x%lX found\n", *addr);
              return MEM_MAP_SUCCESS;
          }

          memory_instance++;
      }
  }

  return MEM_MAP_NO_MEM;
}

