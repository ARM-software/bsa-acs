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

#ifndef __SBSA_ACS_IOVIRT_H__
#define __SBSA_ACS_IOVIRT_H__

#include "sbsa_val_interface.h"

typedef enum {
  NUM_NAMED_COMP = 1,
  NAMED_COMP_CCA_ATTR,
  NAMED_COMP_DEV_OBJ_NAME,
  NAMED_COMP_SMMU_BASE
} NAMED_COMP_INFO_e;

typedef enum {
  PMCG_NUM_CTRL = 1,
  PMCG_CTRL_BASE,
  PMCG_IOVIRT_BLOCK,
  PMCG_NODE_REF,
  PMCG_NODE_SMMU_BASE
} PMCG_INFO_e;

uint64_t val_iovirt_get_named_comp_info(NAMED_COMP_INFO_e type, uint32_t index);
uint64_t val_iovirt_get_pmcg_info(PMCG_INFO_e type, uint32_t index);
uint32_t val_iovirt_get_rc_index(uint32_t rc_seg_num);

#endif

