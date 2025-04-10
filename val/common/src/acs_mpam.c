/** @file
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "common/include/acs_mmu.h"
#include "common/include/acs_mpam.h"
#include "common/include/acs_memory.h"
#include "common/include/acs_common.h"
#include "common/include/acs_memory.h"
#include "sbsa/include/sbsa_val_interface.h"
#include "common/include/acs_mpam_reg.h"

static MPAM_INFO_TABLE *g_mpam_info_table;
static SRAT_INFO_TABLE *g_srat_info_table;
static HMAT_INFO_TABLE *g_hmat_info_table;

uint8_t **g_shared_memcpy_buffer;

/**
  @brief   This API provides a 'C' interface to call MPAM system register reads
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   reg_id  - the system register index for which data is returned.
  @return  the value read from the system register.
**/
uint64_t
val_mpam_reg_read(MPAM_SYS_REGS reg_id)
{
  switch (reg_id) {
  case MPAMIDR_EL1:
      return AA64ReadMpamidr();
  case MPAM2_EL2:
      return AA64ReadMpam2();
  case MPAM1_EL1:
      return AA64ReadMpam1();
  default:
      val_report_status(val_pe_get_index_mpid(val_pe_get_mpid()),
                        RESULT_FAIL(0, STATUS_SYS_REG_ACCESS_FAIL), NULL);
  }

  return 0;
}

/**
  @brief   This API provides a 'C' interface to call MPAM system register writes
           1. Caller       -  Test Suite
           2. Prerequisite -  None
  @param   reg_id  - the system register index for which data is written
  @param   write_data - the 64-bit data to write to the system register
  @return  None
**/
void
val_mpam_reg_write(MPAM_SYS_REGS reg_id, uint64_t write_data)
{
  switch (reg_id) {
  case MPAM2_EL2:
      AA64WriteMpam2(write_data);
      break;
  case MPAM1_EL1:
      AA64WriteMpam1(write_data);
      break;
  default:
      val_report_status(val_pe_get_index_mpid(val_pe_get_mpid()),
                        RESULT_FAIL(0, STATUS_SYS_REG_ACCESS_FAIL), NULL);
  }

  return;
}

/**
  @brief   This API returns requested MSC or resource info.

  @param   type       - the type of information being requested.
  @param   msc_index  - index of the MSC node in the MPAM info table.
  @param   rsrc_index - index of the resource node in the MPAM MSC node if resource
                        related info is requested, set this parameter 0 otherwise.

  @return  requested data if found, otherwise MPAM_INVALID_INFO.
**/
uint64_t
val_mpam_get_info(MPAM_INFO_e type, uint32_t msc_index, uint32_t rsrc_index)
{
  uint32_t i = 0;
  MPAM_MSC_NODE *msc_entry;

  if (g_mpam_info_table == NULL) {
      val_print(ACS_PRINT_WARN, "\n   MPAM info table not found", 0);
      return MPAM_INVALID_INFO;
  }

  if (msc_index > g_mpam_info_table->msc_count) {
      val_print(ACS_PRINT_ERR, "Invalid MSC index = 0x%lx ", msc_index);
      return 0;
  }

  /* Walk the MPAM info table and return requested info */
  msc_entry = &g_mpam_info_table->msc_node[0];
  for (i = 0; i < g_mpam_info_table->msc_count; i++, msc_entry = MPAM_NEXT_MSC(msc_entry)) {
      if (msc_index == i) {
          if (rsrc_index > msc_entry->rsrc_count - 1) {
              val_print(ACS_PRINT_ERR,
                      "\n   Invalid MSC resource index = 0x%lx for", rsrc_index);
              val_print(ACS_PRINT_ERR, "MSC index = 0x%lx ", msc_index);
              return MPAM_INVALID_INFO;
          }
          switch (type) {
          case MPAM_MSC_RSRC_COUNT:
              return msc_entry->rsrc_count;
          case MPAM_MSC_RSRC_RIS:
              return msc_entry->rsrc_node[rsrc_index].ris_index;
          case MPAM_MSC_RSRC_TYPE:
              return msc_entry->rsrc_node[rsrc_index].locator_type;
          case MPAM_MSC_RSRC_DESC1:
              return msc_entry->rsrc_node[rsrc_index].descriptor1;
          case MPAM_MSC_RSRC_DESC2:
              return msc_entry->rsrc_node[rsrc_index].descriptor2;
          case MPAM_MSC_BASE_ADDR:
              return msc_entry->msc_base_addr;
          case MPAM_MSC_ADDR_LEN:
              return msc_entry->msc_addr_len;
          case MPAM_MSC_NRDY:
              return msc_entry->max_nrdy;
          case MPAM_MSC_OF_INTR:
              return msc_entry->of_intr;
          case MPAM_MSC_OF_INTR_FLAGS:
              return msc_entry->of_intr_flags;
          case MPAM_MSC_ERR_INTR:
              return msc_entry->err_intr;
          case MPAM_MSC_ERR_INTR_FLAGS:
              return msc_entry->err_intr_flags;
          case MPAM_MSC_ID:
              return msc_entry->identifier;
          case MPAM_MSC_INTERFACE_TYPE:
              return msc_entry->intrf_type;
          default:
              val_print(ACS_PRINT_ERR,
                       "\n   This MPAM info option for type %d is not supported", type);
              return MPAM_INVALID_INFO;
          }
      }
  }
  return MPAM_INVALID_INFO;
}

/**
  @brief   This API returns requested Base address or Address length or num of mem ranges info.
           1. Caller       - Test Suite
           2. Prerequisite - val_srat_create_info_table
  @param   type - the type of information being requested.
  @param   data - proximity domain or uid.

  @return  requested data if found, otherwise SRAT_INVALID_INFO.
**/
uint64_t
val_srat_get_info(SRAT_INFO_e type, uint64_t data)
{
  uint32_t i = 0;

  if (g_srat_info_table == NULL) {
      val_print(ACS_PRINT_WARN, "\n   SRAT info table not found", 0);
      return SRAT_INVALID_INFO;
  }

  switch (type) {
  case SRAT_MEM_NUM_MEM_RANGE:
      return g_srat_info_table->num_of_mem_ranges;
  case SRAT_MEM_BASE_ADDR:
      for (i = 0; i < g_srat_info_table->num_of_srat_entries; i++) {
          if (g_srat_info_table->srat_info[i].node_type == SRAT_NODE_MEM_AFF) {
              if (data == g_srat_info_table->srat_info[i].node_data.mem_aff.prox_domain) {
                  return g_srat_info_table->srat_info[i].node_data.mem_aff.addr_base;
              }
          }
      }
      break;
  case SRAT_MEM_ADDR_LEN:
      for (i = 0; i < g_srat_info_table->num_of_srat_entries; i++) {
          if (g_srat_info_table->srat_info[i].node_type == SRAT_NODE_MEM_AFF) {
              if (data == g_srat_info_table->srat_info[i].node_data.mem_aff.prox_domain) {
                  return g_srat_info_table->srat_info[i].node_data.mem_aff.addr_len;
              }
          }
      }
      break;
  case SRAT_GICC_PROX_DOMAIN:
      for (i = 0; i < g_srat_info_table->num_of_srat_entries; i++) {
          if (g_srat_info_table->srat_info[i].node_type == SRAT_NODE_GICC_AFF) {
              if (data == g_srat_info_table->srat_info[i].node_data.gicc_aff.proc_uid) {
                  return g_srat_info_table->srat_info[i].node_data.gicc_aff.prox_domain;
              }
          }
      }
      break;
  case SRAT_GICC_PROC_UID:
      for (i = 0; i < g_srat_info_table->num_of_srat_entries; i++) {
          if (g_srat_info_table->srat_info[i].node_type == SRAT_NODE_GICC_AFF) {
              if (data == g_srat_info_table->srat_info[i].node_data.gicc_aff.prox_domain) {
                  return g_srat_info_table->srat_info[i].node_data.gicc_aff.proc_uid;
              }
          }
      }
      return SRAT_INVALID_INFO;
      break;
  case SRAT_GICC_REMOTE_PROX_DOMAIN:
      for (i = 0; i < g_srat_info_table->num_of_srat_entries; i++) {
          if (g_srat_info_table->srat_info[i].node_type == SRAT_NODE_GICC_AFF) {
              if (g_srat_info_table->srat_info[i].node_data.gicc_aff.prox_domain != data) {
                  return g_srat_info_table->srat_info[i].node_data.gicc_aff.prox_domain;
              }
          }
      }
      return SRAT_INVALID_INFO;
      break;
  default:
      val_print(ACS_PRINT_ERR,
                    "\n    This SRAT info option for type %d is not supported", type);
      break;
  }
  return SRAT_INVALID_INFO;
}

