#/** @file
# Copyright (c) 2016-2020,2021,2024, Arm Limited or its affiliates. All rights reserved.
# SPDX-License-Identifier : Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#**/

//
// Private worker functions for ASM_PFX()
//
#define _CONCATENATE(a, b)  __CONCATENATE(a, b)
#define __CONCATENATE(a, b) a ## b

#define __USER_LABEL_PREFIX__
//
// The __USER_LABEL_PREFIX__ macro predefined by GNUC represents the prefix
// on symbols in assembly language.
//
#define ASM_PFX(name) _CONCATENATE (__USER_LABEL_PREFIX__, name)

#define GCC_ASM_EXPORT(func__)  \
       .global  _CONCATENATE (__USER_LABEL_PREFIX__, func__)    ;\
       .type ASM_PFX(func__), %function


.text
.align 3


GCC_ASM_EXPORT (ArmReadMpidr)
GCC_ASM_EXPORT (ArmReadIdPfr0)
GCC_ASM_EXPORT (ArmReadIdPfr1)
GCC_ASM_EXPORT (AA64ReadMmfr0)
GCC_ASM_EXPORT (AA64ReadMmfr1)
GCC_ASM_EXPORT (AA64ReadMmfr2)
GCC_ASM_EXPORT (AA64ReadCtr)
GCC_ASM_EXPORT (ArmReadMmfr0)
GCC_ASM_EXPORT (AA64ReadIsar0)
GCC_ASM_EXPORT (AA64ReadIsar1)
GCC_ASM_EXPORT (AA64ReadIsar2)
GCC_ASM_EXPORT (AA64ReadSctlr3)
GCC_ASM_EXPORT (AA64ReadSctlr2)
GCC_ASM_EXPORT (AA64ReadSctlr1)
GCC_ASM_EXPORT (AA64ReadPmcr)
GCC_ASM_EXPORT (AA64ReadIdDfr0)
GCC_ASM_EXPORT (AA64ReadIdDfr1)
GCC_ASM_EXPORT (ArmReadHcr)
GCC_ASM_EXPORT (AA64ReadCurrentEL)
GCC_ASM_EXPORT (AA64ReadIdMdrar)
GCC_ASM_EXPORT (AA64ReadMdcr2)
GCC_ASM_EXPORT (AA64WriteMdcr2)
GCC_ASM_EXPORT (AA64ReadVbar2)
GCC_ASM_EXPORT (AA64WriteVbar2)
GCC_ASM_EXPORT (AA64WritePmcr)
GCC_ASM_EXPORT (AA64WritePmovsset)
GCC_ASM_EXPORT (AA64WritePmintenset)
GCC_ASM_EXPORT (AA64WritePmovsclr)
GCC_ASM_EXPORT (AA64WritePmintenclr)
GCC_ASM_EXPORT (AA64ReadCcsidr)
GCC_ASM_EXPORT (AA64ReadCsselr)
GCC_ASM_EXPORT (AA64WriteCsselr)
GCC_ASM_EXPORT (AA64ReadClidr)
GCC_ASM_EXPORT (ArmReadDfr0)
GCC_ASM_EXPORT (ArmReadIsar0)
GCC_ASM_EXPORT (ArmReadIsar1)
GCC_ASM_EXPORT (ArmReadIsar2)
GCC_ASM_EXPORT (ArmReadIsar3)
GCC_ASM_EXPORT (ArmReadIsar4)
GCC_ASM_EXPORT (ArmReadIsar5)
GCC_ASM_EXPORT (ArmReadMmfr0)
GCC_ASM_EXPORT (ArmReadMmfr1)
GCC_ASM_EXPORT (ArmReadMmfr2)
GCC_ASM_EXPORT (ArmReadMmfr3)
GCC_ASM_EXPORT (ArmReadMmfr4)
GCC_ASM_EXPORT (ArmReadPfr0)
GCC_ASM_EXPORT (ArmReadPfr1)
GCC_ASM_EXPORT (ArmReadMidr)
GCC_ASM_EXPORT (ArmReadMvfr0)
GCC_ASM_EXPORT (ArmReadMvfr1)
GCC_ASM_EXPORT (ArmReadMvfr2)
GCC_ASM_EXPORT (AA64ReadPmceid0)
GCC_ASM_EXPORT (AA64ReadPmceid1)
GCC_ASM_EXPORT (AA64ReadVmpidr)
GCC_ASM_EXPORT (AA64ReadVpidr)
GCC_ASM_EXPORT (AA64ReadPmbidr)
GCC_ASM_EXPORT (AA64ReadPmsidr)
GCC_ASM_EXPORT (AA64ReadLorid)
GCC_ASM_EXPORT (AA64ReadErridr)
GCC_ASM_EXPORT (AA64ReadErr0fr)
GCC_ASM_EXPORT (AA64ReadErr1fr)
GCC_ASM_EXPORT (AA64ReadErr2fr)
GCC_ASM_EXPORT (AA64ReadErr3fr)
GCC_ASM_EXPORT (AA64WritePmsirr)
GCC_ASM_EXPORT (AA64WritePmscr2)
GCC_ASM_EXPORT (AA64WritePmsfcr)
GCC_ASM_EXPORT (AA64WritePmbptr)
GCC_ASM_EXPORT (AA64WritePmblimitr)
GCC_ASM_EXPORT (AA64ReadEsr2)
GCC_ASM_EXPORT (AA64ReadSp)
GCC_ASM_EXPORT (AA64WriteSp)
GCC_ASM_EXPORT (AA64ReadFar2)
GCC_ASM_EXPORT (ArmRdvl)
GCC_ASM_EXPORT (AA64ReadMair1)
GCC_ASM_EXPORT (AA64ReadMair2)
GCC_ASM_EXPORT (AA64ReadTcr1)
GCC_ASM_EXPORT (AA64ReadTcr2)
GCC_ASM_EXPORT (AA64ReadTtbr0El1)
GCC_ASM_EXPORT (AA64ReadTtbr0El2)
GCC_ASM_EXPORT (AA64ReadTtbr1El1)
GCC_ASM_EXPORT (AA64ReadTtbr1El2)
GCC_ASM_EXPORT (AA64ReadDbgbcr0El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr1El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr2El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr3El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr4El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr5El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr6El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr7El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr8El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr9El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr10El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr11El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr12El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr13El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr14El1)
GCC_ASM_EXPORT (AA64ReadDbgbcr15El1)
GCC_ASM_EXPORT (AA64ReadZfr0)
GCC_ASM_EXPORT (AA64ReadBrbidr0)
GCC_ASM_EXPORT (AA64ReadTrbidr)
GCC_ASM_EXPORT (AA64ReadTrcidr0)
GCC_ASM_EXPORT (AA64ReadTrcidr4)
GCC_ASM_EXPORT (AA64ReadTrcidr5)
GCC_ASM_EXPORT (AA64ReadVtcr)
GCC_ASM_EXPORT (AA64SetupTraceAccess)
GCC_ASM_EXPORT (AA64EnableETETrace)
GCC_ASM_EXPORT (AA64EnableTRBUTrace)
GCC_ASM_EXPORT (AA64GenerateETETrace)
GCC_ASM_EXPORT (AA64DisableTRBUTrace)
GCC_ASM_EXPORT (AA64DisableETETrace)
GCC_ASM_EXPORT (AA64ReadTrbPtrEl1)
GCC_ASM_EXPORT (AA64EnableTFO)
GCC_ASM_EXPORT (AA64DisableTFO)
GCC_ASM_EXPORT (AA64ReadTrblimitr1)
GCC_ASM_EXPORT (AA64WriteTrblimitr1)

