/** @file
 * Copyright (c) 2016-2018, 2020-2023, Arm Limited or its affiliates. All rights reserved.
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

#include <Include/libfdt.h>

#include <Protocol/AcpiTable.h>
#include "Include/IndustryStandard/Acpi61.h"
#include "Include/IndustryStandard/SerialPortConsoleRedirectionTable.h"

#include "include/pal_uefi.h"
#include "include/bsa_pcie_enum.h"
#include "../include/platform_override.h"
#include "include/pal_dt.h"
#include "include/pal_dt_spec.h"

#define USB_CLASSCODE   0x0C0300
#define SATA_CLASSCODE  0x010600
#define BAR0            0
#define BAR1            1
#define BAR2            2

UINT64
pal_get_spcr_ptr();

UINT32
pal_strncmp(CHAR8 *str1, CHAR8 *str2, UINT32 len);

static char usb_dt_compatible[][USB_COMPATIBLE_STR_LEN] = {
    "generic-ohci",
    "generic-ehci",
    "generic-xhci"
};

static char sata_dt_compatible[][SATA_COMPATIBLE_STR_LEN] = {
    "generic-ahci"
};

static char uart_dt_compatible[][UART_COMPATIBLE_STR_LEN] = {
    "arm,sbsa-uart",
    "arm,pl011",
    "ns16550",
    "ns16550a",
    "ns8250",
};

/**
  @brief  This API fills in the PERIPHERAL_INFO_TABLE with information about USB
          in the system. This is achieved by parsing the DT.

  @param  peripheralInfoTable  - Address where the Peripheral information needs to be filled.

  @return  None
**/
VOID
pal_peripheral_usb_create_info_table_dt(PERIPHERAL_INFO_TABLE *peripheralInfoTable)
{
  PERIPHERAL_INFO_BLOCK *per_info = NULL;
  int i, offset, parent_offset, prop_len;
  UINT64 dt_ptr = 0;
  UINT32 *Preg, *Pintr;
  int addr_cell, size_cell, index = 0;
  int interrupt_cell;

  if (peripheralInfoTable == NULL)
    return;

  dt_ptr = pal_get_dt_ptr();
  if (dt_ptr == 0) {
      bsa_print(ACS_PRINT_ERR, L" dt_ptr is NULL\n");
      return;
  }

  per_info = peripheralInfoTable->info;
  peripheralInfoTable->header.num_usb = 0;

  for (i = 0; i < (sizeof(usb_dt_compatible)/USB_COMPATIBLE_STR_LEN); i++) {

      /* Search for USB nodes*/
      offset = fdt_node_offset_by_compatible((const void *)dt_ptr, -1, usb_dt_compatible[i]);
      if (offset < 0) {
          bsa_print(ACS_PRINT_DEBUG, L"  USB compatible value not found for index:%d\n", i);
          continue; /* Search for next compatible item*/
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
      while (offset != -FDT_ERR_NOTFOUND) {

          per_info->type  = PERIPHERAL_TYPE_USB;

          /* Get reg property to update base */
          Preg = (UINT32 *)fdt_getprop_namelen((void *)dt_ptr, offset, "reg", 3, &prop_len);
          if ((prop_len < 0) || (Preg == NULL)) {
              bsa_print(ACS_PRINT_ERR, L"  PROPERTY REG offset %x, Error %d\n", offset, prop_len);
              return;
          }

          /* Get interrupts property from frame */
          Pintr = (UINT32 *)
                    fdt_getprop_namelen((void *)dt_ptr, offset, "interrupts", 10, &prop_len);
          if ((prop_len < 0) || (Pintr == NULL)) {
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

          index = 0;
          if (addr_cell == 2) {
              per_info->base0 = fdt32_to_cpu(Preg[index++]);
              per_info->base0 = ((per_info->base0 << 32) |
                  fdt32_to_cpu(Preg[index]));
          }
          else
            per_info->base0 = fdt32_to_cpu(Preg[index]);

          index = 0;
          if ((interrupt_cell == 3) || (interrupt_cell == 4)) {
              if (fdt32_to_cpu(Pintr[index++]) == GIC_SPI)
                per_info->irq = fdt32_to_cpu(Pintr[index++]);
              else
                per_info->irq = 0;
          } else
              per_info->irq = fdt32_to_cpu(Pintr[index++]);

          per_info->bdf   = 0; /* NA in DT*/
          per_info->flags = 0; /* NA in DT*/
          per_info->platform_type = PLATFORM_TYPE_DT;

          if (!(pal_strncmp(usb_dt_compatible[i], "generic-ohci", sizeof("generic-ohci"))))
            per_info->interface_type = USB_TYPE_OHCI;
          else if (!(pal_strncmp(usb_dt_compatible[i], "generic-ehci", sizeof("generic-ehci"))))
            per_info->interface_type = USB_TYPE_EHCI;
          else if (!(pal_strncmp(usb_dt_compatible[i], "generic-xhci", sizeof("generic-xhci"))))
            per_info->interface_type = USB_TYPE_XHCI;

          peripheralInfoTable->header.num_usb++;
          per_info++;
          offset =
              fdt_node_offset_by_compatible((const void *)dt_ptr, offset, usb_dt_compatible[i]);
      }
  }
}

/**
  @brief  This API fills in the PERIPHERAL_INFO_TABLE with information about SATA
          in the system. This is achieved by parsing the DT.

  @param  peripheralInfoTable  - Address where the Peripheral information needs to be filled.

  @return  None
**/
VOID
pal_peripheral_sata_create_info_table_dt(PERIPHERAL_INFO_TABLE *peripheralInfoTable)
{
  PERIPHERAL_INFO_BLOCK *per_info = NULL;
  int i, offset, parent_offset, prop_len;
  UINT64 dt_ptr = 0;
  UINT32 *Preg, *Pintr;
  int addr_cell, size_cell, index = 0;
  int interrupt_cell;

  if (peripheralInfoTable == NULL)
    return;

  dt_ptr = pal_get_dt_ptr();
  if (dt_ptr == 0) {
      bsa_print(ACS_PRINT_ERR, L" dt_ptr is NULL\n");
      return;
  }

  per_info = &peripheralInfoTable->info[peripheralInfoTable->header.num_usb];

  peripheralInfoTable->header.num_sata = 0;

  for (i = 0; i < (sizeof(sata_dt_compatible)/SATA_COMPATIBLE_STR_LEN); i++) {

      /* Search for sata node*/
      offset = fdt_node_offset_by_compatible((const void *)dt_ptr, -1, sata_dt_compatible[i]);
      if (offset < 0) {
          bsa_print(ACS_PRINT_DEBUG, L"  SATA compatible value not found for index:%d\n", i);
          continue; /* Search for next compatible item*/
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
      while (offset != -FDT_ERR_NOTFOUND) {

          per_info->type  = PERIPHERAL_TYPE_SATA;

          /* Get reg property to update base */
          Preg = (UINT32 *)fdt_getprop_namelen((void *)dt_ptr, offset, "reg", 3, &prop_len);
          if ((prop_len < 0) || (Preg == NULL)) {
              bsa_print(ACS_PRINT_ERR, L"  PROPERTY REG offset %x, Error %d\n", offset, prop_len);
              return;
          }

          /* Get interrupts property from frame */
          Pintr = (UINT32 *)
                    fdt_getprop_namelen((void *)dt_ptr, offset, "interrupts", 10, &prop_len);
          if ((prop_len < 0) || (Pintr == NULL)) {
              bsa_print(ACS_PRINT_ERR, L"  PROPERTY interrupts offset %x, Error %d\n",
                        offset, prop_len);
              return;
          }

          interrupt_cell = fdt_interrupt_cells((const void *)dt_ptr, offset);
          bsa_print(ACS_PRINT_DEBUG, L"  interrupt_cell  %d\n", interrupt_cell);
          if (interrupt_cell < INTERRUPT_CELLS_MIN || interrupt_cell > INTERRUPT_CELLS_MAX) {
              bsa_print(ACS_PRINT_ERR, L" Invalid interrupt cell : %d\n", interrupt_cell);
              return;
          }

          index = 0;
          if (addr_cell == 2) {
              per_info->base1 = fdt32_to_cpu(Preg[index++]);
              per_info->base1 = ((per_info->base1 << 32) |
                  fdt32_to_cpu(Preg[index]));
          }
          else
            per_info->base1 = fdt32_to_cpu(Preg[index]);

          index = 0;
          if ((interrupt_cell == 3) || (interrupt_cell == 4)) {
              if (fdt32_to_cpu(Pintr[index++]) == GIC_SPI)
                per_info->irq = fdt32_to_cpu(Pintr[index++]);
              else
                per_info->irq = 0;
          } else
              per_info->irq = fdt32_to_cpu(Pintr[index++]);

          per_info->bdf   = 0; /* NA in DT*/
          per_info->flags = 0; /* NA in DT*/
          per_info->platform_type = PLATFORM_TYPE_DT;

          if (!(pal_strncmp(sata_dt_compatible[i], "generic-ahci", sizeof("generic-ahci"))))
            per_info->interface_type = SATA_TYPE_AHCI;

          peripheralInfoTable->header.num_sata++;
          per_info++;
          offset =
              fdt_node_offset_by_compatible((const void *)dt_ptr, offset, sata_dt_compatible[i]);
      }
  }
}

/**
  @brief  This API fills in the PERIPHERAL_INFO_TABLE with information about UART
          in the system. This is achieved by parsing the DT.

  @param  peripheralInfoTable  - Address where the Peripheral information needs to be filled.

  @return  None
**/
VOID
pal_peripheral_uart_create_info_table_dt(PERIPHERAL_INFO_TABLE *peripheralInfoTable)
{
  PERIPHERAL_INFO_BLOCK *per_info = NULL;
  int i, offset, parent_offset, prop_len;
  UINT64 dt_ptr = 0;
  UINT64 range_node_addr, parent_offset_addr;
  UINT64 temp_child_addr;
  UINT32 *Preg, *Pintr, *Pranges;
  CHAR8  *Pstatus;
  int addr_cell, size_cell, index = 0;
  int interrupt_cell, range_node_left;
  int child_addr_cell, parent_addr_cell;
  int child_size_cell, range_index;
  int range_node_offset, range_parent_offset;


  if (peripheralInfoTable == NULL)
    return;

  dt_ptr = pal_get_dt_ptr();
  if (dt_ptr == 0) {
      bsa_print(ACS_PRINT_ERR, L" dt_ptr is NULL\n");
      return;
  }

  per_info = &peripheralInfoTable->info[peripheralInfoTable->header.num_usb +
                                        peripheralInfoTable->header.num_sata];

  peripheralInfoTable->header.num_uart = 0;

  for (i = 0; i < (sizeof(uart_dt_compatible) / UART_COMPATIBLE_STR_LEN); i++) {

      /* Search for uart nodes*/
      offset = fdt_node_offset_by_compatible((const void *)dt_ptr, -1, uart_dt_compatible[i]);
      if (offset < 0) {
          bsa_print(ACS_PRINT_DEBUG, L"  UART compatible value not found for index:%d\n", i);
          continue; /* Search for next compatible item*/
      }

      /* Get Address_cell & Size_cell length to parse reg property of uart*/
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
      while (offset != -FDT_ERR_NOTFOUND) {

          /* Consider only that UART which is visible in non-secure world
             Status fields either not present or if present should not be disabled */
          Pstatus = (CHAR8 *)fdt_getprop_namelen((void *)dt_ptr, offset, "status", 6, &prop_len);
          if ((prop_len > 0) && (Pstatus != NULL)) {
              bsa_print(ACS_PRINT_DEBUG, L"  Status field length %d\n", prop_len);
              if (pal_strncmp(Pstatus, "disabled", 9) == 0) {
                  bsa_print(ACS_PRINT_DEBUG, L"  UART access is secure\n");
                  offset = fdt_node_offset_by_compatible((const void *)dt_ptr, offset,
                                                          uart_dt_compatible[i]);
                  continue;
              }
          }

          /* Get reg property to update base */
          Preg = (UINT32 *)fdt_getprop_namelen((void *)dt_ptr, offset, "reg", 3, &prop_len);
          if ((prop_len < 0) || (Preg == NULL)) {
              bsa_print(ACS_PRINT_ERR, L"  PROPERTY REG offset %x, Error %d\n", offset, prop_len);
              return;
          }

          index = 0;
          if (addr_cell == 2) {
              per_info->base0 = fdt32_to_cpu(Preg[index++]);
              per_info->base0 = ((per_info->base0 << 32) | fdt32_to_cpu(Preg[index]));
          } else
              per_info->base0 = fdt32_to_cpu(Preg[index]);

          /* some boards use ranges property to map device address to cpu address space
         1. check if device parent has ranges, if not address is in cpu address space
         2. else if device parent has empty ranges, then needs to search ranges of parent's parent
         3. In case parent has non empty range, search child node addr in that.
         4. If its not found and range has single tuple value, continue search with parent's
         5. Since dt are quite varied, as fail-safe only search upto 3 parents else break
          */
  /* Start with searching current node address in parent ranges, so treat current node as child */
          range_node_offset = offset;
          range_node_addr = per_info->base0;
          range_parent_offset = fdt_parent_offset((const void *) dt_ptr, offset);
          parent_offset_addr = 0;
          range_node_left = 3; /* how many parent nodes will search */
          while (range_node_left > 0) {
              Pranges = (UINT32 *)
                  fdt_getprop_namelen((void *)dt_ptr, range_parent_offset, "ranges", 6, &prop_len);
              bsa_print(ACS_PRINT_DEBUG, L"  Parent ranges length %d\n", prop_len);
              if ((prop_len < 0) || (Pranges == NULL)) {
                  bsa_print(ACS_PRINT_DEBUG, L"  No ranges is present\n");
                  range_node_left = 0;
                  break;
              }
              range_node_left--;
              if ((Pranges != NULL) && (prop_len == 0)) {// Empty ranges
                  bsa_print(ACS_PRINT_DEBUG, L"  Empty ranges is present\n");
                  range_parent_offset = fdt_parent_offset((const void *) dt_ptr,
                                                                             range_parent_offset);
              } else {
                  range_node_offset = range_parent_offset;
                 range_parent_offset = fdt_parent_offset((const void *) dt_ptr, range_node_offset);
                  /* ranges = <child addr cell  parent addr cell   child size cell> */
                  child_addr_cell = fdt_address_cells((const void *) dt_ptr, range_node_offset);
                  parent_addr_cell = fdt_address_cells((const void *) dt_ptr, range_parent_offset);
                  child_size_cell = fdt_size_cells((const void *) dt_ptr, range_node_offset);

                  bsa_print(ACS_PRINT_DEBUG, L"  child addr cell %d\n", child_addr_cell);
                  bsa_print(ACS_PRINT_DEBUG, L"  parent addr cell %d\n", parent_addr_cell);
                  bsa_print(ACS_PRINT_DEBUG, L"  child size cell %d\n", child_size_cell);
                  if ((child_addr_cell < 1 || child_addr_cell > 2) ||
                     (parent_addr_cell < 1 || parent_addr_cell > 2) || (child_size_cell < 0))
                      return;
                  range_index = 0;
                  if (prop_len == (4 * (child_addr_cell + parent_addr_cell + child_size_cell))) {
                      range_index += child_addr_cell;
                      range_node_addr = fdt32_to_cpu(Pranges[range_index++]);
                      if (parent_addr_cell == 2)
                          range_node_addr = (range_node_addr << 32) |
                                                fdt32_to_cpu(Pranges[range_index++]);
                      bsa_print(ACS_PRINT_DEBUG, L"  range node addr %lx\n", range_node_addr);
                      continue;
                  }
                  while (range_index < (prop_len / 4)) {
                      bsa_print(ACS_PRINT_DEBUG, L"  range_index %d\n", range_index);
                      temp_child_addr = fdt32_to_cpu(Pranges[range_index++]);
                      if (child_addr_cell == 2)
                          temp_child_addr = (temp_child_addr << 32) |
                                                    fdt32_to_cpu(Pranges[range_index++]);
                      bsa_print(ACS_PRINT_DEBUG, L"  temp node addr %lx\n", temp_child_addr);
                      if (temp_child_addr == range_node_addr) {
                          parent_offset_addr = fdt32_to_cpu(Pranges[range_index++]);
                      bsa_print(ACS_PRINT_DEBUG, L"  parent offset addr %lx\n", parent_offset_addr);
                      bsa_print(ACS_PRINT_DEBUG, L"  range index %d\n", range_index);
                          if (parent_addr_cell == 2) {
                              parent_offset_addr = (parent_offset_addr << 32) |
                                                            fdt32_to_cpu(Pranges[range_index++]);
                      bsa_print(ACS_PRINT_DEBUG, L"  2 parent offset addr %lx\n",
                                                                              parent_offset_addr);
                      bsa_print(ACS_PRINT_DEBUG, L"  2 range index %d\n", range_index);
                          }
                          break;
                      }
                      range_index += (parent_addr_cell + child_size_cell);
                  }
                  range_node_left = 0; break;
              }
          }

         bsa_print(ACS_PRINT_DEBUG, L"  parent offset addr  %lx\n", parent_offset_addr);
         per_info->base0 += parent_offset_addr;

          /* Get interrupts property from frame */
          Pintr = (UINT32 *)
                    fdt_getprop_namelen((void *)dt_ptr, offset, "interrupts", 10, &prop_len);
          if ((prop_len < 0) || (Pintr == NULL)) {
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

          per_info->type  = PERIPHERAL_TYPE_UART;

          index = 0;
          if ((interrupt_cell == 3) || (interrupt_cell == 4)) {
              if (fdt32_to_cpu(Pintr[index++]) == GIC_SPI)
                per_info->irq = fdt32_to_cpu(Pintr[index++]) + SPI_OFFSET;
              else
                per_info->irq = 0;
          } else
              per_info->irq = fdt32_to_cpu(Pintr[index++]) + SPI_OFFSET;

          if (!(pal_strncmp(uart_dt_compatible[i], "ns16550", sizeof("ns16550")))
              || !(pal_strncmp(uart_dt_compatible[i], "ns16550a", sizeof("ns16550a")))
              || !(pal_strncmp(uart_dt_compatible[i], "ns8250", sizeof("ns8250a"))))
          {
              per_info->interface_type = COMPATIBLE_FULL_16550;
          } else
              per_info->interface_type = ARM_PL011_UART;

          per_info->bdf   = 0; /* NA in DT*/
          per_info->flags = 0; /* NA in DT*/
          per_info->baud_rate = 0; /* NA in DT*/
          peripheralInfoTable->header.num_uart++;
          per_info++;
          offset =
              fdt_node_offset_by_compatible((const void *)dt_ptr, offset, uart_dt_compatible[i]);
      }
  }
}
/**
  @brief  This API fills in the PERIPHERAL_INFO_TABLE with information about peripherals
          in the system. This is achieved by parsing the ACPI - SPCR table and PCIe config space.

  @param  peripheralInfoTable  - Address where the Peripheral information needs to be filled.

  @return  None
**/
VOID
pal_peripheral_create_info_table(PERIPHERAL_INFO_TABLE *peripheralInfoTable)
{
  UINT32   DeviceBdf = 0;
  UINT32   StartBdf  = 0;
  UINT32   bar_index = 0;
  PERIPHERAL_INFO_BLOCK *per_info = NULL;
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE *spcr = NULL;

  if (peripheralInfoTable == NULL) {
    bsa_print(ACS_PRINT_ERR,
              L" Input Peripheral Table Pointer is NULL. Cannot create Peripheral INFO\n");
    return;
  }

  per_info = peripheralInfoTable->info;

  peripheralInfoTable->header.num_usb = 0;
  peripheralInfoTable->header.num_sata = 0;
  peripheralInfoTable->header.num_uart = 0;
  peripheralInfoTable->header.num_all = 0;
  per_info->base0 = 0;


  /* check for any USB Controllers */
  do {

       DeviceBdf = palPcieGetBdf(USB_CLASSCODE, StartBdf);
       if (DeviceBdf != 0) {
          per_info->type  = PERIPHERAL_TYPE_USB;
          for (bar_index = 0; bar_index < TYPE0_MAX_BARS; bar_index++)
          {
              per_info->base0 = palPcieGetBase(DeviceBdf, bar_index);
              if (per_info->base0 != 0)
                  break;
          }
          per_info->bdf   = DeviceBdf;
          bsa_print(ACS_PRINT_INFO, L"  Found a USB controller %4x\n", per_info->base0);
          peripheralInfoTable->header.num_usb++;
          peripheralInfoTable->header.num_all++;
          per_info++;
       }
       StartBdf = incrementBusDev(DeviceBdf);

  } while (DeviceBdf != 0);

  if (peripheralInfoTable->header.num_usb == 0) { /* Search for USB in Device tree*/
    pal_peripheral_usb_create_info_table_dt(peripheralInfoTable);
    per_info += peripheralInfoTable->header.num_usb;
  }

  StartBdf = 0;
  /* check for any SATA Controllers */
  do {

       DeviceBdf = palPcieGetBdf(SATA_CLASSCODE, StartBdf);
       if (DeviceBdf != 0) {
          per_info->type  = PERIPHERAL_TYPE_SATA;
          for (bar_index = 0; bar_index < TYPE0_MAX_BARS; bar_index++)
          {
              per_info->base0 = palPcieGetBase(DeviceBdf, bar_index);
              if (per_info->base0 != 0)
                  break;
          }
          per_info->bdf   = DeviceBdf;
          bsa_print(ACS_PRINT_INFO, L"  Found a SATA controller %4x\n", per_info->base0);
          peripheralInfoTable->header.num_sata++;
          peripheralInfoTable->header.num_all++;
          per_info++;
       }
       //Increment and check if we have more controllers
       StartBdf = incrementBusDev(DeviceBdf);

  } while (DeviceBdf != 0);

  if (peripheralInfoTable->header.num_sata == 0) { /* Search for SATA in Device tree*/
    pal_peripheral_sata_create_info_table_dt(peripheralInfoTable);
    per_info += peripheralInfoTable->header.num_sata;
  }

  /* Search for a SPCR table in the system to get the UART details */
  spcr = (EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE *)pal_get_spcr_ptr();

  if (spcr) {
    peripheralInfoTable->header.num_uart++;
    peripheralInfoTable->header.num_all++;
    per_info->base0 = spcr->BaseAddress.Address;
    per_info->width = 1 << (spcr->BaseAddress.AccessSize + 2);  // Convert GAS to 8/16/32
    per_info->irq   = spcr->GlobalSystemInterrupt;
    per_info->type  = PERIPHERAL_TYPE_UART;
    per_info++;
  }

  if (peripheralInfoTable->header.num_uart == 0) { /* Search for UART in Device tree*/
    pal_peripheral_uart_create_info_table_dt(peripheralInfoTable);
    per_info += peripheralInfoTable->header.num_uart;
  }

  if (PLATFORM_GENERIC_UART_BASE) {
    peripheralInfoTable->header.num_uart++;
    peripheralInfoTable->header.num_all++;
    per_info->base0 = PLATFORM_GENERIC_UART_BASE;
    per_info->irq   = PLATFORM_GENERIC_UART_INTID;
    per_info->type  = PERIPHERAL_TYPE_UART;
    per_info++;
  }

  per_info->type = 0xFF; //indicate end of table
  dt_dump_peripheral_table(peripheralInfoTable);
}


/**
  @brief  Check if the memory type is reserved for UEFI

  @param  EFI_MEMORY_TYPE  - Type of UEFI memory.

  @return  true   if memory reserved for UEFI usage
           false  otherwise
**/
BOOLEAN
IsUefiMemory(EFI_MEMORY_TYPE type)
{

  switch(type) {
    case  EfiReservedMemoryType:
    case  EfiLoaderCode:
    case  EfiLoaderData:
    case  EfiBootServicesCode:
    case  EfiBootServicesData:
    case  EfiRuntimeServicesCode:
    case  EfiRuntimeServicesData:
    case  EfiACPIReclaimMemory:
    case  EfiACPIMemoryNVS:
      return TRUE;
    default:
      return FALSE;
  }

}

/**
  @brief  Check if the memory type is normal

  @param  EFI_MEMORY_TYPE  - Type of UEFI memory.

  @return  true   if memory is normal
           false  otherwise
**/
BOOLEAN
IsNormalMemory(EFI_MEMORY_TYPE type)
{

  switch(type) {
    case EfiConventionalMemory:
      return TRUE;
    default:
      return FALSE;
  }

}

/**
  @brief  Check if the memory type is device

  @param  EFI_MEMORY_TYPE  - Type of UEFI memory.

  @return  true   if memory is device
           false  otherwise
**/
BOOLEAN
IsDeviceMemory(EFI_MEMORY_TYPE type)
{

  switch(type) {
    case  EfiMemoryMappedIO:
    case  EfiMemoryMappedIOPortSpace:
    case  EfiPersistentMemory:
      return TRUE;
    default:
      return FALSE;
  }
}


/**
  @brief  This API fills in the MEMORY_INFO_TABLE with information about memory in the
          system. This is achieved by parsing the UEFI memory map.

  @param  memory_info_table Address where the memory info table is created
  @return  None
**/
VOID
pal_memory_create_info_table(MEMORY_INFO_TABLE *memoryInfoTable)
{

  UINTN                 MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
  EFI_MEMORY_DESCRIPTOR *MemoryMapPtr = NULL;
  EFI_PHYSICAL_ADDRESS  Address;
  UINTN                 MapKey;
  UINTN                 DescriptorSize;
  UINT32                DescriptorVersion;
  UINTN                 Pages;
  EFI_STATUS            Status;
  UINT32                Index, i = 0;

  if (memoryInfoTable == NULL) {
    bsa_print(ACS_PRINT_ERR, L" Input Memory Table Pointer is NULL. Cannot create Memory INFO\n");
    return;
  }

// Retrieve the UEFI Memory Map

  MemoryMap = NULL;
  MemoryMapSize = 0;
  Status = gBS->GetMemoryMap (&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    // The UEFI specification advises to allocate more memory for the MemoryMap buffer between successive
    // calls to GetMemoryMap(), since allocation of the new buffer may potentially increase memory map size.
    Pages = EFI_SIZE_TO_PAGES (MemoryMapSize) + 1;
    gBS->AllocatePages (AllocateAnyPages, EfiBootServicesData, Pages, &Address);
    MemoryMap = (EFI_MEMORY_DESCRIPTOR *)Address;
    if (MemoryMap == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      return;
    }
    Status = gBS->GetMemoryMap (&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
  }

  // Go through the list and add the reserved region to the Device Tree
  if (!EFI_ERROR (Status)) {
    MemoryMapPtr = MemoryMap;
    for (Index = 0; Index < (MemoryMapSize / DescriptorSize); Index++) {
          bsa_print(ACS_PRINT_INFO, L"  Reserved region of type %d [0x%lX, 0x%lX]\n",
            MemoryMapPtr->Type, (UINTN)MemoryMapPtr->PhysicalStart,
            (UINTN)(MemoryMapPtr->PhysicalStart + MemoryMapPtr->NumberOfPages * EFI_PAGE_SIZE));
      if (IsUefiMemory ((EFI_MEMORY_TYPE)MemoryMapPtr->Type)) {
        memoryInfoTable->info[i].type      = MEMORY_TYPE_RESERVED;
      } else {
        if (IsNormalMemory ((EFI_MEMORY_TYPE)MemoryMapPtr->Type)) {
          memoryInfoTable->info[i].type      = MEMORY_TYPE_NORMAL;
        } else {
          if (IsDeviceMemory ((EFI_MEMORY_TYPE)MemoryMapPtr->Type)) {
            memoryInfoTable->info[i].type      = MEMORY_TYPE_DEVICE;
          } else {
            memoryInfoTable->info[i].type      = MEMORY_TYPE_NOT_POPULATED;
          }
        }
      }
      memoryInfoTable->info[i].phy_addr  = MemoryMapPtr->PhysicalStart;
      memoryInfoTable->info[i].virt_addr = MemoryMapPtr->VirtualStart;
      memoryInfoTable->info[i].size      = (MemoryMapPtr->NumberOfPages * EFI_PAGE_SIZE);
      i++;
      if (i >= MEM_INFO_TBL_MAX_ENTRY) {
        bsa_print(ACS_PRINT_DEBUG, L"  Memory Info tbl limit exceeded, Skipping remaining\n", 0);
        break;
      }

      MemoryMapPtr = (EFI_MEMORY_DESCRIPTOR*)((UINTN)MemoryMapPtr + DescriptorSize);
    }
    memoryInfoTable->info[i].type      = MEMORY_TYPE_LAST_ENTRY;
  }

}

/**
  @brief Maps the physical memory region into the virtual address space

  @param ptr Pointer to physical memory region
  @param size Size
  @param attr Attributes

  @return Pointer to mapped virtual address space
**/
UINT64
pal_memory_ioremap(VOID *ptr, UINT32 size, UINT32 attr)
{


  return (UINT64)ptr;
}

/**
  @brief Removes the physical memory to virtual address space mapping

  @param ptr Pointer to mapped space

  @return None
**/
VOID
pal_memory_unmap(VOID *ptr)
{

  return;
}

/**
  @brief  Return the address of unpopulated memory of requested
          instance from the GCD memory map.

  @param  addr      - Address of the unpopulated memory
          instance  - Instance of memory

  @return  EFI_STATUS
**/
UINT64
pal_memory_get_unpopulated_addr(UINT64 *addr, UINT32 instance)
{
  EFI_STATUS                        Status;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR  *MemorySpaceMap = NULL;
  UINT32                            Index;
  UINTN                             NumberOfDescriptors;
  UINT32                            Memory_instance = 0;

  return 0; /* TBD-DT : uboot is not supporting DxeServicesTable, removing that dependency
          from BsaPal.inf results in gDS ptr not found, temp returning from here */
  /* Get the Global Coherency Domain Memory Space map table */
  Status = gDS->GetMemorySpaceMap(&NumberOfDescriptors, &MemorySpaceMap);
  if (Status != EFI_SUCCESS)
    return Status;

  for (Index = 0; Index < NumberOfDescriptors; Index++, MemorySpaceMap++)
  {
    if (MemorySpaceMap->GcdMemoryType == EfiGcdMemoryTypeNonExistent)
    {
      if (Memory_instance == instance)
      {
        *addr = MemorySpaceMap->BaseAddress;
        if (*addr == 0)
          continue;

        bsa_print(ACS_PRINT_INFO,L"  Unpopulated region with base address 0x%lX found\n", *addr);
        return EFI_SUCCESS;
      }

      Memory_instance++;
    }
  }

  return PCIE_NO_MAPPING;
}

/**
  @brief  Platform specific code for UART initialisation

  @param   None
  @return  None
**/
VOID
pal_peripheral_uart_setup(VOID)
{
  return;
}
