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

#ifndef __PAL_SBSA_UEFI_H__
#define __PAL_SBSA_UEFI_H__

/* include ACPI specification headers */
#include "Include/Guid/Acpi.h"
#include <Protocol/AcpiTable.h>
#include "Include/IndustryStandard/Acpi.h"


#define PLATFORM_TIMEOUT_MEDIUM 0x1000

UINT64 pal_get_acpi_table_ptr(UINT32 table_signature);
/**
  @brief  Instance of system pmu info
**/
typedef struct {
    UINT8  type;                /* The component that this PMU block is associated with*/
    UINT64 primary_instance;    /* Primary node instance, specific to the PMU type*/
    UINT32 secondary_instance;  /* Secondary node instance, specific to the PMU type*/
    UINT8  dual_page_extension; /* Support of the dual-page mode*/
    UINT64 base0;               /* Base address of Page 0 of the PMU*/
    UINT64 base1;               /* Base address of Page 1 of the PMU,
                                   valid only if dual_page_extension is 1*/
    UINT32 coresight_compliant; /* node is CS arch complaint or not */
} PMU_INFO_BLOCK;

typedef struct {
    UINT32  pmu_count;          /* Total number of PMU info blocks*/
    PMU_INFO_BLOCK  info[];     /* PMU info blocks for each PMU nodes*/
} PMU_INFO_TABLE;

/*
 * @brief Mpam Resource Node
 */
typedef struct {
    UINT8    ris_index;
    UINT8    locator_type;  /* Identifies location of this resource */
    UINT64   descriptor1;   /* Primary acpi description of location */
    UINT32   descriptor2;   /* Secondary acpi description of location */
} MPAM_RESOURCE_NODE;

/*
 * @brief MPAM MSC Node
 */
typedef struct {
    UINT8            intrf_type;    /* type of interface to this MPAM MSC */
    UINT32           identifier;    /* unique id to reference the node */
    UINT64           msc_base_addr; /* base addr of mem-mapped reg space or PCC
                                       subspace ID based on interface type. */
    UINT32           msc_addr_len;  /* MSC mem map size */
    UINT32           max_nrdy;      /* max time in microseconds that MSC not ready
                                         after config change */
    UINT32           rsrc_count;    /* number of resource nodes */
    MPAM_RESOURCE_NODE rsrc_node[]; /* Details of resource node */
} MPAM_MSC_NODE;

#define MPAM_INTERFACE_TYPE_MMIO 0x00
#define MPAM_INTERFACE_TYPE_PCC  0x0A
/*
 * @brief Mpam info table
 */

#define MPAM_NEXT_MSC(msc_entry) \
        (MPAM_MSC_NODE *)((UINT8 *)(&msc_entry->rsrc_node[0]) \
        + msc_entry->rsrc_count * sizeof(MPAM_RESOURCE_NODE))

typedef struct {
    UINT32          msc_count;  /* Number of MSC node */
    MPAM_MSC_NODE   msc_node[]; /* Details of MSC node */
} MPAM_INFO_TABLE;


/* Platform Communication Channel (PCC) info table */
typedef struct {
  UINT64                            base_addr;               /* base addr of shared mem-region */
  EFI_ACPI_6_5_GENERIC_ADDRESS_STRUCTURE
                                    doorbell_reg;            /* doorbell register */
  UINT64                            doorbell_preserve;       /* doorbell register preserve mask */
  UINT64                            doorbell_write;          /* doorbell register set mask */
  UINT32                            min_req_turnaround_usec; /* minimum request turnaround time */
  EFI_ACPI_6_5_GENERIC_ADDRESS_STRUCTURE
                                    cmd_complete_chk_reg;    /* command complete check register */
  UINT64                            cmd_complete_chk_mask;   /* command complete check mask */
  EFI_ACPI_6_5_GENERIC_ADDRESS_STRUCTURE
                                    cmd_complete_update_reg; /* command complete update register */
  UINT64                            cmd_complete_update_preserve;
                                                             /* command complete update preserve */
  UINT64                            cmd_complete_update_set; /* command complete update set mask */
} PCC_SUBSPACE_TYPE_3;

