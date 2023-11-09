/** @file
 * Copyright (c) 2016-2018,2021, 2023, Arm Limited or its affiliates. All rights reserved.
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


#define TEST_NUM   (ACS_PE_TEST_NUM_BASE  +  10)
#define TEST_RULE  "B_PE_10"
#define TEST_DESC  "Check PMU Overflow signal             "

static uint32_t int_id;
static void *branch_to_test;

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

static
void
set_pmu_overflow()
{
  uint64_t pmcr;

  //Initializing the state of overflow status and interrupt request registers
  val_pe_reg_write(PMINTENCLR_EL1, 0xFFFFFFFF);
  val_pe_reg_write(PMOVSCLR_EL0, 0xFFFFFFFF);

  //Sequence to generate PMUIRQ
  pmcr = val_pe_reg_read(PMCR_EL0);
  val_pe_reg_write(PMCR_EL0, pmcr|0x1);

  val_pe_reg_write(PMINTENSET_EL1, 0x1);
  val_pe_reg_write(PMOVSSET_EL0, 0x1);

}


static
void
isr()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  /* We received our interrupt, so disable PMUIRQ from generating further interrupts */
  val_pe_reg_write(PMOVSCLR_EL0, 0x1);
  val_print(ACS_PRINT_INFO, "\n Received PMUIRQ ", 0);
  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
  val_gic_end_of_interrupt(int_id);

  return;
}


static
void
payload()
{
  uint32_t timeout = 0x100000;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t data = 0;

  /* Check ID_AA64DFR0_EL1[11:8] for PMUver */
  data = VAL_EXTRACT_BITS(val_pe_reg_read(ID_AA64DFR0_EL1), 8, 11);
  if ((data == 0x0) || (data == 0xF)) {
      /* PMUver not implemented, Skipping. */
      val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
      return;
  }

  int_id = val_pe_get_pmu_gsiv(index);
  if (int_id == 0) {
      /* PMU interrupt number not updated */
      val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
      return;
  }

  if (val_gic_install_isr(int_id, isr)) {
      val_print(ACS_PRINT_ERR, "\n       GIC Install Handler Failed...", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
      return;
  }

  val_pe_install_esr(EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS, esr);
  val_pe_install_esr(EXCEPT_AARCH64_SERROR, esr);

  branch_to_test = &&exception_taken;

  set_pmu_overflow();

  while ((--timeout > 0) && \
         (IS_RESULT_PENDING(val_get_status(index))));

exception_taken:
  if (timeout == 0) {
      val_print(ACS_PRINT_ERR, "\n       Interrupt not recieved within timeout", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
  }
}

/**
  @brief  Install ISR and verify PMU Overflow Interrupt by programming System registers
**/
uint32_t
os_c010_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;
  num_pe = 1;

  status = val_initialize_test(TEST_NUM, TEST_DESC, val_pe_get_num());
  if (status != ACS_STATUS_SKIP)
      /* execute payload on present PE and then execute on other PE */
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
