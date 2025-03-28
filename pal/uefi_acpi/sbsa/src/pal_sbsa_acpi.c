/** @file
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
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
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ShellLib.h>
#include "Include/IndustryStandard/ArmErrorSourceTable.h"

#include "Include/Guid/Acpi.h"
#include <Protocol/AcpiTable.h>
#include "Include/IndustryStandard/Acpi61.h"

#include "common/include/pal_uefi.h"
#include "common/include/pal_mpam.h"
#include "sbsa/include/pal_sbsa_uefi.h"
#include "sbsa/include/pal_sbsa_pmu.h"

UINT64
pal_get_xsdt_ptr();


/**
    @brief  Iterate through the tables pointed by XSDT and return AEST Table address

    @param  None

    @return 64-bit AEST address
  **/
UINT64
pal_get_aest_ptr()
{
  EFI_ACPI_DESCRIPTION_HEADER   *Xsdt;
  UINT64                        *Entry64;
  UINT32                        Entry64Num;
  UINT32                        Idx;

  Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *) pal_get_xsdt_ptr();
  if (Xsdt == NULL) {
      acs_print(ACS_PRINT_ERR, L" XSDT not found\n");
      return 0;
  }

  Entry64  = (UINT64 *)(Xsdt + 1);
  Entry64Num = (Xsdt->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) >> 3;
  for (Idx = 0; Idx < Entry64Num; Idx++) {
    if (*(UINT32 *)(UINTN)(Entry64[Idx]) == EFI_ACPI_6_3_ARM_ERROR_SOURCE_TABLE_SIGNATURE) {
      return(UINT64)(Entry64[Idx]);
    }
  }

  return 0;
}

  /**
    @brief  Iterate through the tables pointed by XSDT and return APMT Table address

    @param  None

    @return 64-bit APMT address
  **/
UINT64
pal_get_apmt_ptr()
{
  EFI_ACPI_DESCRIPTION_HEADER   *Xsdt;
  UINT64                        *Entry64;
  UINT32                        Entry64Num;
  UINT32                        Idx;

  Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *) pal_get_xsdt_ptr();
  if (Xsdt == NULL) {
      acs_print(ACS_PRINT_ERR, L" XSDT not found\n");
      return 0;
  }

  Entry64  = (UINT64 *)(Xsdt + 1);
  Entry64Num = (Xsdt->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) >> 3;
  for (Idx = 0; Idx < Entry64Num; Idx++) {
    if (*(UINT32 *)(UINTN)(Entry64[Idx]) == ARM_PERFORMANCE_MONITORING_TABLE_SIGNATURE) {
      return(UINT64)(Entry64[Idx]);
    }
  }

  return 0;
}
