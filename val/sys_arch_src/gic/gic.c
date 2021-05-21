/** @file
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
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
#include "include/bsa_acs_pe.h"
#include "gic_v3.h"
#include "gic_v2.h"

/**
  @brief  Initializes the GIC
  @param  none
  @return init success or failure
**/
void
val_bsa_gic_init(void)
{
  uint32_t gic_version;

  gic_version = val_gic_get_info(GIC_INFO_VERSION);
  if ((gic_version == 3) || (gic_version == 4))
      v3_Init();
  else
      v2_Init();
}

/**
  @brief  Enable Interrupt Source
  @param  interrupt id
  @return none
**/
void
val_bsa_gic_enableInterruptSource(uint32_t int_id)
{
  uint32_t gic_version;

  gic_version = val_gic_get_info(GIC_INFO_VERSION);
  if ((gic_version == 3) || (gic_version == 4))
      v3_EnableInterruptSource(int_id);
  else
      v2_EnableInterruptSource(int_id);
}

/**
  @brief  Disable Interrupt Source
  @param  interrupt id
  @return none
**/
void
val_bsa_gic_disableInterruptSource(uint32_t int_id)
{
  uint32_t gic_version;

  gic_version = val_gic_get_info(GIC_INFO_VERSION);
  if ((gic_version == 3) || (gic_version == 4))
      v3_DisableInterruptSource(int_id);
  else
      v2_DisableInterruptSource(int_id);
}

/**
  @brief  Acknowledges interrupt
  @param  none
  @return none
**/
uint32_t
val_bsa_gic_acknowledgeInterrupt(void)
{
  uint32_t gic_version;

  gic_version = val_gic_get_info(GIC_INFO_VERSION);
  if ((gic_version == 3) || (gic_version == 4))
      return v3_AcknowledgeInterrupt();
  else
      return v2_AcknowledgeInterrupt();
}

/**
  @brief  End of interrupt
  @param  interrupt
  @return none
**/
void
val_bsa_gic_endofInterrupt(uint32_t int_id)
{
  uint32_t gic_version;

  gic_version = val_gic_get_info(GIC_INFO_VERSION);
  if ((gic_version == 3) || (gic_version == 4))
      v3_EndofInterrupt(int_id);
  else
      v2_EndofInterrupt(int_id);
}
