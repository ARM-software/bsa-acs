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
#include "sbsa/include/sbsa_val_interface.h"
#include "sbsa/include/sbsa_acs_iovirt.h"

extern IOVIRT_INFO_TABLE *g_iovirt_info_table;

#if defined(TARGET_LINUX) || defined(TARGET_EMULATION)
/**
  @brief   This API will call PAL layer to fill in the path of the hid passed in the
           hid parameter.
           1. Caller       -  Application layer.
  @param   hid      hardware ID of the device to which the path is filled
  @param   hid_path 2D array in which the path is stored
  @return  Error if not able to find the path of given hid
**/
uint32_t
val_get_device_path(const char *hid, char hid_path[][MAX_NAMED_COMP_LENGTH])
{
  uint32_t status = 0;

  status = pal_get_device_path(hid, hid_path);

  return status;
}

/**
  @brief   This API will call PAL layer to check if etr is behind the catu.
           1. Caller       -  Application layer.
  @param   etr_path  path of ETR
  @return  Error if CATU is not behind ETR device
**/

uint32_t
val_smmu_is_etr_behind_catu(char *etr_path)
{
  uint32_t status = 0;

  status = pal_smmu_is_etr_behind_catu(etr_path);

  return status;
}
#endif

/**
  @brief   This API is a single point of entry to retrieve
           Named component info stored in the iovirt info table.
           1. Caller       -  Test suite
           2. Prerequisite -  val_iovirt_create_info_table
  @param   type   the type of information being requested.
  @param   index  the index of named component info instance.
  @return  64-bit data
**/
uint64_t
val_iovirt_get_named_comp_info(NAMED_COMP_INFO_e type, uint32_t index)
{
  uint32_t i, j = 0;
  IOVIRT_BLOCK *block;

  if (g_iovirt_info_table == NULL)
  {
      val_print(ACS_PRINT_ERR, "GET_NAMED_COMP_INFO: iovirt info table is not created\n", 0);
      return 0; /* imply no named components parsed */
  }

  if (type == NUM_NAMED_COMP)
      return g_iovirt_info_table->num_named_components;

  /* check if index in range */
  if (index > g_iovirt_info_table->num_named_components - 1) {
      val_print(ACS_PRINT_ERR,
                "GET_NAMED_COMP_INFO: Index (%d) is greater than num of Named components\n",
                 index);
      return INVALID_NAMED_COMP_INFO;
  }

  /* Go through the table return the relevant field value for the Named component block */
  /* at the index position */
  block = &g_iovirt_info_table->blocks[0];
  for (i = 0; i < g_iovirt_info_table->num_blocks; i++, block = IOVIRT_NEXT_BLOCK(block))
  {
      block = ALIGN_MEMORY_ACCESS(block);
      if (block->type == IOVIRT_NODE_NAMED_COMPONENT)
      {
          if (j == index)
          {
              switch (type)
              {
                  case NAMED_COMP_CCA_ATTR:
                      return block->data.named_comp.cca;
                  case NAMED_COMP_DEV_OBJ_NAME:
                      /* caller needs to typecast data to (char *) to retrieve full path
                        to named component in namespace */
                      return (uint64_t)block->data.named_comp.name;
                  case NAMED_COMP_SMMU_BASE:
                      return block->data.named_comp.smmu_base;
                  default:
                      val_print(ACS_PRINT_ERR,
                                "This Named component info option not supported %d\n", type);
                      return INVALID_NAMED_COMP_INFO;
              }
          }
          j++;
      }
  }

  return INVALID_NAMED_COMP_INFO;
}

/**
  @brief   This API is a single point of entry to retrieve
           PMCG information stored in the IoVirt Info table
           1. Caller       -  Test suite
           2. Prerequisite -  val_iovirt_create_info_table
  @param   type   the type of information being requested
  @param   index  the index of pmcg info instance.
  @return  64-bit data
**/
uint64_t
val_iovirt_get_pmcg_info(PMCG_INFO_e type, uint32_t index)
{
  uint32_t i, j = 0;
  IOVIRT_BLOCK *block;

  if (g_iovirt_info_table == NULL)
  {
      val_print(ACS_PRINT_ERR, "GET_PMCG_INFO: iovirt info table is not created\n", 0);
      return 0;
  }

  if (type == PMCG_NUM_CTRL)
       return g_iovirt_info_table->num_pmcgs;

  /* Go through the table return the relevant field value for the SMMU block */
  /* at the index position */
  block = &g_iovirt_info_table->blocks[0];
  for (i = 0; i < g_iovirt_info_table->num_blocks; i++, block = IOVIRT_NEXT_BLOCK(block))
  {
      block = ALIGN_MEMORY_ACCESS(block);
      if (block->type == IOVIRT_NODE_PMCG)
      {
          if (j == index)
          {
              switch (type)
              {
                  case PMCG_CTRL_BASE:
                      return block->data.pmcg.base;
                  case PMCG_IOVIRT_BLOCK:
                      return (uint64_t)block;
                  case PMCG_NODE_REF:
                      return block->data.pmcg.node_ref;
                  case PMCG_NODE_SMMU_BASE:
                      return block->data.pmcg.smmu_base;
                  default:
                      val_print(ACS_PRINT_ERR, "This PMCG info option not supported %d\n", type);
                      return 0;
              }
          }
          j++;
      }
  }

  if (index > j-1)
  {
      val_print(ACS_PRINT_ERR, "GET_PMCG_INFO: Index (%d) is greater than num of PMCG\n", index);
      return 0;
  }
  return j;
}

uint32_t
val_iovirt_get_rc_index(uint32_t rc_seg_num)
{
  uint32_t i, j = 0;
  IOVIRT_BLOCK *block;

  if (g_iovirt_info_table == NULL)
  {
      val_print(ACS_PRINT_ERR, "GET_PCIe_RC_INFO: iovirt info table is not created\n", 0);
      return 0;
  }

  /* Go through the table to reach a RC with the segment number */
  block = &g_iovirt_info_table->blocks[0];
  for (i = 0; i < g_iovirt_info_table->num_blocks; i++, block = IOVIRT_NEXT_BLOCK(block))
  {
      block = ALIGN_MEMORY_ACCESS(block);
      if (block->type == IOVIRT_NODE_PCI_ROOT_COMPLEX)
      {
          if (block->data.rc.segment == rc_seg_num)
          {
             break;
          }
          j++;
      }
  }
  if (i >=  g_iovirt_info_table->num_blocks)
  {
      val_print(ACS_PRINT_ERR, "GET_PCIe_RC_INFO: segemnt (%d) is not valid\n", rc_seg_num);
      return ACS_INVALID_INDEX;
  }
  return j;

}
