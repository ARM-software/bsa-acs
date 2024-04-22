/*
 * Boot entry point and assembler functions for aarch64 tests.
 *
 * Copyright (C) 2017, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 * Copyright (C) 2023, Arm Ltd.
 *
 * This work is licensed under the terms of the GNU GPL, version 2.
 */
#define __ASSEMBLY__
#define BSA_ACS
#include <auxinfo.h>
#include <asm/assembler.h>
#include <asm/ptrace.h>
#include <asm/page.h>
#include <asm/pgtable-hwdef.h>
#include <asm/thread_info.h>
#include <asm/sysreg.h>

#define S_X0 0 /* offsetof(struct pt_regs, regs[0]) */
#define S_X1 8 /* offsetof(struct pt_regs, regs[1]) */
#define S_X2 16 /* offsetof(struct pt_regs, regs[2]) */
#define S_X3 24 /* offsetof(struct pt_regs, regs[3]) */
#define S_X4 32 /* offsetof(struct pt_regs, regs[4]) */
#define S_X5 40 /* offsetof(struct pt_regs, regs[5]) */
#define S_X6 48 /* offsetof(struct pt_regs, regs[6]) */
#define S_X7 56 /* offsetof(struct pt_regs, regs[7]) */
#define S_LR 240 /* offsetof(struct pt_regs, regs[30]) */
#define S_SP 248 /* offsetof(struct pt_regs, sp) */
#define S_PC 256 /* offsetof(struct pt_regs, pc) */
#define S_PSTATE 264 /* offsetof(struct pt_regs, pstate) */
#define S_ORIG_X0 272 /* offsetof(struct pt_regs, orig_x0) */
#define S_SYSCALLNO 280 /* offsetof(struct pt_regs, syscallno) */
#define S_FRAME_SIZE 288 /* sizeof(struct pt_regs) */

    .irp    num,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30
    .equ    .L__reg_num_x\num, \num
    .endr
    .equ    .L__reg_num_xzr, 31

    .macro    mrs_s, rt, sreg
    .inst    0xd5200000|(\sreg)|(.L__reg_num_\rt)
    .endm

    .macro    msr_s, sreg, rt
    .inst    0xd5000000|(\sreg)|(.L__reg_num_\rt)
    .endm

    .macro    raw_dcache_line_size, reg, tmp
    mrs    \tmp, ctr_el0            // read CTR
    ubfx    \tmp, \tmp, #16, #4        // cache line size encoding
    mov    \reg, #4            // bytes per word
    lsl    \reg, \reg, \tmp        // actual cache line size
    .endm

    .macro dcache_by_line_op op, domain, addr, size, tmp1, tmp2
    raw_dcache_line_size \tmp1, \tmp2
    add    \size, \addr, \size
    sub    \tmp2, \tmp1, #1
    bic    \addr, \addr, \tmp2
9998:
    dc    \op, \addr
    add    \addr, \addr, \tmp1
    cmp    \addr, \size
    b.lo    9998b
    dsb    \domain
    .endm


.text

/*
 * psci_invoke_hvc / psci_invoke_smc
 *
 * Inputs:
 *   w0 -- function_id
 *   x1 -- arg0
 *   x2 -- arg1
 *   x3 -- arg2
 *
 * Outputs:
 *   x0 -- return code
 */
.globl psci_invoke_hvc
psci_invoke_hvc:
    hvc    #0
    ret

.globl psci_invoke_smc
psci_invoke_smc:
    smc    #0
    ret

.globl secondary_entry
secondary_entry:
    mrs   x0, CurrentEL
    cmp   x0, CurrentEL_EL2
    b.ne  1f
    bl    asm_setup_el2
    bl    asm_drop_el1
1:
    /* Enable FP/ASIMD */
    mov    x0, #(3 << 20)
    msr    cpacr_el1, x0

    /* set up exception handling */
    bl    exceptions_init

    /* enable the MMU unless requested off */
    adrp    x0, mmu_idmap
    ldr    x0, [x0, :lo12:mmu_idmap]
    bl    asm_mmu_enable

    /* set the stack */
    adrp    x0, secondary_data
    ldr    x0, [x0, :lo12:secondary_data]
    mov    sp, x0

    /* finish init in C code */
    bl    secondary_cinit

    /* x0 is now the entry function, run it */
    blr    x0
    b    do_idle

