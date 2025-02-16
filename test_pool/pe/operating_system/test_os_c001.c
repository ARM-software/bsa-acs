/** @file
 * Copyright (c) 2016-2018, 2021, 2024-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "val/common/include/acs_pe.h"
#include "val/common/include/acs_memory.h"

#define TEST_NUM   (ACS_PE_TEST_NUM_BASE  +  1)
#define TEST_RULE  "B_PE_01"
#define TEST_DESC  "Check Arch symmetry across PE         "

#define NUM_OF_REGISTERS  37

#define RAS               1
#define SPE               2
#define LOR               3
#define AA32              4
#define PMUV3             5

#define MASK_AA64MMFR0    0xF
#define MASK_MIDR         0x00F0FFFF
#define MASK_MPIDR        0xFF3FFFFFFF
#define MASK_CTR          0xC000
#define MASK_CCSIDR_LS    0xFFFFFFFFFFFFFFF8

#define MAX_CACHE_LEVEL   7

uint64_t rd_data_array[NUM_OF_REGISTERS];
uint64_t cache_list[MAX_CACHE_LEVEL];

typedef struct{
    uint32_t reg_name;
    uint64_t reg_mask;
    char     reg_desc[30];
    uint8_t  dependency;
}reg_details;

typedef struct{
    uint64_t pe_cache[MAX_CACHE_LEVEL];
    uint64_t cache_status[MAX_CACHE_LEVEL];
    uint64_t reg_data[NUM_OF_REGISTERS];
    uint64_t reg_status[NUM_OF_REGISTERS];
} pe_reg_info;

pe_reg_info *g_pe_reg_info;

reg_details reg_list[] = {
    {CCSIDR_EL1,       MASK_CCSIDR_LS, "CCSIDR_EL1      ", 0x0 },
    {MIDR_EL1,         MASK_MIDR,      "MIDR_EL1        ", 0x0 },
    {MPIDR_EL1,        MASK_MPIDR,     "MPIDR_EL1       ", 0x0 },
    {CTR_EL0,          MASK_CTR,       "CTR_EL0         ", 0x0 },
    {ID_AA64PFR0_EL1,  0x0,            "ID_AA64PFR0_EL1 ", 0x0 },
    {ID_AA64PFR1_EL1,  0x0,            "ID_AA64PFR1_EL1 ", 0x0 },
    {ID_AA64DFR0_EL1,  0x0,            "ID_AA64DFR0_EL1 ", 0x0 },
    {ID_AA64DFR1_EL1,  0x0,            "ID_AA64DFR1_EL1 ", 0x0 },
    {ID_AA64MMFR0_EL1, MASK_AA64MMFR0, "ID_AA64MMFR0_EL1", 0x0 },
    {ID_AA64MMFR1_EL1, 0x0,            "ID_AA64MMFR1_EL1", 0x0 },
    {ID_AA64MMFR2_EL1, 0x0,            "ID_AA64MMFR2_EL1", 0x0 },
    {ID_AA64ISAR0_EL1, 0x0,            "ID_AA64ISAR0_EL1", 0x0 },
    {ID_AA64ISAR1_EL1, 0x0,            "ID_AA64ISAR1_EL1", 0x0 },
    {PMCEID0_EL0,      0x0,            "PMCEID0_EL0     ", PMUV3},
    {PMCEID1_EL0,      0x0,            "PMCEID1_EL0     ", PMUV3},
    {PMCR_EL0,         0x0,            "PMCR_EL0        ", PMUV3},
    {PMBIDR_EL1,       0x0,            "PMBIDR_EL1      ", SPE },
    {PMSIDR_EL1,       0x0,            "PMSIDR_EL1      ", SPE },
    {ERRIDR_EL1,       0x0,            "ERRIDR_EL1      ", RAS },
    {LORID_EL1,        0x0,            "LORID_EL1       ", LOR },
    {ID_DFR0_EL1,      0x0,            "ID_DFR0_EL1     ", AA32},
    {ID_ISAR0_EL1,     0x0,            "ID_ISAR0_EL1    ", AA32},
    {ID_ISAR1_EL1,     0x0,            "ID_ISAR1_EL1    ", AA32},
    {ID_ISAR2_EL1,     0x0,            "ID_ISAR2_EL1    ", AA32},
    {ID_ISAR3_EL1,     0x0,            "ID_ISAR3_EL1    ", AA32},
    {ID_ISAR4_EL1,     0x0,            "ID_ISAR4_EL1    ", AA32},
    {ID_ISAR5_EL1,     0x0,            "ID_ISAR5_EL1    ", AA32},
    {ID_MMFR0_EL1,     0x0,            "ID_MMFR0_EL1    ", AA32},
    {ID_MMFR1_EL1,     0x0,            "ID_MMFR1_EL1    ", AA32},
    {ID_MMFR2_EL1,     0x0,            "ID_MMFR2_EL1    ", AA32},
    {ID_MMFR3_EL1,     0x0,            "ID_MMFR3_EL1    ", AA32},
    {ID_MMFR4_EL1,     0x0,            "ID_MMFR4_EL1    ", AA32},
    {ID_PFR0_EL1,      0x0,            "ID_PFR0_EL1     ", AA32},
    {ID_PFR1_EL1,      0x0,            "ID_PFR1_EL1     ", AA32},
    {MVFR0_EL1,        0x0,            "MVFR0_EL1       ", AA32},
    {MVFR1_EL1,        0x0,            "MVFR1_EL1       ", AA32},
    {MVFR2_EL1,        0x0,            "MVFR2_EL1       ", AA32}
};

uint64_t
return_reg_value(uint32_t reg, uint8_t dependency)
{
  uint64_t temp=0;

  if(dependency == 0)
      return val_pe_reg_read(reg);

  switch(dependency)
  {
    case RAS: // If RAS is not supported, then skip register check
        temp = val_pe_reg_read(ID_AA64PFR0_EL1);
        temp = (temp >> 28) & 0xf;
        if(temp == 1)
            return val_pe_reg_read(reg);
        else
            return 0;
        break;

    case SPE: // If Statistical Profiling Extension is not supported, then skip register check
        temp = val_pe_reg_read(ID_AA64DFR0_EL1);
        temp = (temp >> 32) & 0xf;
        if(temp == 1)
            return val_pe_reg_read(reg);
        else
            return 0;
        break;

    case LOR: // If Limited Ordering Region is not supported, then skip register check
        temp = val_pe_reg_read(ID_AA64MMFR1_EL1);
        temp = (temp >> 16) & 0xf;
        if(temp == 1)
            return val_pe_reg_read(reg);
        else
            return 0;
        break;

    case AA32: // If the register is UNK in pure AArch64 implementation, then skip register check
        temp = val_pe_reg_read(ID_AA64PFR0_EL1);
        temp = (temp & 1);
        if(temp == 0)
            return val_pe_reg_read(reg);
        else
            return 0;
        break;

    case PMUV3: // If PMUv3 is not supported, then skip register check
        temp = val_pe_reg_read(ID_AA64DFR0_EL1);
        temp = (temp >> 8) & 0xf;
        if (temp != 0 && temp != 0xf)
            return val_pe_reg_read(reg);
        else
            return 0;
        break;

    default:
        val_print(ACS_PRINT_ERR, "\n Unknown dependency = %d ", dependency);
        return 0x0;
  }

}

void
id_regs_check(void)
{
  uint64_t reg_read_data;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t i = 0, check = 0;
  pe_reg_info *secondary_buffer;
  uint64_t buffer_ptr, addr;

  /* Loop CLIDR to check if a cache level is implemented before comparing */
  val_get_test_data(index, &addr, &buffer_ptr);
  secondary_buffer = (pe_reg_info *)buffer_ptr;
  secondary_buffer = secondary_buffer + index;

  while (i < MAX_CACHE_LEVEL) {
      reg_read_data = val_pe_reg_read(CLIDR_EL1);
      if (reg_read_data & ((0x7) << (i * 3))) {

          /* Select the correct cache in csselr register */
          val_pe_reg_write(CSSELR_EL1, i << 1);
          reg_read_data = 0;
          reg_read_data = return_reg_value(reg_list[0].reg_name, reg_list[0].dependency);

          secondary_buffer->pe_cache[i] = reg_read_data;
          val_data_cache_ops_by_va((addr_t)&secondary_buffer->pe_cache[i], CLEAN_AND_INVALIDATE);

          if ((reg_read_data & (~reg_list[0].reg_mask)) != (cache_list[i] & (~reg_list[0].reg_mask))) {
              secondary_buffer->cache_status[i] = 1;
              val_data_cache_ops_by_va((addr_t)&secondary_buffer->cache_status[i],
                                                                      CLEAN_AND_INVALIDATE);
              check = 1;
          }
      }
      reg_read_data = 0;
      i++;
  }

  for (i = 1; i < NUM_OF_REGISTERS; i++) {
      reg_read_data = return_reg_value(reg_list[i].reg_name, reg_list[i].dependency);

      secondary_buffer->reg_data[i] = reg_read_data;
      val_data_cache_ops_by_va((addr_t)&secondary_buffer->reg_data[i], CLEAN_AND_INVALIDATE);

      if ((reg_read_data & (~reg_list[i].reg_mask)) != (rd_data_array[i] &
                                                                    (~reg_list[i].reg_mask)))
      {
          secondary_buffer->reg_status[i] = 1;
          val_data_cache_ops_by_va((addr_t)&secondary_buffer->reg_status[i], CLEAN_AND_INVALIDATE);
          check = 1;
      }
      reg_read_data = 0;
  }

  if (check == 1)
      val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
  else
      val_set_status(index, RESULT_PASS(TEST_NUM, 1));

  return;
}

