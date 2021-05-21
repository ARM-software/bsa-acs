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
#include "val/include/val_interface.h"

#include "val/include/bsa_acs_timer.h"

#define TEST_NUM   (ACS_TIMER_TEST_NUM_BASE + 3)
#define TEST_DESC  "B_TIME_07,B_TIME_10: Memory mapped timer check"

#define ARBIT_VALUE      0xA000

static
void
payload()
{

  uint64_t cnt_ctl_base, cnt_base_n;
  uint32_t data, status, ns_timer = 0;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint64_t timer_num = val_timer_get_info(TIMER_INFO_NUM_PLATFORM_TIMERS, 0);

  if (!timer_num) {
      val_print(ACS_PRINT_WARN, "\n       No System timers are defined      ", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 0x1));
      return;
  }

  while (timer_num) {
      --timer_num;  //array index starts from 0, so subtract 1 from count

      if (val_timer_get_info(TIMER_INFO_IS_PLATFORM_TIMER_SECURE, timer_num))
          continue;    //Skip Secure Timer

      ns_timer++;
      cnt_ctl_base = val_timer_get_info(TIMER_INFO_SYS_CNTL_BASE, timer_num);
      cnt_base_n   = val_timer_get_info(TIMER_INFO_SYS_CNT_BASE_N, timer_num);

      if (cnt_ctl_base == 0) {
          val_print(ACS_PRINT_WARN, "\n       CNTCTL BASE is zero             ", 0);
          val_set_status(index, RESULT_SKIP(TEST_NUM, 0x2));
          return;
      }
      //Read CNTACR to determine whether access permission from NS state is permitted
      status = val_timer_skip_if_cntbase_access_not_allowed(timer_num);
      if (status == ACS_STATUS_SKIP) {
          val_print(ACS_PRINT_WARN,
                    "\n       Security doesn't allow access to timer registers      ", 0);
          val_set_status(index, RESULT_SKIP(TEST_NUM, 02));
          return;
      }

      // Read write check for a read only register CNTTIDR
      data = val_mmio_read(cnt_ctl_base + CNTTIDR);
      val_mmio_write(cnt_ctl_base + CNTTIDR, 0xFFFFFFFF);
      if (data != val_mmio_read(cnt_ctl_base + CNTTIDR)) {
          val_print(ACS_PRINT_ERR, "\n       Read-write check failed for"
              " CNTCTLBase.CNTTIDR, expected value %x ", data);
          val_set_status(index, RESULT_FAIL(TEST_NUM, 0x2));
          return;
      }

      if (cnt_base_n == 0) {
          val_print(ACS_PRINT_WARN, "\n      CNT_BASE_N is zero                 ", 0);
          val_set_status(index, RESULT_SKIP(TEST_NUM, 0x3));
          return;
      }

      data = val_mmio_read(cnt_base_n + CNTPCT);
      // Writes to Read-Only registers should be ignored
      val_mmio_write(cnt_base_n + CNTPCT, data - ARBIT_VALUE);
      if (val_mmio_read(cnt_base_n + CNTPCT) < data) {
          val_set_status(index, RESULT_FAIL(TEST_NUM, 0x4));
          val_print(ACS_PRINT_ERR, "\n      CNTBaseN offset 0 should be read-only ", 0);
          return;
      }

      /* Write Read check for RW register*/
      data = 0x3;
      val_mmio_write(cnt_base_n + CNTP_CTL, data);
      if (data != (val_mmio_read(cnt_base_n + CNTP_CTL) & 0x3)) {
          val_print(ACS_PRINT_ERR, "\n       Read-write check failed for "
              "CNTBaseN.CNTP_CTL, expected value %x ", data);
          val_print(ACS_PRINT_ERR, "\n       Read value %x ", val_mmio_read(cnt_base_n + CNTP_CTL));
          val_set_status(index, RESULT_FAIL(TEST_NUM, 0x5));
          val_mmio_write(cnt_base_n + CNTP_CTL, 0x0); // Disable the timer before return
          return;
      }
      val_mmio_write(cnt_base_n + CNTP_CTL, 0x0); // Disable timer

      /* Write a random value to RW reg and verify*/
      data = 0xFF00FF00;
      val_mmio_write(cnt_base_n + CNTP_CVAL_LOW, data);
      if (data != val_mmio_read(cnt_base_n + CNTP_CVAL_LOW)) {
          val_print(ACS_PRINT_ERR, "\n       Read-write check failed for "
              "CNTBaseN.CNTP_CVAL[31:0], expected value %x ", data);
          val_set_status(index, RESULT_FAIL(TEST_NUM, 0x6));
          return;
      }

      /* Write a random value to RW reg and verify*/
      data = 0x00FF00FF;
      val_mmio_write(cnt_base_n + CNTP_CVAL_HIGH, data);
      if (data != val_mmio_read(cnt_base_n + CNTP_CVAL_HIGH)) {
          val_print(ACS_PRINT_ERR, "\n       Read-write check failed for"
              " CNTBaseN.CNTP_CVAL[63:32], expected value %x ", data);
          val_set_status(index, RESULT_FAIL(TEST_NUM, 0x7));
          return;
      }

      val_set_status(index, RESULT_PASS(TEST_NUM, 01));
  }

  if (!ns_timer) {
      val_print(ACS_PRINT_WARN, "\n       No non-secure systimer implemented", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 02));
      return;
  }

}

uint32_t
os_t003_entry(uint32_t num_pe)
{
  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This Timer test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe);

  val_report_status(0, BSA_ACS_END(TEST_NUM));

  return status;
}
