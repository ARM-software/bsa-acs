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

#ifndef __SBSA_ACS_EXERCISER_H__
#define __SBSA_ACS_EXERCISER_H__

#include "common/include/acs_exerciser.h"

/* PCIe RAS related Offset, shift and mask */

#define PN_SHIFT       22
#define UE_ER_SHIFT    28
#define UET_SHIFT      20
#define DE_SHIFT       23

#define SERR_MASK      0xFF
#define PN_MASK        0x1
#define UE_ER_MASK     0x3
#define UET_MASK       0x3
#define DE_MASK        0x1

uint32_t e001_entry(void);
uint32_t e002_entry(void);
uint32_t e003_entry(void);
uint32_t e004_entry(void);
uint32_t e005_entry(void);
uint32_t e006_entry(void);
uint32_t e007_entry(void);
uint32_t e008_entry(void);
uint32_t e009_entry(void);
uint32_t e010_entry(void);
uint32_t e011_entry(void);
uint32_t e012_entry(void);

typedef enum {
    CORR_RCVR_ERR = 0x0,
    CORR_BAD_TLP  = 0x1,
    CORR_BAD_DLLP = 0x2,
    CORR_RPL_NUM_ROLL = 0x3,
    CORR_RPL_TMR_TIMEOUT = 0x4,
    CORR_ADV_NF_ERR = 0x5,
    CORR_INT_ERR = 0x6,
    CORR_HDR_LOG_OVRFL = 0x7,
    UNCORR_DL_ERROR = 0x8,
    UNCORR_SD_ERROR = 0x9,
    UNCORR_PTLP_REC = 0xA,
    UNCORR_FL_CTRL_ERR = 0xB,
    UNCORR_CMPT_TO = 0xC,
    UNCORR_AMPT_ABORT = 0xD,
    UNCORR_UNEXP_CMPT = 0xE,
    UNCORR_RCVR_ERR = 0xF,
    UNCORR_MAL_TLP = 0x10,
    UNCORR_ECRC_ERR = 0x11,
    UNCORR_UR = 0x12,
    UNCORR_ACS_VIOL = 0x13,
    UNCORR_INT_ERR = 0x14,
    UNCORR_MC_BLK_TLP = 0x15,
    UNCORR_ATOP_EGR_BLK = 0x16,
    UNCORR_TLP_PFX_EGR_BLK = 0x17,
    UNCORR_PTLP_EGR_BLK = 0x18,
    INVALID_CFG = 0x19
} EXERCISER_ERROR_CODE;

uint32_t val_get_exerciser_err_info(EXERCISER_ERROR_CODE type);
void     val_exerciser_disable_rp_pio_register(uint32_t bdf);
uint32_t val_exerciser_check_poison_data_forwarding_support(void);
uint32_t val_exerciser_get_pcie_ras_compliant_err_node(uint32_t bdf, uint32_t rp_bdf);
uint64_t val_exerciser_get_ras_status(uint32_t ras_node, uint32_t e_bdf, uint32_t erp_bdf);
uint32_t val_exerciser_set_bar_response(uint32_t bdf);
#endif
