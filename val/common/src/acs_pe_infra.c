/** @file
 * Copyright (c) 2016-2018, 2020-2021,2023-2025, Arm Limited or its affiliates. All rights reserved.
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

#include "common/include/acs_val.h"
#include "common/include/acs_pe.h"
#include "common/include/acs_common.h"
#include "common/include/acs_std_smc.h"
#include "common/sys_arch_src/gic/acs_exception.h"
#include "common/include/val_interface.h"
#include "bsa/include/bsa_pal_interface.h"

PE_SMBIOS_PROCESSOR_INFO_TABLE *g_smbios_info_table;
int32_t gPsciConduit;

/* Global variable to store mpidr of primary cpu */
uint64_t g_primary_mpidr = PAL_INVALID_MPID;

/**
  @brief   Pointer to the memory location of the PE Information table
**/
PE_INFO_TABLE *g_pe_info_table;
/**
  @brief   global structure to pass and retrieve arguments for the SMC call
**/
ARM_SMC_ARGS g_smc_args;

/* global variable to store primary PE index */
uint32_t g_primary_pe_index = 0;

/**
  @brief   This API will call PAL layer to fill in the PE information
           into the g_pe_info_table pointer.
           1. Caller       -  Application layer.
           2. Prerequisite -  Memory allocated and passed as argument.
  @param   pe_info_table  pre-allocated memory pointer for pe_info
  @return  Error if Input param is NULL or num_pe is 0.
**/
uint32_t
val_pe_create_info_table(uint64_t *pe_info_table)
{
  gPsciConduit = pal_psci_get_conduit();
  if (gPsciConduit == CONDUIT_NO_TABLE)
      return ACS_STATUS_ERR;

  if (gPsciConduit == CONDUIT_UNKNOWN) {
      val_print(ACS_PRINT_WARN, " FADT not found, assuming SMC as PSCI conduit\n", 0);
      gPsciConduit = CONDUIT_SMC;
  } else if (gPsciConduit == CONDUIT_NONE) {
      val_print(ACS_PRINT_WARN, " PSCI not supported, assuming SMC as conduit for tests\n"
                                " Multi-PE and wakeup tests likely to fail\n", 0);
      gPsciConduit = CONDUIT_SMC;
  } else if (gPsciConduit == CONDUIT_HVC) {
      val_print(ACS_PRINT_INFO, " Using HVC as PSCI conduit\n", 0);
  } else {
      val_print(ACS_PRINT_INFO, " Using SMC as PSCI conduit\n", 0);
  }

  val_print(ACS_PRINT_INFO, " Creating PE INFO table\n", 0);

  if (pe_info_table == NULL) {
      val_print(ACS_PRINT_ERR, "Input memory for PE Info table cannot be NULL\n", 0);
      return ACS_STATUS_ERR;
  }

  g_pe_info_table = (PE_INFO_TABLE *)pe_info_table;

  pal_pe_create_info_table(g_pe_info_table);
  val_data_cache_ops_by_va((addr_t)&g_pe_info_table, CLEAN_AND_INVALIDATE);

  val_print(ACS_PRINT_TEST, " PE_INFO: Number of PE detected       : %4d\n", val_pe_get_num());

  if (val_pe_get_num() == 0) {
      val_print(ACS_PRINT_ERR, "\n *** CRITICAL ERROR: Num PE is 0x0 ***\n", 0);
      return ACS_STATUS_ERR;
  }

#ifndef TARGET_LINUX
val_print(ACS_PRINT_TEST, " Primary PE: MIDR_EL1                 :    0x%llx \n",
                                                                     val_pe_reg_read(MIDR_EL1));
#endif

  /* store primary PE index for debug message printing purposes on
     multi PE tests */
  g_primary_pe_index = val_pe_get_index_mpid(val_pe_get_mpid());
  val_print(ACS_PRINT_DEBUG, " PE_INFO: Primary PE index       : %4d\n",
            g_primary_pe_index);

  return ACS_STATUS_PASS;
}

/**
  @brief  Free the memory allocated for the pe_info_table

  @param  None

  @return None
**/

void
val_pe_free_info_table(void)
{
    if (g_pe_info_table != NULL) {
        pal_mem_free_aligned((void *)g_pe_info_table);
        g_pe_info_table = NULL;
    }
    else {
      val_print(ACS_PRINT_ERR,
                  "\n WARNING: g_pe_info_table pointer is already NULL", 
        0);
    }
}

/**
  @brief   This API returns the number of PE from the g_pe_info_table.
           1. Caller       -  Application layer, test Suite.
           2. Prerequisite -  val_pe_create_info_table.
  @param   none
  @return  the number of pe discovered
**/
uint32_t
val_pe_get_num()
{
  if (g_pe_info_table == NULL) {
      return 0;
  }
  return g_pe_info_table->header.num_of_pe;
}


