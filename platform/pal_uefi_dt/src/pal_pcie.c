/** @file
 * Copyright (c) 2016-2023, Arm Limited or its affiliates. All rights reserved.
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

#include <Include/libfdt.h>
#include "../include/platform_override.h"
#include "include/pal_uefi.h"
#include "include/bsa_pcie_enum.h"
#include "include/pal_dt.h"
#include "include/pal_dt_spec.h"

static char pci_dt_arr[][PCI_COMPATIBLE_STR_LEN] = {
       "pci-host-ecam-generic"
};

static EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER *gMcfgHdr;

PCIE_INFO_TABLE *g_pal_pcie_info_table;

UINT64
pal_get_mcfg_ptr();

/**
  @brief  Returns the PCI ECAM address from the ACPI MCFG Table address

  @param  None

  @return  None
**/
UINT64
pal_pcie_get_mcfg_ecam()
{
  if (g_pal_pcie_info_table)
      return g_pal_pcie_info_table->block[0].ecam_base;

  return 0;
}


/**
  @brief  Fill the PCIE Info table with the details of the PCIe sub-system

  @param  PcieTable - Address where the PCIe information needs to be filled.

  @return  None
 **/
VOID
pal_pcie_create_info_table(PCIE_INFO_TABLE *PcieTable)
{

  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE  *Entry = NULL;
  UINT32 length = sizeof(EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER);
  UINT32 i = 0;

  if (PcieTable == NULL) {
    bsa_print(ACS_PRINT_ERR, L" Input PCIe Table Pointer is NULL. Cannot create PCIe INFO\n");
    return;
  }

  g_pal_pcie_info_table = PcieTable;
  PcieTable->num_entries = 0;

  pal_pcie_create_info_table_dt(PcieTable);
  return;

  gMcfgHdr = (EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER *) pal_get_mcfg_ptr();

  if(PLATFORM_OVERRIDE_PCIE_ECAM_BASE) {
      PcieTable->block[i].ecam_base = PLATFORM_OVERRIDE_PCIE_ECAM_BASE;
      PcieTable->block[i].start_bus_num = PLATFORM_OVERRIDE_PCIE_START_BUS_NUM;
      PcieTable->block[i].segment_num = 0;
      PcieTable->num_entries = 1;
      return;
  }

  Entry = (EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE *) (gMcfgHdr + 1);

  do{
      if (Entry == NULL)  //Due to a buggy MCFG - first entry is null, then exit
          break;
      PcieTable->block[i].ecam_base     = Entry->BaseAddress;
      PcieTable->block[i].segment_num   = Entry->PciSegmentGroupNumber;
      PcieTable->block[i].start_bus_num = Entry->StartBusNumber;
      PcieTable->block[i].end_bus_num   = Entry->EndBusNumber;
      length += sizeof(EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE);
      Entry++;
      i++;
      PcieTable->num_entries++;
  } while((length < gMcfgHdr->Header.Length) && (Entry));

  return;
}

/**
    @brief   Reads 32-bit data from PCIe config space pointed by Bus,
           Device, Function and register offset, using UEFI PciIoProtocol

    @param   Bdf      - BDF value for the device
    @param   offset - Register offset within a device PCIe config space
    @param   *data - 32 bit value at offset from ECAM base of the device specified by BDF value
    @return  success/failure
**/
UINT32
pal_pcie_io_read_cfg(UINT32 Bdf, UINT32 offset, UINT32 *data)
{

  EFI_STATUS                    Status;
  EFI_PCI_IO_PROTOCOL           *Pci;
  UINTN                         HandleCount;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         Seg, Bus, Dev, Func;
  UINT32                        Index;
  UINT32                        InputSeg, InputBus, InputDev, InputFunc;


  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiPciIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR (Status)) {
    bsa_print(ACS_PRINT_INFO,L" No PCI devices found in the system\n");
    return PCIE_NO_MAPPING;
  }

  InputSeg = PCIE_EXTRACT_BDF_SEG(Bdf);
  InputBus = PCIE_EXTRACT_BDF_BUS(Bdf);
  InputDev = PCIE_EXTRACT_BDF_DEV(Bdf);
  InputFunc = PCIE_EXTRACT_BDF_FUNC(Bdf);

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiPciIoProtocolGuid, (VOID **)&Pci);
    if (!EFI_ERROR (Status)) {
      Pci->GetLocation (Pci, &Seg, &Bus, &Dev, &Func);
      if (InputSeg == Seg && InputBus == Bus && InputDev == Dev && InputFunc == Func) {
          Status = Pci->Pci.Read (Pci, EfiPciIoWidthUint32, offset, 1, data);
          pal_mem_free(HandleBuffer);
          if (!EFI_ERROR (Status))
            return 0;
          else
            return PCIE_NO_MAPPING;
      }
    }
  }

  pal_mem_free(HandleBuffer);
  return PCIE_NO_MAPPING;
}

