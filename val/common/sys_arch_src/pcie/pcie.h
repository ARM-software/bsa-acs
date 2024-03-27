/** @file
 * Copyright (c) 2021,2024, Arm Limited or its affiliates. All rights reserved.
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
#ifndef __PCIE_H__
#define __PCIE_H__

#define PCIE_CFG_SIZE  4096

/* Header Offset and Type*/
#define HEADER_OFFSET      0xC
#define TYPE0_HEADER       0
#define TYPE1_HEADER       1

#define TYPE01_RIDR        0x8

#define PCIE_HEADER_TYPE(header_value) ((header_value >> 16) & 0x3)
#define BUS_NUM_REG_CFG(sub_bus, sec_bus, pri_bus) (sub_bus << 16 | sec_bus << 8 | bus)


/*Initial BUS definitions*/
#define PRI_BUS            0
#define SEC_BUS            1
#define BUS_NUM_REG_OFFSET 0x18

void val_bsa_pcie_enumerate(void);

#endif /*__PCIE_H__ */