/**
  @brief   This API returns proximity domain mapped to the memory range.

  @param   none
  @return  proximity domain
**/
uint64_t
val_srat_get_prox_domain(uint64_t mem_range_index)
{
  uint32_t  i = 0;

  if (g_srat_info_table == NULL) {
      val_print(ACS_PRINT_WARN, "\n   SRAT info table not found", 0);
      return SRAT_INVALID_INFO;
  }

  if (mem_range_index > g_srat_info_table->num_of_mem_ranges) {
      val_print(ACS_PRINT_WARN, "\n   Invalid index", 0);
      return SRAT_INVALID_INFO;
  }

  for (i = 0; i < g_srat_info_table->num_of_mem_ranges; i++) {
      if (g_srat_info_table->srat_info[i].node_type == SRAT_NODE_MEM_AFF) {
          if (mem_range_index == 0)
              return g_srat_info_table->srat_info[i].node_data.mem_aff.prox_domain;
          else
              mem_range_index--;
      }
  }
  return SRAT_INVALID_INFO;
}

/**
 * @brief   This API returns numbers of MPAM MSC nodes in the system.
 *
 * @param   None
 * @return  number of MPAM MSC nodes
 */
uint32_t
val_mpam_get_msc_count(void)
{
    if (g_mpam_info_table == NULL) {
        val_print(ACS_PRINT_WARN, "\n   MPAM info table not found", 0);
        return 0;
    }
    else
        return g_mpam_info_table->msc_count;
}

/**
 * @brief   This API returns maximum RIS value supported in MPAMCFG_PART_SEL.
 *
 * @param   msc_index - index of the MSC node in the MPAM info table.
 * @return  maximum RIS value supported.
 */
uint32_t
val_mpam_get_max_ris_count(uint32_t msc_index)
{
    if (val_mpam_msc_supports_ris(msc_index)) {
        return BITFIELD_READ(IDR_RIS_MAX, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));
    }

    return 0;
}

/**
  @brief   This API returns MSC MAPAM version
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  msc mpam version.
**/
uint32_t
val_mpam_msc_get_version(uint32_t msc_index)
{
    return BITFIELD_READ(AIDR_VERSION, val_mpam_mmr_read(msc_index, REG_MPAMF_AIDR));
}

/**
  @brief   This API checks whether resource monitoring is supported by the MPAM MSC.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_msc_supports_mon(uint32_t msc_index)
{
    return BITFIELD_READ(IDR_HAS_MSMON, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));
}

/**
  @brief   This API checks whether MSC has cache portion partitioning.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_supports_cpor(uint32_t msc_index)
{
    return BITFIELD_READ(IDR_HAS_CPOR_PART, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));
}

/**
  @brief   This API checks whether MSC has cache portion partitioning.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_supports_ccap(uint32_t msc_index)
{
    return BITFIELD_READ(IDR_HAS_CCAP_PART, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));
}

/**
  @brief   This API checks whether 64 bit MPAMF_IDR is implemented for the MSC.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_msc_supports_ext_idr(uint32_t msc_index)
{
    return BITFIELD_READ(IDR_EXT, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));
}

/**
  @brief   This API checks whether resource instance selection (RIS) implemented
            for the MSC.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_msc_supports_ris(uint32_t msc_index)
{
    if (val_mpam_msc_supports_ext_idr(msc_index))
      return BITFIELD_READ(IDR_HAS_RIS, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));

    return 0;
}

/**
  @brief   This API checks whether Error Status Register (ESR) is 64 bit
            for the MSC.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_msc_supports_extd_esr(uint32_t msc_index)
{
    if (val_mpam_msc_supports_ext_idr(msc_index))
      return BITFIELD_READ(IDR_HAS_EXTD_ESR, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));

    return 0;
}

/**
  @brief   This API checks whether Error Status Register (ESR) is implemented
            for the MSC.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_msc_supports_esr(uint32_t msc_index)
{
    if (val_mpam_msc_supports_ext_idr(msc_index))
      return BITFIELD_READ(IDR_HAS_ESR, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));

    return 0;
}

/**
  @brief   This API checks if the MSC supports Memory Bandwidth Usage Monitor (MBWU)
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_msc_supports_mbwumon(uint32_t msc_index)
{
    if (val_mpam_msc_supports_mon(msc_index))
        return BITFIELD_READ(MSMON_IDR_MSMON_MBWU,
                   val_mpam_mmr_read(msc_index, REG_MPAMF_MSMON_IDR));
    else
        return 0;
}

/**
  @brief   This API checks if the MSC supports Memory Bandwidth Partitioning
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_msc_supports_mbwpart(uint32_t msc_index)
{

  return BITFIELD_READ(IDR_HAS_MBW_PART,
                   val_mpam_mmr_read(msc_index, REG_MPAMF_IDR));
}

/**
  @brief   This API checks if the MSC supports Memory Bandwidth Portion Bit Map
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_msc_supports_mbwpbm(uint32_t msc_index)
{

    if (val_mpam_msc_supports_mbwpart(msc_index))
        return BITFIELD_READ(HAS_PBM,
                   val_mpam_mmr_read(msc_index, REG_MPAMF_MBW_IDR));
    else
        return 0;
}

/**
  @brief   This API checks if the MSC supports Memory Bandwidth Minimum limit Partitioning
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_msc_supports_mbw_min(uint32_t msc_index)
{

    if (val_mpam_msc_supports_mbwpart(msc_index))
        return BITFIELD_READ(HAS_MIN,
                   val_mpam_mmr_read(msc_index, REG_MPAMF_MBW_IDR));
    else
        return 0;
}

/**
  @brief   This API checks if the MSC supports Memory Bandwidth Maximum limit Partitioning
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_msc_supports_mbw_max(uint32_t msc_index)
{

    if (val_mpam_msc_supports_mbwpart(msc_index))
        return BITFIELD_READ(HAS_MAX,
                   val_mpam_mmr_read(msc_index, REG_MPAMF_MBW_IDR));
    else
        return 0;
}

/**
  @brief   This API checks if the MSC supports PARTID Narrowing feature
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_msc_supports_partid_nrw(uint32_t msc_index)
{

  return BITFIELD_READ(IDR_HAS_PARTID_NRW,
                   val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));
}

/**
  @brief   This API returns number of MBWU monitors in an MSC
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_get_mbwumon_count(uint32_t msc_index)
{
    return BITFIELD_READ(MBWUMON_IDR_NUM_MON, val_mpam_mmr_read(msc_index, REG_MPAMF_MBWUMON_IDR));
}

/**
  @brief   This API returns max bandwidth supported by a memory interface with mbwu attached
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint64_t
val_mpam_msc_get_mscbw(uint32_t msc_index, uint32_t rsrc_index)
{
    uint64_t prox_domain;
    uint32_t i = 0;

    prox_domain = val_mpam_get_info(MPAM_MSC_RSRC_DESC1, msc_index, rsrc_index);

    if (g_hmat_info_table == NULL) {
        val_print(ACS_PRINT_WARN, "\n   HMAT info table not found", 0);
        return HMAT_INVALID_INFO;
    }

    for (i = 0; i < g_hmat_info_table->num_of_mem_prox_domain ; i++) {
        if (g_hmat_info_table->bw_info[i].mem_prox_domain == prox_domain) {
            return (g_hmat_info_table->bw_info[i].write_bw +
                                   g_hmat_info_table->bw_info[i].read_bw);
        }
    }
    val_print(ACS_PRINT_WARN, "\n       Invalid Proximity domain 0x%lx", prox_domain);
    return HMAT_INVALID_INFO;
}

/**
  @brief   This API checks if the MBWU supports 44-bit ot 64-bit counter
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_mbwu_supports_long(uint32_t msc_index)
{
    return BITFIELD_READ(MBWUMON_IDR_HAS_LONG,
                val_mpam_mmr_read(msc_index, REG_MPAMF_MBWUMON_IDR));
}

/**
  @brief   This API checks if the MBWU supports 64 bit counter
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_mbwu_supports_lwd(uint32_t msc_index)
{
    return BITFIELD_READ(MBWUMON_IDR_LWD, val_mpam_mmr_read(msc_index, REG_MPAMF_MBWUMON_IDR));
}

/**
  @brief   This API checks if the MSC supports Cache Usage Monitor (CSU)
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_supports_csumon(uint32_t msc_index)
{
    if (val_mpam_msc_supports_mon(msc_index))
        return BITFIELD_READ(MSMON_IDR_MSMON_CSU,
                   val_mpam_mmr_read(msc_index, REG_MPAMF_MSMON_IDR));
    else
        return 0;
}

/**
  @brief   This API checks if the MSC supports Cache Usage Monitor (CSU)
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  1 if supported 0 otherwise.
**/
uint32_t
val_mpam_get_csumon_count(uint32_t msc_index)
{
    return BITFIELD_READ(CSUMON_IDR_NUM_MON, val_mpam_mmr_read(msc_index, REG_MPAMF_CSUMON_IDR));
}