/**
    @brief Write 32-bit data to PCIe config space pointed by Bus,
           Device, Function and register offset, using UEFI PciIoProtocol

    @param   Bdf      - BDF value for the device
    @param   offset - Register offset within a device PCIe config space
    @param   data - 32 bit value at offset from ECAM base of the device specified by BDF value
    @return  success/failure
**/
VOID
pal_pcie_io_write_cfg(UINT32 Bdf, UINT32 offset, UINT32 data)
{

  EFI_STATUS                    Status;
  EFI_PCI_IO_PROTOCOL           *Pci;
  UINTN                         HandleCount;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         Seg, Bus, Dev, Func;
  UINT32                        Index;
  UINT32                        InputSeg, InputBus, InputDev, InputFunc;


  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiPciIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR (Status)) {
    bsa_print(ACS_PRINT_INFO,L" No PCI devices found in the system\n");
    return;
  }

  InputSeg = PCIE_EXTRACT_BDF_SEG(Bdf);
  InputBus = PCIE_EXTRACT_BDF_BUS(Bdf);
  InputDev = PCIE_EXTRACT_BDF_DEV(Bdf);
  InputFunc = PCIE_EXTRACT_BDF_FUNC(Bdf);

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiPciIoProtocolGuid, (VOID **)&Pci);
    if (!EFI_ERROR (Status)) {
      Pci->GetLocation (Pci, &Seg, &Bus, &Dev, &Func);
      if (InputSeg == Seg && InputBus == Bus && InputDev == Dev && InputFunc == Func) {
          Status = Pci->Pci.Write (Pci, EfiPciIoWidthUint32, offset, 1, &data);
      }
    }
  }

  pal_mem_free(HandleBuffer);
}

/**
    @brief   Reads 32-bit data from BAR space pointed by Bus,
             Device, Function and register offset, using UEFI PciRootBridgeIoProtocol

    @param   Bdf     - BDF value for the device
    @param   address - BAR memory address
    @param   *data   - 32 bit value at BAR address
    @return  success/failure
**/
UINT32
pal_pcie_bar_mem_read(UINT32 Bdf, UINT64 address, UINT32 *data)
{

  EFI_STATUS                       Status;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *Pci;
  UINTN                            HandleCount;
  EFI_HANDLE                       *HandleBuffer;
  UINT32                           Index;
  UINT32                           InputSeg;


  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiPciRootBridgeIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR (Status)) {
    bsa_print(ACS_PRINT_INFO,L" No Root Bridge found in the system\n");
    return PCIE_NO_MAPPING;
  }

  InputSeg = PCIE_EXTRACT_BDF_SEG(Bdf);

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiPciRootBridgeIoProtocolGuid, (VOID **)&Pci);
    if (!EFI_ERROR (Status)) {
      if (Pci->SegmentNumber == InputSeg) {
          Status = Pci->Mem.Read (Pci, EfiPciIoWidthUint32, address, 1, data);
          pal_mem_free(HandleBuffer);
          if (!EFI_ERROR (Status))
            return 0;
          else
            return PCIE_NO_MAPPING;
      }
    }
  }

  pal_mem_free(HandleBuffer);
  return PCIE_NO_MAPPING;
}

/**
    @brief   Write 32-bit data to BAR space pointed by Bus,
             Device, Function and register offset, using UEFI PciRootBridgeIoProtocol

    @param   Bdf     - BDF value for the device
    @param   address - BAR memory address
    @param   data    - 32 bit value to writw BAR address
    @return  success/failure
**/

UINT32
pal_pcie_bar_mem_write(UINT32 Bdf, UINT64 address, UINT32 data)
{

  EFI_STATUS                       Status;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *Pci;
  UINTN                            HandleCount;
  EFI_HANDLE                       *HandleBuffer;
  UINT32                           Index;
  UINT32                           InputSeg;


  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiPciRootBridgeIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR (Status)) {
    bsa_print(ACS_PRINT_INFO,L" No Root Bridge found in the system\n");
    return PCIE_NO_MAPPING;
  }

  InputSeg = PCIE_EXTRACT_BDF_SEG(Bdf);

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiPciRootBridgeIoProtocolGuid, (VOID **)&Pci);
    if (!EFI_ERROR (Status)) {
      if (Pci->SegmentNumber == InputSeg) {
          Status = Pci->Mem.Write (Pci, EfiPciIoWidthUint32, address, 1, &data);
          pal_mem_free(HandleBuffer);
          if (!EFI_ERROR (Status))
            return 0;
          else
            return PCIE_NO_MAPPING;
      }
    }
  }

  pal_mem_free(HandleBuffer);
  return PCIE_NO_MAPPING;
}

