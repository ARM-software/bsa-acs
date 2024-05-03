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
#include "common/include/acs_common.h"
#include "common/include/acs_std_smc.h"
#include "common/sys_arch_src/gic/acs_exception.h"
#include "common/include/val_interface.h"
#include "sbsa/include/sbsa_val_interface.h"

/**
  @brief   Pointer to the memory location of the PE Information table
**/
extern PE_INFO_TABLE *g_pe_info_table;

/**
  @brief   This API returns the index of the PE whose ACPI UID matches with the input UID
           1. Caller       -  Test Suite, VAL
           2. Prerequisite -  val_create_peinfo_table
  @param   mpid - the mpidr value of pE whose index is returned.
  @return  Index of PE
**/
uint32_t
val_pe_get_index_uid(uint32_t uid)
{

  PE_INFO_ENTRY *entry;
  uint32_t i = g_pe_info_table->header.num_of_pe;

  entry = g_pe_info_table->pe_info;

  while (i > 0) {
    if (entry->acpi_proc_uid == uid) {
      return entry->pe_num;
    }
    entry++;
    i--;
  }

  return 0x0;  //Return index 0 as a safe failsafe value
}

/**
  @brief   This API returns the ACPI UID of the PE whose MPIDR matches with the input MPIDR
           1. Caller       -  Test Suite, VAL
           2. Prerequisite -  val_create_pe_info_table
  @param   mpidr - the MPIDR value of PE whose UID is returned.
  @return  ACPI UID of the processor.
**/
uint32_t
val_pe_get_uid(uint64_t mpidr)
{

  PE_INFO_ENTRY *entry;
  uint32_t i = g_pe_info_table->header.num_of_pe;

  entry = g_pe_info_table->pe_info;

  while (i > 0) {
    if (entry->mpidr == mpidr) {
      return entry->acpi_proc_uid;
    }
    entry++;
    i--;
  }

  return INVALID_PE_INFO;
}