typedef union {
  PCC_SUBSPACE_TYPE_3 pcc_ss_type_3;
} PCC_TYPE_SPECIFIC_INFO;

typedef struct {
  UINT32                  subspace_idx;    /* PCC subspace index in PCCT ACPI table */
  UINT32                  subspace_type;   /* type of PCC subspace */
  PCC_TYPE_SPECIFIC_INFO  type_spec_info;  /* PCC subspace type specific info */
} PCC_INFO;

typedef struct {
  UINT32  subspace_cnt; /* number of PCC subspace info stored */
  PCC_INFO  pcc_info[];   /* array of PCC info blocks */
} PCC_INFO_TABLE;

typedef struct {
  UINT32 message_id : 8;      /* Bits [07:00] Message ID */
  UINT32 message_type : 2;    /* Bits [09:08] Message Type */
  UINT32 protocol_id : 8;     /* Bits [17:10] Protocol ID */
  UINT32 token : 10;          /* Bits [27:18] Token Caller-defined value */
  UINT32 reserved : 4;        /* Bits [31:28] Reserved must be zero */
} SCMI_PROTOCOL_MESSAGE_HEADER;

typedef struct {
  UINT32 msc_id;            /* Identifier of the MSC */
  UINT32 flags;             /* Reserved, must be zero */
  UINT32 offset;            /* MPAM register offset to read from */
} PCC_MPAM_MSC_READ_CMD_PARA;

typedef struct {
  INT32  status;             /* command response status code */
  UINT32 val;                /* value read from the register */
} PCC_MPAM_MSC_READ_RESP_PARA;

typedef struct {
  UINT32 msc_id;            /* Identifier of the MSC */
  UINT32 flags;             /* Reserved, must be zero */
  UINT32 val;               /* value to be written to the register */
  UINT32 offset;            /* MPAM register offset to write */
} PCC_MPAM_MSC_WRITE_CMD_PARA;

typedef struct {
  INT32  status;             /* command response status code */
} PCC_MPAM_MSC_WRITE_RESP_PARA;

#define MPAM_FB_PROTOCOL_ID    0x1A
#define MPAM_MSG_TYPE_CMD      0x0
#define MPAM_MSC_READ_CMD_ID   0x4
#define MPAM_MSC_WRITE_CMD_ID  0x5
#define MPAM_PCC_CMD_SUCCESS   0x0
#define RETURN_FAILURE         0xFFFFFFFF
#define PCC_TY3_CMD_OFFSET     12
#define PCC_TY3_COMM_SPACE     16

VOID pal_pcc_store_info(UINT32 subspace_idx);

/**
  @brief  SRAT node type
**/

typedef enum {
  SRAT_NODE_MEM_AFF  = 0x01,
  SRAT_NODE_GICC_AFF = 0x03
} SRAT_NODE_TYPE_e;

/**
  @brief  SRAT GICC Affinity Structure
**/

typedef struct {
  UINT32   prox_domain;      /* Proximity domain*/
  UINT32   proc_uid;         /* ACPI Processor UID */
  UINT32   flags;            /* Flags*/
  UINT32   clk_domain;       /* Clock Domain*/
} SRAT_GICC_AFF_ENTRY;

/**
  @brief  SRAT Memory Affinity Structure
**/

typedef struct {
  UINT32   prox_domain;     /* Proximity domain */
  UINT32   flags;           /* flags */
  UINT64   addr_base;       /* mem range address base */
  UINT64   addr_len;        /* mem range address len */
} SRAT_MEM_AFF_ENTRY;

typedef union {
  SRAT_MEM_AFF_ENTRY mem_aff;
  SRAT_GICC_AFF_ENTRY gicc_aff;
} SRAT_NODE_INFO;

typedef struct {
  UINT32 node_type;         /* Node type*/
  SRAT_NODE_INFO node_data;
} SRAT_INFO_ENTRY;

typedef struct {
  UINT32 num_of_srat_entries;
  UINT32 num_of_mem_ranges;
  SRAT_INFO_ENTRY  srat_info[];
} SRAT_INFO_TABLE;

