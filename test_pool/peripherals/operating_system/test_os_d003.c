
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

#include "val/include/bsa_acs_peripherals.h"
#include "val/include/bsa_acs_gic.h"

#define TEST_NUM   (ACS_PER_TEST_NUM_BASE + 3)
#define TEST_RULE  "B_PER_05"
#define TEST_DESC  "Check Arm BSA UART register offsets   "
#define TEST_NUM1  (ACS_PER_TEST_NUM_BASE + 4)
#define TEST_RULE1 "B_PER_06, B_PER_07"
#define TEST_DESC1 "Check Arm GENERIC UART Interrupt      "

static uint64_t l_uart_base;
static uint32_t int_id;
static void *branch_to_test;
static uint32_t test_fail;

static
void
esr(uint64_t interrupt_type, void *context)
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  /* Update the ELR to point to next instrcution */
  val_pe_update_elr(context, (uint64_t)branch_to_test);

  val_print(ACS_PRINT_ERR, "\n       Error : Received Exception of type %d", interrupt_type);
  val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
}

uint32_t
uart_reg_read(uint32_t offset, uint32_t width_mask)
{
  if (width_mask & WIDTH_BIT8)
      return *((volatile uint8_t *)(l_uart_base + offset));

  if (width_mask & WIDTH_BIT16)
      return *((volatile uint16_t *)(l_uart_base + offset));

  if (width_mask & WIDTH_BIT32)
      return *((volatile uint32_t *)(l_uart_base + offset));

  return 0;
}

void
uart_reg_write(uint32_t offset, uint32_t width_mask, uint32_t data)
{
  if (width_mask & WIDTH_BIT8)
      *((volatile uint8_t *)(l_uart_base + offset)) = (uint8_t)data;

  if (width_mask & WIDTH_BIT16)
      *((volatile uint16_t *)(l_uart_base + offset)) = (uint16_t)data;

  if (width_mask & WIDTH_BIT32)
      *((volatile uint32_t *)(l_uart_base + offset)) = (uint32_t)data;

}

void
uart_enable_txintr()
{
  uint32_t data;

  /* Enable TX interrupt by setting mask bit[5] in UARTIMSC */
  data = uart_reg_read(BSA_UARTIMSC, WIDTH_BIT32);
  data = data | (1<<5);
  uart_reg_write(BSA_UARTIMSC, WIDTH_BIT32, data);
}

void
uart_disable_txintr()
{
  uint32_t data;

  /* Disable TX interrupt by clearing mask bit[5] in UARTIMSC */
  data = uart_reg_read(BSA_UARTIMSC, WIDTH_BIT32);
  data = data & (~(1<<5));
  uart_reg_write(BSA_UARTIMSC, WIDTH_BIT32, data);

}


static
void
isr()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  uart_disable_txintr();
  val_print(ACS_PRINT_DEBUG, "\n       Received interrupt on %d     ", int_id);
  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
  val_gic_end_of_interrupt(int_id);
}

/* Write to a read only register*/
uint32_t
validate_register_access(uint32_t offset, uint32_t width)
{
  if (width & WIDTH_BIT8) {
      uart_reg_write(offset, WIDTH_BIT8, 0xFF);
  }
  if (width & WIDTH_BIT16) {
      uart_reg_write(offset, WIDTH_BIT16, 0xFFFF);
  }
  if (width & WIDTH_BIT32) {
      uart_reg_write(offset, WIDTH_BIT32, 0xFFFFFFFF);
  }
  return ACS_STATUS_PASS;
}

