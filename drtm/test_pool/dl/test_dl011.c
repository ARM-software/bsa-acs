/** @file
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
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

#include "val/common/include/acs_val.h"
#include "val/common/include/acs_memory.h"
#include "val/drtm/include/drtm_val_interface.h"

#define TEST_NUM   (ACS_DRTM_DL_TEST_NUM_BASE  +  11)
#define TEST_RULE  "R313010, R313020"
#define TEST_DESC  "Check Memory Region Desc Requirements "

#define TWO_4KB_REGIONS 2
#define ONE_4KB_REGIONS 1

static
void
payload(uint32_t num_pe)
{

  /* This test will verify the DRTM Dynamic Launch
   * Input parameter will be 64 bit address of DRTM Parameters
   * */
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  int64_t  status;

  DRTM_PARAMETERS *drtm_params;
  uint64_t drtm_params_size = DRTM_SIZE_4K;
  uint64_t dlme_image_addr;
  uint32_t dma_protection_support, max_mem_regions;
  DRTM_MEMORY_REGION_DESCRIPTOR_TABLE *mem_desc_table;
  DRTM_MEMORY_REGION *mem_region1, *mem_region2, *region_address;
  uint64_t region_size = DRTM_SIZE_4K * 2;
  uint64_t mem_desc_table_size;

  /* If complete DMA protection is supported skip as memory
   * regions not allowed through DRTM_PARAMETERS */
  dma_protection_support = VAL_EXTRACT_BITS(g_drtm_features.dma_prot_features.value, 0, 7);
  if (dma_protection_support != DRTM_PROTECTION_REGION_BASED_DMA) {
    val_print(ACS_PRINT_ERR, "\n       Not valid for complete DMA protection. Skipping", 0);
    val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
    return;
  }

  /* If max number of memory regions is zero then memory regions
   * are not allowed through DRTM_PARAMETERS */
  max_mem_regions = VAL_EXTRACT_BITS(g_drtm_features.dma_prot_features.value, 8, 23);
  if (max_mem_regions == 0) {
    val_print(ACS_PRINT_ERR, "\n       No regions in DRTM_PARAMETERS allowed. Skipping", 0);
    val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
    return;
  }

  /* Allocate Memory For DRTM Parameters 4KB Aligned */
  drtm_params = (DRTM_PARAMETERS *)((uint64_t)val_aligned_alloc(DRTM_SIZE_4K, drtm_params_size));
  if (!drtm_params) {
    val_print(ACS_PRINT_ERR, "\n    Failed to allocate memory for DRTM Params", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  status = val_drtm_init_drtm_params(drtm_params);
  if (status != ACS_STATUS_PASS) {
    val_print(ACS_PRINT_ERR, "\n       DRTM Init Params failed err=%d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
    goto free_drtm_params;
  }

  /* DLME Image address is sum of region start and DLME image offset */
  dlme_image_addr = drtm_params->dlme_region_address + drtm_params->dlme_image_start;
  val_memcpy((void *)dlme_image_addr, (void *)g_drtm_acs_dlme, g_drtm_acs_dlme_size);

  /* Size of MEM Descriptor Header + Size for two Memeory Regions */
  mem_desc_table_size = sizeof (DRTM_MEMORY_REGION_HDR) + (2 * sizeof (DRTM_MEMORY_REGION));

  /* Allocate Memory For Memory Region Descriptor Table 4KB Aligned */
  mem_desc_table = (DRTM_MEMORY_REGION_DESCRIPTOR_TABLE *)
                        ((uint64_t)val_aligned_alloc(DRTM_SIZE_4K, mem_desc_table_size));
  if (!mem_desc_table) {
    val_print(ACS_PRINT_ERR, "\n    Failed to allocate memory for Memory Descriptor Table", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
    goto free_drtm_params;
  }

  mem_desc_table->header.revision    = 1;
  mem_desc_table->header.reserved    = 0;

  /* Allocate Memory For Memory Regions Descriptor Table 4KB Aligned */
  region_address = (DRTM_MEMORY_REGION *)
                        ((uint64_t)val_aligned_alloc(DRTM_SIZE_4K, region_size));
  if (!region_address) {
    val_print(ACS_PRINT_ERR, "\n    Failed to allocate memory for Memory Regions", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
    goto free_mem_desc_table;
  }

  /* Part 1 : R313010 : Unaligned Memory Region address
   * Set memory region address as 4KB unaligned and verify MEM_PROTECT_INVALID error */
  drtm_params->mem_prot_table_address  = (uint64_t)mem_desc_table;
  drtm_params->mem_prot_table_size     = mem_desc_table_size - sizeof (DRTM_MEMORY_REGION);
  mem_desc_table->header.num_regions = 1;

  mem_region1 = mem_desc_table->regions;
  mem_region1->start_addr = (uint64_t)region_address + 0x4;
  mem_region1->size_type  =
                (DRTM_CACHEABILITY_WRITE_BACK << DRTM_MEM_PROT_SHIFT_CACHEABILITY_ATTR) |
                (DRTM_REGION_TYPE_NORMAL_CACHEABLE << DRTM_MEM_PROT_SHIFT_REGION_TYPE) |
                (TWO_4KB_REGIONS << DRTM_MEM_PROT_SHIFT_NUMBER_4KB_PAGES);

  /* Invoke DRTM Dynamic Launch, This will return only in case of error */
  status = val_drtm_dynamic_launch(drtm_params);
  /* This will return invalid parameter */
  if (status != DRTM_ACS_MEM_PROTECT_INVALID) {
    val_print(ACS_PRINT_ERR, "\n       Incorrect Status. Expected = -6 Found = %d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 5));
    goto free_memory_region;
  }

  /* If max number of memory regions is one then memory regions
   * overlap test is skipped */
  max_mem_regions = VAL_EXTRACT_BITS(g_drtm_features.dma_prot_features.value, 8, 23);
  if (max_mem_regions > 1) {
    val_print(ACS_PRINT_ERR, "\n       Only one memory region is allowed. Skipping R313020", 0);
    val_set_status(index, RESULT_SKIP(TEST_NUM, 2));
    goto free_memory_region;
  }

  /* Part 2 : R313020 : Overlaped Memory Region Table Addresses */
  mem_desc_table->header.num_regions = 2;

  mem_region1 = mem_desc_table->regions;
  mem_region1->start_addr = (uint64_t)region_address;
  mem_region1->size_type  =
                (DRTM_CACHEABILITY_WRITE_BACK << DRTM_MEM_PROT_SHIFT_CACHEABILITY_ATTR) |
                (DRTM_REGION_TYPE_NORMAL_CACHEABLE << DRTM_MEM_PROT_SHIFT_REGION_TYPE) |
                (TWO_4KB_REGIONS << DRTM_MEM_PROT_SHIFT_NUMBER_4KB_PAGES);

  mem_region2 = mem_desc_table->regions + sizeof (DRTM_MEMORY_REGION);
  mem_region2->start_addr = (uint64_t)region_address + DRTM_SIZE_4K;
  mem_region2->size_type  =
                (DRTM_CACHEABILITY_WRITE_BACK << DRTM_MEM_PROT_SHIFT_CACHEABILITY_ATTR) |
                (DRTM_REGION_TYPE_NORMAL_CACHEABLE << DRTM_MEM_PROT_SHIFT_REGION_TYPE) |
                (ONE_4KB_REGIONS << DRTM_MEM_PROT_SHIFT_NUMBER_4KB_PAGES);

  status = val_drtm_dynamic_launch(drtm_params);
  /* This will return invalid parameter */
  if (status != DRTM_ACS_MEM_PROTECT_INVALID) {
    val_print(ACS_PRINT_ERR, "\n       Incorrect Status. Expected = -6 Found = %d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 6));
    goto free_memory_region;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));

free_memory_region:
  val_memory_free_aligned((void *)region_address);
free_mem_desc_table:
  val_memory_free_aligned((void *)mem_desc_table);
  val_memory_free_aligned((void *)drtm_params->dlme_region_address);
free_drtm_params:
  val_memory_free_aligned((void *)drtm_params);

  return;
}

uint32_t dl011_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
  /* execute payload, which will execute relevant functions on current and other PEs */
      payload(num_pe);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
