/** @file
 * Copyright (c) 2020,2021,2024, Arm Limited or its affiliates. All rights reserved.
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
#include "val/common/include/acs_pcie.h"
#include "val/common/include/acs_pe.h"
#include "val/common/include/acs_memory.h"
#include "val/common/include/acs_iovirt.h"

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 17)
#define TEST_RULE  "PCI_PP_05"
#define TEST_DESC  "Check Direct Transl P2P Support       "

static
void
payload(void)
{

  uint32_t bdf;
  uint32_t pe_index;
  uint32_t tbl_index;
  uint32_t dp_type;
  uint32_t cap_base = 0;
  uint32_t test_fails;
  uint32_t test_skip = 1;
  uint32_t acs_data;
  uint32_t num_pcie_rc;
  uint32_t rc_ats_attr;
  uint32_t rc_ats_supp;
  uint32_t data;
  pcie_device_bdf_table *bdf_tbl_ptr;

  pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

  /* Check If PCIe Hierarchy supports P2P */
  if (val_pcie_p2p_support() == NOT_IMPLEMENTED) {
    val_print(ACS_PRINT_DEBUG, "\n       The test is applicable only if the system supports", 0);
    val_print(ACS_PRINT_DEBUG, "\n       P2P traffic. If the system supports P2P, pass the", 0);
    val_print(ACS_PRINT_DEBUG, "\n       command line option '-p2p' while running the binary", 0);
    val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 1));
    return;
  }

  bdf_tbl_ptr = val_pcie_bdf_table_ptr();

  test_fails = 0;

  num_pcie_rc = val_iovirt_get_pcie_rc_info(NUM_PCIE_RC, 0);

  /* Get the number of Root Complex in the system */
  if (!num_pcie_rc) {
     val_print(ACS_PRINT_DEBUG, "\n       Skip because no PCIe RC detected  ", 0);
     val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 1));
     return;
  }

  /* For each Root Complex, check if it supports ATS capability.
   * This information should be obtained from ACPI-IORT table for UEFI based
   * systems and platform config file for Baremetal based system
   * If RC supports the ATS, RP also supports ATS
   */
  while (num_pcie_rc) {
      num_pcie_rc--;   // Index is one lesser than the component number being accessed
      rc_ats_attr = val_iovirt_get_pcie_rc_info(RC_ATS_ATTRIBUTE, num_pcie_rc);
      rc_ats_supp = rc_ats_attr & 1;

      if (!rc_ats_supp)
      {
          val_print(ACS_PRINT_DEBUG, "\n       ATS Capability Not Present for RC: %x", num_pcie_rc);
          continue;
      }
  }

  /* Check for all the function present in bdf table */
  for (tbl_index = 0; tbl_index < bdf_tbl_ptr->num_entries; tbl_index++)
  {
      bdf = bdf_tbl_ptr->device[tbl_index].bdf;
      dp_type = val_pcie_device_port_type(bdf);

      /* Check entry is RP */
      if (dp_type == RP)
      {
          /* Check If RP supports P2P with other RP's. */
          if (val_pcie_dev_p2p_support(bdf))
              continue;

          /* If test runs for atleast one RP */
          test_skip = 0;

          val_print(ACS_PRINT_DEBUG, "\n       For BDF : 0x%x", bdf);

          /* Read the ACS Capability */
          if (val_pcie_find_capability(bdf, PCIE_ECAP, ECID_ACS, &cap_base) != PCIE_SUCCESS) {
              val_print(ACS_PRINT_ERR, "\n       ACS Capability not supported, Bdf : 0x%x", bdf);
              test_fails++;
              continue;
          }

          val_pcie_read_cfg(bdf, cap_base + ACSCR_OFFSET, &acs_data);

          /* Extract ACS directed translated p2p bit */
          data = VAL_EXTRACT_BITS(acs_data, 6, 6);
          if (data == 0) {
              val_print(ACS_PRINT_ERR,
                              "\n       Directed Translated P2P not supported, Bdf : 0x%x", bdf);
              test_fails++;
          }
      }
  }

  if (test_skip == 1) {
      val_print(ACS_PRINT_DEBUG,
           "\n       No RP type device found with P2P and ATS Support. Skipping test", 0);
      val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 2));
  }
  else if (test_fails)
      val_set_status(pe_index, RESULT_FAIL(TEST_NUM, test_fails));
  else
      val_set_status(pe_index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_p017_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
