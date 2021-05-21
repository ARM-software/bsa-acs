/** @file
 * Copyright (c) 2016-2018,2020-2021 Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
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

#ifndef __BSA_ACS_WD_H
#define __BSA_ACS_WD_H

#define  WD_IIDR_OFFSET          0xFCC
#define  WD_CS_OFFSET            0x0
#define  WD_OR_OFFSET            0x8
#define  WD_OR_UPPER_WORD_OFFSET 0x0C

#define  WD_CSR_RSRV_SHIFT       3
#define  WD_OR_RSRV_SHIFT        16

uint32_t
os_w001_entry(uint32_t num_pe);
uint32_t
os_w002_entry(uint32_t num_pe);

#endif // __BSA_ACS_WD_H