/**
  @brief   This API configures MPAM MBWU selection registers.
           Prerequisite - val_mpam_msc_supports_ris
           This API should only be used for MSC supporting RIS.

  @param   msc_index  - index of the MSC node in the MPAM info table.
  @param   rsrc_index - index of the resource node in the MPAM MSC node.

  @return  null
**/
void
val_mpam_memory_configure_ris_sel(uint32_t msc_index, uint32_t rsrc_index)
{
    uint32_t data;
    uint8_t ris_index;

    ris_index = val_mpam_get_info(MPAM_MSC_RSRC_RIS, msc_index, rsrc_index);

    /*configure MSMON_CFG_MON_SEL.RIS field and write MSMON_CFG_MON_SEL.MON_SEL
       field to be 0 */
    data = BITFIELD_SET(MON_SEL_RIS, ris_index);
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MON_SEL, data);

    /* configure MPAMCFG_PART_SEL.RIS field and write MPAMCFG_PART_SEL.
       PARTID_SEL field to DEFAULT PARTID*/
    data = BITFIELD_SET(PART_SEL_RIS, ris_index)
                           | BITFIELD_SET(PART_SEL_PARTID_SEL, DEFAULT_PARTID);
    val_mpam_mmr_write(msc_index, REG_MPAMCFG_PART_SEL, data);
}

/**
  @brief   This API configures the bandwidth usage monitor.
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
                        - MSC should support MBWU monitoring, can be checked
                          using val_mpam_msc_supports_mbwumon API.

  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  None
**/
void
val_mpam_memory_configure_mbwumon(uint32_t msc_index)
{
    uint32_t data = 0;

    /* select monitor instance zero by writing zero to MSMON_CFG_MON_SEL.MON_SEL */
    data = val_mpam_mmr_read(msc_index, REG_MSMON_CFG_MON_SEL);
    /* retaining other configured fields e.g, RIS index if supported */
    data = BITFIELD_WRITE(data, MON_SEL_MON_SEL, 0);
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MON_SEL, data);

    /* disable monitor instance before configuration */
    val_mpam_memory_mbwumon_disable(msc_index);

    /* configure monitor ctrl reg for default partid and default pmg */
    data = BITFIELD_SET(MBWU_CTL_MATCH_PARTID, 1) | BITFIELD_SET(MBWU_CTL_MATCH_PMG, 1);
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MBWU_CTL, data);

    /* Check if MPAMF_MBWUMON_IDR supports RW bandwidth selection */
    if (BITFIELD_READ(MBWUMON_IDR_HAS_RWBW, val_mpam_mmr_read64(msc_index, REG_MPAMF_MBWUMON_IDR)))
    {
        /* If true, configure monitor filter reg to count both read and write bandwidth */
        data = BITFIELD_SET(MBWU_FLT_RWBW, MBWU_FLT_RWBW_RW);
        val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MBWU_FLT, data);
    }

    /* configure monitor filter reg for default partid and default pmg */
    data = BITFIELD_SET(MBWU_FLT_PARTID, DEFAULT_PARTID) | BITFIELD_SET(MBWU_FLT_PMG, DEFAULT_PMG);
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MBWU_FLT, data);

    /* reset the MBWU monitor count */
    val_mpam_memory_mbwumon_reset(msc_index);
}

/**
  @brief   This API enables the bandwidth usage monitor.
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
                        - MSC should support MBWU monitoring, can be checked
                          using val_mpam_msc_supports_mbwumon API..

  @param   msc_index - index of the MSC node in the MPAM info table.
**/
void
val_mpam_memory_mbwumon_enable(uint32_t msc_index)
{
    /* enable the monitor instance to collect information according to the configuration */
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MBWU_CTL, BITFIELD_SET(MBWU_CTL_EN, 1));
}

/**
  @brief   This API disables the bandwidth usage monitor.
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
                        - MSC should support MBWU monitoring, can be checked
                          using val_mpam_msc_supports_mbwumon API.

  @param   msc_index - index of the MSC node in the MPAM info table.
**/
void
val_mpam_memory_mbwumon_disable(uint32_t msc_index)
{
    /* disable the monitor */
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MBWU_CTL, BITFIELD_SET(MBWU_CTL_EN, 0));
}

