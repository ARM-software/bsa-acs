/** @file
 * Copyright (c) 2018-2019, 2021-2023, Arm Limited or its affiliates. All rights reserved.
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

#include <Protocol/AcpiTable.h>
#include "Include/IndustryStandard/Acpi61.h"

#include <Include/libfdt.h>
#include "include/pal_uefi.h"
#include "../include/platform_override.h"
#include "include/pal_dt.h"
#include "include/pal_dt_spec.h"


static EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE *gGtdtHdr;

static char systimer_dt_arr[][SYSTIMER_COMPATIBLE_STR_LEN] = {
    "arm,armv8-timer",
    "arm,armv7-timer"
};

static char memtimer_dt_arr[][MEMTIMER_COMPATIBLE_STR_LEN] = {
    "arm,armv7-timer-mem"
};

static char wd_dt_arr[][WD_COMPATIBLE_STR_LEN] = {
    "arm,sbsa-gwdt"
};

UINT64
pal_get_gtdt_ptr();

/**
  @brief This API overrides the timer specified by TimerTable
         Note: Information about only one timer can be mentioned as an Override

  @param TimerTable Pointer to timer info table

  @return None
**/
static
VOID
pal_timer_platform_override(TIMER_INFO_TABLE *TimerTable)
{
  if (PLATFORM_OVERRIDE_PLATFORM_TIMER) {
      TimerTable->header.num_platform_timer = 1;
      TimerTable->gt_info[0].block_cntl_base = PLATFORM_OVERRIDE_CNTCTL_BASE;
      TimerTable->gt_info[0].timer_count = 1;
      TimerTable->gt_info[0].GtCntBase[0]  = PLATFORM_OVERRIDE_CNTBASE_N;
      TimerTable->gt_info[0].gsiv[0] = PLATFORM_OVERRIDE_PLATFORM_TIMER_GSIV;
  }

  //GTDT does not have this information yet.
  if (PLATFORM_OVERRIDE_EL2_VIR_TIMER_GSIV) {
      TimerTable->header.el2_virt_timer_gsiv = PLATFORM_OVERRIDE_EL2_VIR_TIMER_GSIV;
  }

}