.globl halt
halt:
1:    wfi
    b    1b

/*
 * asm_mmu_enable
 *   Inputs:
 *     x0 is the base address of the translation table
 *   Outputs: none
 *
 * Adapted from
 *   arch/arm64/kernel/head.S
 *   arch/arm64/mm/proc.S
 */

/*
 * Memory region attributes for LPAE:
 *
 *   n = AttrIndx[2:0]
 *                      n       MAIR
 *   DEVICE_nGnRnE      000     00000000
 *   DEVICE_nGnRE       001     00000100
 *   DEVICE_GRE         010     00001100
 *   NORMAL_NC          011     01000100
 *   NORMAL             100     11111111
 *   NORMAL_WT          101     10111011
 *   DEVICE_nGRE        110     00001000
 */
#define MAIR(attr, mt) ((attr) << ((mt) * 8))

#if PAGE_SIZE == SZ_64K
#define TCR_TG_FLAGS    TCR_TG0_64K | TCR_TG1_64K
#elif PAGE_SIZE == SZ_16K
#define TCR_TG_FLAGS    TCR_TG0_16K | TCR_TG1_16K
#elif PAGE_SIZE == SZ_4K
#define TCR_TG_FLAGS    TCR_TG0_4K | TCR_TG1_4K
#endif

.globl asm_mmu_enable
asm_mmu_enable:
    tlbi    vmalle1            // invalidate I + D TLBs
    dsb    nsh

    /* TCR */
    ldr    x1, =TCR_TxSZ(VA_BITS) |        \
             TCR_TG_FLAGS  |            \
             TCR_IRGN_WBWA | TCR_ORGN_WBWA |    \
             TCR_SHARED |            \
             TCR_EPD1
    mrs    x2, id_aa64mmfr0_el1
    bfi    x1, x2, #32, #3
    msr    tcr_el1, x1

    /* MAIR */
    ldr    x1, =MAIR(0x00, MT_DEVICE_nGnRnE) |    \
             MAIR(0x04, MT_DEVICE_nGnRE) |    \
             MAIR(0x0c, MT_DEVICE_GRE) |    \
             MAIR(0x44, MT_NORMAL_NC) |        \
             MAIR(0xff, MT_NORMAL) |            \
             MAIR(0xbb, MT_NORMAL_WT) |         \
             MAIR(0x08, MT_DEVICE_nGRE)
    msr    mair_el1, x1

    /* TTBR0 */
    msr    ttbr0_el1, x0
    isb

    /* SCTLR */
    mrs    x1, sctlr_el1
    orr    x1, x1, SCTLR_EL1_C
    orr    x1, x1, SCTLR_EL1_I
    orr    x1, x1, SCTLR_EL1_M
    msr    sctlr_el1, x1
    isb

    ret

.globl asm_mmu_disable
asm_mmu_disable:
    /* check current EL */
    mrs x0, CurrentEL
    cmp x0, CurrentEL_EL1
    b.eq 1f

    /* if system is running at EL1 */
    /* Disable data cache */
    /* In case of exclusive caches, the line can ping-pong between caches in a way that
    nullifies Set-Way based CMO, hence disabling data cache */
    mrs x0, sctlr_el2
    bic x0, x0, SCTLR_EL1_C
    msr sctlr_el2, x0
    isb

    /* Clean + invalidate the entire memory */
    mov  x0, #0
    b    asm_dcache_all

    /* disable MMU */
    mrs x0, sctlr_el2
    bic x0, x0, SCTLR_EL1_M
    msr sctlr_el2, x0
    isb

    /* Clean + invalidate the entire memory */
    mov  x0, #0
    b    asm_dcache_all

    /* Invalidate Icache */
    ic iallu
    isb

    /* Disable data and instruction caches*/
    mrs x0, sctlr_el2
    bic x0, x0, SCTLR_EL1_C
    bic x0, x0, SCTLR_EL1_I
    msr sctlr_el2, x0
    isb

    tlbi    vmalle1
    dsb    nsh
    ret
