#/** @file
# Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
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
    dsb sy
    isb

    mov x1, x0
    ldr x0, =DRTM_FN_DYNAMIC_LAUNCH
    smc #0

    /*
     * The SMC call will return only in case of an error
     */
    ret
