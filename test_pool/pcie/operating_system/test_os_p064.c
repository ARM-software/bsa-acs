/** @file
 * Copyright (c) 2016-2018, 2021-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "val/common/include/acs_memory.h"
#include "val/bsa/include/bsa_acs_pcie.h"

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 64)
#define TEST_RULE  "PCI_MSI_2"
#define TEST_DESC  "Check MSI(X) vectors uniqueness       "

/**
    @brief   Returns MSI(X) status of the device

    @param   dev_index    index of PCI device

    @return  0    device does not support MSI(X)
    @return  1    device supports MSI(X)
**/
static uint32_t check_msi_status(uint32_t bdf)
{
  uint32_t msi_cap_offset;

  /* Search for MSI/MSI-X Capability */
  if ((val_pcie_find_capability(bdf, PCIE_CAP, CID_MSIX, &msi_cap_offset)) ||
      (val_pcie_find_capability(bdf, PCIE_CAP, CID_MSI, &msi_cap_offset)))
  {
      val_print(ACS_PRINT_DEBUG, "\n       No MSI/MSI-X Capability for bdf 0x%x", bdf);
      return 0;
  }

  return 1;
}

/**
    @brief   Compare two lists of MSI(X) vectors

    @param   list_one    pointer to a first list of MSI(X) vectors
    @param   list_two    pointer to a second list of MSI(X) vectors

    @return  0    no vectors duplicates are found
    @return  1    lists contain at leas one common MSI(X) vector
**/
static
uint32_t
check_list_duplicates (PERIPHERAL_VECTOR_LIST *list_one, PERIPHERAL_VECTOR_LIST *list_two)
{
  PERIPHERAL_VECTOR_LIST *flist_node;
  PERIPHERAL_VECTOR_LIST *slist_node;

  uint32_t fcount = 0;
  uint32_t scount = 0;
  uint64_t irq_start1, irq_end1;
  uint64_t irq_start2, irq_end2;

  flist_node = list_one;
  slist_node = list_two;

  while (flist_node != NULL) {
    while (slist_node != NULL) {
      irq_start1 = flist_node->vector.vector_irq_base;
      irq_end1 = flist_node->vector.vector_irq_base + flist_node->vector.vector_n_irqs - 1;
      irq_start2 = slist_node->vector.vector_irq_base;
      irq_end2 = slist_node->vector.vector_irq_base + slist_node->vector.vector_n_irqs - 1;
      if (!(irq_end1 < irq_start2 || irq_start1 > irq_end2))
        return 1;
      slist_node = slist_node->next;
      scount++;
    }
    slist_node = list_two;
    flist_node = flist_node->next;
    fcount++;
    scount = 0;
  }

  return 0;
}

/**
    @brief   Free memory allocated for a list of MSI(X) vectors

    @param   list    pointer to a list of MSI(X) vectors
**/
static
void
clean_msi_list (PERIPHERAL_VECTOR_LIST *list)
{
  PERIPHERAL_VECTOR_LIST *next_node;
  PERIPHERAL_VECTOR_LIST *current_node;

  current_node = list;
  while (current_node != NULL) {
    next_node = current_node->next;
    val_memory_free (current_node);
    current_node = next_node;
  }
}

