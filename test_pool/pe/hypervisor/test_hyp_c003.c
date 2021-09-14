/** @file
 * Copyright (c) 2021 Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_val.h"
#include "val/include/bsa_acs_pe.h"

#define TEST_NUM   (ACS_PE_HYP_TEST_NUM_BASE  +  3)
#define TEST_RULE  "B_PE_20"
#define TEST_DESC  "Check Stage2 and Stage1 Granule match "

static
void
payload()
{
  uint64_t data = 0;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint8_t Gran4_2, Gran4;
  uint8_t Gran16_2, Gran16;
  uint8_t Gran64_2, Gran64;

  data = val_pe_reg_read(ID_AA64MMFR0_EL1);

  /* TGran4_2: Stage 2 4KB Granularity support, bits [43:40] */
  Gran4_2 = VAL_EXTRACT_BITS(data, 40, 43);
  /* TGran4, Stage 1 4KB Granularity support, bits [31:28]  */
  Gran4 = VAL_EXTRACT_BITS(data, 28, 31);

  if ((Gran4_2 == 0x1) && (Gran4 != 0xF)) {
     /* 4KB granule not supported at stage 2 & supported at stage 1*/
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  if ((Gran4_2 == 0x2) && (Gran4 != 0x0)) {
    /* 4KB granule size with 48-bit addresses supported at stage 2, but not stage 1*/
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
    return;
  }

  if ((Gran4_2 == 0x3) && (Gran4 != 0x1)) {
    /* 4KB granule size with 52-bit addresses supported at stage 2, but not stage 1*/
    val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
    return;
  }

  /* TGran16_2: Stage 2 16KB Granularity support, bits [35:32] */
  Gran16_2 = VAL_EXTRACT_BITS(data, 32, 35);
  /* TGran16, Stage 1 16KB Granularity support, bits [23:20]  */
  Gran16 = VAL_EXTRACT_BITS(data, 20, 23);

  if ((Gran16_2 == 0x1) && (Gran16 != 0x0)) {
    /* Stage 2 not supported 16KB granules & but stage 1 support */
    val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
    return;
  }

  if ((Gran16_2 == 0x2) && (Gran16 != 0x1)) {
    /* 16KB granule size with 48-bit addresses supported at stage 2, but not stage 1*/
    val_set_status(index, RESULT_FAIL(TEST_NUM, 5));
    return;
  }

  if ((Gran16_2 == 0x3) && (Gran16 != 0x2)) {
    /* 16KB granule size with 52-bit addresses supported at stage 2, but not stage 1*/
    val_set_status(index, RESULT_FAIL(TEST_NUM, 6));
    return;
  }

  /* TGran64_2: Stage 2 64KB Granularity support, bits [39:36] */
  Gran64_2 = VAL_EXTRACT_BITS(data, 36, 39);
  /* TGran64, Stage 1 64KB Granularity support, bits [27:24]  */
  Gran64 = VAL_EXTRACT_BITS(data, 24, 27);

  if ((Gran64_2 == 0x1) && (Gran64 != 0xF)) {
    /* Stage 2 not supported 64KB granules & but stage 1 support */
    val_set_status(index, RESULT_FAIL(TEST_NUM, 7));
    return;
  }

  if ((Gran64_2 == 0x2) && (Gran64 != 0x0)) {
    /* 64KB granule size supported at stage 2, but not stage 1*/
    val_set_status(index, RESULT_FAIL(TEST_NUM, 8));
    return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
hyp_c003_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
