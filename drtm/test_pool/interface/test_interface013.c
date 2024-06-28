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
#include "val/common/include/acs_memory.h"

#define TEST_NUM   (ACS_DRTM_INTERFACE_TEST_NUM_BASE  +  13)
#define TEST_RULE  ""
#define TEST_DESC  "Check SET_TCB_HASHES max hashes case  "

#define SHA_256_DIGEST_SIZE_BYTES 32

static
void
init_hashes(DRTM_TCB_HASH_TABLE *hash_table)
{
  int32_t hash;
  uint32_t num_hashes = VAL_EXTRACT_BITS(g_drtm_features.tcb_hash_features.value, 0, 7);
  uint8_t sha_256_digest_val[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                               };
  /* Fill the TCB Hash Table Header */
  hash_table->header.revision         = 1;
  hash_table->header.num_hashes       = num_hashes + 1;
  hash_table->header.reserved         = 0;
  /* NOTE: Taking SHA-256 as default hash algorithm */
  hash_table->header.hash_algo        = 0x000B; /* 0x000B: SHA-256: 32 bytes */
  /* Fill Dummy Hash values */
  for (hash = 0; hash < hash_table->header.num_hashes; hash++) {
    hash_table->hashes[hash].hash_id   = hash;
    val_memcpy(hash_table->hashes[hash].hash_val, sha_256_digest_val, SHA_256_DIGEST_SIZE_BYTES);
  }
}

static
void
payload(uint32_t num_pe)
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  int64_t  status = g_drtm_features.set_tcb_hash;
  uint32_t num_hashes = VAL_EXTRACT_BITS(g_drtm_features.tcb_hash_features.value, 0, 7);
  DRTM_TCB_HASH_TABLE *drtm_hash_table;

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

  /* num_hashes value zero indicates that hashes cannot be recorded with DRTM_SET_TCB_HASH */
  if (!num_hashes) {
    val_print(ACS_PRINT_ERR, "\n       Max Hashes can be recorded with DRTM_SET_TCB_HASH is 0", 0);
    val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
  }

  /* Max number of hashes can be recorded plus one */
  num_hashes = num_hashes + 1;

  drtm_hash_table = (DRTM_TCB_HASH_TABLE *)val_memory_alloc(sizeof(DRTM_TCB_HASH_TABLE_HDR) +
              ((sizeof(uint32_t) + (sizeof(uint8_t) * SHA_256_DIGEST_SIZE_BYTES)) * num_hashes));
  if (!drtm_hash_table) {
    val_print(ACS_PRINT_ERR, "\n       Failed to allocate tcb hash table", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
    return;
  }

  init_hashes(drtm_hash_table);

  status = val_drtm_set_tcb_hash((uint64_t)drtm_hash_table);
  if (status != DRTM_ACS_OUT_OF_RESOURCE) {
    val_print(ACS_PRINT_ERR, "\n       DRTM set invalid num of Hashes failed %d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
    goto free_tcb_hash;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));

free_tcb_hash:
  val_memory_free(drtm_hash_table);
}

uint32_t
interface013_entry(uint32_t num_pe)
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
