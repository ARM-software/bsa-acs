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

#include "sbsa/include/sbsa_val_interface.h"
#include "common/include/acs_val.h"
#include "common/include/acs_common.h"
#include "common/include/acs_pcie.h"

uint64_t
pal_get_mcfg_ptr(void);

/**
  @brief   This API checks the PCIe device multifunction support
           1. Caller       -  Test Suite
  @param   bdf      - PCIe BUS/Device/Function
  @return  1 - Multifunction not supported 0 - Multifunction supported
**/
uint32_t
val_pcie_multifunction_support(uint32_t bdf)
{
  uint32_t reg_data;
  val_pcie_read_cfg(bdf, TYPE01_CLSR, &reg_data);
  reg_data = ((reg_data >> TYPE01_HTR_SHIFT) & TYPE01_HTR_MASK);

  return !((reg_data >> HTR_MFD_SHIFT) & HTR_MFD_MASK);
}

/**
  @brief  Gets RP support of transaction forwarding.

  @param  bdf   - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @return 0 if rp not involved in transaction forwarding
**/
uint32_t
val_pcie_get_rp_transaction_frwd_support(uint32_t bdf)
{
  return pal_pcie_get_rp_transaction_frwd_support(PCIE_EXTRACT_BDF_SEG (bdf),
                                                  PCIE_EXTRACT_BDF_BUS (bdf),
                                                  PCIE_EXTRACT_BDF_DEV (bdf),
                                                  PCIE_EXTRACT_BDF_FUNC(bdf));
}

/**
  @brief  Returns whether a PCIe Function has an Address Translation Cache

  @param  bdf        - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @return Returns 1  - if Function doesn't have Addr Translation Cache else 0.
**/
uint32_t
val_pcie_is_cache_present(uint32_t bdf)
{
  return pal_pcie_is_cache_present(PCIE_EXTRACT_BDF_SEG (bdf),
                                   PCIE_EXTRACT_BDF_BUS (bdf),
                                   PCIE_EXTRACT_BDF_DEV (bdf),
                                   PCIE_EXTRACT_BDF_FUNC (bdf));
}

/**
  @brief  Checks if link Capabilities is supported

  @param  bdf    -  Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @return 0 if link capability is not supported else 1.
**/
uint32_t
val_pcie_link_cap_support(uint32_t bdf)
{
  uint32_t pciecs_base;
  uint32_t reg_value = 0xFFFFFFFF;

  val_pcie_find_capability(bdf, PCIE_CAP, CID_PCIECS, &pciecs_base);
  val_pcie_read_cfg(bdf, pciecs_base + LCAPR_OFFSET, &reg_value);

  if (reg_value != 0) {
     val_print(ACS_PRINT_ERR, "\n       Link Capabilities reg check failed", 0);
     return 1;
  }

  reg_value = 0xFFFFFFFF;
  val_pcie_read_cfg(bdf, pciecs_base + LCTRLR_OFFSET, &reg_value);
  if (reg_value != 0) {
     val_print(ACS_PRINT_ERR, "\n       Link Capabilities control and status check failed", 0);
     return 1;
  }

  reg_value = 0xFFFFFFFF;
  val_pcie_read_cfg(bdf, pciecs_base + LCAP2R_OFFSET, &reg_value);
  if (reg_value != 0) {
     val_print(ACS_PRINT_ERR, "\n       Link Capabilities 2 reg check failed", 0);
     return 1;
  }

  reg_value = 0xFFFFFFFF;
  val_pcie_read_cfg(bdf, pciecs_base + LCTL2R_OFFSET, &reg_value);
  if (reg_value != 0) {
     val_print(ACS_PRINT_ERR, "\n       Link Capabilities 2 control and status check failed", 0);
     return 1;
  }

  return 0;
}

/**
  @brief   This API scans bridge devices and checks memory type of TYPE1 devices.
  @param   bdf      - PCIe BUS/Device/Function of TYPE1 device

  @return  0 -> 32-bit mem type, 1 -> 64-bit mem type
**/
uint32_t val_pcie_scan_bridge_devices_and_check_memtype(uint32_t bdf)
{

  uint32_t Bus, Dev, Func;
  uint32_t sec_bus, sub_bus;
  uint32_t status = 0;
  uint32_t reg_value;
  uint32_t data;
  uint8_t  mem_type;

  val_pcie_read_cfg(bdf, TYPE1_PBN, &reg_value);
  sec_bus = ((reg_value >> SECBN_SHIFT) & SECBN_MASK);
  sub_bus = ((reg_value >> SUBBN_SHIFT) & SUBBN_MASK);

  for (Bus = sec_bus; Bus <= sub_bus; Bus++)
  {
      for (Dev = 0; Dev < PCIE_MAX_DEV; Dev++)
      {
          for (Func = 0; Func < PCIE_MAX_FUNC; Func++)
          {
              if (val_pcie_function_header_type(bdf) == TYPE0_HEADER)
              {
                  val_pcie_read_cfg(bdf, TYPE01_BAR, &data);
                  if (data)
                  {
                      mem_type = ((data >> BAR_MDT_SHIFT) & BAR_MDT_MASK);
                      if (mem_type != 0) {
                          status = 1;
                          break;
                      }
                  }
              }
          }
      }
  }

  return status;
}

/**
  @brief  Returns whether a PCIe Function is atomicop requester capable

  @param  bdf        - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @return Returns 0 (if Function doesn't supports atomicop requester capable
                     else non-zero value)
**/
uint32_t
val_pcie_get_atomicop_requester_capable(uint32_t bdf)
{
  /* TO DO */
  //return pal_pcie_get_atomicop_requester_capable(bdf);
  (void) bdf;

  return 0;
}