/**
  @brief   This API reads MPIDR system regiser and return the Affinity bits
           1. Caller       -  Test Suite, VAL
           2. Prerequisite -  None
  @param   None
  @return  Affinity Bits of MPIDR
**/
uint64_t
val_pe_get_mpid()
{
  uint64_t data;

  #ifdef TARGET_LINUX
    data = 0;
  #else
    data = val_pe_reg_read(MPIDR_EL1);
  #endif
  /* Return the Affinity bits */
  data = data & MPIDR_AFF_MASK;
  return data;

}

/**
  @brief   This API returns the MPIDR value for the PE indicated by index
           1. Caller       -  Test Suite, VAL
           2. Prerequisite -  val_create_peinfo_table
  @param   index - the index of the PE whose mpidr value is required.
  @return  MPIDR value
**/
uint64_t
val_pe_get_mpid_index(uint32_t index)
{

  PE_INFO_ENTRY *entry;

  if (index > g_pe_info_table->header.num_of_pe) {
        val_report_status(index, RESULT_FAIL(0, 0xFF), NULL);
        return 0xFFFFFF;
  }

  entry = g_pe_info_table->pe_info;

  return entry[index].mpidr;

}


/**
  @brief   This API returns the index of the PE whose MPIDR matches with the input MPIDR
           1. Caller       -  Test Suite, VAL
           2. Prerequisite -  val_create_peinfo_table
  @param   mpid - the mpidr value of pE whose index is returned.
  @return  Index of PE
**/
uint32_t
val_pe_get_index_mpid(uint64_t mpid)
{

  PE_INFO_ENTRY *entry;
  uint32_t i = g_pe_info_table->header.num_of_pe;

  entry = g_pe_info_table->pe_info;

  while (i > 0) {
    val_data_cache_ops_by_va((addr_t)&entry->mpidr, INVALIDATE);
    val_data_cache_ops_by_va((addr_t)&entry->pe_num, INVALIDATE);

    if (entry->mpidr == mpid) {
      return entry->pe_num;
    }
    entry++;
    i--;
  }

  return 0x0;  //Return index 0 as a safe failsafe value
}


/**
  @brief   'C' Entry point for Secondary PE.
           Uses PSCI_CPU_OFF to switch off PE after payload execution.
           1. Caller       -  PAL code
           2. Prerequisite -  Stack pointer for this PE is setup by PAL
  @param   None
  @return  None
**/

void
val_test_entry(void)
{
  uint64_t test_arg;
  ARM_SMC_ARGS smc_args;
  void (*vector)(uint64_t args);

  val_get_test_data(val_pe_get_index_mpid(val_pe_get_mpid()), (uint64_t *)&vector, &test_arg);
  vector(test_arg);

  // We have completed our TEST code. So, switch off the PE now
  smc_args.Arg0 = ARM_SMC_ID_PSCI_CPU_OFF;
  smc_args.Arg1 = val_pe_get_mpid();
  pal_pe_call_smc(&smc_args, gPsciConduit);
}


/**
  @brief   This API initiates the execution of a test on a secondary PE.
           Uses PSCI_CPU_ON to wake a secondary PE
           1. Caller       -  Test Suite
           2. Prerequisite -  val_create_peinfo_table
  @param   index - Index of the PE to be woken up
  @param   payload - Function pointer of the test to be executed on the PE
  @param   test_input - arguments to be passed to the test.
  @return  None
**/
void
val_execute_on_pe(uint32_t index, void (*payload)(void), uint64_t test_input)
{

  int timeout = TIMEOUT_LARGE;
  if (index > g_pe_info_table->header.num_of_pe) {
      val_print(ACS_PRINT_ERR, "Input Index exceeds Num of PE %x\n", index);
      val_report_status(index, RESULT_FAIL(0, 0xFF), NULL);
      return;
  }

  do {
      g_smc_args.Arg0 = ARM_SMC_ID_PSCI_CPU_ON_AARCH64;

      /* Set the TEST function pointer in a shared memory location. This location is
         read by the Secondary PE (val_test_entry()) and executes the test. */
      g_smc_args.Arg1 = val_pe_get_mpid_index(index);

      val_set_test_data(index, (uint64_t)payload, test_input);
      pal_pe_execute_payload(&g_smc_args);

  } while (g_smc_args.Arg0 == (uint64_t)ARM_SMC_PSCI_RET_ALREADY_ON && timeout--);

  if (g_smc_args.Arg0 == (uint64_t)ARM_SMC_PSCI_RET_ALREADY_ON) {
      val_print(ACS_PRINT_ERR, "\n       PSCI_CPU_ON: cpu already on", 0);
      val_print(ACS_PRINT_WARN, "\n       WARNING: Skipping test for PE index %d "
                              "since it is already on\n", index);

      val_set_status(index, RESULT_SKIP(0, 0x120 - (int)g_smc_args.Arg0));
      return;
  }
  else {
      if(g_smc_args.Arg0 == 0) {
          val_print(ACS_PRINT_INFO, "\n       PSCI_CPU_ON: success", 0);
          return;
      }
      else
          val_print(ACS_PRINT_ERR, "\n       PSCI_CPU_ON: failure[%d]", g_smc_args.Arg0);

  }
  val_set_status(index, RESULT_FAIL(0, 0x120 - (int)g_smc_args.Arg0));
}