/**
  @brief   This API reads the MBWU montior counter value.
           Prerequisite - val_mpam_memory_configure_mbwumon,
           This API can be called only after configuring MBWU monitor.

  @param   msc_index  - MPAM feature page index for this MSC.
  @return  MPAM_MON_NOT_READY if monitor has Not Ready status,
           else counter value.
**/
uint64_t
val_mpam_memory_mbwumon_read_count(uint32_t msc_index)
{
    uint64_t count = MPAM_MON_NOT_READY;

    /*if MSMON_MBWU_L is implemented*/
    if (BITFIELD_READ(MBWUMON_IDR_LWD, val_mpam_mmr_read64(msc_index, REG_MPAMF_MBWUMON_IDR))) {
        if (BITFIELD_READ(MBWUMON_IDR_HAS_LONG,
            val_mpam_mmr_read64(msc_index, REG_MPAMF_MBWUMON_IDR))) {
            // (63 bits)
            if (BITFIELD_READ(MSMON_MBWU_L_NRDY,
                val_mpam_mmr_read64(msc_index, REG_MSMON_MBWU_L)) == 0)
                count = BITFIELD_READ(MSMON_MBWU_L_63BIT_VALUE,
                                      val_mpam_mmr_read64(msc_index, REG_MSMON_MBWU_L));
        }
        else {
            // (44 bits)
            if (BITFIELD_READ(MSMON_MBWU_L_NRDY,
                val_mpam_mmr_read64(msc_index, REG_MSMON_MBWU_L)) == 0)
                count = BITFIELD_READ(MSMON_MBWU_L_44BIT_VALUE,
                                      val_mpam_mmr_read64(msc_index, REG_MSMON_MBWU_L));
        }
    }
    else {
        // (31 bits)
        if (BITFIELD_READ(MSMON_MBWU_NRDY, val_mpam_mmr_read(msc_index, REG_MSMON_MBWU)) == 0) {
            count = BITFIELD_READ(MSMON_MBWU_VALUE,
                                  val_mpam_mmr_read(msc_index, REG_MSMON_MBWU));
            /* shift the count if scaling is enabled */
            count = count << BITFIELD_READ(MBWUMON_IDR_SCALE,
                                  val_mpam_mmr_read(msc_index, REG_MPAMF_MBWUMON_IDR));
        }
    }
    return(count);
}

/**
  @brief   This API resets the MBWU montior counter value.
           Prerequisite - val_mpam_memory_configure_mbwumon,
           This API can be called only after configuring MBWU monitor.

  @param   msc_index  - MPAM feature page index for this MSC.
  @return  None
**/
void
val_mpam_memory_mbwumon_reset(uint32_t msc_index)
{
    /*if MSMON_MBWU_L is implemented*/
    if (BITFIELD_READ(MBWUMON_IDR_LWD, val_mpam_mmr_read64(msc_index, REG_MPAMF_MBWUMON_IDR)))
        val_mpam_mmr_write64(msc_index, REG_MSMON_MBWU_L, 0);
    else
       val_mpam_mmr_write(msc_index, REG_MSMON_MBWU, 0);
}


/**
  @brief   Creates a buffer with length equal to size within the
           address range (mem_base, mem_base + mem_size)

  @param   mem_base    - Base address of the memory range
  @param   size        - Buffer size to be created

  @return  Buffer address if SUCCESSFUL, else NULL
**/
void *
val_mem_alloc_at_address(uint64_t mem_base, uint64_t size)
{
  return pal_mem_alloc_at_address(mem_base, size);
}

/**
  @brief   Frees the allocated buffer with length equal to size within the
           address range (mem_base, mem_base + mem_size)

  @param   mem_base    - Base address of the memory range
  @param   size        - Buffer size to be created

  @return  Buffer address if SUCCESSFUL, else NULL
**/
void
val_mem_free_at_address(uint64_t mem_base, uint64_t size)
{
  return pal_mem_free_at_address(mem_base, size);
}
/**
  @brief   Creates a buffer with length equal to size within the
           address range (mem_base, mem_base + mem_size)

  @param   mem_base    - Base address of the memory range
  @param   mem_size    - Size of the memory range of interest
  @param   size        - Buffer size to be created

  @return  Buffer address if SUCCESSFUL, else NULL
**/
uint64_t val_mpam_memory_get_size(uint32_t msc_index, uint32_t rsrc_index)
{
    uint64_t prox_domain;

    prox_domain = val_mpam_get_info(MPAM_MSC_RSRC_DESC1, msc_index, rsrc_index);
    return val_srat_get_info(SRAT_MEM_ADDR_LEN, prox_domain);
}

/**
  @brief   Creates a buffer with length equal to size within the
           address range (mem_base, mem_base + mem_size)

  @param   mem_base    - Base address of the memory range
  @param   mem_size    - Size of the memory range of interest
  @param   size        - Buffer size to be created

  @return  Buffer address if SUCCESSFUL, else NULL
**/
uint64_t val_mpam_memory_get_base(uint32_t msc_index, uint32_t rsrc_index)
{
    uint64_t prox_domain;

    prox_domain = val_mpam_get_info(MPAM_MSC_RSRC_DESC1, msc_index, rsrc_index);
    return val_srat_get_info(SRAT_MEM_BASE_ADDR, prox_domain);
}

static
void
memory_map_msc(void)
{
  uint32_t msc_index;
  uint64_t msc_base;
  uint32_t msc_node_cnt = val_mpam_get_msc_count();

  for (msc_index = 0; msc_index < msc_node_cnt; msc_index++) {
    msc_base  = val_mpam_get_info(MPAM_MSC_BASE_ADDR, msc_index, 0);
    val_mmu_update_entry(msc_base, MPAM_MSC_REGISTER_SPACE);
  }

  return;
}

/**
  @brief   This API will call PAL layer to fill in the MPAM table information
           into the g_mpam_info_table pointer.
           1. Caller       -  Application layer.
           2. Prerequisite -  Memory allocated and passed as argument.
  @param   mpam_info_table  pre-allocated memory pointer for cache info.
  @return  Error if Input parameter is NULL
**/
void
val_mpam_create_info_table(uint64_t *mpam_info_table)
{
  if (mpam_info_table == NULL) {
    val_print(ACS_PRINT_ERR, "\n Pre-allocated memory pointer is NULL\n", 0);
    return;
  }

  g_mpam_info_table = (MPAM_INFO_TABLE *)mpam_info_table;
#ifndef TARGET_LINUX
  pal_mpam_create_info_table(g_mpam_info_table);

  val_print(ACS_PRINT_TEST,
                " MPAM INFO: Number of MSC nodes       :    %d\n", g_mpam_info_table->msc_count);
  val_print(ACS_PRINT_DEBUG, "Memory mapping MSC nodes\n", 0);

  /* TODO - Check if MSC memory mapping requires a flag/ cmdline option */
  memory_map_msc();
#endif
}

/**
  @brief   This API frees the memory allocated for MPAM info table.
  @param   None
  @return  None
**/

void
val_mpam_free_info_table(void)
{
    if (g_mpam_info_table != NULL) {
        pal_mem_free_aligned((void *)g_mpam_info_table);
        g_mpam_info_table = NULL;
    }
    else {
      val_print(ACS_PRINT_ERR,
                  "\n WARNING: g_mpam_info_table pointer is already NULL",
        0);
    }
}