/* SRAT node structure header. Can be removed after it is defined in EDKII*/
typedef struct {
  UINT8    Type;
  UINT8    Length;
} EFI_ACPI_6_4_SRAT_STRUCTURE_HEADER;


/* Cache info table structures and APIs */

#define CACHE_TYPE_SHARED  0x0
#define CACHE_TYPE_PRIVATE 0x1
#define CACHE_INVALID_NEXT_LVL_IDX 0xFFFFFFFF
#define CACHE_INVALID_IDX 0xFFFFFFFF

/*only the fields and flags required by ACS are parsed from ACPI PPTT table*/
/*Cache flags indicate validity of cache info provided by PPTT Table*/
typedef struct {
  UINT32 size_property_valid;
  UINT32 cache_type_valid;
  UINT32 cache_id_valid;
} CACHE_FLAGS;

/* Since most of platform doesn't support cache id field (ACPI 6.4+), ACS uses PPTT offset as key
   to uniquely identify a cache, In future once platforms align with ACPI 6.4+ my_offset member
   might be removed from cache entry*/
typedef struct {
  CACHE_FLAGS flags;        /* Cache flags */
  UINT32 my_offset;         /* Cache PPTT structure offset */
  UINT32 next_level_index;  /* Index of next level cache entry in CACHE_INFO_TABLE */
  UINT32 size;              /* Size of the cache in bytes */
  UINT32 cache_id;          /* Unique, non-zero identifier for this cache */
  UINT32 is_private;        /* Field indicate whether cache is private */
  UINT8  cache_type;        /* Cache type */
} CACHE_INFO_ENTRY;

typedef struct {
  UINT32 num_of_cache;            /* Total of number of cache info entries */
  CACHE_INFO_ENTRY cache_info[];  /* Array of cache info entries */
} CACHE_INFO_TABLE;

/* RAS Information */

typedef enum {
  NODE_TYPE_PE = 0x0,
  NODE_TYPE_MC = 0x1,
  NODE_TYPE_SMMU = 0x2,
  NODE_TYPE_VDR = 0x3,
  NODE_TYPE_GIC = 0x4,
  NODE_TYPE_LAST_ENTRY
} RAS_NODE_TYPE_e;

typedef enum {
  RAS_INTF_TYPE_SYS_REG,     /* System register RAS node interface type */
  RAS_INTF_TYPE_MMIO         /* MMIO RAS node interface type */
} RAS_NODE_INTF_TYPE;
typedef struct {
  UINT32  processor_id;
  UINT32  resource_type;
  UINT32  flags;
  UINT64  affinity;
  UINT64  res_specific_data;  /* Resource Specific Data */
} RAS_NODE_PE_DATA;

typedef struct {
  UINT32  proximity_domain;
} RAS_NODE_MC_DATA;

typedef struct {
  RAS_NODE_INTF_TYPE  intf_type;   /* Interface Type */
  UINT32  flags;
  UINT64  base_addr;               /* Base address to MMIO region, valid for MMIO intf type */
  UINT32  start_rec_index;         /* Start Record Index */
  UINT32  num_err_rec;             /* Number of error records (implemented & unimplemented)*/
  UINT64  err_rec_implement;       /* bitmap of error records implemented */
  UINT64  err_status_reporting;    /* bitmap indicates which error records within this error
                                      node support error status reporting using ERRGSR */
  UINT64  addressing_mode;         /* bitmap based policy for ERR<n>ADDR field of error records */
} RAS_INTERFACE_INFO;

typedef struct {
  UINT32  type;
  UINT32  flag;
  UINT32  gsiv;
  UINT32  its_grp_id;
} RAS_INTERRUPT_INFO;

typedef union {
  RAS_NODE_PE_DATA    pe;
  RAS_NODE_MC_DATA    mc;
} RAS_NODE_DATA;

typedef struct {
  RAS_NODE_TYPE_e     type;             /* Node Type PE/GIC/SMMU */
  UINT16              length;           /* Length of the Node */
  UINT64              num_intr_entries; /* Number of Interrupt Entry */
  RAS_NODE_DATA       node_data;        /* Node Specific Data */
  RAS_INTERFACE_INFO  intf_info;        /* Node Interface Info */
  RAS_INTERRUPT_INFO  intr_info[2];     /* Node Interrupt Info */
} RAS_NODE_INFO;

