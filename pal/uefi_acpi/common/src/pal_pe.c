/** @file
 * Copyright (c) 2016-2025, Arm Limited or its affiliates. All rights reserved.
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
#include <Protocol/Cpu.h>
#include "Include/IndustryStandard/SmBios.h"
#include <Protocol/Smbios.h>

#include "common/include/pal_uefi.h"

static   EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER *gMadtHdr;
UINT8   *gSecondaryPeStack;
UINT64  gMpidrMax;
static UINT32 g_num_pe;
extern INT32 gPsciConduit;

#define MAX_NUM_OF_SMBIOS_SLOTS_SUPPORTED  16
#define SIZE_STACK_SECONDARY_PE  0x100      //256 bytes per core
#define UPDATE_AFF_MAX(src,dest,mask)  ((dest & mask) > (src & mask) ? (dest & mask) : (src & mask))

#define ENABLED_BIT(flags)  (flags & 0x1)
#define ONLINE_CAP_BIT(flags)  ((flags > 3) & 0x1)

UINT64
pal_get_madt_ptr();

UINT64
pal_get_fadt_ptr (
  VOID
  );

VOID
ArmCallSmc (
  IN OUT ARM_SMC_ARGS *Args,
  IN     INT32        Conduit
  );

/**
  @brief  This API fills in the SMBIOS Info Table with information about the processor info
          in the system. This is achieved by parsing the SMBIOS table.

  @param  SmbiosTable  - Address where the processor information needs to be filled.

  @return  None
**/
VOID
pal_smbios_create_info_table(PE_SMBIOS_PROCESSOR_INFO_TABLE *SmbiosTable)
{
  EFI_STATUS Status;
  EFI_SMBIOS_PROTOCOL *SmbiosProtocol = NULL;
  EFI_SMBIOS_HANDLE SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  EFI_SMBIOS_TABLE_HEADER *Record;
  SMBIOS_TABLE_TYPE4 *Type4Record;
  PE_SMBIOS_TYPE4_INFO *Type4Entry = NULL;

  if (SmbiosTable == NULL) {
    acs_print(ACS_PRINT_ERR, L" Input SMBIOS Table Pointer is NULL. Cannot create SMBIOS INFO\n");
    return;
  }

  Type4Entry = SmbiosTable->type4_info;
  SmbiosTable->slot_count = 0;

  /* Get SMBIOS Protocol Handler */
  Status = gBS->LocateProtocol(&gEfiSmbiosProtocolGuid, NULL, (VOID **)&SmbiosProtocol);
  if (EFI_ERROR(Status)) {
    return;
  }

  while (!EFI_ERROR(Status)) {
    /* Get all records from SMBIOS Table */
    Status = SmbiosProtocol->GetNext(SmbiosProtocol, &SmbiosHandle, NULL, &Record, NULL);
    if (EFI_ERROR(Status)) {
      return;
    }

    /* Check of record if of type 4 */
    if (Record->Type == SMBIOS_TYPE_PROCESSOR_INFORMATION) {
      acs_print(ACS_PRINT_DEBUG, L" Smbios type %d found\n", Record->Type);

      Type4Record = (SMBIOS_TABLE_TYPE4 *)Record;

      /* Save Processor family type */
      if (Type4Record->ProcessorFamily == SMBIOS_OBTAIN_PROCESSOR_FAMILY2)
        Type4Entry->processor_family = Type4Record->ProcessorFamily2;
      else
        Type4Entry->processor_family = Type4Record->ProcessorFamily;

      acs_print(ACS_PRINT_DEBUG, L"  Processor Family 0x%x\n", Type4Entry->processor_family);

      /* Save Processor core count */
      if (Type4Record->CoreCount == SMBIOS_OBTAIN_CORE_COUNT2)
        Type4Entry->core_count = Type4Record->CoreCount2;
      else
        Type4Entry->core_count = Type4Record->CoreCount;

      acs_print(ACS_PRINT_DEBUG, L"  Processor Count 0x%x\n", Type4Entry->core_count);

      Type4Entry++;
      SmbiosTable->slot_count++;

      if (SmbiosTable->slot_count >= MAX_NUM_OF_SMBIOS_SLOTS_SUPPORTED) {
        acs_print(ACS_PRINT_WARN, L" Total Slots/Sockets 0x%x\n", SmbiosTable->slot_count);
        acs_print(ACS_PRINT_WARN, L" Number of SMBIOS Slots greater than %d\n",
                        MAX_NUM_OF_SMBIOS_SLOTS_SUPPORTED);
        SmbiosTable->slot_count = MAX_NUM_OF_SMBIOS_SLOTS_SUPPORTED;
        return;
      }
    }
  }
  acs_print(ACS_PRINT_DEBUG, L" Total Slots/Sockets 0x%x\n", SmbiosTable->slot_count);
}