/**
  @brief   This API checks the PCIe Hierarchy Supports P2P
           1. Caller       -  Test Suite
  @return  1 - P2P feature not supported 0 - P2P feature supported
**/
UINT32
pal_pcie_p2p_support()
{
  /*
   * This is platform specific API which needs to be populated with system p2p capability
   * PCIe support for peer to peer
   * transactions is platform implementation specific
   */
  if (g_pcie_p2p)
      return 0;
  else
      return NOT_IMPLEMENTED;
}

/**
  @brief   This API checks the PCIe device P2P support
           1. Caller       -  Test Suite

  @param   Seg       PCI segment number
  @param   Bus        PCI bus address
  @param   Dev        PCI device address
  @param   Fn         PCI function number
  @return  1 - P2P feature not supported 0 - P2P feature supported
**/
UINT32
pal_pcie_dev_p2p_support (
  UINT32 Seg,
  UINT32 Bus,
  UINT32 Dev,
  UINT32 Fn)
{
  /*
   * This is platform specific API which needs to be populated with pcie device  p2p capability
   * Root port or Switch support for peer to peer
   * transactions is platform implementation specific
   */

  return 1;
}

/**
    @brief   Create a list of MSI(X) vectors for a device

    @param   Seg        PCI segment number
    @param   Bus        PCI bus address
    @param   Dev        PCI device address
    @param   Fn         PCI function number
    @param   MVector    pointer to a MSI(X) list address

    @return  mvector    list of MSI(X) vectors
    @return  number of MSI(X) vectors
**/
UINT32
pal_get_msi_vectors (
  UINT32 Seg,
  UINT32 Bus,
  UINT32 Dev,
  UINT32 Fn,
  PERIPHERAL_VECTOR_LIST **MVector
  )
{
  return 0;
}

/**
    @brief   Get legacy IRQ routing for a PCI device
             This is Platform dependent API and needs to be filled
             with legacy IRQ map for a pcie devices.
    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number
    @param   irq_map    pointer to IRQ map structure

    @return  irq_map    IRQ routing map
    @return  status code If the device legacy irq map information is filled
                         return 0, else returns NOT_IMPLEMENTED
**/
UINT32
pal_pcie_get_legacy_irq_map (
  UINT32 Seg,
  UINT32 Bus,
  UINT32 Dev,
  UINT32 Fn,
  PERIPHERAL_IRQ_MAP *IrqMap
  )
{
  return NOT_IMPLEMENTED;
}

/** Place holder function. Need to be implemented if needed in later releases
  @brief Returns the Bus, Device, and Function values of the Root Port of the device.

  @param   Seg        PCI segment number
  @param   Bus        PCI bus address
  @param   Dev        PCI device address
  @param   Fn         PCI function number

  @return 0 if success; 1 if input BDF device cannot be found
          2 if root Port for the input device cannot be determined
**/
UINT32
pal_pcie_get_root_port_bdf (
  UINT32 *Seg,
  UINT32 *Bus,
  UINT32 *Dev,
  UINT32 *Func
  )
{
  return 0;
}

/**
  @brief   Checks the Address Translation Cache Support for BDF
           Platform dependent API. Fill this with system ATC support
           information for bdf's
           1. Caller       -  Test Suite
  @return  0 - ATC not supported 1 - ATC supported
**/
UINT32
pal_pcie_is_cache_present (
  UINT32 Seg,
  UINT32 Bus,
  UINT32 Dev,
  UINT32 Fn
  )
{
  if (g_pcie_cache_present)
      return 1;
  else
      return NOT_IMPLEMENTED;
}

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
  @brief  Returns whether a PCIe Function is an on-chip peripheral or not

  @param  bdf        - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @return Returns TRUE if the Function is on-chip peripheral, FALSE if it is
          not an on-chip peripheral
**/
UINT32
pal_pcie_is_onchip_peripheral(UINT32 bdf)
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

/**
  @brief  Checks the discovered PCIe hierarchy is matching with the
          topology described in info table.
  @return Returns 0 if device entries matches , 1 if there is mismatch.
**/
UINT32
pal_pcie_check_device_list(void)
{
  return 0;
}

