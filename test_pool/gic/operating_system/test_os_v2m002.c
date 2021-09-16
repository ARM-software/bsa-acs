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

#define TEST_NUM   (ACS_GIC_V2M_TEST_NUM_BASE + 2)
#define TEST_RULE  "Appendix I.9"
#define TEST_DESC  "Check GICv2m MSI Frame Register       "

static
void
payload()
{

  uint32_t num_spi;
  uint32_t data, new_data;
  uint32_t fail_cnt, instance;
  uint32_t msi_frame, min_spi_id;
  uint64_t frame_base;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  msi_frame = val_gic_get_info(GIC_INFO_NUM_MSI_FRAME);
  if (msi_frame == 0) {
      val_print(ACS_PRINT_DEBUG, "\n       No MSI frame, Skipping               ", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
      return;
  }

  fail_cnt = 0;

  for (instance = 0; instance < msi_frame; instance++) {
    frame_base = val_gic_v2m_get_info(V2M_MSI_FRAME_BASE, instance);

    /* Part 1 : GICv2m_MSI_TYPER Read Only Register */
    data = val_mmio_read(frame_base + GICv2m_MSI_TYPER);

    /* Try Changing all the bits of the RO register */
    val_mmio_write(frame_base + GICv2m_MSI_TYPER, (data ^ 0xFFFFFFFF));

    new_data = val_mmio_read(frame_base + GICv2m_MSI_TYPER);
    if (data != new_data) {
      fail_cnt++;
      val_print(ACS_PRINT_DEBUG, "\n       MSI_TYPER RO Check Failed for instance %d", instance);
    }

    /* Part 2 : Check SPI ID allocated is b/w 32-1020 */
    min_spi_id = val_gic_v2m_get_info(V2M_MSI_SPI_BASE, instance);
    num_spi  = val_gic_v2m_get_info(V2M_MSI_SPI_NUM, instance);

    if ((min_spi_id < 32) || (min_spi_id + num_spi) > 1020) {
      fail_cnt++;
      val_print(ACS_PRINT_DEBUG, "\n       SPI ID Check Failed for instance %d", instance);
    }

    /* Part 3 : GICv2m_MSI_IIDR Read Only Register */
    data = val_mmio_read(frame_base + GICv2m_MSI_IIDR);

    /* Try Changing all the bits of the RO register */
    val_mmio_write(frame_base + GICv2m_MSI_IIDR, (data ^ 0xFFFFFFFF));

    new_data = val_mmio_read(frame_base + GICv2m_MSI_IIDR);
    if (data != new_data) {
      fail_cnt++;
      val_print(ACS_PRINT_DEBUG, "\n       MSI_IIDR RO Check Failed for instance %d", instance);
    }

  }

  if (fail_cnt) {
      val_print(ACS_PRINT_ERR, "\n       MSI_TYPER Register Check Failed", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
      return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_v2m002_entry(uint32_t num_pe)
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
