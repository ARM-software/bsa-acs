/** @file
 * Copyright (c) 2016-2018,2021 Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/bsa_acs_pe.h"

#define TEST_NUM    (ACS_PE_TEST_NUM_BASE  +  11)
#define TEST_DESC  "B_PE_11:  Check num of Breakpoints & type  "

static
void
payload()
{
  uint64_t data = 0;
  int32_t i, breakpointcount;
  uint32_t bt, addr_breakpoint = 0;
  uint32_t contextid_breakpoint = 0;
  uint32_t vmid_breakpoint = 0;
  uint32_t pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

  data = val_pe_reg_read(ID_AA64DFR0_EL1);

  breakpointcount = VAL_EXTRACT_BITS(data, 12, 15);
  if (breakpointcount < 5) { //bits 15:12 for Number of breakpoints - 1
      val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
      return;
  }

  for (i = 0; i <= breakpointcount; i++) {

      data = val_pe_reg_read(i + DBGBCR0_EL1);
      /* breakpoint type : bits 23:20
       * Shift by 1 to get only 3 bit to ignore LSB*/
      bt = (((VAL_EXTRACT_BITS(data, 20, 23)) >> 1) & 0x7);

      /* BT is 0b000x: virtual address*/
      if (bt == 0x00)
        addr_breakpoint++;

      /* BT is 0b001x, 0b011x, or 0b110x: Single Context ID.
         BT is 0b111x: Two Context ID values*/
      if (bt == 0x01 || bt == 0x03 || bt == 0x06 || bt == 0x07)
        contextid_breakpoint++;

      /*  BT is 0b100x: VMID*/
      if (bt == 0x4)
        vmid_breakpoint++;

      /*  BT is 0b101x: VMID and a Context ID */
      if (bt == 0x5) {
          vmid_breakpoint++;
          contextid_breakpoint++;
      }
  }

  if ((addr_breakpoint >= 2) || (contextid_breakpoint >= 2) || (vmid_breakpoint >= 2))
      val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));
  else
      val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));

  return;

}

/**
  @brief   Check for the number of breakpoints available
**/
uint32_t
os_c011_entry(uint32_t num_pe)
{
  uint32_t status = ACS_STATUS_FAIL;

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      /* execute payload on present PE and then execute on other PE */
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe);

  val_report_status(0, BSA_ACS_END(TEST_NUM));

  return status;
}

