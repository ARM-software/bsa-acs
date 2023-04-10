/** @file
 * Copyright (c) 2016-2018, 2021, 2023 Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_smmu.h"
#include "val/include/bsa_acs_pe.h"

#define TEST_NUM   (ACS_SMMU_TEST_NUM_BASE + 2)
#define TEST_RULE  "B_SMMU_02"
#define TEST_DESC  "Check SMMU Granule Support            "

static
void
payload()
{
  uint64_t data_pe_mmfr0, data_smmu_idr;
  uint32_t num_smmu;
  uint32_t is_smmu_4k = 0;
  uint32_t is_smmu_16k = 0;
  uint32_t is_smmu_64k = 0;
  uint32_t index;
  uint64_t data_oas = 0;

  index = val_pe_get_index_mpid(val_pe_get_mpid());

  num_smmu = val_smmu_get_info(SMMU_NUM_CTRL, 0);
  if (num_smmu == 0) {
      val_print(ACS_PRINT_ERR, "\n       No SMMU Controllers are discovered                  ", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
      return;
  }

  data_pe_mmfr0 = val_pe_reg_read(ID_AA64MMFR0_EL1);

  while (num_smmu--) {

      if (val_smmu_get_info(SMMU_CTRL_ARCH_MAJOR_REV, num_smmu) == 2) {
          data_smmu_idr = val_smmu_read_cfg(SMMUv2_IDR2, num_smmu);
          is_smmu_4k = VAL_EXTRACT_BITS(data_smmu_idr, 12, 12);
          is_smmu_16k = VAL_EXTRACT_BITS(data_smmu_idr, 13, 13);
          is_smmu_64k = VAL_EXTRACT_BITS(data_smmu_idr, 14, 14);
      } else {
          data_smmu_idr = val_smmu_read_cfg(SMMUv3_IDR5, num_smmu);
          is_smmu_4k = VAL_EXTRACT_BITS(data_smmu_idr, 4, 4);
          is_smmu_16k = VAL_EXTRACT_BITS(data_smmu_idr, 5, 5);
          is_smmu_64k = VAL_EXTRACT_BITS(data_smmu_idr, 6, 6);
          data_oas = VAL_EXTRACT_BITS(val_smmu_read_cfg(SMMUv3_IDR5, num_smmu), 0, 2);
      }

      /* If PE Supports 4KB granule size and implements FEAT_LPA2
       * then TGran4 == 0x1 or TGran4_2 == 0x3*/
      if ((VAL_EXTRACT_BITS(data_pe_mmfr0, 28, 31) == 0x1) ||
          (VAL_EXTRACT_BITS(data_pe_mmfr0, 40, 43) == 0x3)) {
          /*PE FEAT_LPA2 is implemeted, SMMU must support 4KB granule size and 52-bit
           * addressing. i.e data_oas == 0x6*/
          if ((is_smmu_4k != 1) && (data_oas != 0x6)) {
              val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
              val_print(ACS_PRINT_ERR, "\n       PE supports 4kB granule and FEAT_LPA2 is "
                                       "implemented, but SMMU %x does not support 52 bit "
                                       "addressing", num_smmu);
              return;
          }
      }

      /* If PE supports 16KB granule size and implements FEAT_LPA2
       * then TGran16 == 0x2 or TGran16_2 == 0x3 */
      if ((VAL_EXTRACT_BITS(data_pe_mmfr0, 20, 23) == 0x2) ||
          (VAL_EXTRACT_BITS(data_pe_mmfr0, 32, 35) == 0x3)) {
          /*PE FEAT_LPA2 is implemeted, SMMU must support 16KB granule size and 52-bit
           * addressing. i.e data_oas == 0x6*/
          if ((is_smmu_16k != 1) && (data_oas != 0x6)) {
              val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
              val_print(ACS_PRINT_ERR, "\n       PE supports 16kB granule and FEAT_LPA2 is "
                                       "implemented, but SMMU %x does not support 52 bit "
                                       " addressing", num_smmu);
              return;
          }
      }

      /* If PE Supports 4KB Gran, (TGran4 == 0) or (TGran4_2 == 0x2) */
      if ((VAL_EXTRACT_BITS(data_pe_mmfr0, 28, 31) == 0x0) ||
          (VAL_EXTRACT_BITS(data_pe_mmfr0, 40, 43) == 0x2)) {
          if (is_smmu_4k != 1) {
              val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
              val_print(ACS_PRINT_ERR, "\n       PE supports 4kB granules, "
                                       "but SMMU %x does not", num_smmu);
              return;
          }
      }

      /* If PE Supports 16KB Gran, (TGran16 == 1) or (TGran16_2 == 0x2) */
      if ((VAL_EXTRACT_BITS(data_pe_mmfr0, 20, 23) == 0x1) ||
          (VAL_EXTRACT_BITS(data_pe_mmfr0, 32, 35) == 0x2)) {
          if (is_smmu_16k != 1) {
              val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
              val_print(ACS_PRINT_ERR, "\n       PE supports 16kB granules, "
                                       "but SMMU %x does not", num_smmu);
              return;
          }
      }

      /* If PE Supports 64KB Gran, (TGran64 == 0) or (TGran64_2 == 0x2) */
      if ((VAL_EXTRACT_BITS(data_pe_mmfr0, 24, 27) == 0x0) ||
          (VAL_EXTRACT_BITS(data_pe_mmfr0, 36, 39) == 0x2)) {
          if (is_smmu_64k != 1) {
              val_set_status(index, RESULT_FAIL(TEST_NUM, 5));
              val_print(ACS_PRINT_ERR, "\n       PE supports 64kB granules, "
                                       "but SMMU %x does not", num_smmu);
              return;
          }
      }
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_i002_entry(uint32_t num_pe)
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
