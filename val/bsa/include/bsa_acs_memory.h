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

#ifndef __BSA_ACS_MEMORY_H__
#define __BSA_ACS_MEMORY_H__

#include "bsa_val_interface.h"

addr_t val_memory_get_addr(MEMORY_INFO_e mem_type, uint32_t instance, uint64_t *attr);
void *val_memory_alloc_cacheable(uint32_t bdf, uint32_t size, void **pa);
void val_memory_free_cacheable(uint32_t bdf, uint32_t size, void *va, void *pa);

uint32_t os_m001_entry(uint32_t num_pe);
uint32_t os_m002_entry(uint32_t num_pe);
uint32_t os_m003_entry(uint32_t num_pe);
uint32_t os_m004_entry(uint32_t num_pe);

#endif
