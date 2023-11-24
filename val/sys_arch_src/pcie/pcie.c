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
#include "include/bsa_acs_pcie.h"
#include "pcie.h"
/**
  @brief   This API performs the PCIe bus enumeration
  @param   bus,sec_bus - Bus(8-bits), secondary bus (8-bits)

  @return  sub_bus - Subordinate bus
**/
static
uint32_t pcie_enumerate_device(uint32_t bus, uint32_t sec_bus)
{

  uint32_t vendor_id;
  uint32_t header_value;
  uint32_t sub_bus = bus;
  uint32_t dev;
  uint32_t func;
  uint32_t bdf;

  if (bus == (PCIE_MAX_BUS-1))
      return sub_bus;

  for (dev = 0; dev < PCIE_MAX_DEV; dev++)
  {
    for (func = 0; func < PCIE_MAX_FUNC; func++)
    {
        bdf = PCIE_CREATE_BDF(0, bus, dev, func);
        val_pcie_read_cfg(bdf, 0, &vendor_id);
        if ((vendor_id == 0x0) || (vendor_id == 0xFFFFFFFF))
                continue;

        val_pcie_read_cfg(bdf, HEADER_OFFSET, &header_value);
        if (PCIE_HEADER_TYPE(header_value) == TYPE1_HEADER)
        {
            val_print(ACS_PRINT_INFO, " TYPE1 HEADER found\n", 0);
            val_pcie_write_cfg(bdf, BUS_NUM_REG_OFFSET, BUS_NUM_REG_CFG(0xFF, sec_bus, bus));
            sub_bus = pcie_enumerate_device(sec_bus, (sec_bus+1));
            val_pcie_write_cfg(bdf, BUS_NUM_REG_OFFSET, BUS_NUM_REG_CFG(sub_bus, sec_bus, bus));
            sec_bus = sub_bus + 1;
        }

        if (PCIE_HEADER_TYPE(header_value) == TYPE0_HEADER)
        {
            val_print(ACS_PRINT_INFO, " END POINT found\n", 0);
            sub_bus = sec_bus - 1;
        }

      }
    }
    return sub_bus;
}

/**
  @brief  Does PCIE enumeration and programs the Primary/Sub/Sec bus
          NOTE: This was required as in U boot seen the Secondary Bus of RP which has switch
          below it not getting programmed correctly
  @param  none
  @return none
**/
void val_bsa_pcie_enumerate(void)
{
  val_print(ACS_PRINT_INFO, "\n Starting Enumeration\n", 0);
  pcie_enumerate_device(PRI_BUS, SEC_BUS);
}