static
void
payload (void)
{

  uint32_t index = val_pe_get_index_mpid (val_pe_get_mpid());
  uint8_t status;
  PERIPHERAL_VECTOR_LIST *current_dev_mvec;
  PERIPHERAL_VECTOR_LIST *next_dev_mvec;
  uint64_t current_dev_bdf;
  uint64_t next_dev_bdf;
  uint32_t test_skip = 1;
  pcie_device_bdf_table *bdf_tbl_ptr;
  uint32_t tbl_index = 0;
  uint32_t tbl_index_next = 0;
  uint32_t bdf, dp_type;
  uint32_t class_code;
  uint32_t base_cc;
  status = 0;

  bdf_tbl_ptr = val_pcie_bdf_table_ptr();

  while (tbl_index < bdf_tbl_ptr->num_entries && !status) {
      bdf = bdf_tbl_ptr->device[tbl_index].bdf;
      tbl_index_next = tbl_index + 1;

      val_pcie_read_cfg(bdf, TYPE01_RIDR, &class_code);
      val_print(ACS_PRINT_DEBUG, "\n     Primary  BDF is 0x%x", bdf);
      val_print(ACS_PRINT_DEBUG, "\n         Class code is 0x%x", class_code);

      base_cc = class_code >> TYPE01_BCC_SHIFT;

      /* Skip Network Controller devices as locking access to MSI
       * descriptors is causing an exception and for the devices
       * with base class codes greater than 13h as they
       * are reserved */
      if ((base_cc == CNTRL_CC) || (base_cc > RES_CC))
      {
        tbl_index++;
        val_print(ACS_PRINT_DEBUG, "\n        Skipping ..N/W ctrl device", 0);
        continue;
      }

      dp_type = val_pcie_device_port_type(bdf);
      /* Check entry is EP. Else move to next BDF. */
      if (dp_type == EP) {

        val_print(ACS_PRINT_DEBUG, "\n        Continuing...device is EP", 0);
        current_dev_bdf = bdf;
        if (!check_msi_status(current_dev_bdf))
        {
          tbl_index++;
          continue;
        }

        if (val_get_msi_vectors (current_dev_bdf, &current_dev_mvec)) {
            tbl_index_next = tbl_index + 1;
            while (tbl_index_next < bdf_tbl_ptr->num_entries && !status)
            {
                bdf = bdf_tbl_ptr->device[tbl_index_next].bdf;

                val_pcie_read_cfg(bdf, TYPE01_RIDR, &class_code);
                val_print(ACS_PRINT_DEBUG, "\n     Secondary  BDF is 0x%x", bdf);
                val_print(ACS_PRINT_DEBUG, "\n       Class code is 0x%x", class_code);

                base_cc = class_code >> TYPE01_BCC_SHIFT;

               /* Skip Network Controller devices as locking access to MSI
                * descriptors is causing an exception and for the devices
                * with base class codes greater than 13h as they
                * are reserved */
                if ((base_cc == CNTRL_CC) || (base_cc > RES_CC))
                {
                  tbl_index_next++;
                  val_print(ACS_PRINT_DEBUG, "\n        Skipping ..N/W ctrl device", 0);
                  continue;
                }

                dp_type = val_pcie_device_port_type(bdf);
                /* Check entry is EP. Else move to next BDF. */
                if (dp_type == EP)
                {
                  val_print(ACS_PRINT_DEBUG, "\n        Continuing...device is EP", 0);
                  next_dev_bdf = bdf;
                  if (!check_msi_status(next_dev_bdf))
                  {
                    tbl_index_next++;
                    continue;
                  }

                  /* Read MSI(X) vectors */
                  if (val_get_msi_vectors (next_dev_bdf, &next_dev_mvec))
                  {
                    test_skip = 0;
                    /* Compare two lists of MSI(X) vectors */
                    if (check_list_duplicates (current_dev_mvec, next_dev_mvec))
                    {
                      val_print (ACS_STATUS_ERR, "\n       Allocated MSIs are not unique", 0);
                      val_set_status (index, RESULT_FAIL(TEST_NUM, 01));
                      status = 1;
                    }
                    clean_msi_list (next_dev_mvec);
                  }
                }
                tbl_index_next++;
            }
            clean_msi_list (current_dev_mvec);
        }
      }
      else {
        val_print (ACS_PRINT_DEBUG, "\n       BDF is not EP 0x%x", bdf);
      }
      tbl_index++;
  }

  if (test_skip) {
    val_print(ACS_PRINT_ERR, "\n       No MSI vectors found ", 0);;
    val_set_status (index, RESULT_SKIP(TEST_NUM, 01));
  } else  if (!status) {
    val_set_status (index, RESULT_PASS(TEST_NUM, 01));
  }
}

uint32_t
os_p064_entry(uint32_t num_pe)
{
  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP) {
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);
  }

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
