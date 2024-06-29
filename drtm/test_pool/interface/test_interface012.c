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

#include "val/common/include/acs_val.h"
#include "val/drtm/include/drtm_val_interface.h"

#define TEST_NUM   (ACS_DRTM_INTERFACE_TEST_NUM_BASE  +  12)
#define TEST_RULE  "R31010"
#define TEST_DESC  "Check DRTM_LOCK_TCB_HASHES function   "

static
void
payload(uint32_t num_pe)
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  int64_t  status = g_drtm_features.set_tcb_hash;

  /* Check if DRTM_SET_TCB_HASH is implemented */
  if (status != DRTM_ACS_SUCCESS) {
    val_print(ACS_PRINT_DEBUG,
              "\n       DRTM_SET_TCB_HASH function not supported err=%d", status);
    val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
    return;
  }

  /* If DRTM_SET_TCB_HASH is implemented then DRTM_LOCK_TCB_HASHES must be implemented */
  status = g_drtm_features.lock_tcb_hashes;
  if (status != DRTM_ACS_SUCCESS) {
    val_print(ACS_PRINT_ERR,
              "\n       DRTM_LOCK_TCB_HASHES function not supported err=%d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
interface012_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
  /* execute payload, which will execute relevant functions on current and other PEs */
      payload(num_pe);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
