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

#ifndef __SBSA_ACS_SMMU_H__
#define __SBSA_ACS_SMMU_H__

#include "common/include/acs_smmu.h"

/* PMCG CNTBaseN register offset*/
#define SMMU_PMCG_CFGR 0xE00

uint32_t
i001_entry(uint32_t num_pe);
uint32_t
i002_entry(uint32_t num_pe);
uint32_t
i003_entry(uint32_t num_pe);
uint32_t
i004_entry(uint32_t num_pe);
uint32_t
i005_entry(uint32_t num_pe);
uint32_t
i006_entry(uint32_t num_pe);
uint32_t
i007_entry(uint32_t num_pe);
uint32_t
i008_entry(uint32_t num_pe);
uint32_t
i009_entry(uint32_t num_pe);
uint32_t
i010_entry(uint32_t num_pe);
uint32_t
i011_entry(uint32_t num_pe);
uint32_t
i012_entry(uint32_t num_pe);
uint32_t
i013_entry(uint32_t num_pe);
uint32_t
i014_entry(uint32_t num_pe);
uint32_t
i015_entry(uint32_t num_pe);
uint32_t
i016_entry(uint32_t num_pe);

#endif
