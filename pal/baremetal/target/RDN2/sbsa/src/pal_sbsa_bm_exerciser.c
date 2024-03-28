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

#include "pal_common_support.h"
#include "pal_pcie_enum.h"
#include "platform_override_sbsa_fvp.h"

uint64_t
pal_exerciser_get_ecsr_base(uint32_t Bdf, uint32_t BarIndex);

/**
  @brief   This API disables the RP-PIO register support of the RP
  @param   type         - RP BDF of which the RP-PIO needs to be disabled
  @return  None
**/
void
pal_exerciser_disable_rp_pio_register(uint32_t bdf)
{
/* Code to disable the RP_PIO register for the RP BDF */
  (void) bdf;
}

/**
  @brief   This API checks if forwarding poison data forwarding is supported or not
  @return  status      - 1 if poison data forwarding is supported
                         0 if poison data forwarding is not supported
**/
uint32_t
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
uint32_t
pal_exerciser_get_pcie_ras_compliant_err_node(uint32_t bdf, uint32_t rp_bdf)
{
  (void) bdf;
  return rp_bdf;
}

/**
  @brief   This API return the status register of the RAS node that recorded the PCIe errors
  @param   ras_node    - RAS node that recorded the PCIe errors
  @param   bdf         - BDF of the device
  @param   rp_bdf      - Root port BDF of the device
  @return  status      - status register value of the RAS node that recorded the PCIe errors
**/
uint64_t
pal_exerciser_get_ras_status(uint32_t ras_node, uint32_t bdf, uint32_t rp_bdf)
{
  (void) bdf;
  (void) ras_node;
  uint64_t EcamBAR0;
  uint64_t status_reg;

  EcamBAR0 = pal_exerciser_get_ecsr_base(rp_bdf, 0);
  EcamBAR0 = EcamBAR0 & BAR64_MASK;
  status_reg = EcamBAR0 + RAS_OFFSET + STATUS_OFFSET;
  return status_reg;
}

/**
  @brief   This API ensures that an external abort is obtained when MMIO soace is targeted
           with reads
  @param   bdf         - BDF of the device
  @return  status      - 0 if implemented, else
                       - NOT_IMPLEMENTED
**/
uint32_t
pal_exerciser_set_bar_response(uint32_t bdf)
{
  (void) bdf;
  return 0;
}