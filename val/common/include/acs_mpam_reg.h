/** @file
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __MPAM_REG_H__
#define __MPAM_REG_H__

/* Macro can be used to declare MASK and SHIFT for bitfields using MSB and LSB */
#define BITFIELD_DECL(type, name, msb, lsb) \
    static const uint32_t name##_SHIFT = lsb; \
    static const type name##_MASK = ((((type)0x1) << (msb - lsb + 1)) - 1);

/* Macro can be used to read bitfields with name##_MASK and name##_SHIFT
   already defined or declared  for it*/
#define BITFIELD_READ(name, val) ((val >> name##_SHIFT) & name##_MASK)

/* Macro can be used to set bitfields with name##_MASK and name##_SHIFT
   already defined or declared  for it*/
#define BITFIELD_SET(name, val) ((val & name##_MASK) << name##_SHIFT)

/* Macro can be used write a particular bitfield with name##_MASK and name##_SHIFT
   already defined or declared  for it without affecting other fields*/
#define BITFIELD_WRITE(reg_val, name, val) \
        ((reg_val & ~(name##_MASK << name##_SHIFT)) | val << name##_SHIFT)

/*******************************************************************************
 * MPAM memory mapped register offsets
 ******************************************************************************/
#define REG_MPAMF_IDR               0x0000
#define REG_MPAMF_SIDR              0x0008
#define REG_MPAMF_IIDR              0x0018
#define REG_MPAMF_AIDR              0x0020
#define REG_MPAMF_IMPL_IDR          0x0028
#define REG_MPAMF_CPOR_IDR          0x0030
#define REG_MPAMF_CCAP_IDR          0x0038
#define REG_MPAMF_MBW_IDR           0x0040
#define REG_MPAMF_PRI_IDR           0x0048
#define REG_MPAMF_PARTID_NRW_IDR    0x0050
#define REG_MPAMF_MSMON_IDR         0x0080
#define REG_MPAMF_CSUMON_IDR        0x0088
#define REG_MPAMF_MBWUMON_IDR       0x0090
#define REG_MPAMF_ECR               0x00F0
#define REG_MPAMF_ESR               0x00F8

#define REG_MPAMCFG_PART_SEL        0x0100
#define REG_MPAMCFG_CMAX            0x0108
#define REG_MPAMCFG_MBW_MIN         0x0200
#define REG_MPAMCFG_EN              0x0300
#define REG_MPAMCFG_MBW_MAX         0x0208
#define REG_MPAMCFG_INTPARTID       0x0600
#define REG_MPAMCFG_CPBM            0x1000
#define REG_MPAMCFG_MBW_PBM         0x2000

#define REG_MSMON_CFG_MON_SEL       0x0800
#define REG_MSMON_CAPT_EVNT         0x0808
#define REG_MSMON_CFG_CSU_FLT       0x0810
#define REG_MSMON_CFG_CSU_CTL       0x0818
#define REG_MSMON_CFG_MBWU_FLT      0x0820
#define REG_MSMON_CFG_MBWU_CTL      0x0828
#define REG_MSMON_CSU               0x0840
#define REG_MSMON_CSU_CAPTURE       0x0848
#define REG_MSMON_CSU_OFSR          0x0858
#define REG_MSMON_MBWU              0x0860
#define REG_MSMON_MBWU_CAPTURE      0x0868
#define REG_MSMON_MBWU_L            0x0880
#define REG_MSMON_MBWU_L_CAPTURE    0x0890

/* MPAMF_IDR bit definitions */
BITFIELD_DECL(uint64_t, IDR_PARTID_MAX, 15, 0)
BITFIELD_DECL(uint64_t, IDR_PMG_MAX, 23, 16)
BITFIELD_DECL(uint64_t, IDR_HAS_CCAP_PART, 24, 24)
BITFIELD_DECL(uint64_t, IDR_HAS_CPOR_PART, 25, 25)
BITFIELD_DECL(uint64_t, IDR_HAS_MBW_PART, 26, 26)
BITFIELD_DECL(uint64_t, IDR_EXT, 28, 28)
BITFIELD_DECL(uint64_t, IDR_HAS_IMPL_IDR, 29, 29)
BITFIELD_DECL(uint64_t, IDR_HAS_MSMON, 30, 30)
BITFIELD_DECL(uint64_t, IDR_HAS_PARTID_NRW, 31, 31)
BITFIELD_DECL(uint64_t, IDR_HAS_RIS, 32, 32)
BITFIELD_DECL(uint64_t, IDR_NO_IMPL_PART, 36, 36)
BITFIELD_DECL(uint64_t, IDR_NO_IMPL_MSMON, 37, 37)
BITFIELD_DECL(uint64_t, IDR_HAS_EXTD_ESR, 38, 38)
BITFIELD_DECL(uint64_t, IDR_HAS_ESR, 39, 39)
BITFIELD_DECL(uint64_t, IDR_HAS_ERR_MSI, 40, 40)
BITFIELD_DECL(uint64_t, IDR_SP4, 41, 41)
BITFIELD_DECL(uint64_t, IDR_HAS_ENDIS, 42, 42)
BITFIELD_DECL(uint64_t, IDR_HAS_NFU, 43, 43)
BITFIELD_DECL(uint64_t, IDR_RIS_MAX, 59, 56)

/* MPAMF_AIDR bit definitions */
BITFIELD_DECL(uint32_t, AIDR_VERSION, 7, 0)

/* MPAMF_CCAP_IDR bit definitions */
BITFIELD_DECL(uint32_t, CMAX_WD, 5, 0)
BITFIELD_DECL(uint32_t, CCAP_IDR_HAS_CASSOC, 28, 28)
BITFIELD_DECL(uint32_t, CCAP_IDR_HAS_CMIN, 29, 29)
BITFIELD_DECL(uint32_t, CCAP_IDR_NO_CMAX, 30, 30)
BITFIELD_DECL(uint32_t, CCAP_IDR_HAS_CMAX_SOFTLIM, 31, 31)

/* MPAMF_MBW_IDR bit definitions */
#define MBWPOR_BITMAP_DEF_VAL 0xFFFFFFFF

BITFIELD_DECL(uint32_t, BWA_WD, 5, 0)
BITFIELD_DECL(uint32_t, HAS_MIN, 10, 10)
BITFIELD_DECL(uint32_t, HAS_MAX, 11, 11)
BITFIELD_DECL(uint32_t, HAS_PBM, 12, 12)
BITFIELD_DECL(uint32_t, BWPBM_WD, 28, 16)

/* MPAMF_PARTID_NRW_IDR bit definitions */
BITFIELD_DECL(uint32_t, INTPARTID_MAX, 15, 0)

/* MPAMF_MSMON_IDR bit definitions */
BITFIELD_DECL(uint32_t, MSMON_IDR_MSMON_CSU, 16, 16)
BITFIELD_DECL(uint32_t, MSMON_IDR_MSMON_MBWU, 17, 17)
BITFIELD_DECL(uint32_t, MSMON_IDR_HAS_LOCAL_CAPT_EVN, 31, 31)
BITFIELD_DECL(uint32_t, MSMON_IDR_HAS_OFLOW_SR, 28, 28)
BITFIELD_DECL(uint32_t, MSMON_IDR_HAS_OFLW_MSI, 29, 29)
BITFIELD_DECL(uint32_t, MSMON_IDR_NO_OFLW_INTR, 30, 30)

/* MPAMF_ECR bit definitions */
#define ECR_ENABLE_INTEN_SHIFT      0
#define ECR_ENABLE_INTEN_BIT        (1 << ECR_ENABLE_INTEN_SHIFT)

/* MPAM Error Status Register Bit Shift and Mask */
#define ESR_ERRCODE_SHIFT           0x0018
#define ESR_ERRCODE_MASK            0x000F

/* MPAMCFG_CMAX Register Bit Shift */
#define MPAMCFG_CMAX_SOFTLIM_SHIFT          31

/* MPAMCFG_MBW_MAX Register Bit Shift */
#define MPAMCFG_MBW_MAX_HARDLIM_SHIFT       31

/* MPAMCFG_INTPARTID bit definitions */
#define MPAMCFG_INTPARTID_INTPARTID_INTERNAL_SHIFT   16

/* MSMON_CFG_MBWU_CTL bit definitions */
#define MBWU_CTL_OFLOW_INTR_L_SHIFT         14
#define MBWU_CTL_OFLOW_STATUS_L_SHIFT       15
#define MBWU_CTL_ENABLE_MATCH_PARTID_SHIFT  16
#define MBWU_CTL_ENABLE_MATCH_PMG_SHIFT     17
#define MBWU_CTL_SELECT_SUBTYPE_SHIFT       20
#define MBWU_CTL_ENABLE_OFLOW_FRZ_SHIFT     24
#define MBWU_CTL_ENABLE_OFLOW_INTR_SHIFT    25
#define MBWU_CTL_OFLOW_STATUS_BIT_SHIFT     26
#define MBWU_CTL_SELECT_CAPT_EVNT_SHIFT     28
#define MBWU_CTL_ENABLE_SHIFT               31

/* MSMON_MBWU bit definitions */
#define MBWU_VALUE_SHIFT    0
#define MBWU_NRDY_SHIFT     31
#define MBWU_VALUE_MASK     0x7fffffff
#define MBWU_NRDY_MASK      0x1

/* MS Monitor Counter Max values */
#define MSMON_COUNT_63BIT   0x7fffffffffffffff
#define MSMON_COUNT_44BIT   0x1FFFFFFFFFFF
#define MSMON_COUNT_31BIT   0x7FFFFFFF

/* MPAMF_ESR bit values */
#define ESR_ERRCODE_NO_ERROR            0b0000
#define ESR_ERRCODE_PARTID_SEL_RANGE    0b0001
#define ESR_ERRCODE_Req_PARTID_Range    0b0010
#define ESR_ERRCODE_MSMONCFG_ID_RANGE   0b0011
#define ESR_ERRCODE_REQ_PMG_RANGE       0b0100
#define ESR_ERRCODE_MON_RANGE           0b0101
#define ESR_ERRCODE_INTPARTID_RANGE     0b0110
#define ESR_ERRCODE_UNEXPECTED_INTERNAL 0b0111
#define ESR_ERRCODE_UNDEF_RIS_PART_SEL  0b1000
#define ESR_ERRCODE_RIS_NO_CTRL         0b1001
#define ESR_ERRCODE_UNDEF_RIS_MON_SEL   0b1010
#define ESR_ERRCODE_RIS_NO_MON          0b1011

/* MPAMF_MBWUMON_IDR bit definitions */
BITFIELD_DECL(uint32_t, MBWUMON_IDR_NUM_MON, 15, 0)
BITFIELD_DECL(uint32_t, MBWUMON_IDR_SCALE, 20, 16)
BITFIELD_DECL(uint32_t, MBWUMON_IDR_HAS_OFSR, 26, 26)
BITFIELD_DECL(uint32_t, MBWUMON_IDR_HAS_OFLOW_LNKG, 27, 27)
BITFIELD_DECL(uint32_t, MBWUMON_IDR_HAS_RWBW, 28, 28)
BITFIELD_DECL(uint32_t, MBWUMON_IDR_LWD, 29, 29)
BITFIELD_DECL(uint32_t, MBWUMON_IDR_HAS_LONG, 30, 30)
BITFIELD_DECL(uint32_t, MBWUMON_IDR_HAS_CAPTURE, 31, 31)

/* MSMON_CFG_MBWU_FLT.RWBW bit values */
#define MBWU_FLT_RWBW_RW 0b00
#define MBWU_FLT_RWBW_W  0b01
#define MBWU_FLT_RWBW_R  0b10

/* MSMON_CFG_MBWU_FLT bit definitions */
BITFIELD_DECL(uint32_t, MBWU_FLT_PARTID, 15, 0)
BITFIELD_DECL(uint32_t, MBWU_FLT_PMG, 23, 16)
BITFIELD_DECL(uint32_t, MBWU_FLT_RWBW, 31, 30)

/* MSMON_CFG_MBWU_CTL bit definitions */
BITFIELD_DECL(uint32_t, MBWU_CTL_TYPE, 7, 0)
BITFIELD_DECL(uint32_t, MBWU_CTL_MATCH_PARTID, 16, 16)
BITFIELD_DECL(uint32_t, MBWU_CTL_MATCH_PMG, 17, 17)
BITFIELD_DECL(uint32_t, MBWU_CTL_SUBTYPE, 23, 20)
BITFIELD_DECL(uint32_t, MBWU_CTL_OFLOW_FRZ, 24, 24)
BITFIELD_DECL(uint32_t, MBWU_CTL_OFLOW_INTR, 25, 25)
BITFIELD_DECL(uint32_t, MBWU_CTL_OFLOW_STATUS, 26, 26)
BITFIELD_DECL(uint32_t, MBWU_CTL_CAPT_RESET, 27, 27)
BITFIELD_DECL(uint32_t, MBWU_CTL_CAPT_EVNT, 30, 28)
BITFIELD_DECL(uint32_t, MBWU_CTL_EN, 31, 31)

/* MSMON_MBWU bit definitions */
BITFIELD_DECL(uint32_t, MSMON_MBWU_VALUE, 30, 0)
BITFIELD_DECL(uint32_t, MSMON_MBWU_NRDY, 31, 31)

/* MSMON_MBWU_CAPTURE bit definitions */
BITFIELD_DECL(uint32_t, MSMON_MBWU_CAPTURE_VALUE, 30, 0)
BITFIELD_DECL(uint32_t, MSMON_MBWU_CAPTURE_NRDY, 31, 31)

/* MSMON_MBWU_L bit definitions */
BITFIELD_DECL(uint64_t, MSMON_MBWU_L_NRDY, 63, 63)
BITFIELD_DECL(uint64_t, MSMON_MBWU_L_44BIT_VALUE, 43, 0)
BITFIELD_DECL(uint64_t, MSMON_MBWU_L_63BIT_VALUE, 62, 0)

/* MSMON_MBWU_L_CAPTURE bit definitions */
BITFIELD_DECL(uint64_t, MSMON_MBWU_L_CAPTURE_NRDY, 63, 63)
BITFIELD_DECL(uint64_t, MSMON_MBWU_L_CAPTURE_44BIT_VALUE, 43, 0)
BITFIELD_DECL(uint64_t, MSMON_MBWU_L_CAPTURE_63BIT_VALUE, 62, 0)

/* MSMON_CFG_MON_SEL bit definitions */
BITFIELD_DECL(uint32_t, MON_SEL_MON_SEL, 15, 0)
BITFIELD_DECL(uint32_t, MON_SEL_RIS, 27, 24)

/* MPAMCFG_PART_SEL bit definitions */
BITFIELD_DECL(uint32_t, PART_SEL_PARTID_SEL, 15, 0)
BITFIELD_DECL(uint32_t, PART_SEL_INTERNAL, 16, 16)
BITFIELD_DECL(uint32_t, PART_SEL_RIS, 27, 24)

/* MPAMF_CSUMON_IDR bit definitions */
BITFIELD_DECL(uint32_t, CSUMON_IDR_NUM_MON, 15, 0)
BITFIELD_DECL(uint32_t, CSUMON_IDR_HAS_CEVNT_OFLW, 25, 25)
BITFIELD_DECL(uint32_t, CSUMON_IDR_HAS_OFSR, 26, 26)
BITFIELD_DECL(uint32_t, CSUMON_IDR_HAS_OFLOW_LNKG, 27, 27)
BITFIELD_DECL(uint32_t, CSUMON_IDR_HAS_XCL, 29, 29)
BITFIELD_DECL(uint32_t, CSUMON_IDR_CSU_RO, 30, 30)

/* MPAMF_CPOR_IDR bit definitions */
BITFIELD_DECL(uint32_t, CPOR_IDR_CPBM_WD, 15, 0)

/* MSMON_CFG_CSU_CTL bit definitions */
BITFIELD_DECL(uint32_t, CSU_CTL_TYPE, 7, 0)
BITFIELD_DECL(uint32_t, CSU_CTL_MATCH_PARTID, 16, 16)
BITFIELD_DECL(uint32_t, CSU_CTL_MATCH_PMG, 17, 17)
BITFIELD_DECL(uint32_t, CSU_CTL_SUBTYPE, 22, 20)
BITFIELD_DECL(uint32_t, CSU_CTL_OFLOW_FRZ, 24, 24)
BITFIELD_DECL(uint32_t, CSU_CTL_OFLOW_INTR, 25, 25)
BITFIELD_DECL(uint32_t, CSU_CTL_OFLOW_STATUS, 26, 26)
BITFIELD_DECL(uint32_t, CSU_CTL_CAPT_RESET, 27, 27)
BITFIELD_DECL(uint32_t, CSU_CTL_CAPT_EVNT, 30, 28)
BITFIELD_DECL(uint32_t, CSU_CTL_EN, 31, 31)

/* MSMON_CFG_CSU_FLT bit definitions */
BITFIELD_DECL(uint32_t, CSU_FLT_PARTID, 15, 0)
BITFIELD_DECL(uint32_t, CSU_FLT_PMG, 23, 16)

/* MSMON_CSU bit definitions */
BITFIELD_DECL(uint32_t, MSMON_CSU_VALUE, 30, 0)
BITFIELD_DECL(uint32_t, MSMON_CSU_NRDY, 31, 31)

#endif /*__MPAM_REG_H__ */
