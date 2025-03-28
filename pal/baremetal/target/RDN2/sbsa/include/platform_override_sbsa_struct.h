/** @file
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
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

#include <stdio.h>
#include <stdint.h>
#include "platform_override_sbsa_fvp.h"

#define MAX_CS_COMP_LENGTH 256

typedef struct {
  char identifier[MAX_CS_COMP_LENGTH];    // Hardware ID for Coresight ARM implementations
  char dev_name[MAX_CS_COMP_LENGTH];      // Device name of the Coresight components
} PLATFORM_OVERRIDE_CORESIGHT_COMP_INFO_BLOCK;

typedef struct {
  PLATFORM_OVERRIDE_CORESIGHT_COMP_INFO_BLOCK component[CS_COMPONENT_COUNT];
} PLATFORM_OVERRIDE_CS_COMP_NODE_DATA;

typedef struct {
  uint32_t flags;             /* Cache flags */
  uint32_t offset;            /* Cache PPTT structure offset */
  uint32_t next_level_index;  /* Index of next level cache entry in CACHE_INFO_TABLE */
  uint32_t size;              /* Size of the cache in bytes */
  uint32_t cache_id;          /* Unique, non-zero identifier for this cache */
  uint32_t is_private;        /* Field indicate whether cache is private */
  uint8_t  cache_type;        /* Cache type */
} PLATFORM_OVERRIDE_CACHE_INFO_ENTRY;

typedef struct {
  uint32_t num_of_cache;                             /* Total of number of cache info entries */
  PLATFORM_OVERRIDE_CACHE_INFO_ENTRY cache_info[];  /* Array of cache info entries */
} PLATFORM_OVERRIDE_CACHE_INFO_TABLE;

typedef struct {
  uint32_t cache_id[MAX_L1_CACHE_RES];
} PLATFORM_OVERRIDE_PPTT_INFO;

typedef struct {
  PLATFORM_OVERRIDE_PPTT_INFO pptt_info[PLATFORM_OVERRIDE_PE_CNT];
} PLATFORM_OVERRIDE_PPTT_INFO_TABLE;

typedef struct {
  uint32_t   prox_domain;      /* Proximity domain*/
  uint32_t   proc_uid;         /* ACPI Processor UID */
  uint32_t   flags;            /* Flags*/
  uint32_t   clk_domain;       /* Clock Domain*/
} PLATFORM_OVERRIDE_SRAT_GICC_AFF_ENTRY;

typedef struct {
  uint32_t   prox_domain;     /* Proximity domain */
  uint32_t   flags;           /* flags */
  uint64_t   addr_base;       /* mem range address base */
  uint64_t   addr_len;        /* mem range address len */
} PLATFORM_OVERRIDE_SRAT_MEM_AFF_ENTRY;

typedef struct {
  PLATFORM_OVERRIDE_SRAT_MEM_AFF_ENTRY mem_aff[PLATFORM_OVERRIDE_MEM_AFF_CNT];
  PLATFORM_OVERRIDE_SRAT_GICC_AFF_ENTRY gicc_aff[PLATFORM_OVERRIDE_GICC_AFF_CNT];
} PLATFORM_OVERRIDE_SRAT_NODE_INFO_TABLE;

typedef struct {
  uint32_t mem_prox_domain;             /* Proximity domain of the memory region*/
  uint64_t max_write_bw;                    /* Maximum write bandwidth */
  uint64_t max_read_bw;                     /* Maximum read bandwidth */
} PLATFORM_OVERRIDE_HMAT_MEM_ENTRY;

typedef struct {
  PLATFORM_OVERRIDE_HMAT_MEM_ENTRY bw_mem_info[PLATFORM_OVERRIDE_HMAT_MEM_ENTRIES];
} PLATFORM_OVERRIDE_HMAT_MEM_TABLE;

typedef struct {
  uint32_t type;
  uint32_t data_type;             /* Proximity domain of the memory region*/
  uint32_t flags;                    /* Maximum write bandwidth */
  uint64_t entry_base_unit;                     /* Maximum read bandwidth */
} PLATFORM_OVERRIDE_HMAT_BW_ENTRY;

