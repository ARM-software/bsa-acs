/** @file
 * Copyright (c) 2016-2018, 2021-2024, Arm Limited or its affiliates. All rights reserved.
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
#include "common/include/acs_gic.h"
#include "common/include/acs_gic_support.h"
#include "common/include/acs_common.h"
#include "common/sys_arch_src/gic/gic.h"
#include "bsa/include/bsa_pal_interface.h"

GIC_INFO_TABLE  *g_gic_info_table;

/**
  @brief   This API will call PAL layer to fill in the GIC information
           into the g_gic_info_table pointer.
           1. Caller       -  Application layer.
           2. Prerequisite -  Memory allocated and passed as argument.
  @param   gic_info_table  pre-allocated memory pointer for gic_info
  @return  Error if Input param is NULL
**/
uint32_t
val_gic_create_info_table(uint64_t *gic_info_table)
{
  uint32_t gic_version, num_msi_frame;

  if (gic_info_table == NULL) {
      val_print(ACS_PRINT_ERR, "Input for Create Info table cannot be NULL\n", 0);
      return ACS_STATUS_ERR;
  }
  val_print(ACS_PRINT_INFO, " Creating GIC INFO table\n", 0);

  g_gic_info_table = (GIC_INFO_TABLE *)gic_info_table;

  pal_gic_create_info_table(g_gic_info_table);

  /* print GIC version */
  gic_version = val_gic_get_info(GIC_INFO_VERSION);
  num_msi_frame = val_gic_get_info(GIC_INFO_NUM_MSI_FRAME);
  if ((gic_version != 2) || (num_msi_frame == 0)) /* check if not a GICv2m system */
      val_print(ACS_PRINT_TEST, " GIC INFO: GIC version                :    v%d\n", gic_version);
  else
      val_print(ACS_PRINT_TEST, " GIC INFO: GIC version                :    v2m\n", 0);

  val_print(ACS_PRINT_TEST, " GIC_INFO: Number of GICD             : %4d\n",
                                                             g_gic_info_table->header.num_gicd);
  val_print(ACS_PRINT_TEST, " GIC_INFO: Number of GICR RD          : %4d\n",
                                                             g_gic_info_table->header.num_gicr_rd);
  if (g_gic_info_table->header.num_gicr_rd == 0) {
      val_print(ACS_PRINT_TEST, " GIC_INFO: Number of GICC RD          : %4d\n",
                                                             g_gic_info_table->header.num_gicc_rd);
  }
  val_print(ACS_PRINT_TEST, " GIC_INFO: Number of ITS              : %4d\n",
                                                             g_gic_info_table->header.num_its);

  if (g_gic_info_table->header.num_gicd == 0) {
      val_print(ACS_PRINT_ERR,"\n ** CRITICAL ERROR: GIC Distributor count is 0 **\n", 0);
      return ACS_STATUS_ERR;
  }

#ifndef SBSA
  if (pal_target_is_dt())
      val_bsa_gic_init();
#endif
  if (pal_target_is_bm())
      val_bsa_gic_init();

  return ACS_STATUS_PASS;
}

/**
  @brief   This API frees the memory assigned for gic info table
           1. Caller       -  Application Layer
           2. Prerequisite -  val_gic_create_info_table
  @param   None
  @return  None
**/
void
val_gic_free_info_table(void)
{
  pal_mem_free((void *)g_gic_info_table);
}

/**
  @brief   This API returns the base address of the GIC Distributor.
           The assumption is we have only 1 GIC Distributor. IS this true?
           1. Caller       -  VAL
           2. Prerequisite -  val_gic_create_info_table
  @param   None
  @return  Address of GIC Distributor
**/
addr_t
val_get_gicd_base(void)
{

  GIC_INFO_ENTRY  *gic_entry;

  if (g_gic_info_table == NULL) {
      val_print(ACS_PRINT_ERR, "GIC INFO table not available\n", 0);
      return 0;
  }

  gic_entry = g_gic_info_table->gic_info;

  while (gic_entry->type != 0xFF) {
    if (gic_entry->type == ENTRY_TYPE_GICD) {
        return gic_entry->base;
    }
    gic_entry++;
  }

  return 0;
}

