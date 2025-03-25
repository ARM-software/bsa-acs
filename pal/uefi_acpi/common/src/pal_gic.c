/** @file
 * Copyright (c) 2016-2021, 2023-2025, Arm Limited or its affiliates. All rights reserved.
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
#include <Protocol/AcpiTable.h>
#include <Protocol/HardwareInterrupt.h>
#include <Protocol/HardwareInterrupt2.h>

#include "common/include/pal_uefi.h"
#include "common/include/bsa_pcie_enum.h"

static EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER *gMadtHdr;

static UINT32 g_non_gic_interrupt_count;

EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;

UINT64
pal_get_madt_ptr();

/**
  @brief  Populate information about the GIC sub-system at the input address.
          In a UEFI-ACPI framework, this information is part of the MADT table.

  @param  GicTable  Address of the memory region where this information is to be filled in

  @return None
**/
VOID
pal_gic_create_info_table(GIC_INFO_TABLE *GicTable)
{
  EFI_ACPI_6_1_GIC_STRUCTURE    *Entry = NULL;
  GIC_INFO_ENTRY                *GicEntry = NULL;
  UINT32                         Length= 0;
  UINT32                         TableLength;
  UINT32                         is_gicr_present = 0;

  if (GicTable == NULL) {
    acs_print(ACS_PRINT_ERR, L" Input GIC Table Pointer is NULL. Cannot create GIC INFO\n");
    return;
  }

  GicEntry = GicTable->gic_info;

  GicTable->header.gic_version = 0;
  GicTable->header.num_gicr_rd = 0;
  GicTable->header.num_gicc_rd = 0;
  GicTable->header.num_gicd = 0;
  GicTable->header.num_its = 0;
  GicTable->header.num_msi_frame = 0;

  gMadtHdr = (EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER *) pal_get_madt_ptr();

  if (gMadtHdr != NULL) {
    TableLength =  gMadtHdr->Header.Length;
    acs_print(ACS_PRINT_INFO, L"  MADT is at %x and length is %x\n", gMadtHdr, TableLength);
  } else {
    acs_print(ACS_PRINT_ERR, L" MADT not found\n");
    return;
  }

  Entry = (EFI_ACPI_6_1_GIC_STRUCTURE *) (gMadtHdr + 1);
  Length = sizeof (EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER);

  /* Check if GICR Structure is present */
  do {

    if (Entry->Type == EFI_ACPI_6_1_GICR) {
        is_gicr_present = 1;
        break;
    }

    Length += Entry->Length;
    Entry = (EFI_ACPI_6_1_GIC_STRUCTURE *) ((UINT8 *)Entry + (Entry->Length));

  } while(Length < TableLength);

  Entry = (EFI_ACPI_6_1_GIC_STRUCTURE *) (gMadtHdr + 1);
  Length = sizeof (EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER);

  do {

    /* As Per Latest UEFI Spec 6.5
     * The Value 0x0b - 0x0f Represents GIC Structure */
    if (Entry->Type <= 0x17 && (Entry->Type <= 0xA || Entry->Type >= 0x10))
        g_non_gic_interrupt_count++;

    if (Entry->Type == EFI_ACPI_6_1_GIC) {
      if (Entry->PhysicalBaseAddress != 0) {
        GicEntry->type = ENTRY_TYPE_CPUIF;
        GicEntry->base = Entry->PhysicalBaseAddress;
        acs_print(ACS_PRINT_INFO, L"  GIC CPUIF base %lx\n", GicEntry->base);
        GicEntry++;
      }

      if (Entry->GICRBaseAddress != 0) {
        /* Add this entry if GICR is not present */
        if (is_gicr_present == 0) {
          GicEntry->type = ENTRY_TYPE_GICC_GICRD;
          GicEntry->base = Entry->GICRBaseAddress;
          GicEntry->length = 0;
          acs_print(ACS_PRINT_INFO, L"  GICC RD base %lx\n", GicEntry->base);
          GicTable->header.num_gicc_rd++;
          GicEntry++;
        } else {
          acs_print(ACS_PRINT_INFO,
                    L"  Warning : GICR Structure Present, GICC RD Base Non-Zero\n", 0);
        }
      }

      if (Entry->GICH != 0) {
        GicEntry->type = ENTRY_TYPE_GICH;
        GicEntry->base = Entry->GICH;
        GicEntry->length = 0;
        acs_print(ACS_PRINT_INFO, L"  GICH base %lx\n", GicEntry->base);
        GicEntry++;
      }
    }

    if (Entry->Type == EFI_ACPI_6_1_GICD) {
        GicEntry->type = ENTRY_TYPE_GICD;
        GicEntry->base = ((EFI_ACPI_6_1_GIC_DISTRIBUTOR_STRUCTURE *)Entry)->PhysicalBaseAddress;
        GicTable->header.gic_version = ((EFI_ACPI_6_1_GIC_DISTRIBUTOR_STRUCTURE *)Entry)->GicVersion;
        acs_print(ACS_PRINT_INFO, L"  GIC DIS base %lx\n", GicEntry->base);
        GicTable->header.num_gicd++;
        GicEntry++;
    }

    if (Entry->Type == EFI_ACPI_6_1_GICR) {
        GicEntry->type = ENTRY_TYPE_GICR_GICRD;
        GicEntry->base = ((EFI_ACPI_6_1_GICR_STRUCTURE *)Entry)->DiscoveryRangeBaseAddress;
        GicEntry->length = ((EFI_ACPI_6_1_GICR_STRUCTURE *)Entry)->DiscoveryRangeLength;
        acs_print(ACS_PRINT_INFO, L"  GICR RD base %lx\n", GicEntry->base);
        acs_print(ACS_PRINT_INFO, L"  GICR RD Length %lx\n", GicEntry->length);
        GicTable->header.num_gicr_rd++;
        GicEntry++;
    }

    if (Entry->Type == EFI_ACPI_6_1_GIC_ITS) {
        GicEntry->type = ENTRY_TYPE_GICITS;
        GicEntry->base = ((EFI_ACPI_6_1_GIC_ITS_STRUCTURE *)Entry)->PhysicalBaseAddress;
        GicEntry->entry_id = ((EFI_ACPI_6_1_GIC_ITS_STRUCTURE *)Entry)->GicItsId;
        acs_print(ACS_PRINT_INFO, L"  GIC ITS base %lx\n", GicEntry->base);
        acs_print(ACS_PRINT_INFO, L"  GIC ITS ID%x\n", GicEntry->entry_id);
        GicTable->header.num_its++;
        GicEntry++;
    }

    if (Entry->Type == EFI_ACPI_6_1_GIC_MSI_FRAME) {
        GicEntry->type = ENTRY_TYPE_GIC_MSI_FRAME;
        GicEntry->base = ((EFI_ACPI_6_1_GIC_MSI_FRAME_STRUCTURE *)Entry)->PhysicalBaseAddress;
        GicEntry->entry_id = ((EFI_ACPI_6_1_GIC_MSI_FRAME_STRUCTURE *)Entry)->GicMsiFrameId;
        GicEntry->flags = ((EFI_ACPI_6_1_GIC_MSI_FRAME_STRUCTURE *)Entry)->Flags;
        GicEntry->spi_count = ((EFI_ACPI_6_1_GIC_MSI_FRAME_STRUCTURE *)Entry)->SPICount;
        GicEntry->spi_base = ((EFI_ACPI_6_1_GIC_MSI_FRAME_STRUCTURE *)Entry)->SPIBase;
        acs_print(ACS_PRINT_INFO, L"  GIC MSI Frame base %lx\n", GicEntry->base);
        acs_print(ACS_PRINT_INFO, L"  GIC MSI SPI base %x\n", GicEntry->spi_base);
        acs_print(ACS_PRINT_INFO, L"  GIC MSI SPI Count %x\n", GicEntry->spi_count);
        GicTable->header.num_msi_frame++;
        GicEntry++;
    }
    Length += Entry->Length;
    Entry = (EFI_ACPI_6_1_GIC_STRUCTURE *) ((UINT8 *)Entry + (Entry->Length));


  } while(Length < TableLength);

  GicEntry->type = 0xFF;  //Indicate end of data

}

