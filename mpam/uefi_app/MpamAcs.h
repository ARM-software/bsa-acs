/** @file
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __MPAM_ACS_LEVEL_H__
#define __MPAM_ACS_LEVEL_H__


#define MPAM_ACS_MAJOR_VER      0
#define MPAM_ACS_MINOR_VER      5
#define MPAM_ACS_SUBMINOR_VER   0

#define G_PRINT_LEVEL       ACS_PRINT_TEST

#define PE_INFO_TBL_SZ         16384  /*Supports maximum 400 PEs*/
                                      /*[40 B Each + 4 B Header] */
#define GIC_INFO_TBL_SZ        240000 /* Supports maximum 832 gic info */
                                      /* (GICH, CPUIF, RD, ITS, MSI, D) */
                                      /*[48 B Each + 24 B Header]*/
#define SRAT_INFO_TBL_SZ       16384  /*Support maximum of 500 mem proximity domain entries*/
                                      /*[32 B Each + 8 B Header]*/
#define CACHE_INFO_TBL_SZ      262144 /*Support maximum of 7280 cache entries*/
                                      /*[36 B Each + 4 B Header]*/
#define MPAM_INFO_TBL_SZ       262144 /*Supports maximum of 1800 MSC entries*/
                                      /*[24+(24*5) B Each + 4 B Header]*/
#define HMAT_INFO_TBL_SZ       12288  /*Supports maximum of 400 Proximity domains*/
                                      /*[24 B Each + 8 B Header]*/
#define PCC_INFO_TBL_SZ        262144 /*Supports maximum of 234 PCC info entries*/
                                      /*[112 B Each + 4B Header]*/
#endif
