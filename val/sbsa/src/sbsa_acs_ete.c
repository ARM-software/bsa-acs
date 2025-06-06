/** @file
 * Copyright (c) 2023-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "common/include/acs_common.h"
#include "sbsa/include/sbsa_acs_ete.h"
#include "sbsa/include/sbsa_val_interface.h"

uint64_t val_ete_get_trace_timestamp(uint64_t buffer_address)
{
  uint8_t trace_bytes[100];
  uint32_t i = 0;
  uint32_t data, trace_addr_len = 0;
  uint64_t timestamp = 0;
  uint64_t ts_value = 0;
  uint32_t ts_start_byte = 0, ts_continue = 0;
  uint32_t tr_addr_start_byte_num = 0;
  uint32_t trace_info_len = TR_TRACE_INFO_V1_LEN;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  val_memcpy(trace_bytes, (void *)buffer_address, 100);

  /* Calculate trace info length */
  if (VAL_EXTRACT_BITS(trace_bytes[TR_ALIGN_SYNC_PKT_LEN + TR_TRACE_INFO_V1_LEN - 1], 0, 0))
    trace_info_len = trace_info_len + TR_TRACE_INFO_INFO_LEN;

  if (VAL_EXTRACT_BITS(trace_bytes[TR_ALIGN_SYNC_PKT_LEN + TR_TRACE_INFO_V1_LEN - 1], 2, 2))
    trace_info_len = trace_info_len + TR_TRACE_INFO_SPEC_LEN;

  if (VAL_EXTRACT_BITS(trace_bytes[TR_ALIGN_SYNC_PKT_LEN + TR_TRACE_INFO_V1_LEN - 1], 3, 3))
    trace_info_len = trace_info_len + TR_TRACE_INFO_CYCT_LEN;

  tr_addr_start_byte_num = TR_ALIGN_SYNC_PKT_LEN + trace_info_len + TR_TRACE_ON_LEN;

  /* Calculate length of Trace Address Packet */
  switch (trace_bytes[tr_addr_start_byte_num]) {
  case TR_I_ADDR_CTXT_L_32IS0:
  case TR_I_ADDR_CTXT_L_32IS1:
    data = VAL_EXTRACT_BITS(trace_bytes[tr_addr_start_byte_num + TR_ADDR_CTXT_L_32_MID], 6, 7);
    switch (data) {
    case 0:
      trace_addr_len = TR_ADDR_CTXT_L_32_V1_LEN;
      break;
    case 1:
      trace_addr_len = TR_ADDR_CTXT_L_32_V2_LEN;
      break;
    case 2:
      trace_addr_len = TR_ADDR_CTXT_L_32_V3_LEN;
      break;
    case 3:
      trace_addr_len = TR_ADDR_CTXT_L_32_V4_LEN;
      break;
    default:
      trace_addr_len = 0;
      break;
    }
    break;
  case TR_I_ADDR_CTXT_L_64IS0:
  case TR_I_ADDR_CTXT_L_64IS1:
    data = VAL_EXTRACT_BITS(trace_bytes[tr_addr_start_byte_num + TR_ADDR_CTXT_L_64_MID], 6, 7);
    switch (data) {
    case 0:
      trace_addr_len = TR_ADDR_CTXT_L_64_V1_LEN;
      break;
    case 1:
      trace_addr_len = TR_ADDR_CTXT_L_64_V2_LEN;
      break;
    case 2:
      trace_addr_len = TR_ADDR_CTXT_L_64_V3_LEN;
      break;
    case 3:
      trace_addr_len = TR_ADDR_CTXT_L_64_V4_LEN;
      break;
    default:
      trace_addr_len = 0;
      break;
    }
    break;
  case TR_I_ADDR_L_32IS0:
  case TR_I_ADDR_L_32IS1:
    trace_addr_len = TR_ADDR_L_32_LEN;
    break;
  case TR_I_ADDR_L_64IS0:
  case TR_I_ADDR_L_64IS1:
    trace_addr_len = TR_ADDR_L_64_LEN;
    break;
  default:
    trace_addr_len = 0;
    break;
  }

  if (trace_addr_len == 0) {
    val_print_primary_pe(ACS_PRINT_DEBUG,
                         "\n       Target Address Parsing failed", 0, index);
    return 0;
  }

  /* Timestamp Packet Parsing */
  ts_start_byte = tr_addr_start_byte_num + trace_addr_len;
  if (trace_bytes[ts_start_byte] == TR_I_TS_MARKER)
      ts_start_byte++;

  if ((trace_bytes[ts_start_byte] == TR_I_TS_PKT_V1) ||
      (trace_bytes[ts_start_byte] == TR_I_TS_PKT_V2)) {
      ts_continue = 1;
      ts_start_byte++;
      i = 0;
      while (ts_continue && (i < 9)) {
        ts_value = (trace_bytes[ts_start_byte + i] & TS_VALUE_MASK);
        timestamp = timestamp | ((ts_value << (i * 8)) >> i);
        ts_continue = (trace_bytes[ts_start_byte + i] & TS_CONTINUE_MASK);
        i++;
      }
  }

  if (timestamp == 0) {
    val_print_primary_pe(ACS_PRINT_DEBUG, "\n       Timestamp Parsing failed", 0, index);
    return 0;
  }

  return timestamp;
}

uint64_t val_ete_generate_trace(uint64_t buffer_addr, uint32_t self_hosted_trace_enabled)
{
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
    uint64_t trbptr_before = 0;
    uint64_t trbptr_after = 0;

    /* If SelfHostedTraceEnabled = FALSE, Enable TFO */
    if (self_hosted_trace_enabled == SH_TRACE_ENABLE_FALSE)
        AA64EnableTFO();

    /* Update MDCR_EL2.E2TB to allow Tracing at EL2 */
    AA64SetupTraceAccess();

    /* Update TRBLIMITR_EL1_E/XE Based on SelfHostedTraceEnabled & Enable Trace Buffer Unit */
    if (self_hosted_trace_enabled == SH_TRACE_ENABLE_TRUE)
        AA64EnableTRBUTrace(index, buffer_addr, ACS_TRBLIMITR_EL1_E);
    else
        AA64EnableTRBUTrace(index, buffer_addr, ACS_TRBLIMITR_EL1_XE | ACS_TRBLIMITR_EL1_nVM);

    AA64EnableETETrace();

    /* Read TRBPTR_EL1 before generating the trace */
    trbptr_before = AA64ReadTrbPtrEl1();

    /* Generate Trace */
    AA64GenerateETETrace();

    /* Disable Trace */
    AA64DisableETETrace();
    AA64DisableTRBUTrace();

    /* If SelfHostedTraceEnabled = FALSE, Disable TFO */
    if (self_hosted_trace_enabled == SH_TRACE_ENABLE_FALSE)
        AA64DisableTFO();

    /* Read TRBPTR_EL1 after generating the trace */
    trbptr_after = AA64ReadTrbPtrEl1();

    /* If Trace is not generated or timestamp for current PE not updated */
    if (trbptr_before == trbptr_after)
        return ACS_STATUS_FAIL;

    return val_ete_get_trace_timestamp(buffer_addr + (index << 12));
}
