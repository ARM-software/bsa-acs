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

/* This file contains APIs used by other modules/components */

#include "common/include/acs_val.h"
#include "common/include/acs_common.h"

static PCC_INFO_TABLE *g_pcc_info_table;

/* PCCT related APIs */

/**
  @brief   This API will call PAL layer to initialise PCC table information
           into the g_pcc_info_table pointer.
           1. Caller       -  Application layer.
           2. Prerequisite -  Memory allocated and passed as argument.
  @param   pcc_info_table  pre-allocated memory pointer for pcc info.
  @return  None
**/
void
val_pcc_create_info_table(uint64_t *pcc_info_table)
{
    /* store pointer to pcc info table */
    g_pcc_info_table = (PCC_INFO_TABLE *)pcc_info_table;

    pal_pcc_create_info_table(g_pcc_info_table);

    return;
}

/**
  @brief  This API return index to PCC info block in PCC info table for
          corresponding subspace id input.

  @param  subspace_idx  - Subspace id, used to index PCCT array.

  @return  index of the pcc info.
**/
uint32_t
val_pcc_get_ss_info_idx(uint32_t subspace_id)
{

  PCC_INFO *entry;
  uint32_t i;

  entry = g_pcc_info_table->pcc_info;

  for (i = 0; i < g_pcc_info_table->subspace_cnt; i++) {
      if (entry->subspace_idx == subspace_id) {
          return i;
      }
      entry++;
  }

  return RETURN_FAILURE;
}

/**
  @brief  This API implements ACPI Doorbell protocol.

  @param  subspace_idx  - Subspace id, used to index PCCT array.
  @param  command       - PCC command header
  @param  data          - pointer to data to be written to communication
                          subspace.
  @param  data_size     - size of data to be written to subspace

  @return pointer to communication subspace with response.
**/
void
*val_pcc_cmd_response(uint32_t subspace_id, uint32_t command, void *data, uint32_t data_size)
{

  uint32_t pcc_idx;
  uint32_t loop_cnt;
  uint32_t cmd_complete;
  uint64_t shared_mem_addr;
  uint64_t cmd_complete_upd_reg;
  uint64_t doorbell_val;
  PCC_SUBSPACE_TYPE_3 *ptr_to_pcc_ss_type_3;


  /* get pcc info block index */
  pcc_idx = val_pcc_get_ss_info_idx(subspace_id);

  /* return if failed to get index */
  if (pcc_idx == RETURN_FAILURE) {
      return NULL;
  }

  /* pointer to PCC info */
  ptr_to_pcc_ss_type_3 = &(g_pcc_info_table->pcc_info[pcc_idx].type_spec_info.pcc_ss_type_3);

  /* Note : For information on Doorbell Protocol refer ACPI 6.5 specification; section 14.5 */

  /* ensuring command complete check is set, indicating shared memory
     exclusively owned by OSPM */
  loop_cnt = 3;
  do {
      /* wait for minimum request turnaround time * 3 to provide time for platform */
      val_time_delay_ms(ptr_to_pcc_ss_type_3->min_req_turnaround_usec);
      /* read command complete check register */
      cmd_complete = val_mmio_read(ptr_to_pcc_ss_type_3->cmd_complete_chk_reg.addr) &
                                ptr_to_pcc_ss_type_3->cmd_complete_chk_mask;
      loop_cnt--;
  } while (cmd_complete == 0 || loop_cnt != 0);

  /* if platform not setting complete, return with failure */
  if (loop_cnt == 0) {
      val_print(ACS_PRINT_ERR,
                "\n    Platform fails to set command complete reg for PCC subspace id : 0x%x",
                subspace_id);
      return NULL;
  }

  /* write command and parameters to PCC shared memory region */
  shared_mem_addr = ptr_to_pcc_ss_type_3->base_addr;
  /* write command */
  val_mmio_write(shared_mem_addr + PCC_TY3_CMD_OFFSET, command);
  /* write parameters */
  val_memcpy((void *)(shared_mem_addr + PCC_TY3_COMM_SPACE), data, data_size);

  /* clear command complete indicating platform to process the command
     using command complete update register */
  cmd_complete_upd_reg = val_mmio_read(ptr_to_pcc_ss_type_3->cmd_complete_update_reg.addr);
  /* modify data as specified in doorbell protocol */
  cmd_complete_upd_reg =
                        (cmd_complete_upd_reg & ptr_to_pcc_ss_type_3->cmd_complete_update_preserve)
                        | ptr_to_pcc_ss_type_3->cmd_complete_update_set;
  /* write command complete update register to clear the complete bit */
  val_mmio_write(ptr_to_pcc_ss_type_3->cmd_complete_update_reg.addr, cmd_complete_upd_reg);

  /* ring doorbell by performing read/modify/write cycle */
  doorbell_val = val_mmio_read(ptr_to_pcc_ss_type_3->doorbell_reg.addr);
  doorbell_val = (doorbell_val & ptr_to_pcc_ss_type_3->doorbell_preserve)
                    | ptr_to_pcc_ss_type_3->doorbell_write;
  val_mmio_write(ptr_to_pcc_ss_type_3->doorbell_reg.addr, doorbell_val);

  /* wait for minimum request turnaround time and poll on the command complete bit for set */
  loop_cnt = 3;
  do {
      /* wait for minimum request turnaround time * 3 to provide time for platform */
      val_time_delay_ms(ptr_to_pcc_ss_type_3->min_req_turnaround_usec);
      /* read command complete check register */
      cmd_complete = val_mmio_read(ptr_to_pcc_ss_type_3->cmd_complete_chk_reg.addr) &
                                ptr_to_pcc_ss_type_3->cmd_complete_chk_mask;
      loop_cnt--;
  } while (cmd_complete == 0 || loop_cnt != 0);

  /* if platform not setting complete, return with failure */
  if (loop_cnt == 0) {
      val_print(ACS_PRINT_ERR,
          "\n    Platform fails to set command complete, post command for PCC subspace id : 0x%x",
          subspace_id);
      return NULL;
  }

  /* process response from platform */
  /* return pointer to communication subspace with response data */
  return (void *)(shared_mem_addr + PCC_TY3_COMM_SPACE);
}

/**
  @brief  Free the memory allocated for the pcc_info_table

  @param  None

  @return None
**/

void
val_pcc_free_info_table(void)
{
    if (g_pcc_info_table != NULL) {
        pal_mem_free_aligned((void *)g_pcc_info_table);
        g_pcc_info_table = NULL;
    }
    else {
      val_print(ACS_PRINT_ERR,
                  "\n WARNING: g_pcc_info_table pointer is already NULL",
        0);
    }
}
