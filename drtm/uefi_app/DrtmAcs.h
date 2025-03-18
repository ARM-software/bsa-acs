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

#ifndef __DRTM_ACS_LEVEL_H__
#define __DRTM_ACS_LEVEL_H__

#define DRTM_ACS_MAJOR_VER      0
#define DRTM_ACS_MINOR_VER      6

#ifdef _AARCH64_BUILD_
unsigned long __stack_chk_guard = 0xBAAAAAAD;
unsigned long __stack_chk_fail =  0xBAAFAAAD;
#endif

/* Please MAKE SURE all the table sizes are 16 Bytes aligned */
#define PE_INFO_TBL_SZ         16384  /*Supports max 400 PEs     */
                                      /*[24 B Each + 4 B Header] */
#define GIC_INFO_TBL_SZ        240000 /*Supports max 832 GIC info (GICH,CPUIF,RD,ITS,MSI,D)*/
                                      /*[48 B Each + 32 B Header]*/

#endif
