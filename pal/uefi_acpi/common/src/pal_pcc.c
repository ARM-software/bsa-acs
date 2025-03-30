
/** @file
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
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
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include "Include/IndustryStandard/Acpi65.h"

#include "common/include/pal_uefi.h"

#define ADD_PTR(t, p, l) ((t *)((UINT8 *)p + l))

static PCC_INFO_TABLE *g_pcc_info_table;

/**
  @brief  This API initialises the static global pointer to PCC
          info table.

  @param  PccInfoTable  - Address where the PCC information needs to be filled.

  @return  None
**/
VOID
pal_pcc_create_info_table(PCC_INFO_TABLE *PccInfoTable)
{

    /* store address to PCC info table in static global variable */
    g_pcc_info_table = PccInfoTable;

    /* initialise pcc info count */
    g_pcc_info_table->subspace_cnt = 0;

    /* this API doesn't parse PCC structure, pal_pcc_store_info API should be
       called by component (e.g, MPAM) which defines PCC shared memory region, to
       populate PCC info table  */

    return;
}

/**
  @brief  This API parses PCCT ACPI structures and stores info required by
          ACS in PCC info table.

  @param  subspace_idx  - Subspace id, used to index PCCT array.

  @return  None
**/
VOID
pal_pcc_store_info(UINT32 subspace_idx)
{
  EFI_ACPI_6_5_PLATFORM_COMMUNICATION_CHANNEL_TABLE_HEADER *pcct;
  EFI_ACPI_6_5_PCCT_SUBSPACE_GENERIC *pcct_subspace, *pcct_end;
  EFI_ACPI_6_5_PCCT_SUBSPACE_3_EXTENDED_PCC *pcct_type_3;
  PCC_SUBSPACE_TYPE_3 *ptr_to_pcc_ss_type_3;

  UINT32 index = 0;

  /* get pointer to PCCT ACPI table*/
  pcct = (EFI_ACPI_6_5_PLATFORM_COMMUNICATION_CHANNEL_TABLE_HEADER *)
          pal_get_acpi_table_ptr(EFI_ACPI_6_5_PLATFORM_COMMUNICATIONS_CHANNEL_TABLE_SIGNATURE);

  /* pointer to start of PCC subspace structure entries */
  pcct_subspace = ADD_PTR(EFI_ACPI_6_5_PCCT_SUBSPACE_GENERIC, pcct,
                          sizeof(EFI_ACPI_6_5_PLATFORM_COMMUNICATION_CHANNEL_TABLE_HEADER));
  pcct_end =  ADD_PTR(EFI_ACPI_6_5_PCCT_SUBSPACE_GENERIC, pcct,
                          pcct->Header.Length);
  while (pcct_subspace < pcct_end) {
    if (index == subspace_idx) {
        /* this API only supports parsing of type 3 PCC structure info */
        if (pcct_subspace->Type != EFI_ACPI_6_5_PCCT_SUBSPACE_TYPE_3_EXTENDED_PCC) {
            acs_print(ACS_PRINT_ERR,
                      L"\n    pal_pcc_store_info API doesn't support PCC structure type : 0x%x",
                      pcct_subspace->Type);
        }

        /* parse PCC structure type 3 */
        pcct_type_3 = (EFI_ACPI_6_5_PCCT_SUBSPACE_3_EXTENDED_PCC *)pcct_subspace;
        g_pcc_info_table->pcc_info[g_pcc_info_table->subspace_cnt].subspace_idx  = subspace_idx;
        g_pcc_info_table->pcc_info[g_pcc_info_table->subspace_cnt].subspace_type
                                                               = pcct_type_3->Type;
        ptr_to_pcc_ss_type_3 =
        &(g_pcc_info_table->pcc_info[g_pcc_info_table->subspace_cnt].type_spec_info.pcc_ss_type_3);
        ptr_to_pcc_ss_type_3->doorbell_reg
                    = pcct_type_3->DoorbellRegister;
        ptr_to_pcc_ss_type_3->cmd_complete_chk_reg
                    = pcct_type_3->CommandCompleteCheckRegister;
        ptr_to_pcc_ss_type_3->cmd_complete_update_reg
                    = pcct_type_3->CommandCompleteUpdateRegister;
        ptr_to_pcc_ss_type_3->cmd_complete_update_preserve
                                            =  pcct_type_3->CommandCompleteUpdatePreserve;
        ptr_to_pcc_ss_type_3->min_req_turnaround_usec
                                            =  pcct_type_3->MinimumRequestTurnaroundTime;
        ptr_to_pcc_ss_type_3->base_addr                 =  pcct_type_3->BaseAddress;
        ptr_to_pcc_ss_type_3->doorbell_preserve         =  pcct_type_3->DoorbellPreserve;
        ptr_to_pcc_ss_type_3->doorbell_write            =  pcct_type_3->DoorbellWrite;
        ptr_to_pcc_ss_type_3->cmd_complete_chk_mask     =  pcct_type_3->CommandCompleteCheckMask;
        ptr_to_pcc_ss_type_3->cmd_complete_update_set   =  pcct_type_3->CommandCompleteUpdateSet;
        g_pcc_info_table->subspace_cnt++;

        break;
    }
    /* point to next PCC subspace entry */
    pcct_subspace = ADD_PTR(EFI_ACPI_6_5_PCCT_SUBSPACE_GENERIC, pcct_subspace,
                          pcct_subspace->Length);
    index++;
  }

}