/**
  @brief   This API will call PAL layer to fill in the HMAT table information
           into the g_hmat_info_table pointer.
           1. Caller       -  Application layer.
           2. Prerequisite -  Memory allocated and passed as argument.
  @param   hmat_info_table  pre-allocated memory pointer for cache info.
  @return  Error if Input parameter is NULL
**/
void
val_hmat_create_info_table(uint64_t *hmat_info_table)
{
  if (hmat_info_table == NULL) {
    val_print(ACS_PRINT_ERR, "\n Pre-allocated memory pointer is NULL\n", 0);
    return;
  }
#ifndef TARGET_LINUX
  g_hmat_info_table = (HMAT_INFO_TABLE *)hmat_info_table;

  pal_hmat_create_info_table(g_hmat_info_table);

  if (g_hmat_info_table->num_of_mem_prox_domain != 0)
      val_print(ACS_PRINT_TEST,
                " HMAT INFO: Number of Prox domains    :    %d\n",
                                    g_hmat_info_table->num_of_mem_prox_domain);
#endif
}

/**
  @brief   This API frees the memory allocated for HMAT info table.
  @param   None
  @return  None
**/
void
val_hmat_free_info_table(void)
{
    pal_mem_free_aligned((void *)g_hmat_info_table);
}

/**
  @brief   This API will call PAL layer to fill in the SRAT table information
           into the g_hmat_info_table pointer.
           1. Caller       -  Application layer.
           2. Prerequisite -  Memory allocated and passed as argument.
  @param   srat_info_table  pre-allocated memory pointer for cache info.
  @return  Error if Input parameter is NULL
**/
void
val_srat_create_info_table(uint64_t *srat_info_table)
{
  if (srat_info_table == NULL) {
    val_print(ACS_PRINT_ERR, "\n Pre-allocated memory pointer is NULL\n", 0);
    return;
  }
#ifndef TARGET_LINUX
  g_srat_info_table = (SRAT_INFO_TABLE *)srat_info_table;

  pal_srat_create_info_table(g_srat_info_table);

  if (g_srat_info_table->num_of_mem_ranges != 0)
      val_print(ACS_PRINT_TEST,
                " SRAT INFO: Number of Memory Ranges   :    %d\n",
                                    g_srat_info_table->num_of_mem_ranges);
#endif
}

/**
  @brief   This API frees the memory allocated for SRAT info table.
  @param   None
  @return  None
**/
void
val_srat_free_info_table(void)
{
    pal_mem_free_aligned((void *)g_srat_info_table);
}

/**
  @brief   This API gets maximum supported value of PMG
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  PMG value.
**/
uint32_t
val_mpam_get_max_pmg(uint32_t msc_index)
{
    return BITFIELD_READ(IDR_PMG_MAX, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));
}

/**
  @brief   This API gets Maximum supported value of PARTID
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  Partion ID value.
**/
uint32_t
val_mpam_get_max_partid(uint32_t msc_index)
{
    return BITFIELD_READ(IDR_PARTID_MAX, val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR));
}

/**
  @brief   This API gets Maximum supported value of Internal PARTID
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  Partion ID value.
**/
uint16_t
val_mpam_get_max_intpartid(uint32_t msc_index)
{
    return BITFIELD_READ(INTPARTID_MAX, val_mpam_mmr_read(msc_index, REG_MPAMF_PARTID_NRW_IDR));
}

/**
  @brief   This API gets number of fractional bits implemented in CCAP control.
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  Partion ID value.
**/
uint32_t
val_mpam_get_cmax_wd(uint32_t msc_index)
{
    return BITFIELD_READ(CMAX_WD, val_mpam_mmr_read(msc_index, REG_MPAMF_CCAP_IDR));
}

/**
  @brief   This API gets number of fractional bits implemented in CCAP control.
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  Partion ID value.
**/
uint32_t
val_mpam_get_bwa_wd(uint32_t msc_index)
{
    return BITFIELD_READ(BWA_WD, val_mpam_mmr_read(msc_index, REG_MPAMF_MBW_IDR));
}
/**
  @brief   This API Configures CPOR settings for given MSC
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
                        - MSC should support CSU monitoring, can be checked
                          using val_mpam_supports_csumon API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @param   partid - PATRTID for CPOR configuration
  @param   cpbm_percentage - Percentage of cache to be partitioned
  @return  void.
**/
void
val_mpam_configure_cpor(uint32_t msc_index, uint16_t partid, uint32_t cpbm_percentage)
{
    uint16_t index;
    uint32_t unset_bitmask;
    uint32_t num_unset_bits;
    uint16_t num_cpbm_bits;
    uint32_t data;

    /* Get CPBM width */
    num_cpbm_bits = val_mpam_get_cpbm_width(msc_index);

    /* retaining other configured fields e.g, RIS index if supported */
    data = val_mpam_mmr_read(msc_index, REG_MPAMCFG_PART_SEL);

    /* Select PARTID */
    data = BITFIELD_WRITE(data, PART_SEL_PARTID_SEL, partid);
    val_mpam_mmr_write(msc_index, REG_MPAMCFG_PART_SEL, partid);

    /*
     * Configure CPBM register to have a 1 in cpbm_percentage
     * bits in the overall CPBM_WD bit positions
     */
    num_cpbm_bits = (num_cpbm_bits * cpbm_percentage) / 100 ;
    for (index = 0; index < (num_cpbm_bits - 31) && index < MAX_CPBM_WIDTH; index += 32)
        val_mpam_mmr_write(msc_index, REG_MPAMCFG_CPBM + (index / 8), CPOR_BITMAP_DEF_VAL);

    /* Unset bits from above step are set */
    num_unset_bits = num_cpbm_bits - index;
    unset_bitmask = (1 << num_unset_bits) - 1;
    if (unset_bitmask)
        val_mpam_mmr_write(msc_index, REG_MPAMCFG_CPBM + (index / 8), unset_bitmask);

    /* Issue a DSB instruction */
    val_mem_issue_dsb();

    return;
}

/**
  @brief   This API Configures CCAP settings for given MSC
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @param   partid - PATRTID for CCAP configuration
  @param   softlim - Enable/ Disable soft limiting for CCAP.
  @param   ccap_percentage - Percentage of cache to be partitioned
  @return  void.
**/
void val_mpam_configure_ccap(uint32_t msc_index, uint16_t partid,
                                                 uint8_t softlim, uint32_t ccap_percentage)
{

    uint8_t num_fractional_bits;
    uint16_t fixed_point_fraction;

    num_fractional_bits = val_mpam_get_cmax_wd(msc_index);
    fixed_point_fraction = ((1 << num_fractional_bits) * ccap_percentage / 100) - 1;

    /* Select the PARTID to configure capacity partition parameters */
    val_mpam_mmr_write(msc_index, REG_MPAMCFG_PART_SEL, partid);

    /*
     * Configure the CMAX register for the max capacity.
     * Use num_fractional_bits fixed-point representation
     */
    val_mpam_mmr_write(msc_index, REG_MPAMCFG_CMAX,
                      (softlim << MPAMCFG_CMAX_SOFTLIM_SHIFT) |
                      ((fixed_point_fraction << (16 - num_fractional_bits)) & 0xFFFF));

    val_mem_issue_dsb();
    return;
}

