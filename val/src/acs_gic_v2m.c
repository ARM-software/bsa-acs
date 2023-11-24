/** @file
 * Copyright (c) 2021, 2023, Arm Limited or its affiliates. All rights reserved.
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

#include "include/bsa_acs_val.h"
#include "include/bsa_acs_gic.h"
#include "include/bsa_acs_gic_support.h"
#include "include/bsa_acs_common.h"
#include "include/bsa_acs_memory.h"

extern GIC_INFO_TABLE * g_gic_info_table;
GICv2m_MSI_FRAME_INFO *g_v2m_msi_info;

/**
  @brief   This function parses the V2M MSI Information from gic info table
           1. Caller       -  Validation Layer
           2. Prerequisite -  val_gic_create_info_table
  @param   None
  @return  Status
**/
uint32_t val_gic_v2m_parse_info(void)
{

  GIC_INFO_ENTRY  *gic_entry;
  uint32_t i;

  if (g_gic_info_table == NULL) {
      val_print(ACS_PRINT_DEBUG, "GIC INFO table not available\n", 0);
      return ACS_STATUS_SKIP;
  }

  /* Allocate memory to store MSI Frame info */
  g_v2m_msi_info = (GICv2m_MSI_FRAME_INFO *) val_aligned_alloc(MEM_ALIGN_4K, 1024);
  if (!g_v2m_msi_info) {
      val_print(ACS_PRINT_DEBUG, "\n       GICv2m : MSI Frame Info Failed.", 0);
      return ACS_STATUS_SKIP;
  }

  gic_entry = g_gic_info_table->gic_info;
  g_v2m_msi_info->num_msi_frame = g_gic_info_table->header.num_msi_frame;
  i = 0;

  while (gic_entry->type != 0xFF) {
    if (gic_entry->type == ENTRY_TYPE_GIC_MSI_FRAME) {
        g_v2m_msi_info->msi_info[i].base      = gic_entry->base;
        g_v2m_msi_info->msi_info[i].entry_id  = gic_entry->entry_id;
        g_v2m_msi_info->msi_info[i].flags     = gic_entry->flags;
        g_v2m_msi_info->msi_info[i].spi_count = gic_entry->spi_count;
        g_v2m_msi_info->msi_info[i].spi_base  = gic_entry->spi_base;
        i++;
    }
    gic_entry++;
  }

  return 0;
}

/**
  @brief   This function gets the V2M MSI Information for Frame instance
           1. Caller       -  Test Suite
           2. Prerequisite -  val_gic_create_info_table
  @param   type Type of information we want to get
  @param   instance v2m MSI Frame Instance number
  @return  Status
**/
uint64_t val_gic_v2m_get_info(V2M_MSI_INFO_e type, uint32_t instance)
{

  uint64_t frame_base;
  uint32_t spi_base, spi_count;

  if (g_v2m_msi_info == NULL) {
      val_print(ACS_PRINT_ERR, "\n   Get GICv2m info called before table is filled ", 0);
      return 0;
  }

  switch (type) {

  case V2M_MSI_FRAME_ID:
      return g_v2m_msi_info->msi_info[instance].entry_id;

  case V2M_MSI_FRAME_BASE:
      return g_v2m_msi_info->msi_info[instance].base;

  case V2M_MSI_SPI_BASE:
      /* Read from Platform Table if Returns 0 then Read from the MSI_TYPER Register */
      spi_base = g_v2m_msi_info->msi_info[instance].spi_base;
      if (!spi_base) {
        frame_base = g_v2m_msi_info->msi_info[instance].base;
        spi_base = VAL_EXTRACT_BITS(val_mmio_read(frame_base + GICv2m_MSI_TYPER), 16, 25);
      }
      return spi_base;

  case V2M_MSI_SPI_NUM:
      /* Read from Platform Table if Returns 0 then Read from the MSI_TYPER Register */
      spi_count = g_v2m_msi_info->msi_info[instance].spi_count;
      if (!spi_count) {
        frame_base = g_v2m_msi_info->msi_info[instance].base;
        spi_count = VAL_EXTRACT_BITS(val_mmio_read(frame_base + GICv2m_MSI_TYPER), 0, 9);
      }
      return spi_count;

  case V2M_MSI_FLAGS:
      return g_v2m_msi_info->msi_info[instance].flags;

  default:
      val_print(ACS_PRINT_ERR, "\n    V2M_MSI_INFO - TYPE not recognized %d  ", type);
      break;
  }
  return ACS_STATUS_ERR;
}
