/** @file
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_gic.h"

#define TEST_NUM   (ACS_GIC_V2M_TEST_NUM_BASE + 1)
#define TEST_RULE  "Appendix I.6"
#define TEST_DESC  "Check MSI SPI are Edge Triggered      "

static
void
payload()
{

  uint32_t status;
  uint32_t test_skip;
  uint32_t fail_cnt, instance;
  uint32_t msi_frame, spi_id;
  uint32_t spi_base, num_spi;
  INTR_TRIGGER_INFO_TYPE_e trigger_type;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  msi_frame   = val_gic_get_info(GIC_INFO_NUM_MSI_FRAME);
  if (msi_frame == 0) {
      val_print(ACS_PRINT_DEBUG, "\n       No MSI frame, Skipping               ", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
      return;
  }

  fail_cnt = 0;
  test_skip = 1;

  for (instance = 0; instance < msi_frame; instance++) {
    spi_base = val_gic_v2m_get_info(V2M_MSI_SPI_BASE, instance);
    num_spi  = val_gic_v2m_get_info(V2M_MSI_SPI_NUM, instance);

    /* Check All the SPIs which are mapped to MSI are Edge Triggered */
    for (spi_id = spi_base; spi_id < spi_base+num_spi; spi_id++) {

      test_skip = 0;

      /* Read GICD_ICFGR register to Check for Level/Edge Sensitive. */
      status = val_gic_get_intr_trigger_type(spi_id, &trigger_type);
      if (status) {
        val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
        return;
      }

      if (trigger_type != INTR_TRIGGER_INFO_EDGE_RISING) {
        val_print(ACS_PRINT_DEBUG, "\n       Error : SPI ID 0x%x Level Triggered ", spi_id);
        fail_cnt++;
      }
    }
  }

  if (test_skip) {
      val_print(ACS_PRINT_WARN, "\n       No SPI Information Found. Skipping   ", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
      return;
  }

  if (fail_cnt) {
      val_print(ACS_PRINT_ERR, "\n       SPI Trigger Type Check Failed", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
      return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_v2m001_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This GIC test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
