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

static PCC_INFO_TABLE *g_pcc_info_table;

/**
  @brief  This API initialises the static global pointer to PCC
          info table.

  @param  PccInfoTable  - Address where the PCC information needs to be filled.

  @return  None
**/
void
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