typedef struct {
  uint32_t num_of_prox_domain;      /* Number of Memory Proximity Domains */
  PLATFORM_OVERRIDE_HMAT_BW_ENTRY bw_info[];            /* Array of bandwidth info based on proximity domain */
} PLATFORM_OVERRIDE_HMAT_INFO_TABLE;

typedef struct {
  uint8_t  type;                /* The component that this PMU block is associated with */
  uint64_t primary_instance;    /* Primary node instance, specific to the PMU type */
  uint32_t secondary_instance;  /* Secondary node instance, specific to the PMU type */
  uint8_t  dual_page_extension; /* Support of the dual-page mode */
  uint64_t base0;               /* Base address of Page 0 of the PMU */
  uint64_t base1;               /* Base address of Page 1 of the PMU,
                                     valid only if dual_page_extension is 1 */
  uint32_t coresight_compliant;  /* node CS arch complaint */
} PLATFORM_OVERRIDE_PMU_INFO_BLOCK;

typedef struct {
  uint32_t  pmu_count; /* Total number of PMU info blocks*/
  /* PMU info blocks for each PMU nodes*/
  PLATFORM_OVERRIDE_PMU_INFO_BLOCK  pmu_info[PLATFORM_OVERRIDE_PMU_NODE_CNT];
} PLATFORM_OVERRIDE_PMU_INFO_TABLE;

/* RAS Information */

typedef struct {
  uint32_t  processor_id;
  uint32_t  resource_type;
  uint32_t  flags;
  uint64_t  affinity;
  uint64_t  res_specific_data;  /* Resource Specific Data */
} PLATFORM_OVERRIDE_RAS_NODE_PE_DATA;

typedef struct {
  uint32_t  proximity_domain;
} PLATFORM_OVERRIDE_RAS_NODE_MC_DATA;

typedef struct {
  uint32_t  intf_type;           /* Interface Type */
  uint32_t  flags;
  uint64_t  base_addr;
  uint32_t  start_rec_index;     /* Start Record Index */
  uint32_t  num_err_rec;
  uint64_t  err_rec_implement;
  uint64_t  err_status_reporting;
  uint64_t  addressing_mode;
} PLATFORM_OVERRIDE_RAS_NODE_INTERFACE;

typedef struct {
  uint32_t  type;
  uint32_t  flag;
  uint32_t  gsiv;
  uint32_t  its_grp_id;
} PLATFORM_OVERRIDE_RAS_NODE_INTERRUPT;

typedef struct {
  PLATFORM_OVERRIDE_RAS_NODE_INTERRUPT intr_info[RAS_MAX_NUM_NODES][RAS_MAX_INTR_TYPE];
} PLATFORM_OVERRIDE_RAS_NODE_INTERRUPT_INFO;

typedef struct {
  PLATFORM_OVERRIDE_RAS_NODE_INTERFACE intf_info[RAS_MAX_NUM_NODES];
} PLATFORM_OVERRIDE_RAS_NODE_INTERFACE_INFO;

typedef union {
  PLATFORM_OVERRIDE_RAS_NODE_PE_DATA pe;
  PLATFORM_OVERRIDE_RAS_NODE_MC_DATA mc;
} PLATFORM_OVERRIDE_RAS_NODE_DATA;

typedef struct {
  PLATFORM_OVERRIDE_RAS_NODE_DATA node_data[RAS_MAX_NUM_NODES];
} PLATFORM_OVERRIDE_RAS_NODE_DATA_INFO;

typedef struct {
  uint32_t  type;
  uint32_t  proximity_domain;      /* Proximity domain of the memory */
  uint32_t  patrol_scrub_support;  /* Patrol srub support flag */
} PLATFORM_OVERRIDE_RAS2_BLOCK;

