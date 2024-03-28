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

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>

#include "common/include/pal_uefi.h"
#include "common/include/pal_uefi.h"
#include "common/include/pal_exerciser.h"

UINT32
pal_mmio_read(UINT64 addr);

UINT64
pal_exerciser_get_ecsr_base (UINT32 Bdf, UINT32 BarIndex);

/**
  @brief   This API disables the RP-PIO register support of the RP
  @param   type         - RP BDF of which the RP-PIO needs to be disabled
  @return  None
**/
VOID
pal_exerciser_disable_rp_pio_register(UINT32 bdf)
{
/* Code to disable the RP_PIO register for the RP BDF */

}

/**
  @brief   This API checks if forwarding poison data forwarding is supported or not
  @return  status      - 1 if poison data forwarding is supported
                         0 if poison data forwarding is not supported
**/
UINT32
pal_exerciser_check_poison_data_forwarding_support()
{
  return 1;
}

/**
  @brief   This API return the RAS node that records the PCIe errors
  @param   bdf         - BDF of the device
  @param   rp_bdf      - Root port BDF of the device
  @return  status      - RAS node that records the PCIe errors
**/
UINT32
pal_exerciser_get_pcie_ras_compliant_err_node(UINT32 bdf, UINT32 rp_bdf)
{
  return rp_bdf;
}

/**
  @brief   This API return the status register of the RAS node that recorded the PCIe errors
  @param   ras_node    - RAS node that recorded the PCIe errors
  @param   bdf         - BDF of the device
  @param   rp_bdf      - Root port BDF of the device
  @return  status      - status register value of the RAS node that recorded the PCIe errors
**/
UINT64
pal_exerciser_get_ras_status(UINT32 ras_node, UINT32 bdf, UINT32 rp_bdf)
{

  UINT64 EcamBAR0;
  UINT64 status_reg;
  UINT32 data;

  EcamBAR0 = pal_exerciser_get_ecsr_base(rp_bdf, 0);
  EcamBAR0 = EcamBAR0 & BAR64_MASK;
  status_reg = EcamBAR0 + RAS_OFFSET + STATUS_OFFSET;
  data = pal_mmio_read(status_reg);
  return data;
}

/**
  @brief   This API ensures that an external abort is obtained when MMIO soace is targeted
           with reads
  @param   bdf         - BDF of the device
  @return  status      - 0 if implemented, else
                       - NOT_IMPLEMENTED
**/
UINT32
pal_exerciser_set_bar_response(UINT32 bdf)
{
  return 0;
}
