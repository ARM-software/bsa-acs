/** @file
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "val/drtm/include/drtm_val_interface.h"
#include "val/common/include/acs_gic.h"

#define TEST_NUM   (ACS_DRTM_INTERFACE_TEST_NUM_BASE  +  10)
#define TEST_RULE  ""
#define TEST_DESC  "Check GIC supports disabling LPIs     "

extern GIC_ITS_INFO    *g_gic_its_info;

static
void
payload(uint32_t num_pe)
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint64_t pe_rdbase, its_base = 0;
  uint32_t gicr_ctrl_value, gits_ctrl_value;
  uint32_t its_id, num_its;

  /* Get RDBase Address for current PE */
  pe_rdbase = val_gic_get_pe_rdbase(index);
  val_print(ACS_PRINT_DEBUG, "\n       PE RD base address %llx", pe_rdbase);
  if (pe_rdbase == 0)
  {
    val_print(ACS_PRINT_ERR, "\n       Could not get RD Base Address", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  gicr_ctrl_value = val_mmio_read(pe_rdbase + ARM_GICR_CTLR);
  val_mmio_write(pe_rdbase + ARM_GICR_CTLR, gicr_ctrl_value & 0x0);
  gicr_ctrl_value = val_mmio_read(pe_rdbase + ARM_GICR_CTLR);

  /* Check that LPI Support is disabled*/
  if ((VAL_EXTRACT_BITS(gicr_ctrl_value, 0, 0) != 0) ||
      (VAL_EXTRACT_BITS(gicr_ctrl_value, 3, 3) != 0)) {
    val_print(ACS_PRINT_ERR, "\n       LPI is not disabled", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
    return;
  }

  num_its = val_gic_get_info(GIC_INFO_NUM_ITS);
  if (num_its == 0) {
    val_print(ACS_PRINT_DEBUG, "\n       No ITS, Skipping Test.", 0);
    val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
    return;
  }

  for (its_id = 0; its_id < num_its; its_id++) {
    /* Get ITS Base for current ITS */
    if (val_gic_its_get_base(its_id, &its_base)) {
      val_print(ACS_PRINT_ERR,
            "\n       Could not find ITS Base for its_id : 0x%x", its_id);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
      return;
    }

    /* Get GITS_CTRL value*/
    gits_ctrl_value = val_mmio_read(its_base + ARM_GITS_CTLR);
    /* Check that ITS is disabled*/
    if ((VAL_EXTRACT_BITS(gits_ctrl_value, 0, 0) != 0) ||
        (VAL_EXTRACT_BITS(gits_ctrl_value, 31, 31) != 1)) {
      val_print(ACS_PRINT_ERR, "\n       ITS is not disabled", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
      return;
    }
  }
  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
interface010_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
  /* execute payload, which will execute relevant functions on current and other PEs */
      payload(num_pe);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
