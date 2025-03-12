#/** @file
# Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
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

.text
.align 3

.equ DRTM_FN_DYNAMIC_LAUNCH, 0xC4000114

/*
 * void __asm_dcache_level(level)
 *
 * flush or invalidate one level cache.
 *
 * x0: cache level
 * x1: 0 clean & invalidate, 1 invalidate only
 * x2~x9: clobbered
 */
.globl asm_flush_dcache_level
asm_flush_dcache_level:
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
.globl asm_flush_dcache_all
asm_flush_dcache_all:
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
    bl    asm_flush_dcache_level     /* x1 = 0 flush, 1 invalidate */
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
    b      cache_flush_done

GCC_ASM_EXPORT(val_drtm_simulate_dl)

ASM_PFX(val_drtm_simulate_dl):
    /*
     * Preserve callee-saved registers,
     * frame and link registers, and stack pointer
     * into g_drtm_acs_dl_saved_state
     */
    ldr x9,  g_drtm_acs_dl_saved_state
    str x19, [x9]
    str x20, [x9, #8]
    str x21, [x9, #16]
    str x22, [x9, #24]
    str x23, [x9, #32]
    str x24, [x9, #40]
    str x25, [x9, #48]
    str x26, [x9, #56]
    str x27, [x9, #64]
    str x28, [x9, #72]
    str x29, [x9, #80]
    str x30, [x9, #88]
    mov x10, sp
    str x10, [x9, #96]
    mrs x10, sp_el0
    str x10, [x9, #104]
    mrs x10, sctlr_el2
    str x10, [x9, #112]
    /* Save Dynamic Launch Argument from caller */
    mov x13, x0
    /* Data Cache Flush */
    mov x0, #0
    b   asm_flush_dcache_all
cache_flush_done:
    /* Invalidate Icache */
    ic iallu
    /* Sync Barrier */
    dsb sy
    isb

    /* Restore Dynamic Launch Argument from caller */
    mov x1, x13
    ldr x0, =DRTM_FN_DYNAMIC_LAUNCH
    smc #0

    /*
     * The SMC call will return only in case of an error
     */
    ret
