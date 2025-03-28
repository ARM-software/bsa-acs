/** @file
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
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

extern GIC_INFO_TABLE  *g_gic_info_table;

/**
  @brief   This function will return '1' if an interrupt is either pending or active.
           1. Caller       -  Test Suite
           2. Prerequisite -  val_gic_create_info_table
  @param   int_id Interrupt ID
  @return  pending/active status
**/
uint32_t val_gic_get_interrupt_state(uint32_t int_id)
{
  uint32_t reg_offset = int_id / 32;
  uint32_t reg_shift  = int_id % 32;
  uint32_t mask = ((uint32_t)1 << reg_shift);
  uint32_t active, pending;

  pending = val_mmio_read(val_get_gicd_base() + GICD_ISPENDR + (4 * reg_offset));
  active = val_mmio_read(val_get_gicd_base() + GICD_ISACTIVER0 + (4 * reg_offset));

  return ((mask & active) || (mask & pending));
}

/**
  @brief   This function will clear an interrupt that is pending or active.
           1. Caller       -  Test Suite
           2. Prerequisite -  val_gic_create_info_table
  @param   int_id Interrupt ID
  @return  none
**/
void val_gic_clear_interrupt(uint32_t int_id)
{
    uint32_t reg_offset = int_id / 32;
    uint32_t reg_shift  = int_id % 32;

    if (val_gic_is_valid_espi(int_id))
      val_gic_clear_espi_interrupt(int_id);
    else if ((int_id > 31) && (int_id < 1020)) {
        val_mmio_write(val_get_gicd_base() + GICD_ICPENDR0 + (4 * reg_offset),
                        ((uint32_t)1 << reg_shift));
        val_mmio_write(val_get_gicd_base() + GICD_ICACTIVER0 + (4 * reg_offset),
                       ((uint32_t)1 << reg_shift));
    }
    else
        val_print(ACS_PRINT_ERR, "\n    Invalid SPI interrupt ID number %d", int_id);
}

/**
  @brief   This function will Get the trigger type Edge/Level for extended SPI int
           1. Caller       -  Test Suite
           2. Prerequisite -  val_gic_create_info_table
  @param   int_id Interrupt ID
  @param   trigger_type to Store the Interrupt Trigger type
  @return  Status
**/
uint32_t val_gic_get_espi_intr_trigger_type(uint32_t int_id,
                                                           INTR_TRIGGER_INFO_TYPE_e *trigger_type)
{
  uint32_t reg_value;
  uint32_t reg_offset;
  uint32_t config_bit_shift;

  if (!(int_id >= 4096 && int_id <= val_gic_max_espi_val())) {
    val_print(ACS_PRINT_ERR, "\n       Invalid Extended Int ID number 0x%x ", int_id);
    return ACS_STATUS_ERR;
  }

  /* 4096 is starting value of extended SPI int */
  reg_offset = (int_id - 4096) / GICD_ICFGR_INTR_STRIDE;
  config_bit_shift  = GICD_ICFGR_INTR_CONFIG1(int_id - 4096);

  reg_value = val_mmio_read(val_get_gicd_base() + GICD_ICFGRE + (4 * reg_offset));

  if ((reg_value & ((uint32_t)1 << config_bit_shift)) == 0)
    *trigger_type = INTR_TRIGGER_INFO_LEVEL_HIGH;
  else
    *trigger_type = INTR_TRIGGER_INFO_EDGE_RISING;

  return 0;
}

/**
  @brief   This function will Set the trigger type Edge/Level based on the GTDT table
           1. Caller       -  Test Suite
           2. Prerequisite -  val_gic_create_info_table
  @param   int_id Interrupt ID
  @param   trigger_type Interrupt Trigger Type
  @return  none
**/
void val_gic_set_intr_trigger(uint32_t int_id, INTR_TRIGGER_INFO_TYPE_e trigger_type)
{
   uint32_t reg_value;
   uint32_t reg_offset;
   uint32_t config_bit_shift;

   val_print(ACS_PRINT_DEBUG, "\n       Setting Trigger type as %d", trigger_type);

   reg_offset = int_id / GICD_ICFGR_INTR_STRIDE;
   config_bit_shift  = GICD_ICFGR_INTR_CONFIG1(int_id);

   reg_value = val_mmio_read(val_get_gicd_base() + GICD_ICFGR + (4 * reg_offset));

   if (trigger_type == INTR_TRIGGER_INFO_EDGE_RISING)
       reg_value = reg_value | ((uint32_t)1 << config_bit_shift);
   else
       reg_value = reg_value & (~((uint32_t)1 << config_bit_shift));

   val_print(ACS_PRINT_INFO, "\n       Writing to Register Address : 0x%llx ",
                           val_get_gicd_base() + GICD_ICFGR + (4 * reg_offset));

   val_mmio_write(val_get_gicd_base() + GICD_ICFGR + (4 * reg_offset), reg_value);
}
