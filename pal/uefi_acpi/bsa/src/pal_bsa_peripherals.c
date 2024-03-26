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

#include <Uefi.h>
#include <PiDxe.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DxeServicesTableLib.h>

#include <Protocol/AcpiTable.h>
#include "Include/IndustryStandard/Acpi61.h"
#include "Include/IndustryStandard/SerialPortConsoleRedirectionTable.h"

#include "common/include/pal_uefi.h"
#include "common/include/bsa_pcie_enum.h"
#include "../include/platform_override.h"

/**
  @brief  Return the address of unpopulated memory of requested
          instance from the GCD memory map.

  @param  addr      - Address of the unpopulated memory
          instance  - Instance of memory

  @return  EFI_STATUS
**/
UINT64
pal_memory_get_unpopulated_addr(UINT64 *addr, UINT32 instance)
{
  EFI_STATUS                        Status;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR  *MemorySpaceMap = NULL;
  UINT32                            Index;
  UINTN                             NumberOfDescriptors;
  UINT32                            Memory_instance = 0;

  /* Get the Global Coherency Domain Memory Space map table */
  Status = gDS->GetMemorySpaceMap(&NumberOfDescriptors, &MemorySpaceMap);
  if (Status != EFI_SUCCESS)
  {
    acs_print(ACS_PRINT_ERR, L" Failed to get GCD memory with error: %x\n", Status);
    if (Status == EFI_NO_MAPPING)
    {
        return MEM_MAP_NO_MEM;
    }

    return MEM_MAP_FAILURE;
  }

  for (Index = 0; Index < NumberOfDescriptors; Index++, MemorySpaceMap++)
  {
    if (MemorySpaceMap->GcdMemoryType == EfiGcdMemoryTypeNonExistent)
    {
      if (Memory_instance == instance)
      {
        *addr = MemorySpaceMap->BaseAddress;
        if (*addr == 0)
          continue;

        acs_print(ACS_PRINT_INFO, L" Unpopulated region with base address 0x%lX found\n", *addr);
        return MEM_MAP_SUCCESS;
      }

      Memory_instance++;
    }
  }

  return PCIE_NO_MAPPING;
}

/**
  @brief  Platform specific code for UART initialisation

  @param   None
  @return  None
**/
VOID
pal_peripheral_uart_setup(VOID)
{
  return;
}
