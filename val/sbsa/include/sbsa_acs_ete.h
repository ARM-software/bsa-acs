/** @file
 * Copyright (c) 2023-2024, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __SBSA_AVS_ETE_H
#define __SBSA_AVS_ETE_H

#define ACS_TRBLIMITR_EL1_E     (1 << 0)
#define ACS_TRBLIMITR_EL1_nVM   (1 << 5)
#define ACS_TRBLIMITR_EL1_XE    (1 << 6)

#define TR_I_ADDR_CTXT_L_32IS0  0x82
#define TR_I_ADDR_CTXT_L_32IS1  0x83
#define TR_I_ADDR_CTXT_L_64IS0  0x85
#define TR_I_ADDR_CTXT_L_64IS1  0x86

#define TR_I_TS_PKT_V1          0x02
#define TR_I_TS_PKT_V2          0x03
#define TR_I_TS_MARKER          0x88
#define TR_I_TS_MARKER          0x88

#define TR_I_ADDR_L_32IS0       0x9A
#define TR_I_ADDR_L_32IS1       0x9B
#define TR_I_ADDR_L_64IS0       0x9C
#define TR_I_ADDR_L_64IS1       0x9D

#define TR_ADDR_START_BYTE_NUM      15

#define TR_ADDR_CTXT_L_32_MID       5
#define TR_ADDR_CTXT_L_32_V1_LEN    6
#define TR_ADDR_CTXT_L_32_V2_LEN    10
#define TR_ADDR_CTXT_L_32_V3_LEN    10
#define TR_ADDR_CTXT_L_32_V4_LEN    14

#define TR_ADDR_CTXT_L_64_MID       9
#define TR_ADDR_CTXT_L_64_V1_LEN    10
#define TR_ADDR_CTXT_L_64_V2_LEN    14
#define TR_ADDR_CTXT_L_64_V3_LEN    14
#define TR_ADDR_CTXT_L_64_V4_LEN    18

#define TR_ADDR_L_32_LEN    5

#define TR_ADDR_L_64_LEN    9

#define SH_TRACE_ENABLE_TRUE    1
#define SH_TRACE_ENABLE_FALSE   0

/* Timestamp */
#define TS_VALUE_MASK       0x7F
#define TS_CONTINUE_MASK    0x80

/* Trace Related Calls */

uint64_t val_ete_get_trace_timestamp(uint64_t buffer_address);
uint64_t val_ete_generate_trace(uint64_t buffer_address, uint32_t self_hosted_trace_enabled);

uint64_t AA64GenerateETETrace(void);
uint64_t AA64EnableETETrace(void);
uint64_t AA64DisableETETrace(void);
uint64_t AA64SetupTraceAccess(void);
uint64_t AA64EnableTRBUTrace(uint32_t index, uint64_t buffer_addr, uint32_t trbu_mode);
uint64_t AA64DisableTRBUTrace(void);
uint64_t AA64ReadTrbPtrEl1(void);

uint64_t AA64ReadTrblimitr1(void);
void AA64WriteTrblimitr1(uint64_t data);
void AA64EnableTFO(void);
void AA64DisableTFO(void);

uint32_t ete001_entry(uint32_t num_pe);
uint32_t ete002_entry(uint32_t num_pe);
uint32_t ete003_entry(uint32_t num_pe);
uint32_t ete004_entry(uint32_t num_pe);
uint32_t ete005_entry(uint32_t num_pe);
uint32_t ete006_entry(uint32_t num_pe);
uint32_t ete007_entry(uint32_t num_pe);
uint32_t ete008_entry(uint32_t num_pe);

#endif // __SBSA_AVS_ETE_H