ASM_PFX(ArmReadMpidr):
  mrs   x0, mpidr_el1           // read EL1 MPIDR
  ret

ASM_PFX(ArmReadIdPfr0):
  mrs   x0, id_aa64pfr0_el1   // Read ID_AA64PFR0 Register
  ret

ASM_PFX(ArmReadIdPfr1):
  mrs   x0, id_aa64pfr1_el1   // Read ID_AA64PFR0 Register
  ret

ASM_PFX(AA64ReadMmfr0):
  mrs   x0, id_aa64mmfr0_el1
  ret

ASM_PFX(AA64ReadMmfr1):
  mrs   x0, id_aa64mmfr1_el1
  ret

ASM_PFX(AA64ReadMmfr2):
  mrs   x0, id_aa64mmfr2_el1
  ret

ASM_PFX(AA64ReadCtr):
  mrs   x0, ctr_el0
  ret

ASM_PFX(AA64ReadIsar0):
  mrs   x0, id_aa64isar0_el1
  ret

ASM_PFX(AA64ReadIsar1):
  mrs   x0, id_aa64isar1_el1
  ret

ASM_PFX(AA64ReadIsar2):
  mrs   x0, id_aa64isar2_el1
  ret

ASM_PFX(AA64ReadSctlr3):
  mrs   x0, sctlr_el3
  ret

