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

#ifndef __SBSA_VAL_INTERFACE_H__
#define __SBSA_VAL_INTERFACE_H__

#include "sbsa_pal_interface.h"

#define INVALID_NAMED_COMP_INFO 0xFFFFFFFFFFFFFFFFULL

#if defined(TARGET_EMULATION) || defined(TARGET_BM_BOOT)
#define BIT0 (1)
#define BIT1 (1 << 1)
#define BIT4 (1 << 4)
#define BIT6 (1 << 6)
#define BIT14 (1 << 14)
#define BIT29 (1 << 29)
#endif


/* PE related APIs */
uint32_t val_pe_get_index_uid(uint32_t uid);
uint32_t val_pe_get_uid(uint64_t mpidr);

#if defined(TARGET_LINUX) || defined(TARGET_EMULATION)
uint32_t val_get_device_path(const char *hid, char hid_path[][MAX_NAMED_COMP_LENGTH]);
uint32_t val_smmu_is_etr_behind_catu(char *etr_path);
#endif

/* PCIE VAL APIs */
void val_pcie_enable_ordering(uint32_t bdf);
void val_pcie_disable_ordering(uint32_t bdf);
uint32_t val_pcie_dsm_ste_tags(void);

/* NIST VAL APIs */
uint32_t val_nist_generate_rng(uint32_t *rng_buffer);

/* PMU test related APIS*/
void     val_pmu_create_info_table(uint64_t *pmu_info_table);
void     val_pmu_free_info_table(void);


uint64_t val_get_primary_mpidr(void);

/* RAS APIs */
#define INVALID_RAS2_INFO 0xFFFFFFFFFFFFFFFFULL
#define INVALID_RAS_REG_VAL 0xDEADDEADDEADDEADULL
#define RAS2_FEATURE_TYPE_MEMORY 0x0

typedef enum {
  RAS2_NUM_MEM_BLOCK,
  RAS2_PROX_DOMAIN,
  RAS2_SCRUB_SUPPORT
} RAS2_MEM_INFO_e;

uint32_t val_ras_create_info_table(uint64_t *ras_info_table);
uint32_t val_ras_get_info(uint32_t info_type, uint32_t param1, uint64_t *ret_data);
void val_ras2_create_info_table(uint64_t *ras2_info_table);
void val_ras2_free_info_table(void);
uint64_t val_ras2_get_mem_info(RAS2_MEM_INFO_e type, uint32_t index);

/* ETE */
uint32_t val_sbsa_ete_execute_tests(uint32_t level, uint32_t num_pe);

#define PMU_INVALID_INFO 0xFFFFFFFFFFFFFFFF
#define PMU_INVALID_INDEX 0xFFFFFFFF

/* PMU info request types */
typedef enum {
  PMU_NODE_TYPE,       /* PMU Node type               */
  PMU_NODE_BASE0,      /* Page 0 Base address         */
  PMU_NODE_BASE1,      /* Page 1 Base address         */
  PMU_NODE_PRI_INST,   /* Primary instance            */
  PMU_NODE_SEC_INST,   /* Secondary instance          */
  PMU_NODE_COUNT,      /* PMU Node count              */
  PMU_NODE_DP_EXTN,    /* Dual page extension support */
  PMU_NODE_CS_COM,     /* Node is Coresight arch complaint */
} PMU_INFO_e;

uint32_t val_sbsa_pe_execute_tests(uint32_t level, uint32_t num_pe);
uint32_t val_sbsa_gic_execute_tests(uint32_t level, uint32_t num_pe);
uint32_t val_sbsa_pcie_execute_tests(uint32_t level, uint32_t num_pe);
uint32_t val_sbsa_wd_execute_tests(uint32_t level, uint32_t num_pe);
uint32_t val_sbsa_timer_execute_tests(uint32_t level, uint32_t num_pe);
uint32_t val_sbsa_memory_execute_tests(uint32_t level, uint32_t num_pe);
uint32_t val_sbsa_smmu_execute_tests(uint32_t level, uint32_t num_pe);
uint32_t val_sbsa_exerciser_execute_tests(uint32_t level);
uint32_t val_sbsa_pmu_execute_tests(uint32_t level, uint32_t num_pe);
uint32_t val_sbsa_mpam_execute_tests(uint32_t level, uint32_t num_pe);
uint32_t val_sbsa_ras_execute_tests(uint32_t level, uint32_t num_pe);
uint32_t val_sbsa_nist_execute_tests(uint32_t level, uint32_t num_pe);

#endif