static
void
payload(uint32_t num_pe)
{
  uint32_t my_index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t i, j, t = 0;
  uint32_t timeout;
  uint64_t reg_read_data;
  uint64_t total_fail = 0;
  uint64_t reg_fail = 0;
  uint64_t cache_fail = 0;
  volatile pe_reg_info *pe_buffer;

  if (num_pe == 1) {
      val_print(ACS_PRINT_DEBUG, "\n       Skipping as num of PE is 1    ", 0);
      val_set_status(my_index, RESULT_SKIP(TEST_NUM, 1));
      return;
  }

  /* Loop CLIDR to check if a cache level is implemented */
  i = 0;

  while (i < MAX_CACHE_LEVEL) {
      reg_read_data = val_pe_reg_read(CLIDR_EL1);
      if (reg_read_data & ((0x7) << (i * 3))) {

         /* Select the correct cache level in csselr register */
         val_pe_reg_write(CSSELR_EL1, i << 1);
         cache_list[i] = return_reg_value(reg_list[0].reg_name, reg_list[0].dependency);
         val_data_cache_ops_by_va((addr_t)(cache_list + i), CLEAN_AND_INVALIDATE);
         val_print(ACS_PRINT_INFO, "\n       Primary PE Index: %d", my_index);
         val_print(ACS_PRINT_INFO, ", cache index: %d", i);
         val_print(ACS_PRINT_INFO, ", size read: 0x%016llx", cache_list[i]);
      }
      i++;
  }

  for (i = 1; i < NUM_OF_REGISTERS; i++) {
      rd_data_array[i] = return_reg_value(reg_list[i].reg_name, reg_list[i].dependency);
      val_print(ACS_PRINT_INFO, "\n       Primary PE Index: %d, ", my_index);
      val_print(ACS_PRINT_INFO, reg_list[i].reg_desc, 0);

      if (reg_list[i].dependency == AA32)
          val_print(ACS_PRINT_INFO, " : 0x%08llx ", rd_data_array[i]);
      else
          val_print(ACS_PRINT_INFO, " : 0x%016llx ", rd_data_array[i]);

      val_data_cache_ops_by_va((addr_t)(rd_data_array + i), CLEAN_AND_INVALIDATE);
  }

  /* Allocate memory to save all PE register read values and status of reg value
     comparision pass and fail */
  g_pe_reg_info = (pe_reg_info *) val_memory_calloc(num_pe, sizeof(pe_reg_info));

  if (g_pe_reg_info == NULL) {
      val_print(ACS_PRINT_ERR, "\n       Allocation for secondary PE Registers Failed \n", 0);
      val_set_status(my_index, RESULT_FAIL(TEST_NUM, 1));
      return;
  }

  for (i = 0; i < num_pe; i++) {
      if (i != my_index) {
          timeout = TIMEOUT_LARGE;
          val_execute_on_pe(i, id_regs_check, (uint64_t)g_pe_reg_info);
          while ((--timeout) && (IS_RESULT_PENDING(val_get_status(i))));

          if(timeout == 0) {
              val_print(ACS_PRINT_ERR, "\n       **Timed out** for PE index = %d", i);
              val_set_status(i, RESULT_FAIL(TEST_NUM, 3));
              return;
          }
      }
  }

  val_print(ACS_PRINT_TEST, "\n       Primary PE Index    : %d", my_index);
  val_print(ACS_PRINT_TEST, "\n       Primary PE MIDR_EL1 : 0x%08llx", rd_data_array[1]);

  for (i = 0; i < num_pe; i++) {
      uint32_t unique = 1;
      if (i != my_index) {
          pe_buffer = g_pe_reg_info + i;
          for (j = 0; j < i; j++) {
              pe_reg_info *pe_buffer_midr = g_pe_reg_info + j;
              if (pe_buffer->reg_data[1] == pe_buffer_midr->reg_data[1]) {
                  unique = 0;
                  break;
              }
          }
          if (unique == 1 && rd_data_array[1] != pe_buffer->reg_data[1]) {
              if (t == 0) {
                  val_print(ACS_PRINT_TEST, "\n       Other Cores         : 0x%08llx      ",
                                                                        pe_buffer->reg_data[1]);
                  t = 1;
              } else {
                  val_print(ACS_PRINT_TEST, "\n                             0x%08llx      ",
                                                                        pe_buffer->reg_data[1]);
                }
           }
      }
  }

  if (t == 0) {
      val_print(ACS_PRINT_TEST, "\n       Other Cores         : Identical       ", 0);
  }

  pe_buffer = NULL;

  for (i = 0; i < num_pe; i++) {
      if (i != my_index) {
          pe_buffer = g_pe_reg_info + i;

          for (int cache_index = 0; cache_index < MAX_CACHE_LEVEL; cache_index++) {
             if (!(pe_buffer->cache_status[cache_index]) &&
                                                    (pe_buffer->pe_cache[cache_index] != 0))
             {
                 val_print(ACS_PRINT_INFO, "\n        PE Index: %d", i);
                 val_print(ACS_PRINT_INFO, ", cache index: %d", cache_index);
                 val_print(ACS_PRINT_INFO, ", size read: 0x%016llx",
                                                             pe_buffer->pe_cache[cache_index]);
             }
             else if (pe_buffer->pe_cache[cache_index] != 0)
             {
                   val_print(ACS_PRINT_ERR, "\n        PE Index: %d", i);
                   val_print(ACS_PRINT_ERR, ", cache index: %d", cache_index);
                   val_print(ACS_PRINT_ERR, ", size read: 0x%016llx     FAIL\n",
                                                             pe_buffer->pe_cache[cache_index]);
                   val_print(ACS_PRINT_ERR, "          Masked Primary PE Value : 0x%016llx \n",
                                              cache_list[cache_index] & (~reg_list[0].reg_mask));
                   val_print(ACS_PRINT_ERR, "          Masked Current PE Value : 0x%016llx ",
                                      pe_buffer->pe_cache[cache_index] & (~reg_list[0].reg_mask));
                   cache_fail = cache_fail + 1;
             }
          }

          for (int reg_index = 1; reg_index < NUM_OF_REGISTERS; reg_index++) {
             if (!(pe_buffer->reg_status[reg_index])) {
                 val_print(ACS_PRINT_INFO, "\n        PE Index: %d, ", i);
                 val_print(ACS_PRINT_INFO, reg_list[reg_index].reg_desc, 0);
                 if (reg_list[reg_index].dependency == AA32)
                     val_print(ACS_PRINT_INFO, "  : 0x%08llx", pe_buffer->reg_data[reg_index]);
                 else
                     val_print(ACS_PRINT_INFO, "  : 0x%016llx", pe_buffer->reg_data[reg_index]);
             } else  {
                 val_print(ACS_PRINT_ERR, "\n        PE Index: %d, ", i);
                 val_print(ACS_PRINT_ERR, reg_list[reg_index].reg_desc, 0);
                 if (reg_list[reg_index].dependency == AA32) {
                     val_print(ACS_PRINT_ERR, "   : 0x%08llx    FAIL\n",
                                                        pe_buffer->reg_data[reg_index]);
                     val_print(ACS_PRINT_ERR, "          Masked Primary PE Value : 0x%08llx \n",
                                       rd_data_array[reg_index] & (~reg_list[reg_index].reg_mask));
                     val_print(ACS_PRINT_ERR, "          Masked Current PE Value : 0x%08llx ",
                                 pe_buffer->reg_data[reg_index] & (~reg_list[reg_index].reg_mask));
                 } else {
                     val_print(ACS_PRINT_ERR, "   : 0x%016llx    FAIL\n",
                                                        pe_buffer->reg_data[reg_index]);
                     val_print(ACS_PRINT_ERR, "          Masked Primary PE Value : 0x%016llx \n",
                                       rd_data_array[reg_index] & (~reg_list[reg_index].reg_mask));
                     val_print(ACS_PRINT_ERR, "          Masked Current PE Value : 0x%016llx ",
                                 pe_buffer->reg_data[reg_index] & (~reg_list[reg_index].reg_mask));
                   }
                 reg_fail = reg_fail + 1;
             }
          }
          total_fail = total_fail + reg_fail + cache_fail;
          reg_fail = 0;
          cache_fail = 0;
      }
  }

  if (total_fail) {
      val_print(ACS_PRINT_ERR, "\n\n    Total Register and cache fail for all PE: %d \n",
                                                                             total_fail);
      val_set_status(total_fail, RESULT_FAIL(TEST_NUM, 4));
  }
  else
      val_set_status(1, RESULT_PASS(TEST_NUM, 2));

  val_memory_free((void *) g_pe_reg_info);
  return;
}

uint32_t
os_c001_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  status = val_initialize_test(TEST_NUM, TEST_DESC, val_pe_get_num());

  if (status != ACS_STATUS_SKIP)
  /* execute payload, which will execute relevant functions on current and other PEs */
      payload(num_pe);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}