/**
  @brief  Enable the interrupt in the GIC Distributor and GIC CPU Interface and hook
          the interrupt service routine for the IRQ to the UEFI Framework

  @param  int_id  Interrupt ID which needs to be enabled and service routine installed for
  @param  isr     Function pointer of the Interrupt service routine

  @return Status of the operation
**/
UINT32
pal_gic_install_isr(UINT32 int_id,  VOID (*isr)())
{

  EFI_STATUS  Status;

 // Find the interrupt controller protocol.
  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  if (EFI_ERROR(Status)) {
    return 0xFFFFFFFF;
  }

  //First disable the interrupt to enable a clean handoff to our Interrupt handler.
  gInterrupt->DisableInterruptSource(gInterrupt, int_id);

  //Register our handler
  Status = gInterrupt->RegisterInterruptSource (gInterrupt, int_id, isr);
  if (EFI_ERROR(Status)) {
    Status =  gInterrupt->RegisterInterruptSource (gInterrupt, int_id, NULL);  //Deregister existing handler
    Status = gInterrupt->RegisterInterruptSource (gInterrupt, int_id, isr);  //register our Handler.
    //Even if this fails. there is nothing we can do in UEFI mode
  }

  return 0;
}

/**
  @brief  Indicate that processing of interrupt is complete by writing to
          End of interrupt register in the GIC CPU Interface

  @param  int_id  Interrupt ID which needs to be acknowledged that it is complete

  @return Status of the operation
**/
UINT32
pal_gic_end_of_interrupt(UINT32 int_id)
{

  EFI_STATUS  Status;

 // Find the interrupt controller protocol.
  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  if (EFI_ERROR(Status)) {
    return 0xFFFFFFFF;
  }

  //EndOfInterrupt.
  gInterrupt->EndOfInterrupt(gInterrupt, int_id);

  return 0;
}

/** Place holder function. Need to be implemented if needed in later releases
  @brief Registers the interrupt handler for a given IRQ

  @param IrqNum Hardware IRQ number
  @param MappedIrqNum Mapped IRQ number
  @param Isr Interrupt Service Routine that returns the status

  @return Status of the operation
**/
UINT32
pal_gic_request_irq(
  UINT32 IrqNum,
  UINT32 MappedIrqNum,
  VOID *Isr
  )
{
    return 0;
}

/** Place holder function. Need to be implemented if needed in later releases
  @brief This function frees the registered interrupt handler for a given IRQ

  @param IrqNum Hardware IRQ number
  @param MappedIrqNum Mapped IRQ number

  @return none
**/
VOID
pal_gic_free_irq(
  UINT32 IrqNum,
  UINT32 MappedIrqNum
  )
{

}




/**
  @brief   This API returns the count of Non Gic Interrupt Controller
           1. Caller       -  Val Suite
           2. Prerequisite -  pal_gic_create_info_table
  @return  Count of Non Gic Interrupt Controller
**/
UINT32 pal_get_num_nongic_ctrl(VOID)
{
    return g_non_gic_interrupt_count;
}

