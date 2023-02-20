/** @file
 * Copyright (c) 2016-2021, 2023 Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/bsa_acs_common.h"
#include "val/include/val_interface.h"

#include "val/include/bsa_acs_gic.h"
#include "val/include/bsa_acs_gic_support.h"

#define TEST_NUM   (ACS_GIC_TEST_NUM_BASE + 5)
#define TEST_RULE  "B_GIC_05"
#define TEST_DESC  "Non-secure SGIs are implemented       "

#define  RD_FRAME_SIZE      0x10000

static
void
payload()
{
  // Assumptions: DS == 0; Execution State always NS
  // * check if GICD_CTLR.EnableGrp1NS is set (proceed if set; else FAIL)
  // * apply mask 0xFF to GICR_ISENABLER0[7:0] if affinity routing enabled
  // * ... apply to GICD_ISENABLER<0> otherwise
  // * result == 1 ? SUCCESS : FAIL

  uint32_t enable_grp1ns;
  uint32_t data;
  uint32_t are_ns = val_gic_get_info(GIC_INFO_AFFINITY_NS);
  uint32_t index, mpid;
  uint64_t pe_rdbase;

  mpid = val_pe_get_mpid();
  val_print(ACS_PRINT_DEBUG, "\n       PE MPIDR 0x%x", mpid);

  index = val_pe_get_index_mpid(mpid);

  // Location of the Group 1 NS enable bit depends on whether affinity routing is enabled
  if (are_ns)
      enable_grp1ns = (val_gic_get_info(GIC_INFO_ENABLE_GROUP1NS) & 0x2) >> 1;
  else
      enable_grp1ns = (val_gic_get_info(GIC_INFO_ENABLE_GROUP1NS) & 0x1);

  // Distributor must forward NS Group 1 interrupt
  if (!enable_grp1ns) {
    val_print(ACS_PRINT_ERR, "\n       Non-secure SGIs not forwarded", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }
  else {
    if (are_ns) {
        /* Derive RDbase for PE */
        pe_rdbase = val_gic_get_pe_rdbase(mpid);
        val_print(ACS_PRINT_DEBUG, "\n       PE RD base address %llx", pe_rdbase);
        if (pe_rdbase == 0) {
            val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
            return;
        }
        /* Read out the bottom 8 bits of GICR_ISENABLER0 */
        data = val_mmio_read(pe_rdbase + RD_FRAME_SIZE + GICR_ISENABLER) | 0xFFFF;

        val_mmio_write(pe_rdbase + RD_FRAME_SIZE + GICR_ISENABLER, data);
        data = val_mmio_read(pe_rdbase + RD_FRAME_SIZE + GICR_ISENABLER) | 0xFFFF;
        val_print(ACS_PRINT_DEBUG, "  data 0x%x", data);
        data = VAL_EXTRACT_BITS(data, 0, 7);
        if (data == 0xFF) {
            val_set_status(index, RESULT_PASS(TEST_NUM, 1));
            return;
        }
        else {
            val_print(ACS_PRINT_DEBUG,
                "\n       GICR_ISENABLER0: %X\n ", data);
            val_print(ACS_PRINT_ERR,
                "\n       INTID 0 - 7 not implemented as non-secure SGIs", 0);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
            return;
        }
    }
    else {
        // Read out the bottom 8 bits of GICD_ISENABLER<0> if ARE == 0
        data = (val_mmio_read(val_get_gicd_base() + GICD_ISENABLER) | 0xFFFF);
        val_mmio_write((val_get_gicd_base() + GICD_ISENABLER), data);
        data = VAL_EXTRACT_BITS(val_gic_get_info(GIC_INFO_SGI_NON_SECURE_LEGACY), 0, 7);
        if (data == 0xFF) {
            val_set_status(index, RESULT_PASS(TEST_NUM, 1));
            return;
        }
        else {
            val_print(ACS_PRINT_DEBUG,
                "\n       GICD_IENABLER<n>: %X\n ", data);
            val_print(ACS_PRINT_ERR,
                "\n       INTID 0 - 7 not implemented as non-secure SGIs", 0);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
            return;
        }
    }
  }
}

uint32_t
os_g005_entry(uint32_t num_pe)
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
