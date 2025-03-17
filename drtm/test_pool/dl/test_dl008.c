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

#include "val/common/include/acs_val.h"
#include "val/common/include/acs_memory.h"
#include "val/drtm/include/drtm_val_interface.h"

#define TEST_NUM   (ACS_DRTM_DL_TEST_NUM_BASE  +  8)
#define TEST_RULE  "R44065"
#define TEST_DESC  "Check Dynamic Launch Invalid Features "

static
void
payload(uint32_t num_pe)
{

  /* This test will verify the DRTM Dynamic Launch
   * Input parameter will be 64 bit address of DRTM Parameters
   * */
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  int64_t  status;

  DRTM_PARAMETERS *drtm_params;
  uint64_t drtm_params_size = DRTM_SIZE_4K;
  uint64_t drtm_feature;

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


  /* Invoke DRTM Dynamic Launch, This will return only in case of error */

  /* Part 1: R44065 : DRTM_PARAMETER should not request features that are
   * not supported by DRTM implementation */
  /* If region memory protection is supported request for all memory protection,
   * otherwise  vise versa */
  drtm_feature = val_drtm_get_feature(DRTM_DRTM_FEATURES_DMA_PROTECTION);

  if (drtm_feature == DRTM_DMA_FEATURES_DMA_PROTECTION_ALL) {
    drtm_params->launch_features = drtm_params->launch_features |
            (DRTM_LAUNCH_FEAT_MEM_PROT_REGION_SUPP << DRTM_LAUNCH_FEATURES_MASK_MEM_PROTECTION);
  } else {
    drtm_params->launch_features = drtm_params->launch_features |
            (DRTM_LAUNCH_FEAT_MEM_PROT_ALL_SUPP << DRTM_LAUNCH_FEATURES_MASK_MEM_PROTECTION);
  }

  status = val_drtm_dynamic_launch(drtm_params);
  /* This will return invalid parameter */
  if (status != DRTM_ACS_INVALID_PARAMETERS) {
    val_print(ACS_PRINT_ERR, "\n       Incorrect Status. Expected = -2 Found = %d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
    if (status == DRTM_ACS_SUCCESS) {
      status = val_drtm_unprotect_memory();
      if (status < DRTM_ACS_SUCCESS) {
        val_print(ACS_PRINT_ERR, "\n       DRTM Unprotect Memory failed err=%d", status);
        val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
      }
    }
    goto free_dlme_region;
  }

  drtm_params->launch_features = 0;

  /* Part 2: R44065 : DRTM_PARAMETER should not request features that are
   * not supported by DRTM implementation */
  /* If Default PCR Usage Schema is supported request for DLME Authorities Schema,
   * otherwise  vise versa */
  drtm_feature = val_drtm_get_feature(DRTM_DRTM_FEATURES_PCR_SCHEMA);

  if (drtm_feature == DRTM_TPM_FEAT_PCR_SCHEMA_DEF_SUPP) {
    drtm_params->launch_features = drtm_params->launch_features |
            (DRTM_LAUNCH_FEAT_DLME_AUTH_SUPP << DRTM_LAUNCH_FEATURES_MASK_PCR_SCHEMA);
  } else {
    drtm_params->launch_features = drtm_params->launch_features |
            (DRTM_LAUNCH_FEAT_PCR_SCHEMA_DEF_SUPP << DRTM_LAUNCH_FEATURES_MASK_PCR_SCHEMA);
  }

  status = val_drtm_dynamic_launch(drtm_params);
  /* This will return invalid parameter */
  if (status != DRTM_ACS_INVALID_PARAMETERS) {
    val_print(ACS_PRINT_ERR, "\n       Incorrect Status. Expected = -2 Found = %d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 5));
    if (status == DRTM_ACS_SUCCESS) {
      status = val_drtm_unprotect_memory();
      if (status < DRTM_ACS_SUCCESS) {
        val_print(ACS_PRINT_ERR, "\n       DRTM Unprotect Memory failed err=%d", status);
        val_set_status(index, RESULT_FAIL(TEST_NUM, 6));
      }
    }
    goto free_dlme_region;
  }

  drtm_params->launch_features = 0;

  /* Part 3: R44065 : DRTM_PARAMETER should not request features that are
   * not supported by DRTM implementation */
  drtm_feature = val_drtm_get_feature(DRTM_DRTM_FEATURES_DLME_IMG_AUTH);

  if (drtm_feature != DRTM_DLME_IMG_FEAT_DLME_IMG_AUTH_SUPP) {
    drtm_params->launch_features = REQUEST_DLME_IMAGE_AUTH;

    status = val_drtm_dynamic_launch(drtm_params);
    /* This will return invalid parameter */
    if (status != DRTM_ACS_INVALID_PARAMETERS) {
      val_print(ACS_PRINT_ERR, "\n       Incorrect Status. Expected = -2 Found = %d", status);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 7));
      if (status == DRTM_ACS_SUCCESS) {
        status = val_drtm_unprotect_memory();
        if (status < DRTM_ACS_SUCCESS) {
          val_print(ACS_PRINT_ERR, "\n       DRTM Unprotect Memory failed err=%d", status);
          val_set_status(index, RESULT_FAIL(TEST_NUM, 8));
        }
      }
      goto free_dlme_region;
    }

    drtm_params->launch_features = 0;
  } else {
      val_print(ACS_PRINT_DEBUG,
              "\n       DRTM implementation supports DLME Image Authentication, skip check", 0);
  }

  /* Part 4: R44065 : DRTM_PARAMETER should not request features that are
   * not supported by DRTM implementation */
  /* If TPM based hashing is not supported, request for TPM based hashing */
  drtm_feature = val_drtm_get_feature(DRTM_DRTM_FEATURES_TPM_BASED_HASHING);

  if (drtm_feature != DRTM_TPM_BASED_HASHING_SUPPORT) {
    drtm_params->launch_features = REQUEST_TPM_BASED_HASHING;

    status = val_drtm_dynamic_launch(drtm_params);
    /* This will return invalid parameter */
    if (status != DRTM_ACS_INVALID_PARAMETERS) {
      val_print(ACS_PRINT_ERR, "\n       Incorrect Status. Expected = -2 Found = %d", status);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 9));
      if (status == DRTM_ACS_SUCCESS) {
        status = val_drtm_unprotect_memory();
        if (status < DRTM_ACS_SUCCESS) {
          val_print(ACS_PRINT_ERR, "\n       DRTM Unprotect Memory failed err=%d", status);
          val_set_status(index, RESULT_FAIL(TEST_NUM, 10));
        }
      }
      goto free_dlme_region;
    }

    drtm_params->launch_features = 0;
  } else {
      val_print(ACS_PRINT_DEBUG,
              "\n       DRTM implementation supports TPM based hashing, skip check", 0);
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));

free_dlme_region:
  val_memory_free_aligned((void *)drtm_params->dlme_region_address);
free_drtm_params:
  val_memory_free_aligned((void *)drtm_params);

  return;
}

uint32_t dl008_entry(uint32_t num_pe)
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
