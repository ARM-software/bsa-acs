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

#define TEST_NUM   (ACS_DRTM_INTERFACE_TEST_NUM_BASE  +  5)
#define TEST_RULE  "R58000"
#define TEST_DESC  "DMA Protection Support/Reserved check "

static
void
payload(uint32_t num_pe)
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  int64_t  status = g_drtm_features.dma_prot_features.status;
  uint64_t features_dma_prot = g_drtm_features.dma_prot_features.value;
  uint32_t dma_prot_support;

  /*Status value less than zero are error case*/
  if (status < DRTM_ACS_SUCCESS) {
    val_print(ACS_PRINT_ERR, "\n       DRTM query DMA protection feature failed err=%d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  /*Status grater than zero indicates availability of feature bits in return value*/
  if (status > DRTM_ACS_SUCCESS) {
    if (val_drtm_reserved_bits_check_is_zero(
                   VAL_EXTRACT_BITS(features_dma_prot, 24, 63)) != ACS_STATUS_PASS) {
      val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
      return;
    }
    /* Check atleast 1 DMA Protection is supported */
    dma_prot_support = VAL_EXTRACT_BITS(g_drtm_features.dma_prot_features.value, 0, 7);
    if (dma_prot_support == 0) {
      val_print(ACS_PRINT_ERR, "\n       DMA Protection Not Supported", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
      return;
    }
  } else {
    val_print(ACS_PRINT_ERR,
              "\n       DMA protection feature value not available in return value", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
    return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
interface005_entry(uint32_t num_pe)
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