static
void
payload()
{

  uint32_t count = val_peripheral_get_info(NUM_UART, 0);
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t interface_type;

  val_pe_install_esr(EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS, esr);
  val_pe_install_esr(EXCEPT_AARCH64_SERROR, esr);

  branch_to_test = &&exception_taken;
  if (count == 0) {
      val_print(ACS_PRINT_ERR, "\n       No UART defined by Platform      ", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
      return;
  }
  val_set_status(index, RESULT_SKIP(TEST_NUM, 1));

  while (count != 0) {
      interface_type = val_peripheral_get_info(UART_INTERFACE_TYPE, count - 1);
      if (interface_type != COMPATIBLE_FULL_16550
           && interface_type != COMPATIBLE_SUBSET_16550
           && interface_type != COMPATIBLE_GENERIC_16550)
      {
          l_uart_base = val_peripheral_get_info(UART_BASE0, count - 1);
          if (l_uart_base == 0) {
              val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
              return;
          }

          val_peripheral_uart_setup();

          /*Make sure  write to a read only register doesn't cause any exceptions*/
          validate_register_access(BSA_UARTFR, WIDTH_BIT8 | WIDTH_BIT16 | WIDTH_BIT32);
          validate_register_access(BSA_UARTRIS, WIDTH_BIT16 | WIDTH_BIT32);
          validate_register_access(BSA_UARTMIS, WIDTH_BIT16 | WIDTH_BIT32);

          val_set_status(index, RESULT_PASS(TEST_NUM, 1));
      }

      count--;
  }
exception_taken:
  return;
}

static
void
payload1()
{
  uint32_t count = val_peripheral_get_info(NUM_UART, 0);
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t timeout = TIMEOUT_LARGE;
  uint32_t interface_type;

  if (count == 0) {
      val_print(ACS_PRINT_ERR, "\n       No UART defined by Platform      ", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM1, 1));
      return;
  }
  val_set_status(index, RESULT_SKIP(TEST_NUM1, 1));
  while (count != 0) {
      timeout = TIMEOUT_MEDIUM;
      int_id    = val_peripheral_get_info(UART_GSIV, count - 1);
      interface_type = val_peripheral_get_info(UART_INTERFACE_TYPE, count - 1);
      l_uart_base = val_peripheral_get_info(UART_BASE0, count - 1);
      if (interface_type != COMPATIBLE_FULL_16550
           && interface_type != COMPATIBLE_SUBSET_16550
           && interface_type != COMPATIBLE_GENERIC_16550) {

          /* If Interrupt ID is available, check for interrupt generation */
          if (int_id != 0x0) {
              /* PASS will be set from ISR */
              val_set_status(index, RESULT_PENDING(TEST_NUM1));

              if (val_gic_install_isr(int_id, isr)) {
                 val_print(ACS_PRINT_ERR, "\n       GIC Install Handler Failed...", 0);
                 val_set_status(index, RESULT_FAIL(TEST_NUM1, 2));
                 return;
              }

              val_peripheral_uart_setup();
              uart_enable_txintr();
              val_print_raw(l_uart_base, g_print_level,
                            "\n       Test Message                          ", 0);

              while ((--timeout > 0) && (IS_RESULT_PENDING(val_get_status(index)))) {
              };

              if (timeout == 0) {
                 val_print(ACS_PRINT_ERR,
                 "\n       Did not receive UART interrupt on %d  ",
                 int_id);
                 test_fail++;
              }
          } else {
              val_set_status(index, RESULT_SKIP(TEST_NUM1, 2));
          }
      }
      count--;
  }

  if (test_fail)
    val_set_status(index, RESULT_FAIL(TEST_NUM1, 3));
  else
    val_set_status(index, RESULT_PASS(TEST_NUM1, 2));

  return;
}


/**
   @brief    Verify UART registers for Read-only bits and also
             enable interrupt generation
**/
uint32_t
os_d003_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  if (!status) {
      status = val_initialize_test(TEST_NUM1, TEST_DESC1, val_pe_get_num());
      if (status != ACS_STATUS_SKIP)
          val_run_test_payload(TEST_NUM1, num_pe, payload1, 0);

      /* get the result from all PE and check for failure */
      status = val_check_for_error(TEST_NUM1, num_pe, TEST_RULE1);
      val_report_status(0, BSA_ACS_END(TEST_NUM1), NULL);
  }

  return status;
}
