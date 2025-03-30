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

#endif