/**
  @brief   This API returns the base address of the GIC Redistributor
           1. Caller       -  Test Suite
           2. Prerequisite -  val_gic_create_info_table
  @param   rdbase_len - To Store the Lenght of the Redistributor
  @param   gicr_rd_index - Used to obtain correct GICR RD base structure
                           address for cases when system has multiple GICR RD structure.
  @return  Address of GIC Redistributor
**/
addr_t
val_get_gicr_base(uint32_t *rdbase_len, uint32_t gicr_rd_index)
{
  uint32_t index = 0;
  GIC_INFO_ENTRY  *gic_entry;

  if (g_gic_info_table == NULL) {
      val_print(ACS_PRINT_ERR, "GIC INFO table not available\n", 0);
      return 0;
  }
  gic_entry = g_gic_info_table->gic_info;

  while (gic_entry->type != 0xFF) {
      if (gic_entry->type == ENTRY_TYPE_GICR_GICRD) {
          if (index == gicr_rd_index) {
              *rdbase_len = gic_entry->length;
              return gic_entry->base;
          }
          index++;
      }
      if (gic_entry->type == ENTRY_TYPE_GICC_GICRD) {
              *rdbase_len = 0;
              return gic_entry->base;
      }
      gic_entry++;
  }

  *rdbase_len = 0;
  return 0;
}

/**
  @brief   This API returns the base address of the GICH.
           1. Caller       -  VAL
           2. Prerequisite -  val_gic_create_info_table
  @param   None
  @return  Address of GICH
**/
addr_t
val_get_gich_base(void)
{

  GIC_INFO_ENTRY  *gic_entry;

  if (g_gic_info_table == NULL) {
      val_print(ACS_PRINT_ERR, "GIC INFO table not available\n", 0);
      return 0;
  }

  gic_entry = g_gic_info_table->gic_info;

  while (gic_entry->type != 0xFF) {
    if (gic_entry->type == ENTRY_TYPE_GICH) {
        return gic_entry->base;
    }
    gic_entry++;
  }

  return 0;
}
/**
  @brief   This API returns the base address of the CPU IF for the current PE
           1. Caller       -  Test Suite
           2. Prerequisite -  val_gic_create_info_table
  @param   None
  @return  Address of GIC Redistributor
**/
addr_t
val_get_cpuif_base(void)
{
  GIC_INFO_ENTRY  *gic_entry;

  if (g_gic_info_table == NULL) {
      val_print(ACS_PRINT_ERR, "GIC INFO table not available\n", 0);
      return 0;
  }

  gic_entry = g_gic_info_table->gic_info;

  if (gic_entry) {
      while (gic_entry->type != 0xFF) {
          if (gic_entry->type == ENTRY_TYPE_CPUIF)
              return gic_entry->base;
          gic_entry++;
      }
  }

  return 0;
}

/**
  @brief   This function is a single point of entry to retrieve
           all GIC related information.
           1. Caller       -  Test Suite
           2. Prerequisite -  val_gic_create_info_table
  @param   type   the type of information being requested
  @return  32-bit data
**/
uint32_t
val_gic_get_info(GIC_INFO_e type)
{
  uint32_t rdbase_len;

  if (g_gic_info_table == NULL) {
      val_print(ACS_PRINT_ERR, "\n   Get GIC info called before gic info table is filled ",        0);
      return 0;
  }

  switch (type) {

      case GIC_INFO_VERSION:
          if (g_gic_info_table->header.gic_version != 0) {
             val_print(ACS_PRINT_INFO, "\n       gic version from info table = %d ",
                       g_gic_info_table->header.gic_version);
             return g_gic_info_table->header.gic_version;
          }
          /* Read Version from GICD_PIDR2 bit [7:4] */
          return VAL_EXTRACT_BITS(val_mmio_read(val_get_gicd_base() + GICD_PIDR2), 4, 7);

      case GIC_INFO_SEC_STATES:
          /* Read DS Bit from GICD_CTLR bit[6] */
          return VAL_EXTRACT_BITS(val_mmio_read(val_get_gicd_base() + GICD_CTLR), 6, 6);

      case GIC_INFO_AFFINITY_NS:
          /* Read ARE_NS Bit from GICD_CTLR bit[5] */
          return VAL_EXTRACT_BITS(val_mmio_read(val_get_gicd_base() + GICD_CTLR), 4, 4);

      case GIC_INFO_ENABLE_GROUP1NS:
          /* Read EnableGrp1NS Bit from GICD_CTLR bit[2] */
          return VAL_EXTRACT_BITS(val_mmio_read(val_get_gicd_base() + GICD_CTLR), 0, 1);

      case GIC_INFO_SGI_NON_SECURE:
          /* The non-RAZ/WI bits from GICR_ISENABLER0 correspond to non-secure SGIs */
          return val_mmio_read(val_get_gicr_base(&rdbase_len, 0) + RD_FRAME_SIZE + GICR_ISENABLER);

      case GIC_INFO_SGI_NON_SECURE_LEGACY:
          /* The non-RAZ/WI bits from GICD_ISENABLER<n> correspond to non-secure SGIs */
          return val_mmio_read(val_get_gicd_base() + GICD_ISENABLER);

      case GIC_INFO_NUM_ITS:
          return g_gic_info_table->header.num_its;

      case GIC_INFO_NUM_MSI_FRAME:
          return g_gic_info_table->header.num_msi_frame;

      default:
          val_print(ACS_PRINT_ERR, "\n    GIC Info - TYPE not recognized %d  ", type);
          break;
  }
  return ACS_STATUS_ERR;
}