typedef struct {
  uint32_t num_all_block;        /* Number of RAS2 feature blocks */
  uint32_t num_of_mem_block;     /* Number of memory feature blocks */
  PLATFORM_OVERRIDE_RAS2_BLOCK blocks[RAS2_MAX_NUM_BLOCKS];
} PLATFORM_OVERRIDE_RAS2_INFO_TABLE;

typedef struct {
    uint8_t    ris_index;
    uint8_t    locator_type;  /* Identifies location of this resource */
    uint64_t   descriptor1;   /* Primary acpi description of location */
    uint32_t   descriptor2;   /* Secondary acpi description of location */
} PLATFORM_OVERRIDE_MPAM_RESOURCE_NODE;

typedef struct {
    uint8_t     intrf_type;    /* type of interface to this MPAM MSC */
    uint32_t    identifier;    /* unique id to reference the node */
    uint64_t    msc_base_addr; /* base addr of mem-map MSC reg */
    uint32_t    msc_addr_len;  /*  MSC mem map size */
    uint32_t    max_nrdy;      /* max time in microseconds that MSC not ready
                                after config change */
    uint32_t    rsrc_count;    /* number of resource nodes */
    /* Details of resource node*/
    PLATFORM_OVERRIDE_MPAM_RESOURCE_NODE rsrc_node[MPAM_MAX_RSRC_NODE];
} PLATFORM_OVERRIDE_MPAM_MSC_NODE;

typedef struct {
    uint32_t          msc_count;  /* Number of MSC node */
    PLATFORM_OVERRIDE_MPAM_MSC_NODE   msc_node[MPAM_MAX_MSC_NODE]; /* Details of MSC node */
} PLATFORM_OVERRIDE_MPAM_INFO_TABLE;

/* Platform Communication Channel (PCC) info table */
#ifndef GAS_STRUCT
#define GAS_STRUCT
typedef struct {
  uint8_t   addr_space_id;
  uint8_t   reg_bit_width;
  uint8_t   reg_bit_offset;
  uint8_t   access_size;
  uint64_t  addr;
} GENERIC_ADDRESS_STRUCTURE;
#endif

typedef struct {
  uint64_t                         base_addr;               /* base addr of shared mem-region */
  GENERIC_ADDRESS_STRUCTURE        doorbell_reg;            /* doorbell register */
  uint64_t                         doorbell_preserve;       /* doorbell register preserve mask */
  uint64_t                         doorbell_write;          /* doorbell register set mask */
  uint32_t                         min_req_turnaround_usec; /* minimum request turnaround time */
  GENERIC_ADDRESS_STRUCTURE        cmd_complete_chk_reg;    /* command complete check register */
  uint64_t                         cmd_complete_chk_mask;   /* command complete check mask */
  GENERIC_ADDRESS_STRUCTURE        cmd_complete_update_reg; /* command complete update register */
  uint64_t                         cmd_complete_update_preserve;
                                                            /* command complete update preserve */
  uint64_t                         cmd_complete_update_set; /* command complete update set mask */
} PLATFORM_OVERRIDE_PCC_SUBSPACE_TYPE_3;

typedef union {
  PLATFORM_OVERRIDE_PCC_SUBSPACE_TYPE_3 pcc_ss_type_3; /* PCC type 3 info */
} PLATFORM_OVERRIDE_PCC_TYPE_SPECIFIC_INFO;

typedef struct {
  uint32_t                 subspace_idx;    /* PCC subspace index in PCCT ACPI table */
  uint32_t                 subspace_type;   /* type of PCC subspace */
  PLATFORM_OVERRIDE_PCC_TYPE_SPECIFIC_INFO
                           type_spec_info;  /* PCC subspace type specific info */
} PLATFORM_OVERRIDE_PCC_INFO;

typedef struct {
  uint32_t                    subspace_cnt; /* number of PCC subspace info stored */
  PLATFORM_OVERRIDE_PCC_INFO  pcc_info[PLATFORM_PCC_SUBSPACE_COUNT];
                                            /* array of PCC info blocks */
} PLATFORM_OVERRIDE_PCC_INFO_TABLE;
