/** @file
 * Copyright (c) 2016-2023, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __BSA_ACS_LEVEL_H__
#define __BSA_ACS_LEVEL_H__



  #define BSA_ACS_MAJOR_VER      1
  #define BSA_ACS_MINOR_VER      0
  #define BSA_ACS_SUBMINOR_VER   7

  #define G_PRINT_LEVEL ACS_PRINT_TEST

  #define G_SW_OS            0
  #define G_SW_HYP           1
  #define G_SW_PS            2

  /* Note : Total Size Required for Info tables ~ 550 KB
   * Table size is required to be updated whenever new members
   * are added in the info table structures
   */

  /* Please MAKE SURE all the table sizes are 16 Bytes aligned */
  #define PE_INFO_TBL_SZ         16384  /*Supports max 400 PEs     */
                                        /*[24 B Each + 4 B Header] */
  #define GIC_INFO_TBL_SZ        240000 /*Supports max 832 GIC info (GICH,CPUIF,RD,ITS,MSI,D)*/
                                        /*[48 B Each + 32 B Header]*/
  #define TIMER_INFO_TBL_SZ      2048   /*Supports max 4 system timers*/
                                        /*[248 B Each + 56 B Header]  */
  #define WD_INFO_TBL_SZ         512    /*Supports max 20 Watchdogs*/
                                        /*[24 B Each + 4 B Header] */
  #define MEM_INFO_TBL_SZ        32768  /*Supports max 800 memory regions*/
                                        /*[40 B Each + 16 B Header]      */
  #define IOVIRT_INFO_TBL_SZ     1048576/*Supports max 2400 nodes of a typical iort table*/
                                        /*[(268+32*5) B Each + 24 B Header]*/
  #define PERIPHERAL_INFO_TBL_SZ 2048   /*Supports max 20 PCIe EPs (USB and SATA controllers)*/
                                        /*[72 B Each + 16 B Header]*/
  #define PCIE_INFO_TBL_SZ       512    /*Supports max 20 RC's    */
                                        /*[24 B Each + 4 B Header]*/

  #ifdef _AARCH64_BUILD_
  unsigned long __stack_chk_guard = 0xBAAAAAAD;
  unsigned long __stack_chk_fail =  0xBAAFAAAD;
  #endif

#endif
