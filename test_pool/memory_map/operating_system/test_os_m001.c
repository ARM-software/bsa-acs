/** @file
 * Copyright (c) 2016-2018, 2020, 2023, Arm Limited or its affiliates. All rights reserved.
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

#define TEST_NUM   (ACS_MEMORY_MAP_TEST_BASE + 1)
#define TEST_RULE  "B_MEM_02"
#define TEST_DESC  "Memory Access to Un-Populated addr    "

#define LOOP_VAR   3          /* Number of Addresses to check */

static void *branch_to_test;

static
void
payload();

static
void
esr(uint64_t interrupt_type, void *context)
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  /* Update the ELR to point to next instrcution */
  val_pe_update_elr(context, (uint64_t)branch_to_test);

  val_print(ACS_PRINT_INFO, "\n       Received Exception of type %d", interrupt_type);
  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

static
void
payload()
{
  addr_t   addr;
  uint64_t attr;
  uint32_t instance = 0;
  uint64_t status;
  uint32_t loop_var = LOOP_VAR;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  val_pe_install_esr(EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS, esr);
  val_pe_install_esr(EXCEPT_AARCH64_SERROR, esr);

  /* If we don't find a single un-populated address, mark this test as skipped */
  val_set_status(index, RESULT_SKIP(TEST_NUM, 1));

  while (loop_var) {
      /* Get the base address of unpopulated region */
      status = val_memory_get_unpopulated_addr(&addr, instance);
      if (status == PCIE_NO_MAPPING) {
          val_print(ACS_PRINT_INFO,
                    "\n       All instances of unpopulated memory were obtained",
                    0);
          return;
      }

      if (status) {
          val_print(ACS_PRINT_ERR,
                    "\n       Error in obtaining unpopulated memory for instance %d",
                    instance);
          return;
      }

      if (val_memory_get_info(addr, &attr) == MEM_TYPE_NOT_POPULATED) {
         /* default value of FAIL, Pass is set in the exception handler */
          val_set_status(index, RESULT_FAIL(TEST_NUM, 1));

          branch_to_test = &&exception_taken;

          *((volatile uint64_t*)addr) = 0x100;
exception_taken:
          /* if the access did not go to our exception handler, fail and exit */
          if (IS_TEST_FAIL(val_get_status(index))) {
              val_print(ACS_PRINT_ERR,
                        "\n       Memory access check fails at address = 0x%llx ",
                        addr);
              return;
          }

      }

      loop_var--;
      instance++;
  }

}

uint32_t
os_m001_entry(uint32_t num_pe)
{

  uint32_t error_flag = 0;
  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, val_pe_get_num());
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  error_flag = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  if (!error_flag)
      status = ACS_STATUS_PASS;
  else
      status = ACS_STATUS_FAIL;

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