/**
  @brief   Queries the FADT ACPI table to check whether PSCI is implemented and,
           if so, using which conduit (HVC or SMC).

  @retval  CONDUIT_UNKNOWN:       The FADT table could not be discovered.
  @retval  CONDUIT_NONE:          PSCI is not implemented
  @retval  CONDUIT_SMC:           PSCI is implemented and uses SMC as
                                  the conduit.
  @retval  CONDUIT_HVC:           PSCI is implemented and uses HVC as
                                  the conduit.
**/
INT32
pal_psci_get_conduit (
  VOID
  )
{
  EFI_ACPI_DESCRIPTION_HEADER   *Xsdt;
  EFI_ACPI_6_1_FIXED_ACPI_DESCRIPTION_TABLE  *Fadt;

  Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *) pal_get_xsdt_ptr();
  if (Xsdt == NULL) {
      acs_print(ACS_PRINT_ERR, L" XSDT not found\n");
      return CONDUIT_NO_TABLE;
  }

  Fadt = (EFI_ACPI_6_1_FIXED_ACPI_DESCRIPTION_TABLE *)pal_get_fadt_ptr ();
  if (!Fadt) {
    return CONDUIT_UNKNOWN;
  } else if (!(Fadt->ArmBootArch & EFI_ACPI_6_1_ARM_PSCI_COMPLIANT)) {
    return CONDUIT_NONE;
  } else if (Fadt->ArmBootArch & EFI_ACPI_6_1_ARM_PSCI_USE_HVC) {
    return CONDUIT_HVC;
  } else {
    return CONDUIT_SMC;
  }
}

/**
  @brief   Return the base address of the region allocated for Stack use for the Secondary
           PEs.
  @param   None
  @return  base address of the Stack
**/
UINT64
PalGetSecondaryStackBase()
{

  return (UINT64)gSecondaryPeStack;
}

/**
  @brief   Return the number of PEs in the System.
  @param   None
  @return  num_of_pe
**/
UINT32
pal_pe_get_num()
{

  return (UINT32)g_num_pe;
}

/**
  @brief   Returns the Max of each 8-bit Affinity fields in MPIDR.
  @param   None
  @return  Max MPIDR
**/
UINT64
PalGetMaxMpidr()
{

  return gMpidrMax;
}

/**
  @brief  Allocate memory region for secondary PE stack use. SIZE of stack for each PE
          is a #define

  @param  mpidr Pass MIPDR register content
  @return  None
**/
VOID
PalAllocateSecondaryStack(UINT64 mpidr)
{
  EFI_STATUS Status;
  UINT8 *Buffer;
  UINT32 NumPe, Aff0, Aff1, Aff2, Aff3, StackSize;

  Aff0 = ((mpidr & 0x00000000ff) >>  0);
  Aff1 = ((mpidr & 0x000000ff00) >>  8);
  Aff2 = ((mpidr & 0x0000ff0000) >> 16);
  Aff3 = ((mpidr & 0xff00000000) >> 32);

  NumPe = ((Aff3+1) * (Aff2+1) * (Aff1+1) * (Aff0+1));

  if (gSecondaryPeStack == NULL) {
      // AllocatePool guarantees 8b alignment, but stack pointers must be 16b
      // aligned for aarch64. Pad the size with an extra 8b so that we can
      // force-align the returned buffer to 16b. We store the original address
      // returned if we do have to align we still have the proper address to
      // free.

      StackSize = (NumPe * SIZE_STACK_SECONDARY_PE) + CPU_STACK_ALIGNMENT;
      Status = gBS->AllocatePool ( EfiBootServicesData,
                    StackSize,
                    (VOID **) &Buffer);
      if (EFI_ERROR(Status)) {
          acs_print(ACS_PRINT_ERR, L"\n FATAL - Allocation for Seconday stack failed %x\n", Status);
      }
      pal_pe_data_cache_ops_by_va((UINT64)&Buffer, CLEAN_AND_INVALIDATE);

      // Check if we need alignment
      if ((UINT8*)(((UINTN) Buffer) & (0xFll))) {
        // Needs alignment, so just store the original address and return +1
        ((UINTN*)Buffer)[0] = (UINTN)Buffer;
        gSecondaryPeStack = (UINT8*)(((UINTN*)Buffer)+1);
      } else {
        // None needed. Just store the address with padding and return.
        ((UINTN*)Buffer)[1] = (UINTN)Buffer;
        gSecondaryPeStack = (UINT8*)(((UINTN*)Buffer)+2);
      }
  }

}

