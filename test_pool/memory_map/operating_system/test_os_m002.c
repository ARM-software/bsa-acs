/** @file
 * Copyright (c) 2021, 2023, Arm Limited or its affiliates. All rights reserved.
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

#define TEST_NUM   (ACS_MEMORY_MAP_TEST_BASE + 2)
#define TEST_RULE  "B_MEM_01"
#define TEST_DESC  "Mem Access Response in finite time    "

#define LOOP_VAR   3          /* Number of Addresses to check */

static uint64_t branch_to_test;
uint32_t loop_var = LOOP_VAR;
uint32_t instance = 0;
uint32_t timeout;

static
void
payload();

static
void
esr(uint64_t interrupt_type, void *context)
{
  /* Update the ELR to point to next instrcution */
  val_pe_update_elr(context, branch_to_test);

  val_print(ACS_PRINT_DEBUG, "\n       Received Exception of type %d", interrupt_type);
}

static
void
payload()
{
  addr_t   addr;
  uint64_t attr;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t original_value;

  val_pe_install_esr(EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS, esr);
  val_pe_install_esr(EXCEPT_AARCH64_SERROR, esr);
  val_set_status(index, RESULT_SKIP(TEST_NUM, 1));

  branch_to_test = (uint64_t)&&exception_taken_d;
  while (loop_var) {
      timeout = TIMEOUT_SMALL;
      /* Get the address of device memory region */
      addr = val_memory_get_addr(MEM_TYPE_DEVICE, instance, &attr);
      if (!addr) {
          val_print(ACS_PRINT_DEBUG, "\n       Error in getting dev mem for"
                                   " index %d  ", instance);
          goto normal_mem_test;
      }

      /* Access must not cause a deadlock */
      original_value = *((volatile addr_t*)addr);
      *((volatile addr_t*)addr) = original_value;
      while (timeout--)
          {};

exception_taken_d:
      val_set_status(index, RESULT_PASS(TEST_NUM, 1));
      loop_var--;
      instance++;
  }

normal_mem_test:
  loop_var = LOOP_VAR;
  instance = 0;
  branch_to_test = (uint64_t)&&exception_taken_n;
  while (loop_var) {
      timeout = TIMEOUT_SMALL;
      /* Get the address of normal memory region */
      addr = val_memory_get_addr((MEMORY_INFO_e)MEMORY_TYPE_NORMAL, instance, &attr);
      if (!addr) {
          val_print(ACS_PRINT_DEBUG, "\n       Error in obtaining normal memory for"
                                   " instance %d", instance);
          return;
      }

      /* Access must not cause a deadlock */
      original_value = *((volatile addr_t*)addr);
      *((volatile addr_t*)addr) = original_value;
      while (timeout--)
          {};

exception_taken_n:
      val_set_status(index, RESULT_PASS(TEST_NUM, 2));
      loop_var--;
      instance++;
  }
}

uint32_t
os_m002_entry(uint32_t num_pe)
{

  uint32_t error_flag = 0;
  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, val_pe_get_num());
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* Get the result from the PE and check for failure */
  error_flag = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  if (!error_flag)
      status = ACS_STATUS_PASS;
  else
      status = ACS_STATUS_FAIL;

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