ASM_PFX(AA64ReadSctlr2):
  mrs   x0, sctlr_el2
  ret

ASM_PFX(AA64ReadSctlr1):
  mrs   x0, sctlr_el1
  ret

ASM_PFX(AA64ReadPmcr):
  mrs   x0, pmcr_el0
  ret

ASM_PFX(AA64ReadIdDfr0):
  mrs   x0, id_aa64dfr0_el1
  ret

ASM_PFX(AA64ReadIdDfr1):
  mrs   x0, id_aa64dfr1_el1
  ret

// UINTN ArmReadHcr(VOID)
ASM_PFX(ArmReadHcr):
  mrs   x0, hcr_el2
  ret

ASM_PFX(AA64ReadCurrentEL):
  mrs   x0, CurrentEL
  ret

ASM_PFX(AA64ReadMdcr2):
  mrs   x0, mdcr_el2
  ret

ASM_PFX(AA64WriteMdcr2):
  msr   mdcr_el2, x0
  isb
  ret

ASM_PFX(AA64ReadVbar2):
  mrs   x0, vbar_el2
  ret

ASM_PFX(AA64WriteVbar2):
  msr   vbar_el2, x0
  isb
  ret

ASM_PFX(AA64WritePmcr):
  msr   pmcr_el0, x0
  isb
  ret

ASM_PFX(AA64WritePmovsset):
  msr   pmovsset_el0, x0
  isb
  ret

ASM_PFX(AA64WritePmovsclr):
  msr   pmovsclr_el0, x0
  isb
  ret

ASM_PFX(AA64WritePmintenset):
  msr   pmintenset_el1, x0
  isb
  ret

ASM_PFX(AA64WritePmintenclr):
  msr   pmintenclr_el1, x0
  isb
  ret

ASM_PFX(AA64ReadCcsidr):
  mrs   x0, ccsidr_el1
  ret

ASM_PFX(AA64ReadCsselr):
  mrs   x0, csselr_el1
  ret

ASM_PFX(AA64WriteCsselr):
  msr   csselr_el1, x0
  isb
  ret

ASM_PFX(AA64ReadClidr):
  mrs   x0, clidr_el1
  ret

ASM_PFX(ArmReadDfr0):
  mrs   x0, id_dfr0_el1
  ret

ASM_PFX(ArmReadIsar0):
  mrs   x0, id_isar0_el1
  ret

ASM_PFX(ArmReadIsar1):
  mrs   x0, id_isar1_el1
  ret

ASM_PFX(ArmReadIsar2):
  mrs   x0, id_isar2_el1
  ret

ASM_PFX(ArmReadIsar3):
  mrs   x0, id_isar3_el1
  ret

ASM_PFX(ArmReadIsar4):
  mrs   x0, id_isar4_el1
  ret

ASM_PFX(ArmReadIsar5):
  mrs   x0, id_isar5_el1
  ret

ASM_PFX(ArmReadMmfr0):
  mrs   x0, id_mmfr0_el1
  ret

ASM_PFX(ArmReadMmfr1):
  mrs   x0, id_mmfr1_el1
  ret

ASM_PFX(ArmReadMmfr2):
  mrs   x0, id_mmfr2_el1
  ret

