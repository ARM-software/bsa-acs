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
#include "common/include/acs_timer_support.h"
#include "common/include/acs_wd.h"
#include "common/include/acs_common.h"
#include "bsa/include/bsa_val_interface.h"
#include "bsa/include/bsa_pal_interface.h"

extern WD_INFO_TABLE  *g_wd_info_table;

/**
  @brief   This API Enables watchdog by writing to Control Base register
  @param   index   - identifies the watchdog instance to enable
  @return  None
 **/
static void
val_wd_enable(uint32_t index)
{

  val_mmio_write((g_wd_info_table->wd_info[index].wd_ctrl_base + WD_CS_OFFSET), 1);

}

/**
  @brief   This API disables watchdog by writing to Control Base register
  @param   index   - identifies the watchdog instance to enable
  @return  None
 **/
static void
val_wd_disable(uint32_t index)
{

  val_mmio_write((g_wd_info_table->wd_info[index].wd_ctrl_base + WD_CS_OFFSET), 0);

}

/**
  @brief   This API arms the watchdog by writing to Control Base register
  @param   index   - identifies the watchdog instance to program
  @param   timeout - ticks to generation of ws0 interrupt
  @return  Success/Failure
 **/
uint32_t
val_wd_set_ws0(uint32_t index, uint32_t timeout)
{
  uint64_t counter_freq;
  uint32_t wor_l;
  uint32_t wor_h = 0;
  uint64_t ctrl_base;
  uint32_t data;

  if (timeout == 0) {
      val_wd_disable(index);
      return 0;
  }

  ctrl_base    = val_wd_get_info(index, WD_INFO_CTRL_BASE);

  /* W_IIDR.Architecture Revision [19:16] = 0x1 for Watchdog Rev 1 */
  data = VAL_EXTRACT_BITS(val_mmio_read(ctrl_base + WD_IIDR_OFFSET), 16, 19);

  counter_freq = val_get_counter_frequency();

  /* Check if the timeout value exceeds */
  if (data == 0)
  {
      if ((counter_freq * timeout) >> 32)
      {
          val_print(ACS_PRINT_ERR, "\nCounter frequency value exceeded", 0);
          return 1;
      }
  }

  wor_l = (uint32_t)(counter_freq * timeout);
  wor_h = (uint32_t)((counter_freq * timeout) >> 32);

  val_mmio_write((g_wd_info_table->wd_info[index].wd_ctrl_base + 8), wor_l);

  /* Upper bits are applicable only for WDog Version 1 */
  if (data == 1)
      val_mmio_write((g_wd_info_table->wd_info[index].wd_ctrl_base + 12), wor_h);

  val_wd_enable(index);

  return 0;

}

/**
  @brief   This API is to get counter frequency
  @param   None
  @return  counter frequency
 **/
uint64_t
val_get_counter_frequency(void)
{
  uint64_t counter_freq;

  /* Option to override system counter frequency value */
  counter_freq = pal_timer_get_counter_frequency();
  if (counter_freq == 0)
      counter_freq = val_timer_get_info(TIMER_INFO_CNTFREQ, 0);

  return counter_freq;
}