/**
  @brief  This API fills in the TIMER_INFO_TABLE with information about local and system
          timers in the system. This is achieved by parsing the ACPI - GTDT table.

  @param  TimerTable  - Address where the Timer information needs to be filled.

  @return  None
**/
VOID
pal_timer_create_info_table(TIMER_INFO_TABLE *TimerTable)
{
  EFI_ACPI_6_1_GTDT_GT_BLOCK_STRUCTURE       *Entry = NULL;
  EFI_ACPI_6_1_GTDT_GT_BLOCK_TIMER_STRUCTURE *GtBlockTimer = NULL;
  TIMER_INFO_GTBLOCK         *GtEntry = NULL;
  UINT32                      Length= 0;
  UINT32                      i;
  UINT32                      num_of_entries;

  if (TimerTable == NULL) {
    bsa_print(ACS_PRINT_ERR, L" Input Timer Table Pointer is NULL. Cannot create Timer INFO\n");
    return;
  }

  GtEntry = TimerTable->gt_info;
  TimerTable->header.num_platform_timer = 0;

  pal_timer_create_info_table_dt(TimerTable);
  return;

  gGtdtHdr = (EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE *) pal_get_gtdt_ptr();

  bsa_print(ACS_PRINT_INFO, L" GTDT is at %x and length is %x\n", gGtdtHdr, gGtdtHdr->Header.Length);

  //Fill in our internal table
  TimerTable->header.s_el1_timer_flag  = gGtdtHdr->SecurePL1TimerFlags;
  TimerTable->header.ns_el1_timer_flag = gGtdtHdr->NonSecurePL1TimerFlags;
  TimerTable->header.el2_timer_flag    = gGtdtHdr->NonSecurePL2TimerFlags;
  TimerTable->header.s_el1_timer_gsiv  = gGtdtHdr->SecurePL1TimerGSIV;
  TimerTable->header.ns_el1_timer_gsiv = gGtdtHdr->NonSecurePL1TimerGSIV;
  TimerTable->header.el2_timer_gsiv    = gGtdtHdr->NonSecurePL2TimerGSIV;
  TimerTable->header.virtual_timer_flag = gGtdtHdr->VirtualTimerFlags;
  TimerTable->header.virtual_timer_gsiv = gGtdtHdr->VirtualTimerGSIV;

  Length         = gGtdtHdr->PlatformTimerOffset;
  Entry          = (EFI_ACPI_6_1_GTDT_GT_BLOCK_STRUCTURE *) ((UINT8 *)gGtdtHdr + Length);
  Length         = sizeof (EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE);
  num_of_entries = gGtdtHdr->PlatformTimerCount;

  while(num_of_entries) {

    if (Entry->Type == EFI_ACPI_6_1_GTDT_GT_BLOCK) {
      bsa_print(ACS_PRINT_INFO, L"  Found block entry\n");
      GtEntry->type = TIMER_TYPE_SYS_TIMER;
      GtEntry->block_cntl_base = Entry->CntCtlBase;
      GtEntry->timer_count     = Entry->GTBlockTimerCount;
      bsa_print(ACS_PRINT_DEBUG, L"  CNTCTLBase = %llx\n", GtEntry->block_cntl_base);
      GtBlockTimer = (EFI_ACPI_6_1_GTDT_GT_BLOCK_TIMER_STRUCTURE *)(((UINT8 *)Entry) + Entry->GTBlockTimerOffset);
      for (i = 0; i < GtEntry->timer_count; i++) {
        bsa_print(ACS_PRINT_INFO, L"  Found timer entry\n");
        GtEntry->frame_num[i]    = GtBlockTimer->GTFrameNumber;
        GtEntry->GtCntBase[i]    = GtBlockTimer->CntBaseX;
        GtEntry->GtCntEl0Base[i] = GtBlockTimer->CntEL0BaseX;
        GtEntry->gsiv[i]         = GtBlockTimer->GTxPhysicalTimerGSIV;
        GtEntry->virt_gsiv[i]    = GtBlockTimer->GTxVirtualTimerGSIV;
        GtEntry->flags[i]        = GtBlockTimer->GTxPhysicalTimerFlags | (GtBlockTimer->GTxVirtualTimerFlags << 8) | (GtBlockTimer->GTxCommonFlags << 16);
        bsa_print(ACS_PRINT_DEBUG, L"  CNTBaseN = %llx for sys counter = %d\n", GtEntry->GtCntBase[i], i);
        GtBlockTimer++;
        TimerTable->header.num_platform_timer++;
      }
      GtEntry++;
    }

    if (Entry->Type == EFI_ACPI_6_1_GTDT_SBSA_GENERIC_WATCHDOG) {
      //This is a Watchdog entry. Skip.. added in a separate function.
    }

    Entry = (EFI_ACPI_6_1_GTDT_GT_BLOCK_STRUCTURE *) ((UINT8 *)Entry + (Entry->Length));
    num_of_entries--;

  };

  pal_timer_platform_override(TimerTable);

}

/**
  @brief  This API gets the counter frequency value from user

  @param  None

  @return Counter frequency value
**/

UINT64
pal_timer_get_counter_frequency(VOID)
{
  return PLATFORM_OVERRIDE_TIMER_CNTFRQ;
}

/**
  @brief This API overrides the watch dog timer specified by WdTable
         Note: Only one watchdog information can be assigned as an override

  @param WdTable Pointer to watch dog timer info table

  @return None
**/
VOID
pal_wd_platform_override(WD_INFO_TABLE *WdTable)
{

  if (PLATFORM_OVERRIDE_WD == 1) {
      WdTable->header.num_wd              = 1;
      WdTable->wd_info[0].wd_refresh_base = PLATFORM_OVERRIDE_WD_REFRESH_BASE;
      WdTable->wd_info[0].wd_ctrl_base    = PLATFORM_OVERRIDE_WD_CTRL_BASE;
      WdTable->wd_info[0].wd_gsiv         = PLATFORM_OVERRIDE_WD_GSIV;
      WdTable->wd_info[0].wd_flags        = 0;
  }

  return;
}

