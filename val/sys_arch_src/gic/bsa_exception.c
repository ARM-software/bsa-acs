/** @file
 * Copyright (c) 2021, 2023, Arm Limited or its affiliates. All rights reserved.
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

#include "bsa_exception.h"
#include "gic.h"
#include "include/bsa_acs_val.h"
#include "include/bsa_acs_common.h"
#include "include/bsa_acs_pe.h"

typedef void (*bsa_fp) (uint64_t, void *);
bsa_fp g_esr_handler[4];

typedef void (*irq_handler) (void);
irq_handler g_intr_handler[NUM_ARM_MAX_INTERRUPT];

static void default_irq_handler(uint64_t exception_type, void *context)
{
  uint32_t ack_interrupt;
  uint32_t iar_ack_val;

  (void) exception_type;
  (void) context;

  iar_ack_val = val_bsa_gic_acknowledgeInterrupt();
  ack_interrupt = iar_ack_val & 0xFFFFFF;

  /* Call Interrupt handler if installed otherwise print err. */
  if (g_intr_handler[ack_interrupt]) {
      g_intr_handler[ack_interrupt]();
  } else {
      val_print(ACS_PRINT_ERR,
                "\n       GIC_INIT: Unregistered Handler for the interrupt_id : 0x%x",
                ack_interrupt);
  }

  /* End Of Interrupt */
  val_bsa_gic_endofInterrupt(ack_interrupt);

  return;
}

void bsa_gic_vector_table_init(void)
{
  val_print(ACS_PRINT_DEBUG, "  GIC_INIT: Setting Up Vector Table...\n", 0);

  /* Setting Up Vector Table */
  bsa_gic_set_el2_vector_table();

  /* Install Default handler for IRQ */
  val_gic_bsa_install_esr(EXCEPT_AARCH64_IRQ, default_irq_handler);
}

uint32_t val_gic_bsa_install_isr(uint32_t interrupt_id, void (*isr)(void))
{
  /* Step 1: Disable Interrupt before registering Handler */
  val_bsa_gic_disableInterruptSource(interrupt_id);

  /* Step 2: Register ISR for the particular interrupt */
  g_intr_handler[interrupt_id] = (irq_handler) isr;

  /* Step 3: Enable Interrupt */
  val_bsa_gic_enableInterruptSource(interrupt_id);

  return 0;
}

void val_gic_bsa_install_esr(uint32_t exception_type, void (*esr)(uint64_t, void *))
{
  g_esr_handler[exception_type] = (bsa_fp) esr;
}

uint32_t common_exception_handler(uint32_t exception_type)
{
  val_print(ACS_PRINT_INFO, "\n       GIC_INIT: In Exception Handler Type : %x", exception_type);

  /* Call Handler for exception, Handler would have
   * already been installed using install_esr call
   */
  g_esr_handler[exception_type](exception_type, NULL);

  val_print(ACS_PRINT_INFO, "\n       GIC_INIT: Common Handler, FAR = %x", bsa_gic_get_far());
  val_print(ACS_PRINT_INFO, "\n       GIC_INIT: Common Handler, ESR = %x", bsa_gic_get_esr());

  /* If ELR is updated inside the handler then skip the elr update in assembly handler
   * Return 1 else return 0
  */
  if (exception_type == EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS)
    return 1;
  else
    return 0;
}
