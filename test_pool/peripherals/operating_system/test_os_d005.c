
/** @file
 * Copyright (c) 2016-2019, 2021 Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_peripherals.h"
#include "val/include/bsa_acs_pcie.h"

#define TEST_NUM   (ACS_PER_TEST_NUM_BASE + 6)
#define TEST_RULE  "B_PER_06"
#define TEST_DESC  "16550 compatible UART                 "

static
void
uart_reg_write(uint64_t uart_base, uint32_t offset, uint32_t width_mask, uint32_t data)
{
  if (width_mask & WIDTH_BIT8)
      *((volatile uint8_t *)(uart_base + offset)) = (uint8_t)data;

  if (width_mask & WIDTH_BIT16)
      *((volatile uint16_t *)(uart_base + offset)) = (uint16_t)data;

  if (width_mask & WIDTH_BIT32)
      *((volatile uint32_t *)(uart_base + offset)) = (uint32_t)data;

}

static
uint32_t
uart_reg_read(uint64_t uart_base, uint32_t offset, uint32_t width_mask)
{
  if (width_mask & WIDTH_BIT8)
      return *((volatile uint8_t *)(uart_base + offset));

  if (width_mask & WIDTH_BIT16)
      return *((volatile uint16_t *)(uart_base + offset));

  if (width_mask & WIDTH_BIT32)
      return *((volatile uint32_t *)(uart_base + offset));

  return 0;
}

static
void
payload()
{

  uint64_t count = val_peripheral_get_info(NUM_UART, 0);
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t interface_type;
  uint32_t baud_rate;
  uint32_t counter_freq;
  uint32_t ier_reg;
  uint32_t ier_scratch2;
  uint32_t ier_scratch3;
  uint32_t mcr_reg;
  uint32_t msr_status;
  uint32_t divisor;
  uint32_t uart_base;
  uint32_t lcr_reg;
  uint32_t lcr_scratch2;
  uint32_t lcr_scratch3;
  uint32_t skip_test = 0;
  uint32_t test_fail = 0;

  if (count == 0) {
      val_print(ACS_PRINT_ERR, "\n       No UART defined by Platform      ", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
      return;
  }

  while (count != 0) {
      interface_type = val_peripheral_get_info(UART_INTERFACE_TYPE, count - 1);
      if (interface_type == COMPATIBLE_FULL_16550
           || interface_type == COMPATIBLE_SUBSET_16550
           || interface_type == COMPATIBLE_GENERIC_16550)
      {
          skip_test = 1;
          val_print(ACS_PRINT_DEBUG,
              "\n         UART 16550 found with instance: %x",
              count - 1);

          /* Check the I/O base address */
          uart_base = val_peripheral_get_info(UART_BASE0, count - 1);
          if (uart_base == 0)
          {
              val_print(ACS_PRINT_ERR, "\n         UART base must be specified"
                                       " for instance: %x", count - 1);
              val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
              return;
          }

          /* Check the Baudrate from the hardware map */
          baud_rate = val_peripheral_get_info(UART_BAUDRATE, count - 1);
          if (baud_rate < BAUDRATE_9600 || baud_rate > BAUDRATE_115200)
          {
              if (baud_rate != 0)
              {
                  val_print(ACS_PRINT_ERR, "\n         Baud rate %d outside"
                                           " supported range", baud_rate);
                  val_print(ACS_PRINT_ERR, " for instance %x", count - 1);
                  test_fail = 1;
              }
          }

          /* Check the baudrate in the UART register. Obtained the divisor by
           * enabling the divisor latch access and reading the divisor latch
           * byte1 and byte2. Divisor = system clock speed / (16 * baudrate)
           */
          uart_reg_write(uart_base, LCR, WIDTH_BIT8, DIVISOR_LATCH_EN);
          divisor = uart_reg_read(uart_base, DIVISOR_LATCH_BYTE1, WIDTH_BIT8);
          divisor |= uart_reg_read(uart_base, DIVISOR_LATCH_BYTE2, WIDTH_BIT8) << 8;
          uart_reg_write(uart_base, LCR, WIDTH_BIT8, DIVISOR_LATCH_DIS);
          counter_freq = val_timer_get_info(TIMER_INFO_CNTFREQ, 0);
          baud_rate = counter_freq / (16 * divisor);
          if (baud_rate < BAUDRATE_1200 || baud_rate > BAUDRATE_115200)
          {
              val_print(ACS_PRINT_ERR,
              "\n        Baud rate %d outside supported range",
              baud_rate);
              val_print(ACS_PRINT_ERR, " for instance %x", count - 1);
              test_fail = 1;
          }

          /* Check the read/write property of Line Control Register */
          lcr_reg = uart_reg_read(uart_base, LCR, WIDTH_BIT8);
          uart_reg_write(uart_base, LCR, WIDTH_BIT8, 0);
          lcr_scratch2 = uart_reg_read(uart_base, LCR, WIDTH_BIT8);
          uart_reg_write(uart_base, LCR, WIDTH_BIT8, 0xFF);
          lcr_scratch3 = uart_reg_read(uart_base, LCR, WIDTH_BIT8);
          uart_reg_write(uart_base, LCR, WIDTH_BIT8, lcr_reg);
          if ((lcr_scratch2 != 0) || (lcr_scratch3 != 0xFF))
          {
              val_print(ACS_PRINT_ERR, "\n   LCR register are not read/write"
                                       " for instance: %x", count - 1);
              test_fail = 1;
          }

          /* Check the read/write property of Interrupt Enable Register */
          ier_reg = uart_reg_read(uart_base, IER, WIDTH_BIT8);
          uart_reg_write(uart_base, IER, WIDTH_BIT8, 0);
          ier_scratch2 = uart_reg_read(uart_base, IER, WIDTH_BIT8) & 0xF;
          uart_reg_write(uart_base, IER, WIDTH_BIT8, 0xF);
          ier_scratch3 = uart_reg_read(uart_base, IER, WIDTH_BIT8);
          uart_reg_write(uart_base, IER, WIDTH_BIT8, ier_reg);
          if ((ier_scratch2 != 0) || (ier_scratch3 != 0xF))
          {
              val_print(ACS_PRINT_ERR, "\n   IER register[0:3] are not read/write"
                                       " for instance: %x", count - 1);
              test_fail = 1;
          }

          /* Check if UART is really present using loopback test mode */
          mcr_reg = uart_reg_read(uart_base, MCR, WIDTH_BIT8);
          uart_reg_write(uart_base, MCR, WIDTH_BIT8, MCR_LOOP | 0xA);
          msr_status = uart_reg_read(uart_base, MSR, WIDTH_BIT8);
          uart_reg_write(uart_base, MCR, WIDTH_BIT8, mcr_reg);
          if (msr_status != CTS_DCD_EN)
          {
              val_print(ACS_PRINT_ERR, "\n   Loopback test mode failed"
                                       " for instance: %x", count - 1);
              test_fail = 1;
          }

      }
      count--;
  }

  if (!skip_test)
      val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
  else if (test_fail)
      val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
  else
      val_set_status(index, RESULT_PASS(TEST_NUM, 1));

  return;
}

/**
  @brief     Read PCI CFG space class and sub-class register
             to determine the USB interface version
**/
uint32_t
os_d005_entry(uint32_t num_pe)
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
