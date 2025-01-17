/** @file
 * Copyright (c) 2024, 2025, Arm Limited or its affiliates. All rights reserved.
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
#include "val/common/include/acs_memory.h"
#include "val/drtm/include/drtm_val_interface.h"

#define TEST_NUM   (ACS_DRTM_DL_TEST_NUM_BASE  +  4)
#define TEST_RULE  ""
#define TEST_DESC  "Check DRTM Close Locality             "

static
void
payload(uint32_t num_pe)
{

  /* This test will verify the DRTM Close Locality */
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  int64_t  status;

  DRTM_PARAMETERS *drtm_params;
  uint64_t drtm_params_size = DRTM_SIZE_4K;
  uint64_t dlme_image_addr;

  /* Allocate Memory For DRTM Parameters 4KB Aligned */
  drtm_params = (DRTM_PARAMETERS *)((uint64_t)val_aligned_alloc(DRTM_SIZE_4K, drtm_params_size));
  if (!drtm_params) {
    val_print(ACS_PRINT_ERR, "\n    Failed to allocate memory for DRTM Params", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  status = val_drtm_init_drtm_params(drtm_params);
  if (status != ACS_STATUS_PASS) {
    val_print(ACS_PRINT_ERR, "\n       DRTM Init Params failed err=%d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
    goto free_drtm_params;
  }

  /* DLME Image address is sum of region start and DLME image offset */
  dlme_image_addr = drtm_params->dlme_region_address + drtm_params->dlme_image_start;
  val_memcpy((void *)dlme_image_addr, (void *)g_drtm_acs_dlme, g_drtm_acs_dlme_size);

 /* Invoke DRTM Dynamic Launch, This will return only in case of error */
  status = val_drtm_dynamic_launch(drtm_params);
  /* This will return only in fail*/
  if (status < 0) {
    val_print(ACS_PRINT_ERR, "\n       DRTM Dynamic Launch failed err=%d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
    goto free_dlme_region;
  }

  status = val_drtm_unprotect_memory();
  if (status < 0) {
    val_print(ACS_PRINT_ERR, "\n       DRTM Unprotect Memory failed err=%d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
    goto free_dlme_region;
  }

  /* Part 1 : Check Close Locality for Locality 1, Should Result INVALID Parameters */
  status = val_drtm_close_locality(DRTM_LOC_1);
  if (status != DRTM_ACS_INVALID_PARAMETERS) {
    val_print(ACS_PRINT_ERR, "\n       Unexpected Status for close locality %d", status);
    val_print(ACS_PRINT_ERR, " Expected %d,", DRTM_ACS_INVALID_PARAMETERS);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 5));
    goto free_dlme_region;
  }

  /* Part 2 : Check Close Locality for Locality 2, Should Result Success
   * As, Locality 2 is used by the DLME to make any needed measurements.
   * After the DLME has completed its measurements. */
  status = val_drtm_close_locality(DRTM_LOC_2);
  if (status != DRTM_ACS_SUCCESS) {
    val_print(ACS_PRINT_ERR, "\n       DRTM Close Locality 2 failed err=%d", status);
    val_print(ACS_PRINT_ERR, " Expected %d,", DRTM_ACS_SUCCESS);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 6));
    goto free_dlme_region;
  }

  /* Part 3 : Check Close Locality for Locality 2 Again, Should Result
   * Already closed, as it is already closed. */
  status = val_drtm_close_locality(DRTM_LOC_2);
  if (status != DRTM_ACS_ALREADY_CLOSED) {
    val_print(ACS_PRINT_ERR, "\n       DRTM Close Locality 2 failed err=%d", status);
    val_print(ACS_PRINT_ERR, " Expected %d,", DRTM_ACS_ALREADY_CLOSED);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 7));
    goto free_dlme_region;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));

free_dlme_region:
  val_memory_free_aligned((void *)drtm_params->dlme_region_address);
free_drtm_params:
  val_memory_free_aligned((void *)drtm_params);

  return;
}

uint32_t dl004_entry(uint32_t num_pe)
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
