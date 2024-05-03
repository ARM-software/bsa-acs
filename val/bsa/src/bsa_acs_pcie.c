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

#include "common/include/acs_val.h"
#include "common/include/acs_common.h"
#include "common/include/acs_pcie_enumeration.h"

#include "common/include/acs_pcie.h"
#include "common/sys_arch_src/pcie/pcie.h"
#include "bsa/include/bsa_pal_interface.h"
#include "bsa/include/bsa_val_interface.h"
#include "bsa/include/bsa_acs_pcie.h"

#define WARN_STR_LEN 7

/**
  @brief   This API checks if device is behind SMMU
           1. Caller       -  Test Suite
  @param   bdf      - PCIe BUS/Device/Function
  @return  0 -> not present, nonzero -> present
**/
uint32_t
val_pcie_is_device_behind_smmu(uint32_t bdf)
{
  return pal_pcie_is_device_behind_smmu(PCIE_EXTRACT_BDF_SEG (bdf),
                                        PCIE_EXTRACT_BDF_BUS (bdf),
                                        PCIE_EXTRACT_BDF_DEV (bdf),
                                        PCIE_EXTRACT_BDF_FUNC (bdf));
}

/**
  @brief   This API checks if device is capable of 64-bit DMA
           1. Caller       -  Test Suite
  @param   bdf      - PCIe BUS/Device/Function
  @return  0 -> not 64-bit DMA capable, 1 -> 64-bit DMA capable
**/
uint32_t
val_pcie_is_devicedma_64bit(uint32_t bdf)
{

  uint32_t seg  = PCIE_EXTRACT_BDF_SEG (bdf);
  uint32_t bus  = PCIE_EXTRACT_BDF_BUS (bdf);
  uint32_t dev  = PCIE_EXTRACT_BDF_DEV (bdf);
  uint32_t func = PCIE_EXTRACT_BDF_FUNC (bdf);

  return pal_pcie_is_devicedma_64bit(seg, bus, dev, func);
}

/**
  @brief   This API checks if device driver present for a pcie device
           1. Caller       -  Test Suite
  @param   bdf      - PCIe BUS/Device/Function
  @return  0 -> not Present, 1 -> Present
**/
uint32_t
val_pcie_device_driver_present(uint32_t bdf)
{

  uint32_t seg  = PCIE_EXTRACT_BDF_SEG (bdf);
  uint32_t bus  = PCIE_EXTRACT_BDF_BUS (bdf);
  uint32_t dev  = PCIE_EXTRACT_BDF_DEV (bdf);
  uint32_t func = PCIE_EXTRACT_BDF_FUNC (bdf);

  return pal_pcie_device_driver_present(seg, bus, dev, func);
}

/**
  @brief  Clears Error detected bit in Device Status Register

  @param  bdf   - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @return None
**/
void
val_pcie_clear_device_status_error(uint32_t bdf)
{

  uint32_t pciecs_base;
  uint32_t reg_value;

  /*
   * Get the PCI Express Capability structure offset and use that
   * offset to write 1b to clear CED, NFED, FED, URD bit in Device Status register
   */
  val_pcie_find_capability(bdf, PCIE_CAP, CID_PCIECS, &pciecs_base);
  val_pcie_read_cfg(bdf, pciecs_base + DCTLR_OFFSET, &reg_value);
  reg_value = reg_value | (0xF << DCTLR_DSR_SHIFT);
  val_pcie_write_cfg(bdf, pciecs_base + DCTLR_OFFSET, reg_value);

}

/**
  @brief  Check Error detected bit in Device Status Register

  @param  bdf   - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @return 1 if Error is detected, 0 if No Error
**/
uint32_t
val_pcie_is_device_status_error(uint32_t bdf)
{

  uint32_t pciecs_base;
  uint32_t reg_value;

  /*
   * Get the PCI Express Capability structure offset and use that
   * offset to check CED, NFED, FED, URD bit in Device Status register
   */
  val_pcie_find_capability(bdf, PCIE_CAP, CID_PCIECS, &pciecs_base);
  val_pcie_read_cfg(bdf, pciecs_base + DCTLR_OFFSET, &reg_value);

  if (reg_value & (0xF << DCTLR_DSR_SHIFT))
      return 1;

  return 0;
}

/**
  @brief  Clear Signaled Target Abort bit in Status/Secondary Status Register
          in Root Port
  @param  bdf   - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @return None
**/
void
val_pcie_clear_sig_target_abort(uint32_t bdf)
{

  uint32_t status_val;
  uint32_t sec_status_val;

  /* Read Status Register at 0x4 Offset */
  val_pcie_read_cfg(bdf, TYPE01_CR, &status_val);
  val_pcie_write_cfg(bdf, TYPE01_CR, (status_val | (1 << SR_STA_SHIFT)));

  /* Read Secondary Status Register at 0x1C Offset */
  val_pcie_read_cfg(bdf, TYPE1_SEC_STA, &sec_status_val);
  val_pcie_write_cfg(bdf, TYPE1_SEC_STA, (sec_status_val | (1 << SSR_STA_SHIFT)));
}

