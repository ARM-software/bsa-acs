/** @file
 * Copyright (c) 2016-2018, 2020, 2021 Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/val_interface.h"

#include "val/include/bsa_acs_peripherals.h"
#include "val/include/bsa_acs_memory.h"
#include "val/include/bsa_acs_pe.h"

#define TEST_NUM   (ACS_MEMORY_MAP_TEST_BASE + 3)
#define TEST_RULE  "B_MEM_05"
#define TEST_DESC  "PE must access all NS addr space      "

static uint64_t check_number_of_bits(uint32_t index, uint64_t data)
{
        /* Read ID_AA64MMFR0_EL1[3:0] to get max no. of bits
         * that PE can access, Valid value: 0x0 to 0x6*/
         data = data & 0xF;
         if (data == 0)
            return ((uint64_t)0x1 << (int)32);
         else if (data == 0x1)
            return ((uint64_t)0x1 << (int)36);
         else if (data == 0x2)
            return ((uint64_t)0x1 << (int)40);
         else if (data == 0x3)
            return ((uint64_t)0x1 << (int)42);
         else if (data == 0x4)
            return ((uint64_t)0x1 << (int)44);
         else if (data == 0x5)
            return ((uint64_t)0x1 << (int)48);
         else if (data == 0x6)
            return ((uint64_t)0x1 << (int)52);
         else
         {
            val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
           return 0;
         }
}

static
void
payload()
{
  addr_t   addr;
  uint64_t data;
  uint64_t value = 0;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  data = val_pe_reg_read(ID_AA64MMFR0_EL1);
  /*Check max bits that PE can access*/
  value = check_number_of_bits(index, data);

  addr = val_get_max_memory();
  if (addr < value)
  {
      /* PE can access the Non-Secure address space*/
      val_set_status(index, RESULT_PASS(TEST_NUM, 1));
      return;
  }

  val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
  return;
}

uint32_t
os_m003_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, val_pe_get_num());
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
