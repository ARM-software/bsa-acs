/** @file
 * Copyright (c) 2016-2018, 2020, 2021 Arm Limited or its affiliates. All rights reserved.
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
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ShellLib.h>
#include <Library/DtPlatformDtbLoaderLib.h>

#include <Include/libfdt.h>
#include "Include/Guid/Acpi.h"
#include <Protocol/AcpiTable.h>
#include "Include/IndustryStandard/Acpi61.h"

#include "include/pal_uefi.h"
#include "include/pal_dt.h"
/**
  @brief   Checks if gic init and interrupt handlers ACS code is used

  @param  None

  @return True/False
*/
UINT32
pal_bsa_gic_imp()
{
  return 1;
}

/**
  @brief   Use UEFI System Table to look up FdtTableGuid and returns the FDT Blob Address

  @param  None

  @return Returns 64-bit FDT blob address
*/
UINT64
pal_get_dt_ptr()
{
  VOID                       *DTB = NULL;
  UINT32                     Index;

  for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
    if (CompareGuid (&gFdtTableGuid, &(gST->ConfigurationTable[Index].VendorGuid))) {
      DTB = gST->ConfigurationTable[Index].VendorTable;
      bsa_print(ACS_PRINT_DEBUG, L" Platform DTB PTR %x \n", DTB);
      break;
    }
  }

  if (!DTB) {
    bsa_print(ACS_PRINT_ERR, L" DTB not present in platform \n");
    return 0; //No fdt blob addr found
  }

  if (fdt_check_header(DTB)) {
    bsa_print(ACS_PRINT_ERR, L"fdt hdr check failed \n");
    return 0;
  }

  return (UINT64) DTB;
}

/**
  @brief   Get frame number from given node
  @param  fdt - 64-bit FDT blob address
          nodeoffset -offset of node want find frame num
  @return frame number
*/
int fdt_frame_number(const void *fdt, int nodeoffset)
{
  const fdt32_t *ic;
  int val;
  int len;

  ic = fdt_getprop(fdt, nodeoffset, "frame-number", &len);
  if (!ic)
    return 2;

  if (len != sizeof(*ic))
    return -FDT_ERR_BADNCELLS;

  val = fdt32_to_cpu(*ic);
  if (val < 0)
    return -FDT_ERR_BADNCELLS;

  return val;
}


/**
  @brief   Get interrupt cell given node
  @param  fdt - 64-bit FDT blob address
          nodeoffset -offset of node want find frame num
  @return interrupt cells
*/
int fdt_interrupt_cells(const void *fdt, int nodeoffset)
{
  const fdt32_t *ic;
  int len;

  do {
      ic = fdt_getprop(fdt, nodeoffset, "#interrupt-cells", &len);
      if (ic > 0)
          break;

      ic = fdt_getprop(fdt, nodeoffset, "interrupt-parent", &len);
      if (ic > 0)
          nodeoffset = fdt_node_offset_by_phandle(fdt, (uint32_t)(fdt32_to_cpu(*ic)));
      else
          nodeoffset = fdt_parent_offset(fdt, nodeoffset);

  } while (nodeoffset >= 0);

  if (nodeoffset < 0) {
      bsa_print(ACS_PRINT_DEBUG, L"No interrupt cell found \n");
      return 3; /* default value 3*/
  }

  return fdt32_to_cpu(*ic);
}

/*
  @brief  API to get node offset using empty property element

  @param  fdt          Address of fdt blob
  @param  startoffset  Offset from which to search in dt
  @param  propname     Name of property to be searched
  @param  proplen      len of propert name

  @return Returns 64-bit FDT blob address
*/
int fdt_node_offset_by_prop_name(const void *fdt, int startoffset, const char *propname,
                                  int proplen)
{
  int offset;
  int len;
  const struct fdt_property *prop;

  for (offset = fdt_next_node(fdt, startoffset, NULL);
       offset >= 0; offset = fdt_next_node(fdt, offset, NULL)) {
      prop = fdt_getprop_namelen(fdt, offset, propname, proplen, &len);
      if (prop)
        return offset;
  }
  return offset;
}