/**
  @brief   This API installs the Exception handler pointed
           by the function pointer to the input exception type.
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   exception_type - one of the four exceptions defined by AARCH64
  @param   esr            - Function pointer of the exception handler
  @return  0 if success or ERROR for invalid Exception type.
**/
uint32_t
val_pe_install_esr(uint32_t exception_type, void (*esr)(uint64_t, void *))
{

  if (exception_type > 3) {
      val_print(ACS_PRINT_ERR, "Invalid Exception type %x\n", exception_type);
      return ACS_STATUS_ERR;
  }
#ifndef TARGET_LINUX
#if !defined(SBSA) && !defined(DRTM)
  if (pal_target_is_dt())
      return val_acs_install_esr(exception_type, esr);
#endif
  if (pal_target_is_bm())
      return val_acs_install_esr(exception_type, esr);
  else
      return pal_pe_install_esr(exception_type, esr);
#endif
  return 0;
}


/**
  @brief  Save context data (LR, SP and ELR in case of unexpected exception)

  @param  sp Stack Pointer
  @param  elr ELR register

  @return None
**/
void
val_pe_context_save(uint64_t sp, uint64_t elr)
{
    g_stack_pointer = sp;
    g_exception_ret_addr = elr;
    g_ret_addr = *(uint64_t *)(g_stack_pointer+8);
}

/**
  @brief  Restore context data (LR, SP for return to a known location)

  @param  sp Stack Pointer

  @return None
**/
void
val_pe_context_restore(uint64_t sp)
{
    sp = 0;
    *(uint64_t *)(g_stack_pointer+8+sp) = g_ret_addr;
}

/**
  @brief  Initialise exception vector with the default handler

  @param  esr Exception Handler function pointer

  @return None
**/
void
val_pe_initialize_default_exception_handler(void (*esr)(uint64_t, void *))
{
    val_pe_install_esr(EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS, esr);
}

/**
  @brief  Default handler which, if installed into exception vector, will be
          called in case of unexpected exceptions

  @param  interrupt_type Type of Interrupt(IRQ/FIQ/ASYNC/SERROR)
  @param  context To restore the context

  @return None
**/
void
val_pe_default_esr(uint64_t interrupt_type, void *context)
{
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
    val_print(ACS_PRINT_WARN, "\n        Unexpected exception of type %d occurred", interrupt_type);

#ifndef TARGET_LINUX
#if !defined(SBSA) && !defined(DRTM)
    if (pal_target_is_dt()) {
      val_print(ACS_PRINT_WARN, "\n        FAR reported = 0x%llx", bsa_gic_get_far());
      val_print(ACS_PRINT_WARN, "\n        ESR reported = 0x%llx", bsa_gic_get_esr());
      val_set_status(index, RESULT_FAIL(0, 1));
      val_pe_update_elr(context, g_exception_ret_addr);
      return;
    }
#endif
    if (pal_target_is_bm()) {
      val_print(ACS_PRINT_WARN, "\n        FAR reported = 0x%llx", bsa_gic_get_far());
      val_print(ACS_PRINT_WARN, "\n        ESR reported = 0x%llx", bsa_gic_get_esr());

    } else {
      val_print(ACS_PRINT_WARN, "\n        FAR reported = 0x%llx", val_pe_get_far(context));
      val_print(ACS_PRINT_WARN, "\n        ESR reported = 0x%llx", val_pe_get_esr(context));
    }
#endif

    val_set_status(index, RESULT_FAIL(0, 1));
    val_pe_update_elr(context, g_exception_ret_addr);
}

/**
  @brief  Cache clean operation on a defined address range

  @param  start_addr Start Address
  @param  length Length of the block

  @return None
**/
void
val_pe_cache_clean_range(uint64_t start_addr, uint64_t length)
{
#ifndef TARGET_LINUX
  uint64_t aligned_addr, end_addr, line_length;

  line_length = 2 << ((val_pe_reg_read(CTR_EL0) >> 16) & 0xf);
  aligned_addr = start_addr - (start_addr & (line_length-1));
  end_addr = start_addr + length;

  while(aligned_addr < end_addr){
      val_data_cache_ops_by_va(aligned_addr, CLEAN);
      aligned_addr += line_length;
  }
#endif
}

