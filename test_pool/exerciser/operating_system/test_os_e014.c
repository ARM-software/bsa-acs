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
#include "val/include/bsa_acs_val.h"
#include "val/include/val_interface.h"

#include "val/include/bsa_acs_pcie.h"
#include "val/include/bsa_acs_memory.h"
#include "val/include/bsa_acs_pcie_enumeration.h"
#include "val/include/bsa_acs_exerciser.h"

#define TEST_NUM   (ACS_EXERCISER_TEST_NUM_BASE + 14)
#define TEST_RULE  "PCI_PP_02"
#define TEST_DESC  "P2P transactions must not deadlock    "

uint32_t
get_target_exer_bdf(uint32_t req_rp_bdf, uint32_t *tgt_e_bdf,
                    uint32_t *tgt_rp_bdf, uint64_t *bar_base)
{

  uint32_t erp_bdf;
  uint32_t e_bdf;
  uint32_t instance;
  uint32_t req_rp_ecam_index;
  uint32_t erp_ecam_index;
  uint32_t status;

  instance = val_exerciser_get_info(EXERCISER_NUM_CARDS);

  while (instance-- != 0)
  {
      /* if init fail moves to next exerciser */
      if (val_exerciser_init(instance))
          continue;

      e_bdf = val_exerciser_get_bdf(instance);

      /* Read e_bdf BAR Register to get the Address to perform P2P */
      /* If No BAR Space, continue */
      val_pcie_get_mmio_bar(e_bdf, bar_base);
      if (*bar_base == 0)
          continue;

      /* Get RP of the exerciser */
      if (val_pcie_get_rootport(e_bdf, &erp_bdf))
          continue;

      if (req_rp_bdf != erp_bdf)
      {
          status = val_pcie_get_ecam_index(req_rp_bdf, &req_rp_ecam_index);
          if (status)
          {
             val_print(ACS_PRINT_ERR, "\n       Error Ecam index for req RP BDF: 0x%x", req_rp_bdf);
             goto test_fail;
          }

          status = val_pcie_get_ecam_index(erp_bdf, &erp_ecam_index);
          if (status)
          {
             val_print(ACS_PRINT_ERR, "\n       Error Ecam index for tgt RP BDF: 0x%x", erp_bdf);
             goto test_fail;
          }

          if (req_rp_ecam_index != erp_ecam_index)
              continue;

          *tgt_e_bdf = e_bdf;
          *tgt_rp_bdf = erp_bdf;

          /* Enable Bus Master Enable */
          val_pcie_enable_bme(e_bdf);
          /* Enable Memory Space Access */
          val_pcie_enable_msa(e_bdf);

          return ACS_STATUS_PASS;
      }
  }

test_fail:
  /* Return failure if No Such Exerciser Found */
  *tgt_e_bdf = 0;
  *tgt_rp_bdf = 0;
  *bar_base = 0;
  return ACS_STATUS_FAIL;
}

uint32_t
check_p2p_transaction(uint32_t req_instance, uint64_t bar_base)
{
  /* P2P transaction must fail */
  val_exerciser_set_param(DMA_ATTRIBUTES, (uint64_t)bar_base, 1, req_instance);
  val_exerciser_ops(START_DMA, EDMA_TO_DEVICE, req_instance);

  return ACS_STATUS_PASS;
}

static
void
payload(void)
{

  uint32_t status;
  uint32_t index;
  uint32_t req_e_bdf;
  uint32_t req_rp_bdf;
  uint32_t tgt_e_bdf;
  uint32_t tgt_rp_bdf;
  uint32_t instance;
  uint32_t test_skip;
  uint64_t bar_base;

  test_skip = 1;
  index = val_pe_get_index_mpid(val_pe_get_mpid());
  instance = val_exerciser_get_info(EXERCISER_NUM_CARDS);

  /* Check If PCIe Hierarchy supports P2P. */
  if (!val_pcie_p2p_support())
  {
    val_print(ACS_PRINT_DEBUG, "\n       P2P is supported, Skipping Test", 0);
    val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
    return;
  }

  while (instance-- != 0)
  {

      /* if init fail moves to next exerciser */
      if (val_exerciser_init(instance))
          continue;

      req_e_bdf = val_exerciser_get_bdf(instance);
      val_print(ACS_PRINT_DEBUG, "\n       Requester exerciser BDF - 0x%x", req_e_bdf);

      /* Get RP of the exerciser */
      if (val_pcie_get_rootport(req_e_bdf, &req_rp_bdf))
          continue;

      /* Find another exerciser on other rootport,
         Skip the current exerciser if no such exerciser if found */
      if (get_target_exer_bdf(req_rp_bdf, &tgt_e_bdf, &tgt_rp_bdf, &bar_base))
          continue;

      val_print(ACS_PRINT_DEBUG, "\n       Target exerciser BDF - 0x%x", tgt_e_bdf);
      test_skip = 0;

      /* Check if P2P transaction causes any deadlock */
      status = check_p2p_transaction(instance, bar_base);
      if (status)
      {
          val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
          return;
      }

      /* Clear Error Status Bits */
      val_pcie_clear_device_status_error(req_rp_bdf);
      val_pcie_clear_sig_target_abort(req_rp_bdf);
    }

    if (test_skip) {
        val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
        return;
    }

  /* Pass Test */
  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_e014_entry(void)
{

  uint32_t status = ACS_STATUS_FAIL;

  uint32_t num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