/**
  @brief   This API returns the max interrupt ID supported by the GIC Distributor
           1. Caller       -  VAL
           2. Prerequisite -  val_gic_create_info_table
  @param   None
  @return  Maximum Interrupt ID
**/
uint32_t
val_get_max_intid(void)
{
  return (32 * ((val_mmio_read(val_get_gicd_base() + GICD_TYPER) & 0x1F) + 1));
}

/**
  @brief   This function will initialize CPU interface registers required for interrupt
           routing to a given PE
           1. Caller       -  Test Suite
           2. Prerequisite -  val_gic_create_info_table
  @param   none
  @return  none
**/
void val_gic_cpuif_init(void)
{
  val_gic_reg_write(ICC_BPR1_EL1, 0x7);
  val_gic_reg_write(ICC_PMR_EL1, 0xff);
  val_gic_reg_write(ICC_IGRPEN1_EL1, 0x1);
}

/**
  @brief   This function will Get the trigger type Edge/Level
           1. Caller       -  Test Suite
           2. Prerequisite -  val_gic_create_info_table
  @param   int_id Interrupt ID
  @param   trigger_type to Store the Interrupt Trigger type
  @return  Status
**/
uint32_t val_gic_get_intr_trigger_type(uint32_t int_id, INTR_TRIGGER_INFO_TYPE_e *trigger_type)
{
  uint32_t reg_value;
  uint32_t reg_offset;
  uint32_t config_bit_shift;

  if (int_id > val_get_max_intid()) {
    val_print(ACS_PRINT_ERR, "\n       Invalid Interrupt ID number 0x%x ", int_id);
    return ACS_STATUS_ERR;
  }

  reg_offset = int_id / GICD_ICFGR_INTR_STRIDE;
  config_bit_shift  = GICD_ICFGR_INTR_CONFIG1(int_id);

  reg_value = val_mmio_read(val_get_gicd_base() + GICD_ICFGR + (4 * reg_offset));

  if ((reg_value & ((uint32_t)1 << config_bit_shift)) == 0)
    *trigger_type = INTR_TRIGGER_INFO_LEVEL_HIGH;
  else
    *trigger_type = INTR_TRIGGER_INFO_EDGE_RISING;

  return 0;
}

/**
  @brief   This API returns if extended SPI supported in system
  @param   None
  @return  0 not supported, 1 supported
**/
uint32_t
val_gic_espi_supported(void)
{
  uint32_t espi_support;

  espi_support = val_bsa_gic_espi_support();

  val_print(ACS_PRINT_INFO, "\n    ESPI supported %d  ", espi_support);
  return espi_support;
}

/**
  @brief   This API returns max extended SPI interrupt value
  @param   None
  @return  max extended spi int value
**/
uint32_t
val_gic_max_espi_val(void)
{
  uint32_t max_espi_val;

  max_espi_val = val_bsa_gic_max_espi_val();

  val_print(ACS_PRINT_INFO, "\n    max ESPI value %d  ", max_espi_val);
  return max_espi_val;
}

/**
  @brief  API used to check whether int_id is a espi interrupt
  @param  interrupt
  @return 1: espi interrupt, 0: non-espi interrupt
**/
uint32_t
val_gic_is_valid_espi(uint32_t int_id)
{
  return val_bsa_gic_check_espi_interrupt(int_id);
}

/**
  @brief  API used to check whether int_id is a Extended PPI
  @param  interrupt
  @return 1: eppi interrupt, 0: non-eppi interrupt
**/
uint32_t
val_gic_is_valid_eppi(uint32_t int_id)
{
  return val_bsa_gic_check_eppi_interrupt(int_id);
}