1:
    /* if system is running at EL1 */
    /* Clean + invalidate the entire memory */
    adrp    x0, __phys_offset
    ldr    x0, [x0, :lo12:__phys_offset]
    adrp    x1, __phys_end
    ldr    x1, [x1, :lo12:__phys_end]
    sub    x1, x1, x0
    dcache_by_line_op civac, sy, x0, x1, x2, x3

    /* Invalidate Icache */
    ic    iallu
    isb

    /* Disable cache, Icache and MMU */
    mrs x0, sctlr_el1
    bic x0, x0, SCTLR_EL1_C
    bic x0, x0, SCTLR_EL1_I
    bic x0, x0, SCTLR_EL1_M
    msr sctlr_el1, x0
    isb

    /* Invalidate TLB */
    tlbi    vmalle1
    dsb    nsh
    ret

install_el2_stub:
    ldr    x0, =INIT_SCTLR_EL1_MMU_OFF
    msr    sctlr_el1, x0

    /* Coprocessor traps. */
    mov    x0, #0x33ff
    msr    cptr_el2, x0                    // Disable copro. traps to EL2

    /* SVE register access */
    mrs    x1, id_aa64pfr0_el1
    ubfx   x1, x1, #ID_AA64PFR0_SVE_SHIFT, #4
    cbz    x1, 1f

    bic    x0, x0, #CPTR_EL2_TZ            // Also disable SVE traps
    msr    cptr_el2, x0                    // Disable copro. traps to EL2
    isb
    mov    x1, #ZCR_ELx_LEN_MASK           // SVE: Enable full vector
    msr_s  SYS_ZCR_EL2, x1                 // length for EL1.

    /* Hypervisor stub */
1:
    msr    vbar_el2, xzr

    /* spsr */
    mov    x0, #(PSR_F_BIT | PSR_I_BIT | PSR_A_BIT | PSR_D_BIT |\
            PSR_MODE_EL1h)
    msr    spsr_el2, x0
    ret

.globl asm_setup_el2
asm_setup_el2:
    ldr    x0, =SCTLR_EL2_RES1
    msr    sctlr_el2, x0

    /* Hyp configuration. */
    ldr    x0, =HCR_HOST_NVHE_FLAGS
    msr    hcr_el2, x0
    isb

    /*
    * Allow Non-secure EL1 and EL0 to access physical timer and counter.
    * This is not necessary for VHE, since the host kernel runs in EL2,
    * Note that when HCR_EL2.E2H == 1, CNTHCTL_EL2 has the same bit layout
    * as CNTKCTL_EL1, and CNTKCTL_EL1 accessing instructions are redefined
    * to access CNTHCTL_EL2. This allows the kernel designed to run at EL1
    * to transparently mess with the EL0 bits via CNTKCTL_EL1 access in
    * EL2.
    */
    mrs    x0, cnthctl_el2
    /* Enable EL1 physical timers. */
    orr    x0, x0, #3
    msr    cnthctl_el2, x0

    /* Clear the timer virtual offset. */
    msr    cntvoff_el2, xzr

    /* GICv3 system register access */
    mrs    x0, id_aa64dfr0_el1
    ubfx   x0, x0, #ID_AA64PFR0_GIC_SHIFT, #4
    cbz    x0, 1f

    mrs_s  x0, SYS_ICC_SRE_EL2
    orr    x0, x0, #ICC_SRE_EL2_SRE        // Set ICC_SRE_EL2.SRE==1
    orr    x0, x0, #ICC_SRE_EL2_ENABLE     // Set ICC_SRE_EL2.Enable==1
    msr_s  SYS_ICC_SRE_EL2, x0
    isb                                      // Make sure SRE is now set
    mrs_s    x0, SYS_ICC_SRE_EL2             // Read SRE back,
    tbz      x0, #0, 1f                      // and check that it sticks
    msr_s    SYS_ICH_HCR_EL2, xzr            // Reset ICC_HCR_EL2 to defaults
1:
    /* Populate ID registers. */
    mrs    x0, midr_el1
    mrs    x1, mpidr_el1
    msr    vpidr_el2, x0
    msr    vmpidr_el2, x1

    /* EL2 debug */
    mrs     x1, id_aa64dfr0_el1
    sbfx    x0, x1, #ID_AA64DFR0_PMUVER_SHIFT, #4
    cmp     x0, #1
    b.lt    1f                              // Skip if no PMU present
    mrs     x0, pmcr_el0                    // Disable debug access traps
    ubfx    x0, x0, #11, #5                 // to EL2 and allow access to
