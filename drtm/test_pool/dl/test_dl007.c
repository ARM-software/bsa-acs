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

#define TEST_NUM   (ACS_DRTM_DL_TEST_NUM_BASE  +  7)
#define TEST_RULE  ""
#define TEST_DESC  "Check Dynamic Launch Sec PE on        "

/* This will be used to inform secondary PE that dynamic launch command is issued */
volatile uint32_t dl_done = 0;

/* Payload to run on secondary PE */
void payload_secondary(void)
{
  /* Wait until DL is called on Primary PE */
  uint32_t timeout = TIMEOUT_MEDIUM;
  while (timeout-- && (dl_done == 0));
}

static
void
payload(uint32_t num_pe)
{

  /* This test will verify the DRTM Dynamic Launch
   * Input parameter will be 64 bit address of DRTM Parameters
   * */
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  int64_t  status;
  uint32_t sec_pe_index;
  uint32_t timeout = TIMEOUT_MEDIUM;

  DRTM_PARAMETERS *drtm_params;
  uint64_t drtm_params_size = DRTM_SIZE_4K;
  uint64_t dlme_image_addr;
  uint32_t num_of_pe;

  num_of_pe = val_pe_get_num();
  if (num_of_pe < 2) {
    /* Skip the test as there is no secondary PE */
    val_print(ACS_PRINT_ERR, "\n       No secondary PE Present. Skipping", 0);
    val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
    return;
  }

  for (uint8_t i = 0; i < num_of_pe; i++) {
      if (i == index)
        continue;

      sec_pe_index = i;
      break;
  }

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

  /* Switch on secondary PE */
  val_execute_on_pe(sec_pe_index, payload_secondary, 0);

  /* Invoke DRTM Dynamic Launch, This will return only in case of error */
  status = val_drtm_dynamic_launch(drtm_params);

  dl_done = 1;
  /* This should return Fail as secondary pe is on */
  if (status != DRTM_ACS_SECONDARY_PE_NOT_OFF) {
    val_print(ACS_PRINT_ERR, "\n       DRTM Dynamic Launch failed, Expected = %d",
                            DRTM_ACS_SECONDARY_PE_NOT_OFF);
    val_print(ACS_PRINT_ERR, " Found = %d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
    goto free_dlme_region;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));

free_dlme_region:
  val_memory_free_aligned((void *)drtm_params->dlme_region_address);
free_drtm_params:
  val_memory_free_aligned((void *)drtm_params);

  /* Wait for some time to make sure PE switched off */
  while (timeout--);

  return;
}

uint32_t dl007_entry(uint32_t num_pe)
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
