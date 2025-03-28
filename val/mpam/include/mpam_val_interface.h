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

#include "../../common/include/pal_interface.h"
#include "mpam_pal_interface.h"

extern uint32_t g_num_skip;
extern uint32_t *g_skip_test_num;
extern uint32_t g_curr_module;

#define ACS_MPAM_REGISTER_TEST_NUM_BASE     0
#define ACS_MPAM_CACHE_TEST_NUM_BASE        100
#define ACS_MPAM_ERROR_TEST_NUM_BASE        200
#define ACS_MPAM_MEMORY_TEST_NUM_BASE       300

#define GET_MAX_VALUE(ax, ay) (((ax) > (ay)) ? (ax) : (ay))
#define GET_MIN_VALUE(ax, ay) (((ax) > (ay)) ? (ay) : (ax))


#define SIZE_1K    1024ULL
#define SIZE_4K    4 * SIZE_1K
#define SIZE_16K   4 * SIZE_4K
#define SIZE_1M    SIZE_1K * SIZE_1K
#define SIZE_1G    SIZE_1M * SIZE_1K

#define SOFTLIMIT_DIS 0x0
#define SOFTLIMIT_EN 0x1
#define HARDLIMIT_DIS 0x0
#define HARDLIMIT_EN 0x1

typedef enum {
    REGISTER_MODULE,
    CACHE_MODULE,
    ERROR_MODULE,
    MEMORY_MODULE
} MPAM_MODULE_ID_e;

// Module entry functions.
uint32_t val_mpam_execute_register_tests(uint32_t num_pe);
uint32_t val_mpam_execute_error_tests(uint32_t num_pe);
uint32_t val_mpam_execute_cache_tests(uint32_t num_pe);
uint32_t val_mpam_execute_membw_tests(uint32_t num_pe);

// VAL API prototypes
uint32_t val_mpam_msc_reset_errcode(uint32_t msc_index);
uint32_t val_mpam_msc_get_errcode(uint32_t msc_index);
void     val_mpam_msc_generate_psr_error(uint32_t msc_index);
void     val_mpam_msc_generate_msr_error(uint32_t msc_index, uint16_t mon_count);
uint32_t val_mpam_msc_generate_por_error(uint32_t msc_index);
uint32_t val_mpam_msc_generate_pmgor_error(uint32_t msc_index);
void     val_mpam_msc_generate_msmon_config_error(uint32_t msc_index, uint16_t mon_count);
void     val_mpam_msc_generate_msmon_oflow_error(uint32_t msc_index, uint16_t mon_count);
void     val_mpam_msc_trigger_intr(uint32_t msc_index);

// Register tests entry calls
uint32_t reg001_entry(void);
uint32_t reg002_entry(void);
uint32_t reg003_entry(void);

// Memory Bandwidth partitioning tests entry calls
uint32_t mem001_entry(void);
uint32_t mem002_entry(void);
uint32_t mem003_entry(void);

// Error and Interrupt tests entry calls
uint32_t error001_entry(void);
uint32_t error002_entry(void);
uint32_t error003_entry(void);
uint32_t error004_entry(void);
uint32_t error005_entry(void);
uint32_t error006_entry(void);
uint32_t error007_entry(void);
uint32_t error008_entry(void);
uint32_t error009_entry(void);
uint32_t error010_entry(void);
uint32_t error011_entry(void);
uint32_t intr001_entry(void);
uint32_t intr002_entry(void);
uint32_t intr003_entry(void);

/* Cache Tests */
uint32_t partition001_entry(void);
uint32_t partition002_entry(void);
uint32_t partition003_entry(void);

uint32_t monitor001_entry(void);
uint32_t monitor002_entry(void);
uint32_t monitor003_entry(void);
uint32_t monitor004_entry(void);

// Accessing system registers from .S -> can be moved to respective .h
uint64_t arm64_write_sp(uint64_t write_data);
uint64_t arm64_read_sp(void);