/**
  @brief  Cache clean operation on a defined address range

  @param  start_addr Start Address
  @param  length Length of the block

  @return None
**/
void
val_pe_cache_clean_invalidate_range(uint64_t start_addr, uint64_t length)
{
#ifndef TARGET_LINUX
  uint64_t aligned_addr, end_addr, line_length;

  line_length = 2 << ((val_pe_reg_read(CTR_EL0) >> 16) & 0xf);
  aligned_addr = start_addr - (start_addr & (line_length-1));
  end_addr = start_addr + length;

  while (aligned_addr < end_addr) {
      val_data_cache_ops_by_va(aligned_addr, CLEAN_AND_INVALIDATE);
      aligned_addr += line_length;
  }
#endif
}

/**
  @brief  Cache clean operation on a defined address range

  @param  start_addr Start Address
  @param  length Length of the block

  @return None
**/
void
val_pe_cache_invalidate_range(uint64_t start_addr, uint64_t length)
{
#ifndef TARGET_LINUX
  uint64_t aligned_addr, end_addr, line_length;

  line_length = 2 << ((val_pe_reg_read(CTR_EL0) >> 16) & 0xf);
  aligned_addr = start_addr - (start_addr & (line_length-1));
  end_addr = start_addr + length;

  while (aligned_addr < end_addr) {
      val_data_cache_ops_by_va(aligned_addr, INVALIDATE);
      aligned_addr += line_length;
  }
#endif
}


/**
  @brief   This API returns the index of primary PE on which system is booted.
           1. Caller       -  Test Suite, VAL
           2. Prerequisite -  val_create_peinfo_table
  @param   None
  @return  Primary PE index.
**/
uint32_t
val_pe_get_primary_index(void)
{
  return g_primary_pe_index;
}

#ifndef TARGET_LINUX
void
val_smbios_create_info_table(uint64_t *smbios_info_table)
{
  val_print(ACS_PRINT_INFO, " Creating SMBIOS INFO table\n", 0);

  if (smbios_info_table == NULL) {
    val_print(ACS_PRINT_ERR, "Input memory for SMBIOS Info table cannot be NULL\n", 0);
    return;
  }

  g_smbios_info_table = (PE_SMBIOS_PROCESSOR_INFO_TABLE *)smbios_info_table;

  pal_smbios_create_info_table(g_smbios_info_table);
  val_print(ACS_PRINT_TEST, " SMBIOS: Num of slots :    %d\n", g_smbios_info_table->slot_count);
}

/**
  @brief   This API returns the number of Slots in smbios from the g_smbios_info_table.
           1. Caller       -  Application layer, test Suite.
           2. Prerequisite -  val_smbios_create_info_table.
  @param   none
  @return  the number of pe discovered
**/
uint32_t
val_get_num_smbios_slots()
{
  if (g_smbios_info_table == NULL) {
      return 0;
  }
  return g_smbios_info_table->slot_count;
}

/**
  @brief  Free the memory allocated for the smbios_info_table

  @param  None

  @return None
**/
void
val_smbios_free_info_table()
{
  pal_mem_free_aligned((void *)g_smbios_info_table);
}
#endif

uint32_t
val_get_pe_architecture(uint32_t index)
{
  PE_SMBIOS_TYPE4_INFO *type4_entry;
  uint32_t cur_pe_count = 0;
  uint32_t num_slots = g_smbios_info_table->slot_count;

  if (num_slots == 0) {
    return ACS_STATUS_ERR;
  }

  type4_entry = g_smbios_info_table->type4_info;

  while (num_slots > 0) {
    num_slots--;
    cur_pe_count += type4_entry->core_count;
    if (index > cur_pe_count) {
      type4_entry++;
      continue;
    }

    return type4_entry->processor_family;
  }
  return ACS_STATUS_ERR;
}

#ifdef TARGET_BM_BOOT
/**
 *   @brief    Convert mpidr to logical cpu number
 *   @param    mpidr    - mpidr value
 *   @return   Logical cpu number
**/
// This API is only used for baremetal boot at which point PE info table is not created.
uint32_t val_get_pe_id(uint64_t mpidr)
{
    uint32_t pe_index = 0;
    uint32_t total_pe_num = pal_get_pe_count();
    uint64_t *phy_mpidr_list = pal_get_phy_mpidr_list_base();

    mpidr = mpidr & PAL_MPIDR_AFFINITY_MASK;

    for (pe_index = 0; pe_index < total_pe_num; pe_index++)
    {
        if (mpidr == phy_mpidr_list[pe_index])
            return pe_index;
    }

    /* In case virtual mpidr returned for realm */
    for (pe_index = 0; pe_index < total_pe_num; pe_index++)
    {
        if (mpidr == pe_index)
            return pe_index;
    }

    return PAL_INVALID_MPID;
}
#endif //TARGET_BM_BOOT
