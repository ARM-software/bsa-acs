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
#include "val/include/bsa_acs_pe.h"
#include "val/include/bsa_acs_memory.h"

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 8)
#define TEST_RULE  "PCI_IN_16"
#define TEST_DESC  "Check all 1's for out of range        "

/* Returns the maximum bdf value for that segment from bdf table */
static uint32_t get_max_bdf(uint32_t segment, uint32_t end_bus)
{
  pcie_device_bdf_table *bdf_tbl_ptr;
  uint32_t seg_num;
  uint32_t bus_num;
  uint32_t bdf;
  uint32_t tbl_index = 0;
  uint32_t max_bdf = 0;

  bdf_tbl_ptr = val_pcie_bdf_table_ptr();
  for (tbl_index = 0; tbl_index < bdf_tbl_ptr->num_entries; tbl_index++)
  {
      bdf = bdf_tbl_ptr->device[tbl_index].bdf;
      seg_num = PCIE_EXTRACT_BDF_SEG(bdf);
      bus_num = PCIE_EXTRACT_BDF_BUS(bdf);

      if ((segment == seg_num) && (bus_num <= end_bus))
          max_bdf = bdf;
  }

  val_print(ACS_PRINT_DEBUG, "\n max bdf 0x%x", max_bdf);
  return max_bdf;
}

/* Returns the maximum subordinate bus value for that segment from the RP's in the BDF table */
static uint32_t get_max_bus(uint32_t segment, uint32_t end_bus)
{
  pcie_device_bdf_table *bdf_tbl_ptr;
  uint32_t seg_num;
  uint32_t bdf, dp_type;
  uint32_t rp_bdf, sub_bus, bus_num;
  uint32_t reg_value;
  uint32_t tbl_index = 0;
  uint32_t max_sub_bus = 0;

  bdf_tbl_ptr = val_pcie_bdf_table_ptr();
  for (tbl_index = 0; tbl_index < bdf_tbl_ptr->num_entries; tbl_index++)
  {
      bdf = bdf_tbl_ptr->device[tbl_index].bdf;
      dp_type = val_pcie_device_port_type(bdf);

      /* Since RCEC or RCiEP has no RP. Skip for them. */
      if ((dp_type == RCEC) || (dp_type == RCiEP))
          continue;

      rp_bdf = bdf_tbl_ptr->device[tbl_index].rp_bdf;
      seg_num = PCIE_EXTRACT_BDF_SEG(rp_bdf);
      bus_num = PCIE_EXTRACT_BDF_BUS(rp_bdf);

      val_pcie_read_cfg(rp_bdf, TYPE1_PBN, &reg_value);
      sub_bus = ((reg_value >> SUBBN_SHIFT) & SUBBN_MASK);
      if ((segment == seg_num) && (bus_num <= end_bus) && (max_sub_bus < sub_bus))
          max_sub_bus = sub_bus;
  }

  val_print(ACS_PRINT_DEBUG, "\n Maximum sub bus %x", max_sub_bus);
  return max_sub_bus;
}

static
void
payload(void)
{

  uint32_t reg_value;
  uint32_t pe_index;
  uint32_t ecam_index;
  uint32_t num_ecam;
  uint32_t end_bus;
  uint32_t sub_bus;
  uint32_t cfg_addr;
  uint32_t bus_index;
  uint32_t dev_index;
  uint32_t func_index;
  uint32_t bdf;
  uint32_t segment = 0;
  addr_t   ecam_base = 0;

  pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
  num_ecam = val_pcie_get_info(PCIE_INFO_NUM_ECAM, 0);

  for (ecam_index = 0; ecam_index < num_ecam; ecam_index++)
  {
      /* Get the maximum bus value from PCIe info table */
      end_bus = val_pcie_get_info(PCIE_INFO_END_BUS, ecam_index);
      segment = val_pcie_get_info(PCIE_INFO_SEGMENT, ecam_index);
      val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 1));

      /* Get the highest BDF value for that segment */
      bdf = get_max_bdf(segment, end_bus);

      /* Get the maximum subordinate bus for that segment */
      sub_bus = get_max_bus(segment, end_bus);

      /* Get the least highest of max bus number */
      bus_index = (PCIE_EXTRACT_BDF_BUS(bdf) < end_bus) ? sub_bus:end_bus;
      bus_index += 1;
      val_print(ACS_PRINT_INFO, "\n       Maximum bus value is 0x%x", bus_index);

      /* Bus value must not exceed 255 */
      if (bus_index > end_bus) {
          val_print(ACS_PRINT_DEBUG, "\n       Bus index exceeded END_BUS Number", 0);
          val_set_status(pe_index, RESULT_SKIP(TEST_NUM, 2));
          return;
      }

      for (dev_index = 0; dev_index < PCIE_MAX_DEV; dev_index++)
      {
          for (func_index = 0; func_index < PCIE_MAX_FUNC; func_index++)
          {
              /* Form bdf using seg, bus, device, function numbers.
               * This BDF does not fall into the secondary and subordinate
               * bus of any of the rootports because the bus value is one
               * greater than the higest bus value. This BDF also doesn't
               * match any of the existing BDF.
               */
              bdf = PCIE_CREATE_BDF(segment, bus_index, dev_index, func_index);
              ecam_base = val_pcie_get_info(PCIE_INFO_ECAM, ecam_index);

              if (ecam_base == 0) {
                  val_print(ACS_PRINT_ERR, "\n       ECAM Base is zero ", 0);
                  val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 1));
                  return;
              }

              /* There are 8 functions / device, 32 devices / Bus and each has a 4KB config space */
              cfg_addr = (bus_index * PCIE_MAX_DEV * PCIE_MAX_FUNC * 4096) + \
                         (dev_index * PCIE_MAX_FUNC * 4096) + (func_index * 4096);

              val_print(ACS_PRINT_INFO, "\n       Calculated config address is %lx",
                                                  ecam_base + cfg_addr + TYPE01_VIDR);
              reg_value = pal_mmio_read(ecam_base + cfg_addr + TYPE01_VIDR);
              if (reg_value != PCIE_UNKNOWN_RESPONSE)
              {
                  val_print(ACS_PRINT_ERR, "\n       Failed for BDF: 0x%x", bdf);
                  val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 2));
                  return;
              }

              val_set_status(pe_index, RESULT_PASS(TEST_NUM, 1));
          }
      }
  }
}

uint32_t
os_p008_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from single PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
