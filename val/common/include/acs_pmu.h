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

#ifndef __ACS_PMU_H__
#define __ACS_PMU_H__

/* EL2 Cycle Count Filter Enable */
#define NSH_EN    27
/* Cycle Counter Enable */
#define CC_EN     31
/* Long Cycle Counter Enable */
#define LCC_EN    06
/* Cycle Count Reset */
#define CCR_SET   02
/* Global Cycle Counter enable */
#define GCC_EN    00

#define PMCCFILTR_NSH_EN_BIT    27
#define PMCNTENSET_C_EN_BIT     31
#define PMCR_LC_EN_BIT          6
#define PMCR_C_RESET_BIT        2
#define PMCR_EN_BIT             0

uint64_t AA64ReadPmccntr(void);
uint64_t AA64ReadPmccfiltr(void);
uint64_t AA64ReadPmcntenset(void);
void AA64WritePmccntr(uint64_t WriteData);
void AA64WritePmccfiltr(uint64_t WriteData);
void AA64WritePmcntenset(uint64_t WriteData);

#endif