/**
  @brief  This API fills in the PCIE_INFO Table with information about the PCIe's in the
          system. This is achieved by parsing the DT blob.

  @param  PcieTable  - Address where the PcieTable information needs to be filled.

  @return  None
**/
VOID
pal_pcie_create_info_table_dt(PCIE_INFO_TABLE *PcieTable)
{
  UINT64 dt_ptr;
  UINT32 *Preg_val, *Pbus_val;
  int prop_len, addr_cell, size_cell;
  int offset, parent_offset, i;

  if (PcieTable == NULL)
    return;

  dt_ptr = pal_get_dt_ptr();
  if (dt_ptr == 0) {
    bsa_print(ACS_PRINT_ERR, L" dt_ptr is NULL\n");
    return;
  }

  PcieTable->num_entries = 0;

  for (i = 0; i < sizeof(pci_dt_arr)/PCI_COMPATIBLE_STR_LEN ; i++) {
      offset = fdt_node_offset_by_compatible((const void *)dt_ptr, -1, pci_dt_arr[i]);
      if (offset < 0) {
          bsa_print(ACS_PRINT_DEBUG, L"  PCI node offset not found %d\n", offset);
          continue; /* Search for next compatible node*/
      }

      parent_offset = fdt_parent_offset((const void *) dt_ptr, offset);
      bsa_print(ACS_PRINT_DEBUG, L"  NODE pcie offset %d\n", offset);

      size_cell = fdt_size_cells((const void *) dt_ptr, parent_offset);
      bsa_print(ACS_PRINT_DEBUG, L"  NODE pcie size cell %d\n", size_cell);
      if (size_cell < 0) {
          bsa_print(ACS_PRINT_ERR, L"  Invalid size cell\n");
          return;
      }

      addr_cell = fdt_address_cells((const void *) dt_ptr, parent_offset);
      bsa_print(ACS_PRINT_DEBUG, L"  NODE pcie addr cell %d\n", addr_cell);
      if (addr_cell <= 0 || addr_cell > 2) {
          bsa_print(ACS_PRINT_ERR, L"  Invalid address cell\n");
          return;
      }

      /* Perform a DT traversal till all pcie node are parsed */
      while (offset != -FDT_ERR_NOTFOUND) {

          Preg_val = (UINT32 *)fdt_getprop_namelen((void *)dt_ptr, offset, "reg", 3, &prop_len);
          if ((Preg_val == NULL) || prop_len < 0) {
              bsa_print(ACS_PRINT_ERR, L"  PROPERTY reg offset %x, Error %d\n", offset, prop_len);
              return;
          }

          PcieTable->block[PcieTable->num_entries].ecam_base = fdt32_to_cpu(Preg_val[0]);
          if (addr_cell == 2)
              PcieTable->block[PcieTable->num_entries].ecam_base =
          (PcieTable->block[PcieTable->num_entries].ecam_base  << 32) | fdt32_to_cpu(Preg_val[1]);

          Pbus_val = (UINT32 *)fdt_getprop_namelen((void *)dt_ptr, offset, "bus-range", 9,
                                                                &prop_len);
          if ((Pbus_val == NULL) || prop_len < 0) {
              bsa_print(ACS_PRINT_ERR, L"  PROPERTY reg offset %x, Error %d\n", offset, prop_len);
              return;
          }
          PcieTable->block[PcieTable->num_entries].segment_num = 0;
          PcieTable->block[PcieTable->num_entries].start_bus_num = fdt32_to_cpu(Pbus_val[0]);
          PcieTable->block[PcieTable->num_entries].end_bus_num = fdt32_to_cpu(Pbus_val[1]);
          offset = fdt_node_offset_by_compatible((const void *)dt_ptr, offset, pci_dt_arr[i]);

          PcieTable->num_entries++;
      }
  }
  dt_dump_pcie_table(PcieTable);
}

/**
  @brief  Returns true if PCIe rp buses needs to be reprogrammed.

  @param  None

  @return true/false
**/

UINT32
pal_bsa_pcie_enumerate()
{
  return 1; /* In case of U boot, we have seen RP sec bus not getting programmed correcttly */
}

/**
    @brief   Gets RP support of transaction forwarding.

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number
    @param   seg        PCI segment number

    @return  1 if rp not involved in transaction forwarding
             0 if rp is involved in transaction forwarding
**/
UINT32
pal_pcie_get_rp_transaction_frwd_support(UINT32 seg, UINT32 bus, UINT32 dev, UINT32 fn)
{
  return 1;
}

/**
  @brief  Returns the memory offset that can be
          accessed safely from the BAR base and is within
          BAR limit value

  @param  bdf      - PCIe BUS/Device/Function
  @param  mem_type - If the memory is Pre-fetchable or Non-prefetchable memory
  @return memory offset
**/
UINT32
pal_pcie_mem_get_offset(UINT32 bdf, PCIE_MEM_TYPE_INFO_e mem_type)
{

  return MEM_OFFSET_SMALL;
}