ASM_PFX(ArmReadMmfr3):
  mrs   x0, id_mmfr3_el1
  ret

ASM_PFX(ArmReadMmfr4):
  mrs   x0, id_mmfr4_el1
  ret

ASM_PFX(ArmReadPfr0):
  mrs   x0, id_pfr0_el1
  ret

ASM_PFX(ArmReadPfr1):
  mrs   x0, id_pfr1_el1
  ret

ASM_PFX(ArmReadMidr):
  mrs   x0, midr_el1
  ret

ASM_PFX(ArmReadMvfr0):
  mrs   x0, mvfr0_el1
  ret

ASM_PFX(ArmReadMvfr1):
  mrs   x0, mvfr1_el1
  ret

ASM_PFX(ArmReadMvfr2):
  mrs   x0, mvfr2_el1
  ret

ASM_PFX(AA64ReadPmceid0):
  mrs   x0, pmceid0_el0
  ret

ASM_PFX(AA64ReadPmceid1):
  mrs   x0, pmceid1_el0
  ret

ASM_PFX(AA64ReadVmpidr):
  mrs   x0, vmpidr_el2
  ret

ASM_PFX(AA64ReadVpidr):
  mrs   x0, vpidr_el2
  ret

ASM_PFX(AA64ReadPmbidr):
  mrs   x0, pmbidr_el1
  ret

ASM_PFX(AA64ReadPmsidr):
  mrs   x0, pmsidr_el1
  ret

ASM_PFX(AA64ReadLorid):
  mrs   x0, lorid_el1
  ret

ASM_PFX(AA64ReadErridr):
  mrs   x0, erridr_el1
  ret

ASM_PFX(AA64ReadErr0fr):
 // mrs   x0, err0fr_el1
  ret

ASM_PFX(AA64ReadErr1fr):
  //mrs   x0, err1fr_el1
  ret

ASM_PFX(AA64ReadErr2fr):
  //mrs   x0, err2fr_el1
  ret

ASM_PFX(AA64ReadErr3fr):
  //mrs   x0, err3fr_el1
  ret

ASM_PFX(AA64WritePmsirr):
  //mrs   pmsirr_el1,x0
  isb
  ret

ASM_PFX(AA64WritePmscr2):
  //mrs   pmscr_el2,x0
  isb
  ret

ASM_PFX(AA64WritePmsfcr):
  //mrs   pmsfcr_el1,x0
  isb
  ret

ASM_PFX(AA64WritePmbptr):
  //mrs   pmbptr_el1,x0
  isb
  ret

ASM_PFX(AA64WritePmblimitr):
  //mrs   pmblimitr_el1,x0
  isb
  ret

ASM_PFX(AA64ReadEsr2):
  mrs   x0, esr_el2
  ret

ASM_PFX(AA64ReadSp):
  mov   x0, sp
  ret

ASM_PFX(AA64WriteSp):
  mov   sp, x0
  ret

ASM_PFX(AA64ReadFar2):
  mrs   x0, far_el2
  ret

ASM_PFX(AA64ReadMair1):
  mrs   x0, mair_el1           // read EL1 MAIR
  ret

ASM_PFX(AA64ReadMair2):
  mrs   x0, mair_el2           // read EL2 MAIR
  ret

ASM_PFX(AA64ReadTcr1):
  mrs   x0, tcr_el1           // read EL1 TCR
  ret

ASM_PFX(AA64ReadTcr2):
  mrs   x0, tcr_el2           // read EL2 TCR
  ret

ASM_PFX(AA64ReadTtbr0El2):
  mrs   x0, ttbr0_el2           // read EL2 TTBR0
  ret

ASM_PFX(AA64ReadTtbr1El2):
  mrs   x0, ttbr1_el2           // read EL2 TTBR1
  ret

ASM_PFX(AA64ReadTtbr0El1):
  mrs   x0, ttbr0_el1           // read EL1 TTBR0
  ret

ASM_PFX(AA64ReadTtbr1El1):
  mrs   x0, ttbr1_el1           // read EL1 TTBR1
  ret

