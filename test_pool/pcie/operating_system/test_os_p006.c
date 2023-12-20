/** @file
 * Copyright (c) 2016-2018, 2021, 2023, Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/bsa_acs_gic.h"
#include "val/include/bsa_acs_memory.h"

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 6)
#define TEST_RULE  "PCI_LI_01, PCI_LI_03"
#define TEST_DESC  "Legacy int must be SPI & lvl-sensitive"

static
void
payload(void)
{
  uint32_t status;
  uint32_t bdf;
  uint32_t dp_type;
  uint32_t pe_index;
  uint32_t tbl_index;
  uint32_t reg_value;
  uint32_t test_skip = 1;
  uint32_t intr_pin, intr_line;
  PERIPHERAL_IRQ_MAP *intr_map;
  pcie_device_bdf_table *bdf_tbl_ptr;
  INTR_TRIGGER_INFO_TYPE_e trigger_type;

  pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

  /* Allocate memory for interrupt mappings */
  intr_map = val_aligned_alloc(MEM_ALIGN_4K, sizeof(PERIPHERAL_IRQ_MAP));
  if (!intr_map) {
    val_print(ACS_PRINT_ERR, "\n       Memory allocation error", 0);
    val_set_status(pe_index, RESULT_FAIL (TEST_NUM, 1));
    return;
  }

  tbl_index = 0;
  bdf_tbl_ptr = val_pcie_bdf_table_ptr();

  while (tbl_index < bdf_tbl_ptr->num_entries)
  {
      bdf = bdf_tbl_ptr->device[tbl_index++].bdf;
      dp_type = val_pcie_device_port_type(bdf);

      /* Check entry is RP/EP/DP/UP. Else move to next BDF. */
      if ((dp_type == iEP_EP) || (dp_type == iEP_RP)
          || (dp_type == RCEC) || (dp_type == RCiEP))
          continue;

      val_print(ACS_PRINT_DEBUG, "\n       BDF - 0x%x", bdf);

      /* Read Interrupt Line Register */
      val_pcie_read_cfg(bdf, TYPE01_ILR, &reg_value);

      intr_pin = (reg_value >> TYPE01_IPR_SHIFT) & TYPE01_IPR_MASK;
      if ((intr_pin == 0) || (intr_pin > 0x4))
        continue;

      status = val_pci_get_legacy_irq_map(bdf, intr_map);
      if (status) {
        // Skip the test if the Legacy IRQ map does not exist
          if (status == NOT_IMPLEMENTED) {
            val_print (ACS_PRINT_DEBUG,
                        "\n       pal_pcie_get_legacy_irq_map unimplemented. Skipping test", 0);
            val_print(ACS_PRINT_DEBUG, "\n    The API is platform specific and to be populated", 0);
            val_print(ACS_PRINT_DEBUG, "\n    by partners with system legacy irq map", 0);
            val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 2));
        }
        else {
            val_print (ACS_PRINT_DEBUG,
                        "\n       PCIe Legacy IRQs unmapped. Skipping BDF %llx", bdf);
            val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 3));
            continue;
        }

        return;
      }

      /* If test runs for atleast an endpoint */
      test_skip = 0;

      intr_line = intr_map->legacy_irq_map[intr_pin-1].irq_list[0];

      /* Check if the int falls in SPI range */
      if ((intr_line >= 32 && intr_line <= 1019) ||
          (val_gic_espi_supported() && (intr_line >= 4096 &&
                            intr_line <= val_gic_max_espi_val())))  {
          val_print(ACS_PRINT_INFO, "\n Int is SPI", 0);
      }
      else {
          val_print(ACS_PRINT_ERR, "\n Int id %d is not SPI", intr_line);
          val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 3));
          return;
      }

      /* Read GICD_ICFGR/ICFGR-E register to Check for Level/Edge Sensitive. */
      if (intr_line >= 32 && intr_line <= 1019)
          status = val_gic_get_intr_trigger_type(intr_line, &trigger_type);
      else
          status = val_gic_get_espi_intr_trigger_type(intr_line, &trigger_type);

      if (status) {
        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 4));
        return;
      }

      if (trigger_type != INTR_TRIGGER_INFO_LEVEL_HIGH) {
        val_print(ACS_PRINT_ERR,
            "\n       Legacy interrupt programmed with incorrect trigger type", 0);
        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 5));
        return;
      }
  }

  val_memory_free_aligned(intr_map);

  if (test_skip == 1)
      val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 1));
  else
      val_set_status(pe_index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_p006_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
