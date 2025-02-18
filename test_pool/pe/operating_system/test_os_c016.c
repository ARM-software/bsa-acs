/** @file
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
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
#include "val/common/include/acs_pe.h"
#include "val/common/include/acs_memory.h"

#define TEST_NUM   (ACS_PE_TEST_NUM_BASE  +  16)
#define TEST_RULE  "B_PE_14"
#define TEST_DESC  "Check SVE2 for v9 PE                  "

typedef struct {
  uint64_t data;
  uint32_t status;
} sve_reg_details;

sve_reg_details *g_sve_reg_info;

static
void
payload()
{
  uint32_t pe_family;
  uint64_t data;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  sve_reg_details *tmp_reg_data;

  tmp_reg_data = g_sve_reg_info + index;

  /* Get PE family for each PE index*/
  pe_family = val_get_pe_architecture(index);

  if (pe_family == ACS_STATUS_ERR) {
    val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
    tmp_reg_data->status = ACS_STATUS_ERR;
    return;
  }

  /* If processor is not Armv9, FEAT_SVE2 is not required */
  if (pe_family != PROCESSOR_FAMILY_ARMV9) {
    val_set_status(index, RESULT_SKIP(TEST_NUM, 3));
    tmp_reg_data->status = ACS_STATUS_SKIP;
    return;
  }

 /* Read ID_AA64ZFR0_EL1 for SVE2 support */
  data = val_pe_reg_read(ID_AA64ZFR0_EL1);
  tmp_reg_data->data = data;

  /* For Armv9, the ID_AA64ZFR0_EL1.SVEver, bits [3:0] value 0b0000 is not permitted */
  if (VAL_EXTRACT_BITS(data, 0, 3) == 0) {
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    tmp_reg_data->status = ACS_STATUS_FAIL;
    return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_c016_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;  //default value
  uint32_t i, smbios_slots, index = val_pe_get_index_mpid(val_pe_get_mpid());
  sve_reg_details *reg_buffer;

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  smbios_slots = val_get_num_smbios_slots();
  if (smbios_slots == 0) {
    val_print(ACS_PRINT_WARN, "\n       SMBIOS Table Not Found, Skipping the test\n", 0);
    status = ACS_STATUS_SKIP;
    val_set_status(index, RESULT_SKIP(TEST_NUM, 1));

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, 1, TEST_RULE);
  }

  /* This check is when user is forcing us to skip this test */
  if (status != ACS_STATUS_SKIP) {
    g_sve_reg_info = (sve_reg_details *) val_memory_calloc(num_pe, sizeof(sve_reg_details));
    if (g_sve_reg_info == NULL) {
      val_print(ACS_PRINT_ERR, "\n       Memory Allocation for SVE Register data Failed", 0);
      return ACS_STATUS_FAIL;
    }

    val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    for (i = 0; i < num_pe; i++) {
      reg_buffer = g_sve_reg_info + i;
      val_print(ACS_PRINT_DEBUG, "\n       PE Index = %d", i);

      if (reg_buffer->status == ACS_STATUS_SKIP)
        val_print(ACS_PRINT_DEBUG, "\n       Processor is not v9, Skipping the test", 0);
      else if (reg_buffer->status == ACS_STATUS_FAIL)
        val_print(ACS_PRINT_DEBUG, "\n       Reg Value = 0x%llx  FAIL", reg_buffer->data);
      else if (reg_buffer->status == ACS_STATUS_ERR)
        val_print(ACS_PRINT_DEBUG, "\n       Processor Family Not Found in SMBIOS Table", 0);
      else
        val_print(ACS_PRINT_DEBUG, "\n       Reg Value = 0x%llx  PASS", reg_buffer->data);
    }

    val_memory_free((void *) g_sve_reg_info);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);
  }

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
