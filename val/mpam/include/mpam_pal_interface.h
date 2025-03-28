/** @file
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
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

typedef enum {
    MPAM_NODE_SMMU = 0x0,
    MPAM_NODE_CACHE,
    MPAM_NODE_MEMORY
} MPAM_NODE_TYPE;

// The DS used in enterprise ACS is different than what's used in SBSA ACS.
// Having the data structures for reference. Can be deteled later.

// typedef enum {
//     INTR_EDGE_TRIGGER = 0x0,
//     INTR_LEVEL_TRIGGER
// } INTR_TRIGGER_TYPE;

// typedef struct {
//     uint32_t            error_intr_num;
//     uint32_t            overflow_intr_num;
//     INTR_TRIGGER_TYPE   error_intr_type;
//     INTR_TRIGGER_TYPE   overflow_intr_type;
// } INTR_INFO;

// typedef struct {
//     uint32_t    proximity_domain;
//     uint64_t    base_address;
//     uint64_t    length;
//     uint32_t    flags;
//     addr_t      hwreg_base_addr;
//     uint32_t    not_ready_max_us;
//     INTR_INFO   intr_info;
// } MEMORY_NODE_ENTRY;

// typedef struct {
//     uint32_t    scope_index:24;
//     uint32_t    node_scope:8;
// } CACHE_NODE_INFO;

// typedef struct {
//     uint8_t     alloc_type:2;
//     uint8_t     cache_type:2;
//     uint8_t     write_policy:1;
//     uint8_t     reserved:3;
// } CACHE_NODE_ATTR;

// typedef struct {
//     uint16_t        line_size;
//     uint32_t        size;
//     uint64_t        neighbours[4];
//     CACHE_NODE_INFO info;
//     CACHE_NODE_ATTR attributes;
//     addr_t          hwreg_base_addr;
//     uint32_t        not_ready_max_us;
//     INTR_INFO       intr_info;
// } CACHE_NODE_ENTRY;

// typedef struct {
//     uint32_t            num_cache_nodes;
//     uint32_t            num_memory_nodes;
//     CACHE_NODE_ENTRY    *cache_node;
//     MEMORY_NODE_ENTRY   *memory_node;
// } MPAM_INFO_TABLE;