/**
  @brief  Check Signaled Target Abort bit in Status/Secondary Status Register
          in Root Port
  @param  bdf   - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @return 1 if STA Bit Set, 0 if Not Set
**/
uint32_t
val_pcie_is_sig_target_abort(uint32_t bdf)
{

  uint32_t status_val;
  uint32_t sec_status_val;

  /* Read Status Register at 0x4 Offset */
  val_pcie_read_cfg(bdf, TYPE01_CR, &status_val);
  /* Read Secondary Status Register at 0x1C Offset */
  val_pcie_read_cfg(bdf, TYPE1_SEC_STA, &sec_status_val);

  if (((status_val >> SR_STA_SHIFT) & SR_STA_MASK) ||
      ((sec_status_val >> SSR_STA_SHIFT) & SSR_STA_MASK))
      return 1;

  return 0;
}

/**
  @brief  Checks if a device has rootport as a parent and returns bdf if present

  @param  dsf_bdf - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @param  *rp_bdf - If RP Found, to store the Root Port BDF

  @return Returns 0 if found, otherwise 1
**/
uint8_t
val_pcie_parent_is_rootport(uint32_t dsf_bdf, uint32_t *rp_bdf)
{

  uint8_t dsf_bus;
  uint32_t bdf;
  uint32_t dp_type;
  uint32_t tbl_index;
  uint32_t reg_value;
  pcie_device_bdf_table *bdf_tbl_ptr;

  tbl_index = 0;
  dsf_bus = PCIE_EXTRACT_BDF_BUS(dsf_bdf);
  bdf_tbl_ptr = val_pcie_bdf_table_ptr();

  while (tbl_index < bdf_tbl_ptr->num_entries)
  {
      bdf = bdf_tbl_ptr->device[tbl_index++].bdf;
      dp_type = val_pcie_device_port_type(bdf);

      /* Check if this table entry is a Root Port */
      if ((dp_type == RP) || (dp_type == iEP_RP))
      {
         /* Check if device is a direct child of this root port */
          val_pcie_read_cfg(bdf, TYPE1_PBN, &reg_value);
          if ((dsf_bus == ((reg_value >> SECBN_SHIFT) & SECBN_MASK)) &&
              (dsf_bus <= ((reg_value >> SUBBN_SHIFT) & SUBBN_MASK)))
          {
              *rp_bdf = bdf;
              return 0;
          }
      }
  }

  return 1;
}

/**
  @brief  Returns whether a PCIe Function has detected an Interrupt request

  @param  bdf        - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @return Returns 1 - if the Function has received an Interrupt Request or
                  0 - if it does not detect Interrupt request
**/
uint32_t
val_pcie_check_interrupt_status(uint32_t bdf)
{

  uint32_t reg_value;

  val_pcie_read_cfg(bdf, TYPE01_CR, &reg_value);
  reg_value = (reg_value >> SR_IS_SHIFT) & SR_IS_MASK;

  return reg_value;
}

/**
  @brief  Returns Maximum PASID Width for this bdf

  @param  bdf        - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @param  *max_pasid_width - Stores the max pasid value

  @return Returns 0 if success
**/
uint32_t
val_pcie_get_max_pasid_width(uint32_t bdf, uint32_t *max_pasid_width)
{
  uint32_t status;
  uint32_t pciecs_base;

  status = val_pcie_find_capability(bdf, PCIE_ECAP, ECID_PASID, &pciecs_base);
  if (status)
      return status;

  val_pcie_read_cfg(bdf, pciecs_base + PASID_CAPABILITY_OFFSET, max_pasid_width);
  *max_pasid_width = (*max_pasid_width & MAX_PASID_MASK) >> MAX_PASID_SHIFT;

  return 0;
}

/**
  @brief  Checks if the Transaction pending bit is set in device status register

  @param  bdf   - Segment/Bus/Dev/Func in PCIE_CREATE_BDF format
  @return 0 - TP bit not set, 1 - TP bit set
**/
uint32_t
val_is_transaction_pending_set(uint32_t bdf)
{
  uint32_t pciecs_base;
  uint32_t reg_value;
  uint32_t status;

  /* Get the PCI Express Capability structure offset and
   * use that offset to read pci express capabilities register
   */
  val_pcie_find_capability(bdf, PCIE_CAP, CID_PCIECS, &pciecs_base);
  status = val_pcie_read_cfg(bdf, pciecs_base + DCTLR_OFFSET, &reg_value);

  if (status) {
      val_print(ACS_PRINT_ERR, "\n       Error in reading transaction pending bit", 0);
      return 1;
  }

  reg_value &= DSR_TP_MASK << (DCTLR_DSR_SHIFT + DSR_TP_SHIFT);
  if (reg_value)
      return 1;

  return 0;
}