/**
  @brief  This API fills in the WD_INFO_TABLE with information about Watchdogs
          in the system. This is achieved by parsing the ACPI - GTDT table.

  @param  WdTable  - Address where the Timer information needs to be filled.

  @return  None
**/

VOID
pal_wd_create_info_table(WD_INFO_TABLE *WdTable)
{

  EFI_ACPI_6_1_GTDT_SBSA_GENERIC_WATCHDOG_STRUCTURE    *Entry = NULL;
  WD_INFO_BLOCK               *WdEntry = NULL;
  UINT32                      Length= 0;
  UINT32                      num_of_entries;

  if (WdTable == NULL) {
    bsa_print(ACS_PRINT_ERR,
              L" Input Watchdog Table Pointer is NULL. Cannot create Watchdog INFO\n");
    return;
  }

  WdEntry = WdTable->wd_info;
  WdTable->header.num_wd = 0;

  pal_wd_create_info_table_dt(WdTable);
  return;

  gGtdtHdr = (EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE *) pal_get_gtdt_ptr();

  Length         = gGtdtHdr->PlatformTimerOffset;
  Entry          = (EFI_ACPI_6_1_GTDT_SBSA_GENERIC_WATCHDOG_STRUCTURE *) ((UINT8 *)gGtdtHdr + Length);
  Length         = sizeof (EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE);
  num_of_entries = gGtdtHdr->PlatformTimerCount;

  while(num_of_entries)
  {

    if (Entry->Type == EFI_ACPI_6_1_GTDT_GT_BLOCK) {
      //Skip. this info is added in the timer info function
    }

    if (Entry->Type == EFI_ACPI_6_1_GTDT_SBSA_GENERIC_WATCHDOG) {
      WdEntry->wd_refresh_base = Entry->RefreshFramePhysicalAddress;
      WdEntry->wd_ctrl_base    = Entry->WatchdogControlFramePhysicalAddress;
      WdEntry->wd_gsiv         = Entry->WatchdogTimerGSIV;
      WdEntry->wd_flags        = Entry->WatchdogTimerFlags;
      WdTable->header.num_wd++;
      bsa_print(ACS_PRINT_DEBUG,
                L"  Watchdog base = 0x%llx INTID = 0x%x\n",
                WdEntry->wd_ctrl_base,
                WdEntry->wd_gsiv);
      WdEntry++;
    }
    Entry = (EFI_ACPI_6_1_GTDT_SBSA_GENERIC_WATCHDOG_STRUCTURE *) ((UINT8 *)Entry + (Entry->Length));
    num_of_entries--;

  }

  pal_wd_platform_override(WdTable);

}