/**
  @brief  This API fills in the PE_INFO Table with information about the PEs in the
          system. This is achieved by parsing the ACPI - MADT table.

  @param  PeTable  - Address where the PE information needs to be filled.

  @return  None
**/
VOID
pal_pe_create_info_table(PE_INFO_TABLE *PeTable)
{
  EFI_ACPI_6_1_GIC_STRUCTURE    *Entry = NULL;
  PE_INFO_ENTRY                 *Ptr = NULL;
  UINT32                        TableLength = 0;
  UINT32                        Length = 0;
  UINT64                        MpidrAff0Max = 0, MpidrAff1Max = 0, MpidrAff2Max = 0, MpidrAff3Max = 0;
  UINT32                        Flags;
  UINT32                        i;

  if (PeTable == NULL) {
    acs_print(ACS_PRINT_ERR, L" Input PE Table Pointer is NULL. Cannot create PE INFO\n");
    return;
  }

  /* initialise number of PEs to zero */
  PeTable->header.num_of_pe = 0;

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
  Ptr = PeTable->pe_info;

  do {

    if (Entry->Type == EFI_ACPI_6_1_GIC) {
      //Fill in the cpu num and the mpidr in pe info table
      Flags           = Entry->Flags;
      acs_print(ACS_PRINT_INFO, L"  Flags %x\n", Flags);
      acs_print(ACS_PRINT_DEBUG, L"  PE Enabled %d, Online Capable %d\n", ENABLED_BIT(Flags), ONLINE_CAP_BIT(Flags));

      /* As per MADT (GICC CPU Interface Flags) Processor is usable when
           Enabled bit is set
           Enabled bit is clear and Online Capable bit is set
           if both bits are clear, PE is not usable
      */
      if ((ENABLED_BIT(Flags) == 1) || (ONLINE_CAP_BIT(Flags) == 1))
      {
          Ptr->mpidr      = Entry->MPIDR;
          Ptr->pe_num     = PeTable->header.num_of_pe;
          Ptr->pmu_gsiv   = Entry->PerformanceInterruptGsiv;
          Ptr->gmain_gsiv = Entry->VGICMaintenanceInterrupt;
          Ptr->acpi_proc_uid = Entry->AcpiProcessorUid;

          /* Trbe value is not available in Acpi table less than 6.5 so dummy value will be given
           * Read Trbe Interrupt if Acpi table is greater than or equal to 6.5
           */

          if (gMadtHdr->Header.Revision < 6)
               Ptr->trbe_interrupt = 1;
          else
              Ptr->trbe_interrupt = ((EFI_ACPI_6_5_GIC_STRUCTURE *)
                                     ((UINT8 *)Entry))->TrbeInterrupt;

          acs_print(ACS_PRINT_DEBUG, L"  MADT Revision %llx \n", gMadtHdr->Header.Revision);

          for (i = 0; i < MAX_L1_CACHE_RES; i++)
              Ptr->level_1_res[i] = DEFAULT_CACHE_IDX; //initialize cache index fields with all 1's
          acs_print(ACS_PRINT_DEBUG, L" MPIDR %llx PE num %x\n", Ptr->mpidr, Ptr->pe_num);
          pal_pe_data_cache_ops_by_va((UINT64)Ptr, CLEAN_AND_INVALIDATE);
          Ptr++;
          PeTable->header.num_of_pe++;

          MpidrAff0Max = UPDATE_AFF_MAX(MpidrAff0Max, Entry->MPIDR, 0x000000ff);
          MpidrAff1Max = UPDATE_AFF_MAX(MpidrAff1Max, Entry->MPIDR, 0x0000ff00);
          MpidrAff2Max = UPDATE_AFF_MAX(MpidrAff2Max, Entry->MPIDR, 0x00ff0000);
          MpidrAff3Max = UPDATE_AFF_MAX(MpidrAff3Max, Entry->MPIDR, 0xff00000000);
      }
    }

    Length += Entry->Length;
    Entry = (EFI_ACPI_6_1_GIC_STRUCTURE *) ((UINT8 *)Entry + (Entry->Length));

  }while(Length < TableLength);

  gMpidrMax = MpidrAff0Max | MpidrAff1Max | MpidrAff2Max | MpidrAff3Max;
  g_num_pe = PeTable->header.num_of_pe;
  pal_pe_data_cache_ops_by_va((UINT64)PeTable, CLEAN_AND_INVALIDATE);
  pal_pe_data_cache_ops_by_va((UINT64)&gMpidrMax, CLEAN_AND_INVALIDATE);
  PalAllocateSecondaryStack(gMpidrMax);

}