typedef struct {
  UINT32  num_nodes;      /* Number of total RAS Nodes */
  UINT32  num_pe_node;    /* Number of PE RAS Nodes */
  UINT32  num_mc_node;    /* Number of Memory Controller Nodes */
  RAS_NODE_INFO  node[];  /* Array of RAS nodes */
} RAS_INFO_TABLE;

typedef enum {
    ERR_UC = 0x1,         /* Uncorrectable Error */
    ERR_DE,               /* Deferred Error */
    ERR_CE,               /* Correctable Error */
    ERR_CRITICAL,         /* Critical Error */
    ERR_CONTAINABLE       /* Containable Error */
} RAS_ERROR_TYPE;

typedef struct {
   RAS_ERROR_TYPE ras_error_type;   /* Error Type */
   UINT64 error_pa;                 /* Error Phy Address */
   UINT32 rec_index;                /* Error Record Index */
   UINT32 node_index;               /* Error Node Index in Info table */
   UINT8 is_pfg_check;              /* Pseudo Fault Check or not */
} RAS_ERR_IN_t;

typedef struct {
   UINT32 intr_id;        /* Interrupt ID */
   UINT32 error_record;   /* Error Record Number */
} RAS_ERR_OUT_t;

void pal_ras_create_info_table(RAS_INFO_TABLE *ras_info_table);
UINT32 pal_ras_setup_error(RAS_ERR_IN_t in_param, RAS_ERR_OUT_t *out_param);
UINT32 pal_ras_inject_error(RAS_ERR_IN_t in_param, RAS_ERR_OUT_t *out_param);
void pal_ras_wait_timeout(UINT32 count);
UINT32 pal_ras_check_plat_poison_support(void);


typedef enum {
  RAS2_TYPE_MEMORY = 0   /* RAS2 memory feature type*/
} RAS2_FEAT_TYPE;

typedef struct {
  UINT32  proximity_domain;      /* Proximity domain of the memory */
  UINT32  patrol_scrub_support;  /* Patrol srub support flag */
} RAS2_MEM_INFO;

typedef union {
  RAS2_MEM_INFO mem_feat_info;   /* Memory feature specific info */
} RAS2_BLOCK_INFO;

typedef struct {
  RAS2_FEAT_TYPE type;                     /* RAS2 feature type*/
  RAS2_BLOCK_INFO block_info;     /* RAS2 block info */
} RAS2_BLOCK;

typedef struct {
  UINT32 num_all_block;        /* Number of RAS2 feature blocks */
  UINT32 num_of_mem_block;     /* Number of memory feature blocks */
  RAS2_BLOCK blocks[];
} RAS2_INFO_TABLE;

void pal_ras2_create_info_table(RAS2_INFO_TABLE *ras2_info_table);

/* HMAT info table structures and APIs*/

#define HMAT_MEM_HIERARCHY_MEMORY   0x00
#define HMAT_DATA_TYPE_ACCESS_BW    0x03
#define HMAT_DATA_TYPE_READ_BW      0x04
#define HMAT_DATA_TYPE_WRITE_BW     0x05
#define HMAT_BW_ENTRY_UNREACHABLE   0xFFFF
#define HMAT_BASE_UNIT_48BIT        0xFFFFFFFFFFFFULL
typedef struct {
  UINT32 mem_prox_domain;             /* Proximity domain of the memory region*/
  UINT64 write_bw;                    /* Maximum write bandwidth */
  UINT64 read_bw;                     /* Maximum read bandwidth */
} HMAT_BW_ENTRY;

typedef struct {
  UINT32 num_of_mem_prox_domain;      /* Number of Memory Proximity Domains */
  HMAT_BW_ENTRY bw_info[];            /* Array of bandwidth info based on proximity domain */
} HMAT_INFO_TABLE;

VOID pal_hmat_create_info_table(HMAT_INFO_TABLE *HmatTable);

#endif
