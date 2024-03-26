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

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "Include/IndustryStandard/Acpi61.h"
#include "Include/IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h"
#include <Protocol/AcpiTable.h>
#include <Protocol/HardwareInterrupt.h>

#include "Include/IndustryStandard/Pci.h"
#include "Include/IndustryStandard/Pci22.h"
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>

#include "../include/platform_override.h"
#include "common/include/pal_uefi.h"
#include "common/include/bsa_pcie_enum.h"

/**
    @brief   Checks if device is behind SMMU

    @param   seg        PCI segment number
    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number

    @retval 1 if device is behind SMMU
    @retval 0 if device is not behind SMMU or SMMU is in bypass mode
**/
UINT32
pal_pcie_is_device_behind_smmu(UINT32 seg, UINT32 bus, UINT32 dev, UINT32 fn)
{
      return 0;
}

/**
    @brief   Return the DMA addressability of the device

    @param   seg        PCI segment number
    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number

    @retval 0 if does not support 64-bit transfers
    @retval 1 if supports 64-bit transfers
**/
UINT32
pal_pcie_is_devicedma_64bit(UINT32 seg, UINT32 bus, UINT32 dev, UINT32 fn)
{
  return 0;
}