1:
    csel    x3, xzr, x0, lt                 // all PMU counters from EL1

    /* Statistical profiling */
    ubfx    x0, x1, #ID_AA64DFR0_PMSVER_SHIFT, #4
    cbz     x0, 2f                                     // Skip if SPE not present
    mrs_s   x4, SYS_PMBIDR_EL1                         // If SPE available at EL2,
    and     x4, x4, #(1 << SYS_PMBIDR_EL1_P_SHIFT)
    cbnz    x4, 1f                                     // then permit sampling of physical
    mov     x4, #(1 << SYS_PMSCR_EL2_PCT_SHIFT | \
             1 << SYS_PMSCR_EL2_PA_SHIFT)
    msr_s    SYS_PMSCR_EL2, x4                         // addresses and physical counter
1:
    mov    x1, #(MDCR_EL2_E2PB_MASK << MDCR_EL2_E2PB_SHIFT)
    orr    x3, x3, x1                                       // If we don't have VHE, then
    b    1f                                                 // use EL1&0 translation.

    orr    x3, x3, #MDCR_EL2_TPMS           // and disable access from EL1
1:
    msr    mdcr_el2, x3                     // Configure debug traps

    /* LORegions */
    mrs    x1, id_aa64mmfr1_el1
    ubfx   x0, x1, #ID_AA64MMFR1_LOR_SHIFT, 4
    cbz    x0, 2f
    msr_s  SYS_LORC_EL1, xzr
2:
    /* Stage-2 translation */
    msr    vttbr_el2, xzr
    isb
    b install_el2_stub

.globl asm_drop_el1
asm_drop_el1:
    msr    elr_el2, lr

    /* enable FP/ASIMD */
    mov    x0, #(3 << 20)
    msr    cpacr_el1, x0

    /* set SCTLR_EL1 to a known value */
    ldr    x0, =INIT_SCTLR_EL1_MMU_OFF
    msr    sctlr_el1, x0

    # stack for EL1, reuse current
    mov x0, sp
    msr sp_el1, x0

    eret

/*
 * Vectors
 */

.globl exceptions_init
exceptions_init:
    adrp    x4, vector_table
    add    x4, x4, :lo12:vector_table
    msr    vbar_el1, x4
    isb
    ret

/*
 * Vector stubs
 * Adapted from arch/arm64/kernel/entry.S
 */
