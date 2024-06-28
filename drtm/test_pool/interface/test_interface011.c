/** @file
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
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

#define TEST_NUM   (ACS_DRTM_INTERFACE_TEST_NUM_BASE  +  11)
#define TEST_RULE  ""
#define TEST_DESC  "Check GICR_PENDBASER when LPIs enabled"

extern GIC_ITS_INFO    *g_gic_its_info;

static
void
payload(uint32_t num_pe)
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint64_t pe_rdbase = 0;
  uint32_t gicr_ctrl_value, gicr_pendbaser, new_gicr_pendbaser;

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

  /* Check that LPI Support is enabled */
  if ((VAL_EXTRACT_BITS(gicr_ctrl_value, 0, 0) == 0) &&
      (VAL_EXTRACT_BITS(gicr_ctrl_value, 3, 3) == 0)) {
    val_print(ACS_PRINT_ERR, "\n       LPI is disabled", 0);
    val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
    return;
  }

  /* Get GICR_PENDBASER value */
  gicr_pendbaser = val_mmio_read(pe_rdbase + ARM_GICR_PENDBASER);

  /* Try to change GICR_PENDBASER value by adding 4KB to Phy Addr */
  new_gicr_pendbaser = gicr_pendbaser |
                       ((VAL_EXTRACT_BITS(gicr_pendbaser, 16, 51) + 0x1000) << 16);
  val_mmio_write(pe_rdbase + ARM_GICR_PENDBASER, new_gicr_pendbaser);

  /* Read back GICR_PENDBASER value and Fail if value is changed */
  new_gicr_pendbaser = val_mmio_read(pe_rdbase + ARM_GICR_PENDBASER);
  if (gicr_pendbaser != new_gicr_pendbaser) {
    val_print(ACS_PRINT_ERR, "\n       GICR_PENDBASER value is changed when LPI is enable", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
    return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
interface011_entry(uint32_t num_pe)
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
