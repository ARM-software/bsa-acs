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

#ifndef __BSA_ACS_PE_H__
#define __BSA_ACS_PE_H__


uint32_t os_c001_entry(uint32_t num_pe);
uint32_t os_c002_entry(uint32_t num_pe);
uint32_t os_c003_entry(uint32_t num_pe);
uint32_t os_c004_entry(uint32_t num_pe);
uint32_t os_c006_entry(uint32_t num_pe);
uint32_t os_c007_entry(uint32_t num_pe);
uint32_t os_c008_entry(uint32_t num_pe);
uint32_t os_c009_entry(uint32_t num_pe);
uint32_t os_c010_entry(uint32_t num_pe);
uint32_t os_c011_entry(uint32_t num_pe);
uint32_t os_c012_entry(uint32_t num_pe);
uint32_t os_c013_entry(uint32_t num_pe);
uint32_t os_c014_entry(uint32_t num_pe);
uint32_t os_c015_entry(uint32_t num_pe);
uint32_t os_c016_entry(uint32_t num_pe);

uint32_t hyp_c001_entry(uint32_t num_pe);
uint32_t hyp_c002_entry(uint32_t num_pe);
uint32_t hyp_c003_entry(uint32_t num_pe);
uint32_t hyp_c004_entry(uint32_t num_pe);
uint32_t hyp_c005_entry(uint32_t num_pe);

uint32_t ps_c001_entry(uint32_t num_pe);
#endif