.macro vector_stub, name, vec
.weak \name
\name:
    stp     x0,  x1, [sp, #-S_FRAME_SIZE]!
    stp     x2,  x3, [sp,  #16]
    stp     x4,  x5, [sp,  #32]
    stp     x6,  x7, [sp,  #48]
    stp     x8,  x9, [sp,  #64]
    stp    x10, x11, [sp,  #80]
    stp    x12, x13, [sp,  #96]
    stp    x14, x15, [sp, #112]
    stp    x16, x17, [sp, #128]
    stp    x18, x19, [sp, #144]
    stp    x20, x21, [sp, #160]
    stp    x22, x23, [sp, #176]
    stp    x24, x25, [sp, #192]
    stp    x26, x27, [sp, #208]
    stp    x28, x29, [sp, #224]

    str    x30, [sp, #S_LR]

    .if \vec >= 8
    mrs    x1, sp_el0
    .else
    add    x1, sp, #S_FRAME_SIZE
    .endif
    str    x1, [sp, #S_SP]

    mrs    x1, elr_el1
    mrs    x2, spsr_el1
    stp    x1, x2, [sp, #S_PC]

    mov    x0, \vec
    mov    x1, sp
    mrs    x2, esr_el1
    bl    do_handle_exception

    ldp    x1, x2, [sp, #S_PC]
    msr    spsr_el1, x2
    msr    elr_el1, x1

    .if \vec >= 8
    ldr    x1, [sp, #S_SP]
    msr    sp_el0, x1
    .endif

    ldr    x30, [sp, #S_LR]

    ldp    x28, x29, [sp, #224]
    ldp    x26, x27, [sp, #208]
    ldp    x24, x25, [sp, #192]
    ldp    x22, x23, [sp, #176]
    ldp    x20, x21, [sp, #160]
    ldp    x18, x19, [sp, #144]
    ldp    x16, x17, [sp, #128]
    ldp    x14, x15, [sp, #112]
    ldp    x12, x13, [sp,  #96]
    ldp    x10, x11, [sp,  #80]
    ldp     x8,  x9, [sp,  #64]
    ldp     x6,  x7, [sp,  #48]
    ldp     x4,  x5, [sp,  #32]
    ldp     x2,  x3, [sp,  #16]
    ldp     x0,  x1, [sp], #S_FRAME_SIZE

    eret
.endm

vector_stub    el1t_sync,     0
vector_stub    el1t_irq,      1
vector_stub    el1t_fiq,      2
vector_stub    el1t_error,    3

vector_stub    el1h_sync,     4
vector_stub    el1h_irq,      5
vector_stub    el1h_fiq,      6
vector_stub    el1h_error,    7

vector_stub    el0_sync_64,   8
vector_stub    el0_irq_64,    9
vector_stub    el0_fiq_64,   10
vector_stub    el0_error_64, 11

vector_stub    el0_sync_32,  12
vector_stub    el0_irq_32,   13
vector_stub    el0_fiq_32,   14
vector_stub    el0_error_32, 15

.section .text.ex

.macro ventry, label
.align 7
    b    \label
.endm


/*
 * void __asm_dcache_level(level)
 *
 * flush or invalidate one level cache.
 *
 * x0: cache level
 * x1: 0 clean & invalidate, 1 invalidate only
 * x2~x9: clobbered
 */
.globl asm_dcache_level
asm_dcache_level:
    lsl    x12, x0, #1
    msr    csselr_el1, x12        /* select cache level */
    isb                           /* sync change of cssidr_el1 */
    mrs    x6, ccsidr_el1         /* read the new cssidr_el1 */
    and    x2, x6, #7             /* x2 <- log2(cache line size)-4 */
    add    x2, x2, #4             /* x2 <- log2(cache line size) */
    mov    x3, #0x3ff
    and    x3, x3, x6, lsr #3     /* x3 <- max number of #ways */
    clz    w5, w3                 /* bit position of #ways */
    mov    x4, #0x7fff
    and    x4, x4, x6, lsr #13    /* x4 <- max number of #sets */
    /* x12 <- cache level << 1 */
    /* x2 <- line length offset */
    /* x3 <- number of cache ways - 1 */
    /* x4 <- number of cache sets - 1 */
    /* x5 <- bit position of #ways */
loop_set:
    mov    x6, x3            /* x6 <- working copy of #ways */
loop_way:
    lsl    x7, x6, x5
    orr    x9, x12, x7       /* map way and level to cisw value */
    lsl    x7, x4, x2
    orr    x9, x9, x7        /* map set number to cisw value */
    tbz    w1, #0, 1f
    dc    isw, x9
    b    2f
1:  dc      cisw, x9         /* clean & invalidate by set/way */
2:  subs    x6, x6, #1       /* decrement the way */
    b.ge    loop_way
    subs    x4, x4, #1       /* decrement the set */
    b.ge    loop_set

    ret

/*
 * void __asm_flush_dcache_all(int invalidate_only)
 *
 * x0: 0 clean & invalidate, 1 invalidate only
 *
 * flush or invalidate all data cache by SET/WAY.
 */
.globl asm_dcache_all
asm_dcache_all:
    mov    x1, x0
    dsb    sy
    mrs    x10, clidr_el1       /* read clidr_el1 */
    lsr    x11, x10, #24
    and    x11, x11, #0x7       /* x11 <- loc */
    cbz    x11, finished        /* if loc is 0, exit */
    mov    x15, lr
    mov    x0, #0               /* start flush at cache level 0 */
    /* x0  <- cache level */
    /* x10 <- clidr_el1 */
    /* x11 <- loc */
    /* x15 <- return address */
loop_level:
    lsl    x12, x0, #1
    add    x12, x12, x0        /* x0 <- tripled cache level */
    lsr    x12, x10, x12
    and    x12, x12, #7        /* x12 <- cache type */
    cmp    x12, #2
    b.lt    skip               /* skip if no cache or icache */
    bl    asm_dcache_level     /* x1 = 0 flush, 1 invalidate */
skip:
    add    x0, x0, #1          /* increment cache level */
    cmp    x11, x0
    b.gt    loop_level

    mov    x0, #0
    msr    csselr_el1, x0      /* restore csselr_el1 */
    dsb    sy
    isb
    mov    lr, x15

finished:
    ret
