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

#include "val/common/include/acs_val.h"
#include "val/common/include/acs_pe.h"
#include "val/common/include/acs_memory.h"
#include "val/common/include/acs_mpam.h"
#include "val/common/include/acs_mpam_reg.h"
#include "val/mpam/include/mpam_val_interface.h"

#define TEST_NUM   ACS_MPAM_REGISTER_TEST_NUM_BASE + 3
#define TEST_RULE  ""
#define TEST_DESC  "Check MPAM MSC Feature Test           "

static void payload(void)
{

    uint32_t version;
    uint32_t pe_index;
    uint32_t msc_index;
    uint32_t total_nodes;
    uint64_t idr_value;
    uint32_t ccap_idr_value;
    uint32_t csumon_idr_value;
    uint32_t mbwumon_idr_value;
    uint32_t msmon_idr_value;
    uint32_t test_fail = 0;

    pe_index = val_pe_get_index_mpid(val_pe_get_mpid());

    total_nodes = val_mpam_get_msc_count();

    for (msc_index = 0; msc_index < total_nodes; msc_index++) {
      version = val_mpam_msc_get_version(msc_index);
      val_print(ACS_PRINT_INFO, "\n       MPAM version : v%x", (version >> 4));
      val_print(ACS_PRINT_INFO, ".%x", (version & 0xF));
      idr_value = val_mpam_mmr_read64(msc_index, REG_MPAMF_IDR);
      ccap_idr_value = val_mpam_mmr_read(msc_index, REG_MPAMF_CCAP_IDR);
      csumon_idr_value = val_mpam_mmr_read(msc_index, REG_MPAMF_CSUMON_IDR);
      mbwumon_idr_value = val_mpam_mmr_read(msc_index, REG_MPAMF_MBWUMON_IDR);
      msmon_idr_value = val_mpam_mmr_read(msc_index, REG_MPAMF_MSMON_IDR);

      if (version == MPAM_VERSION_1_0) {

        /* Minimum cache capacity partitioning Prohibited */
        if (BITFIELD_READ(CCAP_IDR_HAS_CMIN, ccap_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MPAMF_CCAP_IDR.HAS_CMIN is Prohibited", 0);
          test_fail++;
        }

        /* No maximum cache capacity partitioning Prohibited */
        if (BITFIELD_READ(CCAP_IDR_NO_CMAX, ccap_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MPAMF_CCAP_IDR.NO_CMAX is Prohibited", 0);
          test_fail++;
        }

        /* Cache maximum associativity partitioning Prohibited */
        if (BITFIELD_READ(CCAP_IDR_HAS_CASSOC, ccap_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MPAMF_CCAP_IDR.HAS_CASSOC is Prohibited", 0);
          test_fail++;
        }

        /* Cache maximum SOFT_LIM Prohibited */
        if (BITFIELD_READ(CCAP_IDR_HAS_CMAX_SOFTLIM, ccap_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MPAMF_CCAP_IDR.HAS_CMAX_SOFTLIM is Prohibited", 0);
          test_fail++;
        }

        /* PARTID Disable Prohibited */
        if (BITFIELD_READ(IDR_HAS_ENDIS, idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MPAMF_IDR.HAS_ENDIS is Prohibited", 0);
          test_fail++;
        }

        /* No Future Use Prohibited */
        if (BITFIELD_READ(IDR_HAS_NFU, idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MPAMF_IDR.HAS_NFU is Prohibited", 0);
          test_fail++;
        }

        /* CSU monitor XCL Prohibited */
        if (BITFIELD_READ(CSUMON_IDR_HAS_XCL, csumon_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       CSUMON_IDR_HAS_XCL is Prohibited", 0);
          test_fail++;
        }

        /* CSU monitor overflow linkage Prohibited */
        if (BITFIELD_READ(CSUMON_IDR_HAS_OFLOW_LNKG, csumon_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       CSUMON_IDR_HAS_OFLOW_LNKG is Prohibited", 0);
          test_fail++;
        }

        /* CSU monitor overflow status Reg Prohibited */
        if (BITFIELD_READ(CSUMON_IDR_HAS_OFSR, csumon_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       CSUMON_IDR_HAS_OFSR is Prohibited", 0);
          test_fail++;
        }

        /* CSU monitor overflow capture Prohibited */
        if (BITFIELD_READ(CSUMON_IDR_HAS_CEVNT_OFLW, csumon_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       CSUMON_IDR_HAS_CEVNT_OFLW is Prohibited", 0);
          test_fail++;
        }

        /* MBWU monitor overflow linkage Prohibited */
        if (BITFIELD_READ(MBWUMON_IDR_HAS_OFLOW_LNKG, mbwumon_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MBWUMON_IDR_HAS_OFLOW_LNKG is Prohibited", 0);
          test_fail++;
        }

        /* MBWU monitor overflow status Reg Prohibited */
        if (BITFIELD_READ(MBWUMON_IDR_HAS_OFSR, mbwumon_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MBWUMON_IDR_HAS_OFSR is Prohibited", 0);
          test_fail++;
        }

        /* MBWU monitor overflow capture Prohibited */
        if (BITFIELD_READ(MBWUMON_IDR_HAS_CAPTURE, mbwumon_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MBWUMON_IDR_HAS_CAPTURE is Prohibited", 0);
          test_fail++;
        }

        /* Monitor overflow status register Prohibited */
        if (BITFIELD_READ(MSMON_IDR_HAS_OFLOW_SR, msmon_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MSMON_IDR_HAS_OFLOW_SR is Prohibited", 0);
          test_fail++;
        }

        /* Monitor overflow MSI Prohibited */
        if (BITFIELD_READ(MSMON_IDR_HAS_OFLW_MSI, msmon_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MSMON_IDR_HAS_OFLW_MSI is Prohibited", 0);
          test_fail++;
        }

        /* No hardwired overflow interrupt Prohibited */
        if (BITFIELD_READ(MSMON_IDR_NO_OFLW_INTR, msmon_idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       MSMON_IDR_NO_OFLW_INTR is Prohibited", 0);
          test_fail++;
        }

        /* Impl IDR no partitioning Prohibited in v1.0, Required in others*/
        if (BITFIELD_READ(IDR_NO_IMPL_PART, idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       IDR_NO_IMPL_PART is Prohibited", 0);
          test_fail++;
        }

        /* Impl IDR no monitoring Prohibited in v1.0, Required in others */
        if (BITFIELD_READ(IDR_NO_IMPL_MSMON, idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       IDR_NO_IMPL_MSMON is Prohibited", 0);
          test_fail++;
        }

        /* Extended ID register Prohibited in v1.0, Required in others */
        if (BITFIELD_READ(IDR_EXT, idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       IDR_EXT is Prohibited", 0);
          test_fail++;
        }

        /* Resource instance selector Prohibited */
        if (BITFIELD_READ(IDR_HAS_RIS, idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       IDR_HAS_RIS is Prohibited", 0);
          test_fail++;
        }

        /* Error status register Prohibited */
        if (BITFIELD_READ(IDR_HAS_ESR, idr_value) || BITFIELD_READ(IDR_HAS_EXTD_ESR, idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       HAS_ESR & HAS_EXTD_ESR is Prohibited", 0);
          test_fail++;
        }

        /* Error MSI Prohibited */
        if (BITFIELD_READ(IDR_HAS_ERR_MSI, idr_value)) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       IDR_HAS_ERR_MSI is Prohibited", 0);
          test_fail++;
        }
      } else if ((version == MPAM_VERSION_1_1) || (version == MPAM_VERSION_0_1)) {
        /* if MPAMv0.1/MPAMv1.1 Check Required Features */

        if (BITFIELD_READ(IDR_HAS_IMPL_IDR, idr_value) == 1) {
          /* Impl IDR no partitioning Required */
          if (BITFIELD_READ(IDR_NO_IMPL_PART, idr_value) == 0) {
            /* fail the test*/
            val_print(ACS_PRINT_ERR, "\n       IDR_NO_IMPL_PART is Required", 0);
            test_fail++;
          }

          /* Impl IDR no monitoring Required */
          if (BITFIELD_READ(IDR_NO_IMPL_MSMON, idr_value) == 0) {
            /* fail the test*/
            val_print(ACS_PRINT_ERR, "\n       IDR_NO_IMPL_MSMON is Required", 0);
            test_fail++;
          }
        }

        /* Extended ID register Required */
        if (BITFIELD_READ(IDR_EXT, idr_value) == 0) {
          /* fail the test*/
          val_print(ACS_PRINT_ERR, "\n       IDR_EXT is Required", 0);
          test_fail++;
        }
      } else {
        /* Invalid */
        val_print(ACS_PRINT_ERR, "\n       MSC Version not valid", 0);
        val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 01));
        return;
      }
    }

    if (test_fail)
      val_set_status(pe_index, RESULT_FAIL(TEST_NUM, 02));
    else
      val_set_status(pe_index, RESULT_PASS(TEST_NUM, 01));
    return;
}

uint32_t reg003_entry(void)
{

    uint32_t status = ACS_STATUS_FAIL;
    uint32_t num_pe = 1;

    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

    if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

    val_report_status(0, ACS_END(TEST_NUM), NULL);

    return status;
}