ASM_PFX(ArmRdvl):
  //RDVL   x0, #8   // TODO once instruction supports Read Vector Length
  .inst 0x04BF5100
  ret

 ASM_PFX(AA64ReadDbgbcr0El1):
  mrs   x0, dbgbcr0_el1          // read EL1 DBGBCR0
  ret

ASM_PFX(AA64ReadDbgbcr1El1):
  mrs   x0, dbgbcr1_el1          // read EL1 DBGBCR1
  ret

ASM_PFX(AA64ReadDbgbcr2El1):
  mrs   x0, dbgbcr2_el1          // read EL1 DBGBCR2
  ret

ASM_PFX(AA64ReadDbgbcr3El1):
  mrs   x0, dbgbcr3_el1          // read EL1 DBGBCR3
  ret

ASM_PFX(AA64ReadDbgbcr4El1):
  mrs   x0, dbgbcr4_el1          // read EL1 DBGBCR4
  ret

ASM_PFX(AA64ReadDbgbcr5El1):
  mrs   x0, dbgbcr5_el1          // read EL1 DBGBCR5
  ret

ASM_PFX(AA64ReadDbgbcr6El1):
  mrs   x0, dbgbcr6_el1          // read EL1 DBGBCR6
  ret

ASM_PFX(AA64ReadDbgbcr7El1):
  mrs   x0, dbgbcr7_el1          // read EL1 DBGBCR7
  ret

ASM_PFX(AA64ReadDbgbcr8El1):
  mrs   x0, dbgbcr8_el1          // read EL1 DBGBCR8
  ret

ASM_PFX(AA64ReadDbgbcr9El1):
  mrs   x0, dbgbcr9_el1          // read EL1 DBGBCR9
  ret

ASM_PFX(AA64ReadDbgbcr10El1):
  mrs   x0, dbgbcr10_el1          // read EL1 DBGBCR10
  ret

ASM_PFX(AA64ReadDbgbcr11El1):
  mrs   x0, dbgbcr11_el1          // read EL1 DBGBCR11
  ret

ASM_PFX(AA64ReadDbgbcr12El1):
  mrs   x0, DBGBCR12_EL1          // read EL1 DBGBCR12
  ret

ASM_PFX(AA64ReadDbgbcr13El1):
  mrs   x0, dbgbcr13_el1          // read EL1 DBGBCR13
  ret

ASM_PFX(AA64ReadDbgbcr14El1):
  mrs   x0, dbgbcr14_el1          // read EL1 DBGBCR14
  ret

ASM_PFX(AA64ReadDbgbcr15El1):
  mrs   x0, dbgbcr15_el1          // read EL1 DBGBCR15
  ret

ASM_PFX(AA64ReadZfr0):
  mrs x0, id_aa64zfr0_el1
  ret

ASM_PFX(AA64ReadBrbidr0):
  mrs x0, brbidr0_el1
  ret

ASM_PFX(AA64ReadTrbidr):
  mrs x0, trbidr_el1
  ret

ASM_PFX(AA64ReadTrcidr0):
  mrs x0, trcidr0
  ret

ASM_PFX(AA64ReadTrcidr4):
  mrs x0, trcidr4
  ret

ASM_PFX(AA64ReadTrcidr5):
  mrs x0, trcidr5
  ret

ASM_PFX(AA64ReadVtcr):
  mrs x0, vtcr_el2
  ret

ASM_PFX(AA64ReadTrblimitr1):
  mrs x0, TRBLIMITR_EL1
  ret

ASM_PFX(AA64WriteTrblimitr1):
  msr TRBLIMITR_EL1, x0
  isb
  ret

ASM_PFX(AA64SetupTraceAccess):
  // Allows TRBU reg writes from EL1 and EL2
  // Programming of MDCR_EL3.NSTB = 0b11 is done in arm-tf
  mrs x1, MDCR_EL2
  bic x1, x1, #(3<<24) // MDCR_EL2.E2TB = 0b00 for EL2 tracing
  msr MDCR_EL2, x1
  isb
  msr OSLAR_EL1, xzr
  isb
  ret