/**
  @brief   This API Configures Memory Bandwidth partition settings for given MSC
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
  @param   msc_index - index of the MSC node in the MPAM info table.
  @param   partid - PATRTID for MBW portion partition configuration
  @param   mbwpbm_percentage - Percentage of memory bandwidth to be partitioned
  @return  void.
**/
void
val_mpam_configure_mbwpbm(uint32_t msc_index, uint16_t partid, uint32_t mbwpbm_percentage)
{

    uint16_t index;
    uint32_t unset_bitmask;
    uint32_t num_unset_bits;
    uint16_t num_mbwpbm_bits;

    num_mbwpbm_bits = val_mpam_get_mbwpbm_width(msc_index);

    /* Select the PARTID to configure portion partition parameters */
    val_mpam_mmr_write(msc_index, REG_MPAMCFG_PART_SEL, partid);

    /*
     * Configure MBWPBM register to have a 1 in mbwpbm_percentage
     * bits in the overall MBWBM_WD bit positions
     */
    num_mbwpbm_bits = num_mbwpbm_bits * mbwpbm_percentage / 100;
    for (index = 0; index < (num_mbwpbm_bits - 31) && index < MAX_BWPBM_WIDTH; index += 32) {
        val_mpam_mmr_write(msc_index, REG_MPAMCFG_MBW_PBM + (index / 8), MBWPOR_BITMAP_DEF_VAL);
    }

    num_unset_bits = num_mbwpbm_bits - index;
    unset_bitmask = (1 << num_unset_bits) - 1;
    if (unset_bitmask) {
        val_mpam_mmr_write(msc_index, REG_MPAMCFG_MBW_PBM + (index / 8), unset_bitmask);
    }

    val_mem_issue_dsb();
    return;
}

/**
  @brief   This API configures MSC settings for Minimum Memory Bandwidth limit partitioning

  @param   msc_index         - index of the MSC node in the MPAM info table.
  @param   partid            - PATRTID for MBW portion partition configuration
  @param   mbwmin_percentage - Min percentage of memory bandwidth to be allocated for partid.

  @return  None
**/
void val_mpam_msc_configure_mbwmin(uint32_t msc_index, uint16_t partid, uint32_t mbwmin_percentage)
{
    uint8_t num_fractional_bits;
    uint16_t fixed_point_fraction;

    num_fractional_bits = val_mpam_get_bwa_wd(msc_index);
    fixed_point_fraction = ((1 << num_fractional_bits) * mbwmin_percentage / 100) - 1;

    /* Select the PARTID to configure minimum bandwidth limit parameters */
    val_mpam_mmr_write(msc_index, REG_MPAMCFG_PART_SEL, partid);

    /*
     * Configure the MBW_MIN register for minimum bandwidth limit.
     * Use num_fractional_bits fixed-point representation
     */
    val_mpam_mmr_write(msc_index, REG_MPAMCFG_MBW_MIN,
                                  (fixed_point_fraction << (16 - num_fractional_bits)) & 0xFFFF);

    /* Issue a DSB instruction */
    val_mem_issue_dsb();
    return;
}

/**
  @brief   This API configures MSC settings for Maximum Memory Bandwidth limit partitioning

  @param   msc_index         - index of the MSC node in the MPAM info table.
  @param   partid            - PATRTID for MBW portion partition configuration
  @param   hardlim           - Enable/ Disable hard limiting for MBW MAX.
  @param   mbwmax_percentage - Max percentage of memory bandwidth to be allocated for partid.

  @return  None
**/
void val_mpam_msc_configure_mbwmax(uint32_t msc_index, uint16_t partid,
                                                      uint8_t hardlim, uint32_t mbwmax_percentage)
{
    uint8_t num_fractional_bits;
    uint16_t fixed_point_fraction;

    num_fractional_bits = val_mpam_get_bwa_wd(msc_index);
    fixed_point_fraction = ((1 << num_fractional_bits) * mbwmax_percentage / 100) - 1;

    /* Select the PARTID to configure maximum bandwidth partition parameters */
    val_mpam_mmr_write(msc_index, REG_MPAMCFG_PART_SEL, partid);

    /*
     * Configure the MBW_MAX register for maximum bandwidth limit.
     * Use num_fractional_bits fixed-point representation
     */
    val_mpam_mmr_write(msc_index, REG_MPAMCFG_MBW_MAX,
                      (hardlim << MPAMCFG_MBW_MAX_HARDLIM_SHIFT) |
                      ((fixed_point_fraction << (16 - num_fractional_bits)) & 0xFFFF));

    /* Issue a DSB instruction */
    val_mem_issue_dsb();

    return;
}

/**
  @brief   This API gets CPBM width
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  Number of bits in the cache portion partitioning bit map.
**/
uint32_t
val_mpam_get_cpbm_width(uint32_t msc_index)
{
    if (val_mpam_supports_cpor(msc_index))
        return BITFIELD_READ(CPOR_IDR_CPBM_WD, val_mpam_mmr_read(msc_index, REG_MPAMF_CPOR_IDR));
    else
        return 0;
}

/**
  @brief   This API gets MBWPBM width
  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  Number of bits in the Memory Bandwidth portion partitioning bit map.
**/
uint32_t
val_mpam_get_mbwpbm_width(uint32_t msc_index)
{
    if (val_mpam_msc_supports_mbwpbm(msc_index))
        return BITFIELD_READ(BWPBM_WD, val_mpam_mmr_read(msc_index, REG_MPAMF_MBW_IDR));
    else
        return 0;
}

/**
  @brief   This API configures the cache storage usage monitor.
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
                        - MSC should support CSU monitoring, can be checked
                          using val_mpam_supports_csumon API.

  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  None
**/
void
val_mpam_configure_csu_mon(uint32_t msc_index, uint16_t partid, uint8_t pmg, uint16_t mon_sel)
{
    uint32_t data;

    /* retaining other configured fields e.g, RIS index if supported */
    data = val_mpam_mmr_read(msc_index, REG_MSMON_CFG_MON_SEL);
    /* Select the monitor instance */
    data = BITFIELD_WRITE(data, MON_SEL_MON_SEL, mon_sel);
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_MON_SEL, data);

    /* Configure the CSU monitor filter register for input PARTID & PMG */
    data = BITFIELD_SET(CSU_FLT_PARTID, partid) | BITFIELD_SET(CSU_FLT_PMG, pmg);
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_CSU_FLT, data);

    /*Disable the monitor */
    val_mpam_csumon_disable(msc_index);

    /* Configure the CSU monitor control register to match input PARTID & PMG */
    data = BITFIELD_SET(CSU_CTL_MATCH_PARTID, 1) | BITFIELD_SET(CSU_CTL_MATCH_PMG, 1);
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_CSU_CTL, data);

    /* Reset CSU Monitor Value */
    /* if CSUMON_IDR.CSU_RO == 0, accesses to this register are RW */
    if (!BITFIELD_READ(CSUMON_IDR_CSU_RO, val_mpam_mmr_read(msc_index, REG_MPAMF_CSUMON_IDR))) {
       val_mpam_mmr_write(msc_index, REG_MSMON_CSU, 0);
    }

    /* Issue a DSB instruction */
    val_mem_issue_dsb();

    return;
}

/**
  @brief   This API enables the Cache storage usage monitor.
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
                        - MSC should support CSU monitoring, can be checked
                          using val_mpam_supports_csumon API..

  @param   msc_index - index of the MSC node in the MPAM info table.
**/
void
val_mpam_csumon_enable(uint32_t msc_index)
{
    uint32_t data;

    /* enable the monitor instance to collect information according to the configuration */
    data = BITFIELD_WRITE(val_mpam_mmr_read(msc_index, REG_MSMON_CFG_CSU_CTL), CSU_CTL_EN, 1);
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_CSU_CTL, data);
}

