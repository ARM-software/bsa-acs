#/** @file
# Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
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

//GCC_ASM_EXPORT (AA64ReadPmcr)
GCC_ASM_EXPORT (AA64ReadPmccntr)
GCC_ASM_EXPORT (AA64ReadPmccfiltr)
GCC_ASM_EXPORT (AA64ReadPmcntenset)
//GCC_ASM_EXPORT (AA64WritePmcr)
GCC_ASM_EXPORT (AA64WritePmccntr)
GCC_ASM_EXPORT (AA64WritePmccfiltr)
GCC_ASM_EXPORT (AA64WritePmcntenset)


//ASM_PFX(AA64ReadPmcr):
//  mrs   x0, pmcr_el0
//  ret

ASM_PFX(AA64ReadPmccntr):
  mrs   x0, pmccntr_el0
  ret

ASM_PFX(AA64ReadPmccfiltr):
  mrs   x0, pmccfiltr_el0
  ret

ASM_PFX(AA64ReadPmcntenset):
  mrs   x0, pmcntenset_el0
  ret

//ASM_PFX(AA64WritePmcr):
//  msr   pmcr_el0, x0
//  isb
//  ret

ASM_PFX(AA64WritePmccntr):
  msr   pmccntr_el0, x0
  isb
  ret

ASM_PFX(AA64WritePmccfiltr):
  msr   pmccfiltr_el0, x0
  isb
  ret

ASM_PFX(AA64WritePmcntenset):
  msr   pmcntenset_el0, x0
  isb
  ret

