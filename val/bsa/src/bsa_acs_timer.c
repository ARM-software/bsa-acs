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
#include "common/include/acs_pe.h"
#include "common/include/acs_timer_support.h"
#include "common/include/acs_timer.h"
#include "common/include/acs_common.h"

/**
  @brief   This API to get the el1 phy timer count.
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   None

  @return  Current timer count
**/
uint64_t
val_get_phy_el1_timer_count(void)
{
  return  ArmArchTimerReadReg(CntpTval);
}

/**
  @brief   This API programs the el2 phy timer with the input timeout value.
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   timeout - clock ticks after which an interrupt is generated.

  @return  None
**/
void
val_timer_set_phy_el2(uint64_t timeout)
{

  if (timeout != 0) {
    ArmGenericTimerDisableTimer(CnthpCtl);
    ArmArchTimerWriteReg(CnthpTval, &timeout);
    ArmGenericTimerEnableTimer(CnthpCtl);
  } else {
    ArmGenericTimerDisableTimer(CnthpCtl);
 }
}

/**
  @brief   This API programs the el2 Virt timer with the input timeout value.
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   timeout - clock ticks after which an interrupt is generated.

  @return  None
**/
void
val_timer_set_vir_el2(uint64_t timeout)
{

  if (timeout != 0) {
    ArmGenericTimerDisableTimer(CnthvCtl);
    ArmArchTimerWriteReg(CnthvTval, &timeout);
    ArmGenericTimerEnableTimer(CnthvCtl);
  } else {
    ArmGenericTimerDisableTimer(CnthvCtl);
 }

}

/**
  @brief  This API will program and start the counter

  @param  cnt_base_n  Counter base address
  @param  timeout     Timeout value

  @return None
**/
void
val_timer_set_system_timer(addr_t cnt_base_n, uint32_t timeout)
{
  /* Start the System timer */
  val_mmio_write(cnt_base_n + CNTP_TVAL, timeout);

  /* enable System timer */
  val_mmio_write(cnt_base_n + CNTP_CTL, 1);

}

/**
  @brief  This API will stop the counter

  @param  cnt_base_n  Counter base address

  @return None
**/
void
val_timer_disable_system_timer(addr_t cnt_base_n)
{

  /* stop System timer */
  val_mmio_write(cnt_base_n + CNTP_CTL, 0);
}

/**
  @brief  This API will read CNTACR (from CNTCTLBase) to determine whether
          access permission from NS state is permitted

  @param  index  index of SYS counter in timer table

  @return Status 0 if success
**/
uint32_t
val_timer_skip_if_cntbase_access_not_allowed(uint64_t index)
{
  uint64_t cnt_ctl_base;
  uint32_t data;
  uint32_t frame_num = 0;

  cnt_ctl_base = val_timer_get_info(TIMER_INFO_SYS_CNTL_BASE, index);
  frame_num = val_timer_get_info(TIMER_INFO_FRAME_NUM, index);

  if (cnt_ctl_base) {
      data = val_mmio_read(cnt_ctl_base + CNTACR + frame_num * 4);
      if ((data & 0x1) == 0x1)
          return 0;
      else {
          data |= 0x1;
          val_mmio_write(cnt_ctl_base + CNTACR + frame_num * 4, data);
          data = val_mmio_read(cnt_ctl_base + CNTACR + frame_num * 4);
          if ((data & 0x1) == 1)
              return 0;
          else
              return ACS_STATUS_SKIP;
      }
  }
  else
      return ACS_STATUS_SKIP;

}
