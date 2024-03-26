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
#include  <Library/ShellCEntryLib.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/UefiLib.h>
#include  <Library/ShellLib.h>
#include  <Library/PrintLib.h>
#include  <Library/BaseMemoryLib.h>
#include <Protocol/Cpu.h>


#include "common/include/pal_uefi.h"

/**
 * @brief  Allocates requested buffer size in bytes in a contiguous cacheable
 *         memory and returns the base address of the range.
 *
 * @param  Size         allocation size in bytes
 * @param  Pa           Pointer to Physical Addr
 * @retval if SUCCESS   Pointer to Virtual Addr
 * @retval if FAILURE   NULL
 */
VOID *
pal_mem_alloc_cacheable (
  UINT32 Bdf,
  UINT32 Size,
  VOID **Pa
  )
{
  EFI_PHYSICAL_ADDRESS      Address;
  EFI_CPU_ARCH_PROTOCOL     *Cpu;
  EFI_STATUS                Status;

  Status = gBS->AllocatePages (AllocateAnyPages,
                               EfiBootServicesData,
                               EFI_SIZE_TO_PAGES(Size),
                               &Address);
  if (EFI_ERROR(Status)) {
    acs_print(ACS_PRINT_ERR, L" Allocate Pool failed %x\n", Status);
    return NULL;
  }

  /* Check Whether Cpu architectural protocol is installed */
  Status = gBS->LocateProtocol ( &gEfiCpuArchProtocolGuid, NULL, (VOID **)&Cpu);
  if (EFI_ERROR(Status)) {
    acs_print(ACS_PRINT_ERR, L" Could not get Cpu Arch Protocol %x\n", Status);
    return NULL;
  }

  /* Set Memory Attributes */
  Status = Cpu->SetMemoryAttributes (Cpu,
                                     Address,
                                     Size,
                                     EFI_MEMORY_WB);
  if (EFI_ERROR (Status)) {
    acs_print(ACS_PRINT_ERR, L" Could not Set Memory Attribute %x\n", Status);
    return NULL;
  }

  *Pa = (VOID *)Address;
  return (VOID *)Address;
}

/**
  @brief  Free the cacheable memory region allocated above

  @param  Bdf          Bus, Device, and Function of the requesting PCIe device
  @param  Size         allocation size in bytes
  @param  Va           Pointer to Virtual Addr
  @param  Pa           Pointer to Physical Addr

  @return None
**/
VOID
pal_mem_free_cacheable (
  UINT32 Bdf,
  UINT32 Size,
  VOID *Va,
  VOID *Pa
  )
{
  gBS->FreePages((EFI_PHYSICAL_ADDRESS)(UINTN)Va, EFI_SIZE_TO_PAGES(Size));
}