/**
  @brief   This API disables the Cache strorage usage monitor.
           Prerequisite - If MSC supports RIS, Resource instance should be
                          selected using val_mpam_memory_configure_ris_sel
                          prior calling this API.
                        - MSC should support CSU monitoring, can be checked
                          using val_mpam_supports_csumon API.

  @param   msc_index - index of the MSC node in the MPAM info table.
  @return  None
**/
void
val_mpam_csumon_disable(uint32_t msc_index)
{
    uint32_t data;

    /* enable the monitor instance to collect information according to the configuration */
    data = BITFIELD_WRITE(val_mpam_mmr_read(msc_index, REG_MSMON_CFG_CSU_CTL), CSU_CTL_EN, 0);
    val_mpam_mmr_write(msc_index, REG_MSMON_CFG_CSU_CTL, data);
}

/**
  @brief   This API reads the CSU montior counter value.
           Prerequisite - val_mpam_configure_csu_mon,
           This API can be called only after configuring CSU monitor.

  @param   msc_index  - MPAM feature page index for this MSC.
  @return  0 if monitor has Not Ready status,
           else counter value.
**/
uint32_t
val_mpam_read_csumon(uint32_t msc_index)
{
    uint32_t count;

    if (BITFIELD_READ(MSMON_CSU_NRDY, val_mpam_mmr_read(msc_index, REG_MSMON_CSU)) == 0) {
        count = BITFIELD_READ(MSMON_CSU_VALUE,
                                      val_mpam_mmr_read(msc_index, REG_MSMON_CSU));
        return count;
    }
    return 0;
}

/**
  @brief   This API reads 32bit MPAM memory mapped register either
           via MMIO or PCC interface.

  @param   msc_index  - MPAM feature page index for this MSC.
  @param   reg_offset - Register offset address.

  @return  Read 32 bit value.
**/
uint32_t
val_mpam_mmr_read(uint32_t msc_index, uint32_t reg_offset)
{
  uint64_t base_addr;
  uint32_t intrf_type;
  uint32_t value;

  base_addr  = val_mpam_get_info(MPAM_MSC_BASE_ADDR, msc_index, 0);
  intrf_type = val_mpam_get_info(MPAM_MSC_INTERFACE_TYPE, msc_index, 0);

  if (intrf_type == MPAM_INTERFACE_TYPE_MMIO) {
      value = val_mmio_read(base_addr + reg_offset);
      val_print(ACS_PRINT_DEBUG, "\n       MPAM Read reg_offset : 0x%x", reg_offset);
      val_print(ACS_PRINT_DEBUG, " value : 0x%llx", value);
      return value;
  } else if (intrf_type == MPAM_INTERFACE_TYPE_PCC) {
      value = val_mpam_pcc_read(msc_index, reg_offset);
      val_print(ACS_PRINT_DEBUG, "\n       MPAM Read reg_offset : 0x%x", reg_offset);
      val_print(ACS_PRINT_DEBUG, " value : 0x%llx", value);
      return value;
  } else {
    val_print(ACS_PRINT_ERR,
              "\n    Invalid interface type reported for MPAM MSC index = %x", msc_index);
    return 0;  /* zero considered as safe return */
    }
}

/**
  @brief   This API reads 64bit MPAM memory mapped register either
           via MMIO or PCC interface.

  @param   msc_index  - MPAM feature page index for this MSC.
  @param   reg_offset - Register offset address.

  @return  Read 64 bit value.
**/
uint64_t
val_mpam_mmr_read64(uint32_t msc_index, uint32_t reg_offset)
{
  uint64_t base_addr;
  uint32_t intrf_type;
  uint64_t value;

  base_addr  = val_mpam_get_info(MPAM_MSC_BASE_ADDR, msc_index, 0);
  intrf_type = val_mpam_get_info(MPAM_MSC_INTERFACE_TYPE, msc_index, 0);

  if (intrf_type == MPAM_INTERFACE_TYPE_MMIO) {
      value = val_mmio_read64(base_addr + reg_offset);
      val_print(ACS_PRINT_DEBUG, "\n       MPAM Read reg_offset : 0x%x", reg_offset);
      val_print(ACS_PRINT_DEBUG, " value : 0x%llx", value);
      return value;
  } else if (intrf_type == MPAM_INTERFACE_TYPE_PCC) {
      /* PCC supports only supports 32 bit read at a time, hence reading twice
         and concating */
      value = ((uint64_t)val_mpam_pcc_read(msc_index, reg_offset + 4) << 32)
                                | val_mpam_pcc_read(msc_index, reg_offset);
      val_print(ACS_PRINT_DEBUG, "\n       MPAM Read reg_offset : 0x%x", reg_offset);
      val_print(ACS_PRINT_DEBUG, " value : 0x%llx", value);
      return value;
  } else {
    val_print(ACS_PRINT_ERR,
              "\n    Invalid interface type reported for MPAM MSC index = %x", msc_index);
    return 0;  /* zero considered as safe return */
  }
}

/**
  @brief   This API writes 32bit MPAM memory mapped register either
           via MMIO or PCC interface.

  @param   msc_index  - MPAM feature page index for this MSC.
  @param   reg_offset - Register offset address.
  @param   data       - Data to be written to register.

  @return  None
**/
void
val_mpam_mmr_write(uint32_t msc_index, uint32_t reg_offset, uint32_t data)
{
  uint64_t base_addr;
  uint32_t intrf_type;

  base_addr  = val_mpam_get_info(MPAM_MSC_BASE_ADDR, msc_index, 0);
  intrf_type = val_mpam_get_info(MPAM_MSC_INTERFACE_TYPE, msc_index, 0);

  if (intrf_type == MPAM_INTERFACE_TYPE_MMIO) {
      val_mmio_write(base_addr + reg_offset, data);
      val_print(ACS_PRINT_DEBUG, "\n       MPAM Write reg_offset : 0x%x", reg_offset);
      val_print(ACS_PRINT_DEBUG, " value : 0x%llx", data);
  } else if (intrf_type == MPAM_INTERFACE_TYPE_PCC) {
      val_mpam_pcc_write(msc_index, reg_offset, data);
      val_print(ACS_PRINT_DEBUG, "\n       MPAM Write reg_offset : 0x%x", reg_offset);
      val_print(ACS_PRINT_DEBUG, " value : 0x%llx", data);
  } else {
    val_print(ACS_PRINT_ERR,
              "\n    Invalid interface type reported for MPAM MSC index = %x", msc_index);
  }
  val_mem_issue_dsb();
}

/**
  @brief   This API writes 64bit MPAM memory mapped register either
           via MMIO or PCC interface.

  @param   msc_index  - MPAM feature page index for this MSC.
  @param   reg_offset - Register offset address.
  @param   data       - Data to be written to register.

  @return  None
**/
void
val_mpam_mmr_write64(uint32_t msc_index, uint32_t reg_offset, uint64_t data)
{
  uint64_t base_addr;
  uint32_t intrf_type;

  base_addr  = val_mpam_get_info(MPAM_MSC_BASE_ADDR, msc_index, 0);
  intrf_type = val_mpam_get_info(MPAM_MSC_INTERFACE_TYPE, msc_index, 0);

  if (intrf_type == MPAM_INTERFACE_TYPE_MMIO) {
      val_mmio_write64(base_addr + reg_offset, data);
      val_print(ACS_PRINT_DEBUG, "\n       MPAM Write reg_offset : 0x%x", reg_offset);
      val_print(ACS_PRINT_DEBUG, " value : 0x%llx", data);
  } else if (intrf_type == MPAM_INTERFACE_TYPE_PCC) {
      val_mpam_pcc_write(msc_index, reg_offset, (uint32_t)(data & 0xFFFFFFFF));
      val_mpam_pcc_write(msc_index, reg_offset + 4, (uint32_t)(data >> 32));
      val_print(ACS_PRINT_DEBUG, "\n       MPAM Write reg_offset : 0x%x", reg_offset);
      val_print(ACS_PRINT_DEBUG, " value : 0x%llx", data);
  } else {
    val_print(ACS_PRINT_ERR,
              "\n    Invalid interface type reported for MPAM MSC index = %x", msc_index);
  }
}

