/** @file
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __SBSA_ACS_GIC_H__
#define __SBSA_ACS_GIC_H__

#include "common/include/acs_gic.h"
#include "common/sys_arch_src/gic/gic.h"

uint32_t
g001_entry(uint32_t num_pe);
uint32_t
g002_entry(uint32_t num_pe);
uint32_t
g003_entry(uint32_t num_pe);
uint32_t
g004_entry(uint32_t num_pe);
uint32_t
g005_entry(uint32_t num_pe);

#endif
