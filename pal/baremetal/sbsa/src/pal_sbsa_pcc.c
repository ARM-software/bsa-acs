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

#include "pal_common_support.h"
#include "pal_sbsa_common_support.h"
#include "pal_pcie_enum.h"
#include "platform_override_struct.h"
#include "platform_override_sbsa_struct.h"

extern PLATFORM_OVERRIDE_PCC_INFO_TABLE platform_pcc_cfg;

/**
  @brief  This API prints cache info table and cache entry indices for each pe.
  @param  CacheTable Pointer to cache info table.
  @param  PeTable Pointer to pe info table.
  @return None
**/
void
pal_pcc_dump_info_table(PCC_INFO_TABLE *PccInfoTable)
{
  PCC_INFO *curr_entry;
  PCC_SUBSPACE_TYPE_3 *ptr_pcc_ss_type_3;
  uint32_t i;

  if (PccInfoTable == NULL) {
      print(ACS_PRINT_ERR, "\nUnable to dump PCC info table, input pointer is NULL\n");
  }

  print(ACS_PRINT_INFO, "\n*** PCC Information ***");
  print(ACS_PRINT_INFO, "\nNumber of PCC subspace entries : %d", PccInfoTable->subspace_cnt);

  curr_entry = PccInfoTable->pcc_info;

  for (i = 0; i < PccInfoTable->subspace_cnt; i++) {
      print(ACS_PRINT_INFO, "\n PCC subspace index                : 0x%x",
                  curr_entry->subspace_idx);
      print(ACS_PRINT_INFO, "\n PCC subspace type                 : 0x%x",
                  curr_entry->subspace_type);

      if (curr_entry->subspace_type == PCCT_SUBSPACE_TYPE_3_EXTENDED_PCC) {
          ptr_pcc_ss_type_3 = &(curr_entry->type_spec_info.pcc_ss_type_3);
          print(ACS_PRINT_INFO, "\n Base address                      : 0x%lx",
                      ptr_pcc_ss_type_3->base_addr);
          print(ACS_PRINT_INFO, "\n Doorbell Register addr            : 0x%lx",
                      ptr_pcc_ss_type_3->doorbell_reg.addr);
          print(ACS_PRINT_INFO, "\n Doorbell preserve Mask            : 0x%lx",
                      ptr_pcc_ss_type_3->doorbell_preserve);
          print(ACS_PRINT_INFO, "\n Doorbell write Mask               : 0x%lx",
                      ptr_pcc_ss_type_3->doorbell_write);
          print(ACS_PRINT_INFO, "\n Min req turnaround time (us)      : 0x%x",
                      ptr_pcc_ss_type_3->min_req_turnaround_usec);
          print(ACS_PRINT_INFO, "\n Command complete check reg addr   : 0x%lx",
                      ptr_pcc_ss_type_3->cmd_complete_chk_reg.addr);
          print(ACS_PRINT_INFO, "\n Command complete check mask       : 0x%lx",
                      ptr_pcc_ss_type_3->cmd_complete_chk_mask);
          print(ACS_PRINT_INFO, "\n Command complete update reg addr  : 0x%lx",
                      ptr_pcc_ss_type_3->cmd_complete_update_reg.addr);
          print(ACS_PRINT_INFO, "\n Command complete update preserve  : 0x%lx",
                      ptr_pcc_ss_type_3->cmd_complete_update_preserve);
          print(ACS_PRINT_INFO, "\n Command complete update set mask  : 0x%lx",
                      ptr_pcc_ss_type_3->cmd_complete_update_set);
      }
  }
}

/**
  @brief  Parses ACPI PCCT info and populates the local PCC info table.

  @param  PccInfoTable Pointer to pre-allocated memory for PCC info table.

  @return None
**/
void
pal_pcc_create_info_table(PCC_INFO_TABLE *PccInfoTable)
{
  uint32_t i;

  if (PccInfoTable == NULL) {
    print(ACS_PRINT_ERR, " Unable to create PCC info table, input pointer is NULL\n");
    return;
  }

  PCC_INFO *curr_entry;

  curr_entry = PccInfoTable->pcc_info;

  /* initialize pcc info table entries */
  PccInfoTable->subspace_cnt =  platform_pcc_cfg.subspace_cnt;

  for (i = 0; i < PccInfoTable->subspace_cnt; i++)
  {
    curr_entry->subspace_idx = platform_pcc_cfg.pcc_info[i].subspace_idx;
    curr_entry->subspace_type = platform_pcc_cfg.pcc_info[i].subspace_type;

    if (curr_entry->subspace_type == PCCT_SUBSPACE_TYPE_3_EXTENDED_PCC) {
        curr_entry->type_spec_info.pcc_ss_type_3.base_addr
          = platform_pcc_cfg.pcc_info[i].type_spec_info.pcc_ss_type_3.base_addr;
        curr_entry->type_spec_info.pcc_ss_type_3.cmd_complete_chk_mask
          = platform_pcc_cfg.pcc_info[i].type_spec_info.pcc_ss_type_3.cmd_complete_chk_mask;
        curr_entry->type_spec_info.pcc_ss_type_3.cmd_complete_chk_reg
          = platform_pcc_cfg.pcc_info[i].type_spec_info.pcc_ss_type_3.cmd_complete_chk_reg;
        curr_entry->type_spec_info.pcc_ss_type_3.cmd_complete_update_preserve
          = platform_pcc_cfg.pcc_info[i].type_spec_info.pcc_ss_type_3.cmd_complete_update_preserve;
        curr_entry->type_spec_info.pcc_ss_type_3.cmd_complete_update_reg
          = platform_pcc_cfg.pcc_info[i].type_spec_info.pcc_ss_type_3.cmd_complete_update_reg;
        curr_entry->type_spec_info.pcc_ss_type_3.cmd_complete_update_set
          = platform_pcc_cfg.pcc_info[i].type_spec_info.pcc_ss_type_3.cmd_complete_update_set;
        curr_entry->type_spec_info.pcc_ss_type_3.doorbell_preserve
          = platform_pcc_cfg.pcc_info[i].type_spec_info.pcc_ss_type_3.doorbell_preserve;
        curr_entry->type_spec_info.pcc_ss_type_3.doorbell_reg
          = platform_pcc_cfg.pcc_info[i].type_spec_info.pcc_ss_type_3.doorbell_reg;
        curr_entry->type_spec_info.pcc_ss_type_3.doorbell_write
          = platform_pcc_cfg.pcc_info[i].type_spec_info.pcc_ss_type_3.doorbell_write;
        curr_entry->type_spec_info.pcc_ss_type_3.min_req_turnaround_usec
          = platform_pcc_cfg.pcc_info[i].type_spec_info.pcc_ss_type_3.min_req_turnaround_usec;
    }
    curr_entry++;
   }
  pal_pcc_dump_info_table(PccInfoTable);
  return;
}