/**
  @brief   This API constructs header and parameter for the
           MPAM_MSC_READ PCC command and calls doorbell protocol.

  @param   msc_index  - MPAM feature page index for this MSC.
  @param   reg_offset - Register offset address.

  @return  None
**/
uint32_t
val_mpam_pcc_read(uint32_t msc_index, uint32_t reg_offset)
{
  SCMI_PROTOCOL_MESSAGE_HEADER header;
  PCC_MPAM_MSC_READ_CMD_PARA parameter;
  PCC_MPAM_MSC_READ_RESP_PARA *response;
  uint32_t subspace_id;

  /* if MSC interface type is PCC (0x0A), the Base address field
     captures index to PCCT ACPI structure */
  subspace_id = (uint32_t)val_mpam_get_info(MPAM_MSC_BASE_ADDR, msc_index, 0);

  /* construct the message header */
  header.reserved = 0;
  header.protocol_id = MPAM_FB_PROTOCOL_ID;
  header.message_type = MPAM_MSG_TYPE_CMD;
  header.message_id = MPAM_MSC_READ_CMD_ID;
  /* token is user defined value for book keeping */
  header.token = 1;

  /* construct parameter payload */
  parameter.msc_id = val_mpam_get_info(MPAM_MSC_ID, msc_index, 0);
  parameter.flags = 0;
  parameter.offset = reg_offset;

  response = (PCC_MPAM_MSC_READ_RESP_PARA *) val_pcc_cmd_response(
              (uint32_t)subspace_id, *(uint32_t *)&header, (void *)&parameter, sizeof(parameter));

  if (response == NULL || response->status != MPAM_PCC_CMD_SUCCESS) {
      val_print(ACS_PRINT_ERR,
                "\n    Failed to read MPAM register with offset (0x%x) via PCC", reg_offset);
      val_print(ACS_PRINT_ERR, " for MSC index = 0x%x", msc_index);
      if (response != NULL) {
          val_print(ACS_PRINT_ERR, "\n    PCC command response code = 0x%x", response->status);
      }
      return MPAM_PCC_SAFE_RETURN;
  } else {
      return response->val;
  }
}

/**
  @brief   This API constructs header and parameter for the
           MPAM_MSC_WRITE PCC command and calls doorbell protocol.

  @param   msc_index  - MPAM feature page index for this MSC.
  @param   reg_offset - Register offset address.

  @return  None
**/
void
val_mpam_pcc_write(uint32_t msc_index, uint32_t reg_offset, uint32_t data)
{
  SCMI_PROTOCOL_MESSAGE_HEADER header;
  PCC_MPAM_MSC_WRITE_CMD_PARA parameter;
  PCC_MPAM_MSC_WRITE_RESP_PARA *response;
  uint32_t subspace_id;

  /* if MSC interface type is PCC (0x0A), the Base address field
     captures index to PCCT ACPI structure */
  subspace_id = val_mpam_get_info(MPAM_MSC_BASE_ADDR, msc_index, 0);

  /* construct the message header */
  header.reserved = 0;
  header.protocol_id = MPAM_FB_PROTOCOL_ID;
  header.message_type = MPAM_MSG_TYPE_CMD;
  header.message_id = MPAM_MSC_WRITE_CMD_ID;
  /* token is user defined value for book keeping */
  header.token = 1;

  /* construct parameter payload */
  parameter.msc_id = val_mpam_get_info(MPAM_MSC_ID, msc_index, 0);
  parameter.flags = 0;
  parameter.val = data;
  parameter.offset = reg_offset;

  response = (PCC_MPAM_MSC_WRITE_RESP_PARA *) val_pcc_cmd_response(
              (uint32_t)subspace_id, *(uint32_t *)&header, (void *)&parameter, sizeof(parameter));

  if (response == NULL || response->status != MPAM_PCC_CMD_SUCCESS) {
      val_print(ACS_PRINT_ERR,
                "\n    Failed to read MPAM register with offset (0x%x) via PCC", reg_offset);
      val_print(ACS_PRINT_ERR, " for MSC index = 0x%x", msc_index);
      if (response != NULL) {
          val_print(ACS_PRINT_ERR, "\n    PCC command response code = 0x%x", response->status);
      }
      return;
  }
  return;
}

/**
 * @brief   Free the shared memcopy buffers for num_pe PEs
 *
 * @param   num_pe        number of pes holding shared buffers
 *
 * @result  None
 */
void val_mem_free_shared_memcpybuf(uint32_t num_pe)
{
  uint32_t pe_index;

  for (pe_index = 0; pe_index < num_pe; pe_index++) {
      val_memory_free((void *)(uint64_t)g_shared_memcpy_buffer[pe_index]);
  }

  val_memory_free(g_shared_memcpy_buffer);
}

/**
 * @brief   Allocate a 2D buffers to share across PEs from specific input memory node
 *
 * @param   mem_base       base address of the memory node
 * @param   buffer_size    size of each shared buffer
 * @param   pe_count       number of pes to create shared buffers
 *
 * @result  status      1 for success, 0 for failure
 */
uint32_t val_alloc_shared_memcpybuf(uint64_t mem_base, uint64_t buffer_size, uint32_t pe_count)
{
  void *buffer;
  uint32_t pe_index;

  buffer = NULL;
  g_shared_memcpy_buffer = NULL;

  buffer = (void *)val_memory_alloc(pe_count * sizeof(uint64_t));

  if (!buffer) {
      val_print(ACS_PRINT_ERR, "Allocate Pool for shared memcpy buf failed\n", 0);
      return 0;
  }

  g_shared_memcpy_buffer = (uint8_t **)buffer;

  for (pe_index = 0; pe_index < pe_count; pe_index++) {
      g_shared_memcpy_buffer[pe_index] = (uint8_t *) val_mem_alloc_at_address (
                                                    mem_base + (pe_index * buffer_size),
                                                    buffer_size);

      if (g_shared_memcpy_buffer[pe_index] == NULL) {
          val_print(ACS_PRINT_ERR, "Alloc address for shared memcpy buffer failed %x\n", pe_index);
          val_mem_free_shared_memcpybuf(pe_index);
          return 0;
      }
  }

  pal_pe_data_cache_ops_by_va((uint64_t)&g_shared_memcpy_buffer, CLEAN_AND_INVALIDATE);

  return 1;
}

/**
 * @brief   This API returns the address to store memcopy latency on the current PE
 *
 * @param   pe_index    current PE index offset into latency buffer
 *
 * @return  64-bit source buf address, dest buf is assumed to be contiguous
 */
uint64_t val_get_shared_memcpybuf(uint32_t pe_index)
{
    return (uint64_t) (g_shared_memcpy_buffer[pe_index]);
}
