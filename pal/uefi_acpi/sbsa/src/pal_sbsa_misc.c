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
  @brief  Allocates requested buffer size in bytes in a contiguous cacheable
          memory and returns the base address of the range.

  @param  Bdf          Bus, Device, and Function of the requesting PCIe device
  @param  Size         allocation size in bytes
  @param  Pa           Pointer to Physical Addr

  @return if SUCCESS   Pointer to Virtual Addr ; if FAILURE   NULL
**/
VOID *
pal_mem_alloc_at_address (
  UINT64 mem_base,
  UINT64 Size
  )
{
  EFI_STATUS Status;
  EFI_PHYSICAL_ADDRESS PageBase;

  PageBase = mem_base;
  Status = gBS->AllocatePages (AllocateAddress,
                               EfiBootServicesData,
                               EFI_SIZE_TO_PAGES(Size),
                               &PageBase);
  if (EFI_ERROR(Status))
  {
    acs_print(ACS_PRINT_ERR, L" Allocate Pages failed %x\n", Status);
    return NULL;
  }

  return (VOID*)(UINTN)PageBase;
}

/**
  @brief Free number of pages in the memory as requested.

  @param PageBase Address from where we need to free
  @param NumPages Number of memory pages needed

  @return None
**/
VOID
pal_mem_free_at_address(
  UINT64 mem_base,
  UINT64 Size
  )
{
  gBS->FreePages(mem_base, EFI_SIZE_TO_PAGES(Size));
}

