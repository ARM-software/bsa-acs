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

#include "common/include/acs_val.h"
#include "common/include/acs_common.h"
#include "common/include/acs_iovirt.h"
#include "common/include/acs_smmu.h"

extern IOVIRT_INFO_TABLE *g_iovirt_info_table;

/**
  @brief   This API is a single point of entry to retrieve
           ITS information stored in the IoVirt Info table
           1. Prerequisite -  val_iovirt_create_info_table
  @param   type           : The type of information being requested
  @param   group_index    : ITS group index
  @param   param          : value to be passed based on use-case
  @param   return_value   : return data
  @return  Status
**/
int
val_iovirt_get_its_info(
  uint32_t type, uint32_t group_index, uint32_t param, uint32_t *return_value)
{
  uint32_t i = 0;
  uint32_t j = 0;
  uint32_t it = 0;
  IOVIRT_BLOCK *block;


  if (g_iovirt_info_table == NULL) {
    val_print(ACS_PRINT_ERR, "GET_ITS_INFO: iovirt info table is not created\n", 0);
    return ACS_STATUS_ERR;
  }
  if (!return_value) {
      val_print(ACS_PRINT_ERR, "GET_ITS_INFO: Return pointer is NULL\n", 0);
      return ACS_STATUS_ERR;
  }

  if (type == ITS_NUM_GROUPS) {
      *return_value = g_iovirt_info_table->num_its_groups;
      return ACS_STATUS_PASS;
   }

  /* Go through the table return the relevant field value for the ITS block */
  /* at the index position */
  block = &g_iovirt_info_table->blocks[0];

  for (i = 0; i < g_iovirt_info_table->num_blocks; i++, block = IOVIRT_NEXT_BLOCK(block))
  {
      block = ALIGN_MEMORY_ACCESS(block);
      if (block->type == IOVIRT_NODE_ITS_GROUP) {
          if (type == ITS_GET_GRP_INDEX_FOR_ID) {
              /* Return the ITS Group Index for ITS_ID = param */
              for (it = 0; it < block->data.its_count; it++) {
                if (block->data_map[0].id[it] == param) {
                  *return_value = j;
                  return ACS_STATUS_PASS;
                }
              }
              j++;
              continue;
          }
          if (group_index == j) {
              switch (type)
              {
                case ITS_GROUP_NUM_BLOCKS: /* Return Number of ITS blocks in this group */
                  *return_value = block->data.its_count;
                  return ACS_STATUS_PASS;
                case ITS_GET_ID_FOR_BLK_INDEX:
                  /* Get ITS Block ID for index = param in this current ITS Group */
                  for (it = 0; it < block->data.its_count; it++) {
                      if (it == param) {
                          *return_value = block->data_map[0].id[it];
                          return ACS_STATUS_PASS;
                      }
                  }
                  /* Return Error if block not found */
                  return ACS_INVALID_INDEX;
                case ITS_GET_BLK_INDEX_FOR_ID:
                  /* Get ITS Block index for ITS_ID = param in ITS Group = group_index */
                  for (it = 0; it < block->data.its_count; it++) {
                      if (block->data_map[0].id[it] == param) {
                        *return_value = it;
                        return ACS_STATUS_PASS;
                      }
                  }
                  /* ITS_ID not found in current group, return error */
                  return ACS_INVALID_INDEX;
                default:
                  val_print(ACS_PRINT_ERR, "This ITS info option not supported %d\n", type);
                  return ACS_STATUS_ERR;
              }
              break;
          }
          j++;
      }
  }

  val_print(ACS_PRINT_ERR, "GET_ITS_INFO: ITS Group not found %d\n", group_index);
  return ACS_INVALID_INDEX;
}

/**
  @brief Check if given SMMU node has unique context bank interrupt ids

  @param smmu_index smmu index in iovirt table

  @return 0 if test fail ; 1 if test pass
**/
uint32_t
val_iovirt_check_unique_ctx_intid(uint32_t smmu_index)
{
  uint64_t smmu_block = val_iovirt_get_smmu_info(SMMU_IOVIRT_BLOCK, smmu_index);
  return pal_iovirt_check_unique_ctx_intid(smmu_block);
}


