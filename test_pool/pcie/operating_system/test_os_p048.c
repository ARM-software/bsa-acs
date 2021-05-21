/** @file
 * Copyright (c) 2016-2018, 2021, Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_pe.h"
#include "val/include/bsa_acs_pcie.h"

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 48)
#define TEST_DESC  "PCI_IN_13: PHB, RP must recognize Txn from upstream"

#define KNOWN_DATA  0xABABABAB

static void *branch_to_test;

static
void
esr(uint64_t interrupt_type, void *context)
{
  uint32_t pe_index;

  pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

  /* Update the ELR to return to test specified address */
  val_pe_update_elr(context, (uint64_t)branch_to_test);

  val_print(ACS_PRINT_INFO, "\n       Received exception of type: %d", interrupt_type);
  val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
}

static
void
payload(void)
{

  uint32_t bdf;
  uint32_t dp_type;
  uint32_t pe_index;
  uint32_t tbl_index;
  uint32_t read_value, old_value;
  uint32_t test_skip = 1;
  uint64_t mem_base;
  uint64_t mem_lim;
  pcie_device_bdf_table *bdf_tbl_ptr;

  tbl_index = 0;
  bdf_tbl_ptr = val_pcie_bdf_table_ptr();
  pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

  /* Install sync and async handlers to handle exceptions.*/
  val_pe_install_esr(EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS, esr);
  val_pe_install_esr(EXCEPT_AARCH64_SERROR, esr);
  branch_to_test = &&exception_return;

  /* Since this is a memory space access test.
   * Enable BME & MSE for all the BDFs.
  */
  while (tbl_index < bdf_tbl_ptr->num_entries)
  {
      bdf = bdf_tbl_ptr->device[tbl_index++].bdf;
      /* Enable Bus Master Enable */
      val_pcie_enable_bme(bdf);
      /* Enable Memory Space Access */
      val_pcie_enable_msa(bdf);
  }

  tbl_index = 0;
  while (tbl_index < bdf_tbl_ptr->num_entries)
  {
      bdf = bdf_tbl_ptr->device[tbl_index++].bdf;
      dp_type = val_pcie_device_port_type(bdf);

      if ((dp_type == RP) || (dp_type == iEP_RP)) {
        /* Part 1:
         * Check When Address is within the Range of Non-Prefetchable
         * Memory Range.
        */
        /* Clearing UR in Device Status Register */
        val_pcie_clear_urd(bdf);

        /* Read Function's NP Memory Base Limit Register */
        val_pcie_read_cfg(bdf, TYPE1_NP_MEM, &read_value);
        if (read_value == 0)
          continue;

        mem_base = (read_value & MEM_BA_MASK) << MEM_BA_SHIFT;
        mem_lim = (read_value & MEM_LIM_MASK) | MEM_LIM_LOWER_BITS;

        /* If Memory Limit is programmed with value less the Base, then Skip.*/
        if (mem_lim < mem_base)
          continue;

        /* If test runs for atleast an endpoint */
        test_skip = 0;

        /* Write known value to an address which is in range
         * Base + 0x10 will always be in the range.
         * Read the same
        */
        old_value = (*(volatile uint32_t *)(mem_base + MEM_OFFSET_10));
        *(volatile uint32_t *)(mem_base + MEM_OFFSET_10) = KNOWN_DATA;
        read_value = (*(volatile uint32_t *)(mem_base + MEM_OFFSET_10));

exception_return:
        /* Memory Space might have constraint on RW/RO behaviour
         * So not checking for Read-Write Data mismatch.
        */
        if (IS_TEST_FAIL(val_get_status(pe_index))) {
          val_print(ACS_PRINT_ERR,
            "\n       Failed. Exception on Memory Access For Bdf : 0x%x", bdf);
          val_pcie_clear_urd(bdf);
          return;
        }

        if (val_pcie_is_urd(bdf) ||
          (old_value != read_value && read_value == PCIE_UNKNOWN_RESPONSE)) {
            val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));
            val_pcie_clear_urd(bdf);
            return;
        }
      }
  }

  if (test_skip == 1)
      val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 01));
  else
      val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));
}

uint32_t
os_p048_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe);

  val_report_status(0, BSA_ACS_END(TEST_NUM));

  return status;
}
