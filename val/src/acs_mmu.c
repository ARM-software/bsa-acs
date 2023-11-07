/** @file
 * Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
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

#include "include/val_interface.h"
#include "include/bsa_acs_mmu.h"
#include "include/bsa_acs_pgt.h"
#include "include/bsa_acs_val.h"
#include "include/bsa_acs_memory.h"

#ifdef TARGET_BM_BOOT

static uint8_t mmap_list_curr_index;
memory_region_descriptor_t mmap_region_list[MAX_MMAP_REGION_COUNT];

/**
 * @brief Populate mmap_region_list with given map info
 * @param va_base - VA address
 * @param pa_base - PA address
 * @param length  - Size of the region
 * @param attributes - Map attributes
 * @return Void
**/
void val_mmap_add_region(uint64_t va_base, uint64_t pa_base,
                uint64_t length, uint64_t attributes)
{
    mmap_region_list[mmap_list_curr_index].virtual_address = va_base;
    mmap_region_list[mmap_list_curr_index].physical_address = pa_base;
    mmap_region_list[mmap_list_curr_index].length = length;
    mmap_region_list[mmap_list_curr_index].attributes = attributes;
    mmap_list_curr_index++;
}

/**
 * @brief Setup page table for image regions and device regions
 * @param void
 * @return status
**/
uint32_t val_setup_mmu(void)
{
    memory_region_descriptor_t mem_desc_array[2], *mem_desc;
    pgt_descriptor_t pgt_desc;
    uint8_t i = 0;

    // Memory map the image regions
    val_mmu_add_mmap();

    pgt_desc.ias = MMU_PGT_IAS;
    pgt_desc.oas = MMU_PGT_OAS;

    pgt_desc.pgt_base = (uint64_t) tt_l0_base;
    pgt_desc.stage = PGT_STAGE1;

    val_print(ACS_PRINT_DEBUG, "       mmu: ias=%d\n", pgt_desc.ias);
    val_print(ACS_PRINT_DEBUG, "       mmu: oas=%d\n", pgt_desc.oas);

    /* Map regions */

    val_memory_set(mem_desc_array, sizeof(mem_desc_array), 0);
    mem_desc = &mem_desc_array[0];

    while (i < mmap_list_curr_index)
    {
        mem_desc->virtual_address = mmap_region_list[i].virtual_address;
        mem_desc->physical_address = mmap_region_list[i].physical_address;
        mem_desc->length = mmap_region_list[i].length;
        mem_desc->attributes = mmap_region_list[i].attributes;

        val_print(ACS_PRINT_DEBUG, "\n       Creating page table for region  : 0x%lx",
                                                                        mem_desc->virtual_address);
        val_print(ACS_PRINT_DEBUG, "- 0x%lx\n", (mem_desc->virtual_address + mem_desc->length) - 1);

        if (val_pgt_create(mem_desc, &pgt_desc))
        {
            return ACS_STATUS_ERR;
        }
        i++;
    }

    return ACS_STATUS_PASS;
}

/**
 * @brief Enable mmu through configuring mmu registers
 * @param void
 * @return status
**/
uint32_t val_enable_mmu(void)
{
    uint64_t tcr;
    uint32_t currentEL;
    currentEL = (val_read_current_el() & 0xc) >> 2;

    /*
     * Setup Memory Attribute Indirection Register
     * Attr0 = b01000100 = Normal, Inner/Outer Non-Cacheable
     * Attr1 = b11111111 = Normal, Inner/Outer WB/WA/RA
     * Attr2 = b00000000 = Device-nGnRnE
     */
    val_mair_write(0xFFBB4400, currentEL);

    /* Setup ttbr0 */
    val_ttbr0_write((uint64_t)tt_l0_base, currentEL);

    if (currentEL == 0x02)
    {
        tcr = ((1ull << 20) |          /* TBI, top byte ignored. */
              (TCR_TG0 << 14) |        /* TG0, granule size */
              (3ull << 12) |           /* SH0, inner shareable. */
              (1ull << 10) |           /* ORGN0, normal mem, WB RA WA Cacheable */
              (1ull << 8) |            /* IRGN0, normal mem, WB RA WA Cacheable */
              (64 - MMU_PGT_IAS));               /* T0SZ, input address is 2^40 bytes. */
    }

    val_tcr_write(tcr, currentEL);

    val_print(ACS_PRINT_DEBUG, "       val_setup_mmu: TG0=0x%x\n", TCR_TG0);
    val_print(ACS_PRINT_DEBUG, "       val_setup_mmu: tcr=0x%lx\n", tcr);

    // Enable the MMU
    EnableMMU();

    val_print(ACS_PRINT_DEBUG, "       val_enable_mmu: successful\n", 0);

    return ACS_STATUS_PASS;
}
#endif // TARGET_BM_BOOT