/**
  @brief  This API fills in the WD_INFO Table with information about the WDs in the
          system. This is achieved by parsing the DT blob.

  @param  WdTable  - Address where the WD information needs to be filled.

  @return  None
**/
VOID
pal_wd_create_info_table_dt(WD_INFO_TABLE *WdTable)
{
  WD_INFO_BLOCK *WdEntry = NULL;
  UINT64 dt_ptr = 0;
  int offset, parent_offset;
  int wd_mode, wd_polarity;
  UINT32 *Preg_val, *Pintr_val;
  int prop_len, index = 0, interrupt_cell;
  int addr_cell, size_cell, i, intr_flg;

  if (WdTable == NULL)
    return;

  WdEntry = WdTable->wd_info;
  WdTable->header.num_wd = 0;

  dt_ptr = pal_get_dt_ptr();
  if (dt_ptr == 0) {
    bsa_print(ACS_PRINT_ERR, L" dt_ptr is NULL\n");
    return;
  }

  for (i = 0; i < sizeof(wd_dt_arr)/WD_COMPATIBLE_STR_LEN ; i++) {
      offset = fdt_node_offset_by_compatible((const void *)dt_ptr, -1, wd_dt_arr[i]);
      if (offset < 0) {
          bsa_print(ACS_PRINT_DEBUG, L"  WD node offset not found %d\n", offset);
          continue; /* Search for next compatible wd*/
      }

      parent_offset = fdt_parent_offset((const void *) dt_ptr, offset);
      bsa_print(ACS_PRINT_DEBUG, L"  Parent Node offset %d\n", offset);

      size_cell = fdt_size_cells((const void *) dt_ptr, parent_offset);
      bsa_print(ACS_PRINT_DEBUG, L"  size cell %d\n", size_cell);
      if (size_cell < 1 || size_cell > 2) {
          bsa_print(ACS_PRINT_ERR, L"  Invalid size cell :%d\n", size_cell);
          return;
      }

      addr_cell = fdt_address_cells((const void *) dt_ptr, parent_offset);
      bsa_print(ACS_PRINT_DEBUG, L"  addr cell %d\n", addr_cell);
      if (addr_cell < 1 || addr_cell > 2) {
          bsa_print(ACS_PRINT_ERR, L"  Invalid address cell : %d\n", addr_cell);
          return;
      }

      while (offset != -FDT_ERR_NOTFOUND) {
          bsa_print(ACS_PRINT_DEBUG, L"  WD node:%d offset:%d\n", WdTable->header.num_wd, offset);

          Preg_val = (UINT32 *)fdt_getprop_namelen((void *)dt_ptr, offset, "reg", 3, &prop_len);
          if ((prop_len < 0) || (Preg_val == NULL)) {
              bsa_print(ACS_PRINT_ERR, L"  PROPERTY reg offset %x, Error %d\n", offset, prop_len);
              return;
          }

          Pintr_val =
              (UINT32 *)fdt_getprop_namelen((void *)dt_ptr, offset, "interrupts", 10, &prop_len);
          if ((prop_len < 0) || (Pintr_val == NULL)) {
              bsa_print(ACS_PRINT_ERR, L"  PROPERTY interrupts offset %x, Error %d\n",
                        offset, prop_len);
              return;
          }

          interrupt_cell = fdt_interrupt_cells((const void *)dt_ptr, offset);
          bsa_print(ACS_PRINT_DEBUG, L"  interrupt_cell  %d\n", interrupt_cell);
          if (interrupt_cell < INTERRUPT_CELLS_MIN || interrupt_cell > INTERRUPT_CELLS_MAX) {
              bsa_print(ACS_PRINT_ERR, L"  Invalid interrupt cell : %d\n", interrupt_cell);
              return;
          }

          WdTable->header.num_wd++;
          if (addr_cell == 1) {
              WdEntry->wd_ctrl_base    = fdt32_to_cpu(Preg_val[index++]);
              index += size_cell; /* Skip size cells*/
              WdEntry->wd_refresh_base = fdt32_to_cpu(Preg_val[index]);
          } else {
              WdEntry->wd_ctrl_base    = fdt32_to_cpu(Preg_val[index++]);
              WdEntry->wd_ctrl_base    = (WdEntry->wd_ctrl_base << 32) |
                  fdt32_to_cpu(Preg_val[index++]);
              index += size_cell; /* Skip size cells*/
              WdEntry->wd_refresh_base = fdt32_to_cpu(Preg_val[index++]);
              WdEntry->wd_refresh_base    = (WdEntry->wd_refresh_base << 32) |
                  fdt32_to_cpu(Preg_val[index++]);

          }
          index = 0;
          if ((interrupt_cell == 3) || (interrupt_cell == 4)) {
              if (Pintr_val[index++])
                  WdEntry->wd_gsiv = fdt32_to_cpu(Pintr_val[index++]) + PPI_OFFSET;
              else
                  WdEntry->wd_gsiv = fdt32_to_cpu(Pintr_val[index++]) + SPI_OFFSET;
              intr_flg = fdt32_to_cpu(Pintr_val[index]);
          }
          else if (interrupt_cell == 2) {
              WdEntry->wd_gsiv = fdt32_to_cpu(Pintr_val[index++]); /* Interrupt number*/
              intr_flg = fdt32_to_cpu(Pintr_val[index]);
          } else {
              WdEntry->wd_gsiv = fdt32_to_cpu(Pintr_val[index]); /* Interrupt number*/
              intr_flg = IRQ_TYPE_NONE;
          }
          switch (intr_flg) /* Interrupt flag*/
          {
            case IRQ_TYPE_NONE:
              bsa_print(ACS_PRINT_DEBUG, L"  interrupt type none\n");
              wd_mode = INTERRUPT_IS_LEVEL_TRIGGERED; /* Set default*/
              wd_polarity = INTERRUPT_IS_ACTIVE_HIGH;
              break;
            case IRQ_TYPE_EDGE_RISING:
              wd_mode = INTERRUPT_IS_EDGE_TRIGGERED;
              wd_polarity = INTERRUPT_IS_ACTIVE_HIGH;
              break;
            case IRQ_TYPE_EDGE_FALLING:
              wd_mode = INTERRUPT_IS_EDGE_TRIGGERED;
              wd_polarity = INTERRUPT_IS_ACTIVE_LOW;
              break;
            case IRQ_TYPE_LEVEL_HIGH:
              wd_mode = INTERRUPT_IS_LEVEL_TRIGGERED;
              wd_polarity = INTERRUPT_IS_ACTIVE_HIGH;
              break;
            case IRQ_TYPE_LEVEL_LOW:
              wd_mode = INTERRUPT_IS_LEVEL_TRIGGERED;
              wd_polarity = INTERRUPT_IS_ACTIVE_LOW;
              break;
            default:
              bsa_print(ACS_PRINT_ERR, L"  interrupt type invalid :%X\n",
                        fdt32_to_cpu(Pintr_val[2]));
              return;
          }
          WdEntry->wd_flags = ((wd_polarity << 1) | (wd_mode << 0));
          WdEntry++;
          offset = fdt_node_offset_by_compatible((const void *)dt_ptr, offset, wd_dt_arr[i]);
      }
  }
  pal_wd_platform_override(WdTable);
  dt_dump_wd_table(WdTable);
}