/**
  @brief  Install Exception Handler using UEFI CPU Architecture protocol's
          Register Interrupt Handler API

  @param  ExceptionType  - AARCH64 Exception type
  @param  esr            - Function pointer of the exception handler

  @return status of the API
**/
UINT32
pal_pe_install_esr(UINT32 ExceptionType,  VOID (*esr)(UINT64, VOID *))
{

  EFI_STATUS  Status;
  EFI_CPU_ARCH_PROTOCOL   *Cpu;

  // Get the CPU protocol that this driver requires.
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&Cpu);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Unregister the default exception handler.
  Status = Cpu->RegisterInterruptHandler (Cpu, ExceptionType, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Register to receive interrupts
  Status = Cpu->RegisterInterruptHandler (Cpu, ExceptionType, (EFI_CPU_INTERRUPT_HANDLER)esr);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

/**
  @brief  Make the SMC call using AARCH64 Assembly code
          SMC calls can take up to 7 arguments and return up to 4 return values.
          Therefore, the 4 first fields in the ARM_SMC_ARGS structure are used
          for both input and output values.

  @param  Argumets to pass to the EL3 firmware
  @param  Conduit  SMC or HVC

  @return  None
**/
VOID
pal_pe_call_smc(ARM_SMC_ARGS *ArmSmcArgs, INT32 Conduit)
{
  ArmCallSmc (ArmSmcArgs, Conduit);
}

VOID
ModuleEntryPoint();

/**
  @brief  Make a PSCI CPU_ON call using SMC instruction.
          Pass PAL Assembly code entry as the start vector for the PSCI ON call

  @param  Argumets to pass to the EL3 firmware

  @return  None
**/
VOID
pal_pe_execute_payload(ARM_SMC_ARGS *ArmSmcArgs)
{
  ArmSmcArgs->Arg2 = (UINT64)ModuleEntryPoint;
  pal_pe_call_smc(ArmSmcArgs, gPsciConduit);
}

/**
  @brief Update the ELR to return from exception handler to a desired address

  @param  context - exception context structure
  @param  offset - address with which ELR should be updated

  @return  None
**/
VOID
pal_pe_update_elr(VOID *context, UINT64 offset)
{
  ((EFI_SYSTEM_CONTEXT_AARCH64*)context)->ELR = offset;
}

/**
  @brief Get the Exception syndrome from UEFI exception handler

  @param  context - exception context structure

  @return  ESR
**/
UINT64
pal_pe_get_esr(VOID *context)
{
  return ((EFI_SYSTEM_CONTEXT_AARCH64*)context)->ESR;
}

/**
  @brief Get the FAR from UEFI exception handler

  @param  context - exception context structure

  @return  FAR
**/
UINT64
pal_pe_get_far(VOID *context)
{
  return ((EFI_SYSTEM_CONTEXT_AARCH64*)context)->FAR;
}

VOID
DataCacheCleanInvalidateVA(UINT64 addr);

VOID
DataCacheCleanVA(UINT64 addr);

VOID
DataCacheInvalidateVA(UINT64 addr);

/**
  @brief Perform cache maintenance operation on an address

  @param addr - address on which cache ops to be performed
  @param type - type of cache ops

  @return  None
**/
VOID
pal_pe_data_cache_ops_by_va(UINT64 addr, UINT32 type)
{
  switch(type){
      case CLEAN_AND_INVALIDATE:
          DataCacheCleanInvalidateVA(addr);
      break;
      case CLEAN:
          DataCacheCleanVA(addr);
      break;
      case INVALIDATE:
          DataCacheInvalidateVA(addr);
      break;
      default:
          DataCacheCleanInvalidateVA(addr);
  }

}