ASM_PFX(AA64EnableTFO):
  /* Set OSLK */
  mov x1, 0x1
  msr OSLAR_EL1, x1
  isb
  mrs x1, MDSCR_EL1
  orr x1, x1, #(1<<31)
  msr MDSCR_EL1, x1
  isb
  mrs x1, TRBMAR_EL1
  orr x1, x1, #(1<<10)
  msr TRBMAR_EL1, x1
  isb
  ret

ASM_PFX(AA64DisableTFO):
  /* Set OSLK */
  mov x1, 0x1
  msr OSLAR_EL1, x1
  isb
  mrs x1, MDSCR_EL1
  bic x1, x1, #(1<<31) // MDSCR_EL1.TFO=0b0
  msr MDSCR_EL1, x1
  isb
  ret

ASM_PFX(AA64EnableETETrace):
  /* Set TRCRSR.TA as active */
  mov x0, #(1<<12)
  MSR TRCRSR, x0
  isb

  /* Enable Trace Unit */
  mov x0, #(1<<0)
  msr TRCPRGCTLR, x0
  isb
  ret

ASM_PFX(AA64EnableTRBUTrace):
    /* Add an offset for different PE */
    mov x0, x0, lsl #12   // x0 has Current PE Index
    add x1, x1, x0        // Updated Base Address for Curent PE

    /* Update TRBBASER_EL1, TRBPTR_EL1 */
    msr TRBBASER_EL1, x1
    isb
    msr TRBPTR_EL1, x1
    isb

    /* Program TRBLIMITR_EL1 */
    add x1, x1, #0x1000  // Set 4KB trace buffer limit for Current PE
    add x1, x1, x2
    orr x1, x1, #(3<<1)  // TRBLIMITR_EL1.FM_CIRCULAR_BUFFER_MODE
    orr x1, x1, #(3<<3)  // TRBLIMITR_EL1_TM_IGNORE_TRIGGER
    msr TRBLIMITR_EL1, x1
    isb

    /* Update TRFCR_EL2 to enable trace at EL2 */
    mrs x1, S3_4_C1_C2_1
    orr x1, x1, #(1<<1) // TRFCR_EL2.E2TRE = 0b1
    orr x1, x1, #(1<<3) // TRFCR_EL2.CX = 0b1
    orr x1, x1, #(3<<5) // TRFCR_EL2.TS = 0b11
    msr S3_4_C1_C2_1, x1
    isb

    /* TRCCONFIGR.TS = 1 */
    mrs x1, trcconfigr
    orr x1, x1, #(1<<11)
    msr trcconfigr, x1
    isb

    /* Set TRBU as Running, TRBSR_EL1.S = 0 */
    msr TRBSR_EL1, xzr
    isb

    /* Set TRCVICTLR.SSSTATUS = 1 (started), all exception levels enabled */
    mov x1, 0x201
    msr TRCVICTLR, x1
    isb
    ret

ASM_PFX(AA64DisableTRBUTrace):
    /* Flush trace unit before disabling TRBU */
    tsb CSYNC

    /* TRBSR_EL1.S Set TRBU as Stopped */
    mov x0, 0x20000
    msr TRBSR_EL1, x0
    isb

    /* TRBLIMITR_EL1 Disable TRBU */
    mrs x1, TRBLIMITR_EL1
    bic x1, x1, #(1<<0) // TRBLIMITR_EL1.E = 0b0
    msr TRBLIMITR_EL1, x1
    isb
    ret

ASM_PFX(AA64GenerateETETrace):
  /* Generate trace for sample instructions */
  MOV x0, #1000
  MOV x2, x0
  CMP x2, xzr
  B branch
  return:
    ret

  branch:
    SUB x0, x0, #1
    CMP x0, xzr
    BNE branch
    B return


ASM_PFX(AA64DisableETETrace):
  /* Disable trace */
  ISB
  TSB CSYNC
  ISB

  MSR TRCPRGCTLR, xzr
  ISB

  B branch1
  return1:
    ret

  branch1:
    MOV x2, x0
    CMP x2, xzr
    B return1

ASM_PFX(AA64ReadTrbPtrEl1):
  mrs   x0, TRBPTR_EL1
  ret