/**
   @brief  This API fills in the TIMER_INFO Table with information about the timer in the
           system. This is achieved by parsing the DT blob.

   @param  TimerTable  - Address where the timer information needs to be filled.

   @return  None
**/

VOID
pal_timer_create_info_table_dt(TIMER_INFO_TABLE *TimerTable)
{
  TIMER_INFO_GTBLOCK         *GtEntry = NULL;
  int prop_len;
  UINT32 *Preg, *Pintr, *Palways_on;
  UINT64 dt_ptr = 0;
  int offset = 0, subnode_offset, parent_offset;
  int i = 0, frame_number  = 0;
  int addr_cell, size_cell, index = 0;
  int interrupt_cell;

  if (TimerTable == NULL)
      return;

  dt_ptr = pal_get_dt_ptr();
  if (dt_ptr == 0) {
      bsa_print(ACS_PRINT_ERR, L" dt_ptr is NULL\n");
      return;
  }

  TimerTable->header.num_platform_timer = 0;
  GtEntry = TimerTable->gt_info;
  GtEntry->timer_count = 0;

  /* Reset timer header*/
  TimerTable->header.s_el1_timer_gsiv    = 0;
  TimerTable->header.ns_el1_timer_gsiv   = 0;
  TimerTable->header.virtual_timer_gsiv  = 0;
  TimerTable->header.el2_timer_gsiv      = 0;
  TimerTable->header.el2_virt_timer_gsiv = 0;  /* NA in DT*/
  TimerTable->header.s_el1_timer_flag    = 0;
  TimerTable->header.ns_el1_timer_flag   = 0;
  TimerTable->header.el2_timer_flag      = 0;
  TimerTable->header.el2_virt_timer_flag = 0;  /* NA in DT*/
  TimerTable->header.virtual_timer_flag  = 0;

  /* Search for system timer , either V8 or V7 available*/
  for (i = 0; i < sizeof(systimer_dt_arr)/SYSTIMER_COMPATIBLE_STR_LEN ; i++) {
      offset = fdt_node_offset_by_compatible((const void *)dt_ptr, -1, systimer_dt_arr[i]);
      if (offset >= 0)
        break;
  }
  /* Return if Timer node not found*/
  if (offset < 0) {
      bsa_print(ACS_PRINT_ERR, L"  timer node offset not found\n");
      return;
  }
  interrupt_cell = fdt_interrupt_cells((const void *)dt_ptr, offset);
  bsa_print(ACS_PRINT_DEBUG, L"  interrupt_cell  %d\n", interrupt_cell);
  if (interrupt_cell < INTERRUPT_CELLS_MIN || interrupt_cell > INTERRUPT_CELLS_MAX) {
      bsa_print(ACS_PRINT_ERR, L"  Invalid interrupt cell : %d\n", interrupt_cell);
      return;
  }

    /* Get Interrupt property of timer node*/
  Pintr = (UINT32 *)fdt_getprop_namelen((void *)dt_ptr, offset, "interrupts", 10, &prop_len);
  if ((prop_len < 0) || (Pintr == NULL)) {
      bsa_print(ACS_PRINT_ERR, L"  PROPERTY interrupts offset %x, Error %d\n", offset, prop_len);
      return;
  }

  if (interrupt_cell >= 3) { /* Interrupt type available*/
      if (fdt32_to_cpu(Pintr[index++]) == GIC_PPI)
        TimerTable->header.s_el1_timer_gsiv   = fdt32_to_cpu(Pintr[index++]) + PPI_OFFSET;
      index++; /* Skip interrupt flag*/
      if (interrupt_cell == 4)
         index++; /* Skip CPU affinity*/
      if (fdt32_to_cpu(Pintr[index++]) == GIC_PPI)
        TimerTable->header.ns_el1_timer_gsiv  = fdt32_to_cpu(Pintr[index++]) + PPI_OFFSET;
      index++; /* Skip interrupt flag*/
      if (interrupt_cell == 4)
         index++; /* Skip CPU affinity*/
      if (fdt32_to_cpu(Pintr[index++]) == GIC_PPI)
        TimerTable->header.virtual_timer_gsiv = fdt32_to_cpu(Pintr[index++]) + PPI_OFFSET;
      index++; /* Skip interrupt flag*/
      if (interrupt_cell == 4)
         index++; /* Skip CPU affinity*/
      if (fdt32_to_cpu(Pintr[index++]) == GIC_PPI)
        TimerTable->header.el2_timer_gsiv     = fdt32_to_cpu(Pintr[index++]) + PPI_OFFSET;
      index++; /* Skip interrupt flag*/
      if (interrupt_cell == 4)
         index++; /* Skip CPU affinity*/
  }
  else {
      TimerTable->header.s_el1_timer_gsiv   = fdt32_to_cpu(Pintr[index++]);
      index += (interrupt_cell - 1); /* Skip interrupt flag, if available*/
      TimerTable->header.ns_el1_timer_gsiv  = fdt32_to_cpu(Pintr[index++]);
      index += (interrupt_cell - 1); /* Skip interrupt flag, if available*/
      TimerTable->header.virtual_timer_gsiv = fdt32_to_cpu(Pintr[index++]);
      index += (interrupt_cell - 1); /* Skip interrupt flag, if available*/
      TimerTable->header.el2_timer_gsiv     = fdt32_to_cpu(Pintr[index++]);
  }
  /* Get Always on property of timer node*/
  Palways_on = (UINT32 *)fdt_getprop_namelen((void *)dt_ptr, offset, "always-on", 9, &prop_len);
  if (Palways_on != NULL) {
      TimerTable->header.s_el1_timer_flag    = TIMER_FLAG_ALWAYS_ON;
      TimerTable->header.ns_el1_timer_flag   = TIMER_FLAG_ALWAYS_ON;
      TimerTable->header.el2_timer_flag      = TIMER_FLAG_ALWAYS_ON;
      TimerTable->header.virtual_timer_flag  = TIMER_FLAG_ALWAYS_ON;
  }
  else
    bsa_print(ACS_PRINT_DEBUG, L"  PROPERTY always-on not found\n");

  /* Search for mem mapped timers*/
  for (i = 0; i < sizeof(memtimer_dt_arr)/MEMTIMER_COMPATIBLE_STR_LEN ; i++) {
      offset = fdt_node_offset_by_compatible((const void *)dt_ptr, -1, memtimer_dt_arr[i]);
      if (offset >= 0)
        break;
  }
  if (offset < 0) {
      bsa_print(ACS_PRINT_ERR, L"  MEM timer node offset not found\n");
      return;
  }

  /* Get Address_cell & Size_cell length to parse reg property of timer*/
  parent_offset = fdt_parent_offset((const void *) dt_ptr, offset);
  bsa_print(ACS_PRINT_DEBUG, L"  Parent Node offset %d\n", offset);

  size_cell = fdt_size_cells((const void *) dt_ptr, parent_offset);
  bsa_print(ACS_PRINT_DEBUG, L"  size cell %d\n", size_cell);
  if (size_cell < 0) {
      bsa_print(ACS_PRINT_ERR, L"  Invalid size cell :%d\n", size_cell);
      return;
  }

  addr_cell = fdt_address_cells((const void *) dt_ptr, parent_offset);
  bsa_print(ACS_PRINT_DEBUG, L"  addr cell %d\n", addr_cell);
  if (addr_cell < 1 || addr_cell > 2) {
      bsa_print(ACS_PRINT_ERR, L"  Invalid address cell : %d\n", addr_cell);
      return;
  }

  /* Get reg property to update block_cntl_base */
  Preg = (UINT32 *)fdt_getprop_namelen((void *)dt_ptr, offset, "reg", 3, &prop_len);
  if ((prop_len < 0) || (Preg == NULL)) {
      bsa_print(ACS_PRINT_ERR, L"  PROPERTY REG offset %x, Error %d\n", offset, prop_len);
      return;
  }

  GtEntry->type = TIMER_TYPE_SYS_TIMER;
  if (addr_cell == 1)
    GtEntry->block_cntl_base = fdt32_to_cpu(Preg[0]);
  else
    GtEntry->block_cntl_base = (UINT64)((UINT64)fdt32_to_cpu(Preg[0]) << 32) |
                                fdt32_to_cpu(Preg[1]);

  /* Get Address_cell & Size_cell length to parse reg property of frame*/
  size_cell = fdt_size_cells((const void *) dt_ptr, offset);
  bsa_print(ACS_PRINT_DEBUG, L"  size cell %d\n", size_cell);
  if (size_cell < 0) {
      bsa_print(ACS_PRINT_ERR, L"  Invalid size cell for timer node :%d\n", size_cell);
      return;
  }

  addr_cell = fdt_address_cells((const void *) dt_ptr, offset);
  bsa_print(ACS_PRINT_DEBUG, L"  addr cell %d\n", addr_cell);
  if (addr_cell < 1 || addr_cell > 2) {
      bsa_print(ACS_PRINT_ERR, L"  Invalid address cell for timer node: %d\n", addr_cell);
      return;
  }

  /* Get frame sub node*/
  subnode_offset = fdt_subnode_offset((const void *)dt_ptr, offset, "frame");
  if (subnode_offset < 0) {
      bsa_print(ACS_PRINT_DEBUG, L"  frame node offset not found %d\n", subnode_offset);
      return;
  }

  while (subnode_offset != -FDT_ERR_NOTFOUND) {
      /* Get frame number*/
      frame_number = fdt_frame_number((const void *)dt_ptr, subnode_offset);
      bsa_print(ACS_PRINT_DEBUG, L"  Frame number is  %d\n", frame_number);

      /* Get reg property from frame to update GtCntBase */
      Preg = (UINT32 *)fdt_getprop_namelen((void *)dt_ptr, subnode_offset, "reg", 3, &prop_len);
      if ((prop_len < 0) || (Preg == NULL)) {
          bsa_print(ACS_PRINT_ERR, L"  PROPERTY REG offset %x, Error %d\n", offset, prop_len);
          return;
      }

      /* Get interrupts property from frame */
      Pintr = (UINT32 *)
              fdt_getprop_namelen((void *)dt_ptr, subnode_offset, "interrupts", 10, &prop_len);
      if ((prop_len < 0) || (Pintr == NULL)) {
          bsa_print(ACS_PRINT_ERR, L"  PROPERTY interrupts offset %x, Error %d\n",
                    offset, prop_len);
          return;
      }

      interrupt_cell = fdt_interrupt_cells((const void *)dt_ptr, subnode_offset);
      bsa_print(ACS_PRINT_DEBUG, L"  interrupt_cell for subnode  %d\n", interrupt_cell);
      if (interrupt_cell < INTERRUPT_CELLS_MIN || interrupt_cell > INTERRUPT_CELLS_MAX) {
          bsa_print(ACS_PRINT_ERR, L"  Invalid interrupt cell subnode: %d\n", interrupt_cell);
          return;
      }

      index = 0;
      if (addr_cell == 2) {
        GtEntry->GtCntBase[GtEntry->timer_count] = fdt32_to_cpu(Preg[index++]);
        GtEntry->GtCntBase[GtEntry->timer_count] = (GtEntry->GtCntBase[GtEntry->timer_count] << 32)
                                                   | fdt32_to_cpu(Preg[index]);
      }
      else
        GtEntry->GtCntBase[GtEntry->timer_count] = fdt32_to_cpu(Preg[index]);

      GtEntry->frame_num[GtEntry->timer_count] = frame_number;

      index = 0;
      if (interrupt_cell >= 3) {
          if (fdt32_to_cpu(Pintr[index++]) == GIC_SPI)
            GtEntry->gsiv[GtEntry->timer_count] = fdt32_to_cpu(Pintr[index++]) + SPI_OFFSET;
          else
            GtEntry->gsiv[GtEntry->timer_count] = 0; /* Invalid interrupt type*/

          index++; /* Skip interrupt flag*/
          if (interrupt_cell == 4)
             index++; /* Skip CPU affinity*/
          if ((prop_len/sizeof(UINT32)) > interrupt_cell) {/* virt_gsiv is optional*/
              if (fdt32_to_cpu(Pintr[index++]) == GIC_SPI)
                GtEntry->virt_gsiv[GtEntry->timer_count] = fdt32_to_cpu(Pintr[index]) + SPI_OFFSET;
              else
                GtEntry->virt_gsiv[GtEntry->timer_count] = 0;
          }
          else /* virt_gsiv is not present*/
            GtEntry->virt_gsiv[GtEntry->timer_count] = 0;  /* Invalid interrupt type*/
      }
      else {
          GtEntry->gsiv[GtEntry->timer_count] = fdt32_to_cpu(Pintr[index++]);
          index += (interrupt_cell - 1); /* Skip interrupt flag, if available*/
          if ((prop_len/sizeof(UINT32)) > interrupt_cell) {/* virt_gsiv is optional*/
              GtEntry->virt_gsiv[GtEntry->timer_count]    = fdt32_to_cpu(Pintr[index]);
              index += (interrupt_cell - 1); /* Skip interrupt flag, if available*/
          }
          else /* virt_gsiv is not present*/
            GtEntry->virt_gsiv[GtEntry->timer_count] = 0;  /* Invalid interrupt type*/
      }

      GtEntry->flags[GtEntry->timer_count] = 0;
      GtEntry->GtCntEl0Base[GtEntry->timer_count] = 0; /* NA in dt*/
      GtEntry->timer_count++;

      subnode_offset = fdt_next_subnode((const void *)dt_ptr, subnode_offset);
      bsa_print(ACS_PRINT_DEBUG, L"  timer-mem-fram next node offset %d\n", subnode_offset);
  }
  bsa_print(ACS_PRINT_DEBUG, L"  GT block timer count %d\n", GtEntry->timer_count);
  TimerTable->header.num_platform_timer = GtEntry->timer_count;

  dt_dump_timer_table(TimerTable);
}
