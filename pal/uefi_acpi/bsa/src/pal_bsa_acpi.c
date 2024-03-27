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
#include "common/include/pal_uefi.h"

/**
  @brief Dump DTB to file

  @param None

  @return None
**/
VOID
pal_dump_dtb()
{
  acs_print(ACS_PRINT_ERR, L" DTB dump not available for platform initialized"
                                " with ACPI table\n");
}
/**
  @brief   Checks if System information is passed using Device Tree (DT)
           This api is also used to check if GIC/Interrupt Init ACS Code
           is used or not. In case of DT, ACS Code is used for INIT

  @param  None

  @return True/False
*/
UINT32
pal_target_is_dt()
{
  return 0;
}
