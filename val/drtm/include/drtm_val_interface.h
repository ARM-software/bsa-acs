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

#ifndef __DRTM_VAL_INTERFACE_H
#define __DRTM_VAL_INTERFACE_H

#include "../../common/include/val_interface.h"

#include "drtm_acs_defines.h"

extern uint32_t g_num_skip;
extern uint32_t *g_skip_test_num;
extern uint32_t g_curr_module;

extern uint32_t g_drtm_acs_dlme[];
extern uint64_t g_drtm_acs_dlme_size;

typedef enum {
    INTERFACE_MODULE,
    DYNAMIC_LAUNCH_MODULE
} DRTM_MODULE_ID_e;

#define ACS_DRTM_INTERFACE_TEST_NUM_BASE    0
#define ACS_DRTM_DL_TEST_NUM_BASE           100

typedef struct __attribute__((packed)) {
    /* Callee-saved registers */
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    /* Frame register */
    uint64_t x29;
    /* Link register */
    uint64_t x30;
    /* Stack pointer */
    uint64_t sp;
    /* Stack Pointer when SPSel=0 */
    uint64_t sp_el0;
    /* SCTLR_EL2 */
    uint64_t sctlr_el2;
} DRTM_ACS_DL_SAVED_STATE;

typedef struct __attribute__((packed)) {
    uint64_t x0;
    uint64_t x1;
} DRTM_ACS_DL_RESULT;

extern DRTM_ACS_DL_RESULT      *g_drtm_acs_dl_result;
extern DRTM_ACS_DL_SAVED_STATE *g_drtm_acs_dl_saved_state;

int64_t val_drtm_features(uint64_t fid, uint64_t *feat1, uint64_t *feat2);
uint32_t val_drtm_get_version(void);
int64_t val_drtm_simulate_dl(DRTM_PARAMETERS *drtm_params);
int64_t val_drtm_dynamic_launch(DRTM_PARAMETERS *drtm_params);
int64_t val_drtm_close_locality(uint32_t locality);
int64_t val_drtm_unprotect_memory(void);
int64_t val_drtm_get_error(uint64_t *feat1);
int64_t val_drtm_set_tcb_hash(uint64_t tcb_hash_table_addr);
int64_t val_drtm_lock_tcb_hashes(void);
uint32_t val_drtm_reserved_bits_check_is_zero(uint32_t reserved_bits);
uint32_t val_drtm_get_psci_ver(void);
uint32_t val_drtm_get_smccc_ver(void);

uint32_t val_drtm_create_info_table(void);
int64_t val_drtm_check_dl_result(uint64_t dlme_base_addr, uint64_t dlme_data_offset);
int64_t val_drtm_init_drtm_params(DRTM_PARAMETERS *drtm_params);

uint32_t val_drtm_execute_interface_tests(uint32_t num_pe);
uint32_t val_drtm_execute_dl_tests(uint32_t num_pe);

uint32_t interface001_entry(uint32_t num_pe);
uint32_t interface002_entry(uint32_t num_pe);
uint32_t interface003_entry(uint32_t num_pe);
uint32_t interface004_entry(uint32_t num_pe);
uint32_t interface005_entry(uint32_t num_pe);
uint32_t interface006_entry(uint32_t num_pe);
uint32_t interface007_entry(uint32_t num_pe);
uint32_t interface008_entry(uint32_t num_pe);
uint32_t interface009_entry(uint32_t num_pe);
uint32_t interface010_entry(uint32_t num_pe);
uint32_t interface011_entry(uint32_t num_pe);
uint32_t interface012_entry(uint32_t num_pe);
uint32_t interface013_entry(uint32_t num_pe);
uint32_t interface014_entry(uint32_t num_pe);
uint32_t interface015_entry(uint32_t num_pe);

uint32_t dl001_entry(uint32_t num_pe);
uint32_t dl002_entry(uint32_t num_pe);
uint32_t dl003_entry(uint32_t num_pe);
uint32_t dl004_entry(uint32_t num_pe);
uint32_t dl005_entry(uint32_t num_pe);
uint32_t dl006_entry(uint32_t num_pe);
uint32_t dl007_entry(uint32_t num_pe);

#endif /* __DRTM_VAL_INTERFACE_H */
