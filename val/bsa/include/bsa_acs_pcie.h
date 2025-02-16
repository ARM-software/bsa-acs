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

#ifndef __BSA_ACS_PCIE_H__
#define __BSA_ACS_PCIE_H__

uint32_t
val_pcie_is_device_behind_smmu(uint32_t bdf);

uint32_t
val_pcie_check_interrupt_status(uint32_t bdf);

uint32_t
val_pcie_get_max_pasid_width(uint32_t bdf, uint32_t *max_pasid_width);

uint32_t
val_is_transaction_pending_set(uint32_t bdf);

uint32_t os_p001_entry(uint32_t num_pe);
uint32_t os_p002_entry(uint32_t num_pe);
uint32_t os_p003_entry(uint32_t num_pe);
uint32_t os_p004_entry(uint32_t num_pe);
uint32_t os_p005_entry(uint32_t num_pe);
uint32_t os_p006_entry(uint32_t num_pe);
uint32_t os_p008_entry(uint32_t num_pe);
uint32_t os_p009_entry(uint32_t num_pe);
uint32_t os_p010_entry(uint32_t num_pe);
uint32_t os_p011_entry(uint32_t num_pe);
uint32_t os_p012_entry(uint32_t num_pe);
uint32_t os_p013_entry(uint32_t num_pe);
uint32_t os_p014_entry(uint32_t num_pe);
uint32_t os_p015_entry(uint32_t num_pe);
uint32_t os_p016_entry(uint32_t num_pe);
uint32_t os_p017_entry(uint32_t num_pe);
uint32_t os_p018_entry(uint32_t num_pe);
uint32_t os_p019_entry(uint32_t num_pe);
uint32_t os_p020_entry(uint32_t num_pe);
uint32_t os_p021_entry(uint32_t num_pe);
uint32_t os_p022_entry(uint32_t num_pe);
uint32_t os_p023_entry(uint32_t num_pe);
uint32_t os_p024_entry(uint32_t num_pe);
uint32_t os_p025_entry(uint32_t num_pe);
uint32_t os_p026_entry(uint32_t num_pe);
uint32_t os_p027_entry(uint32_t num_pe);
uint32_t os_p029_entry(uint32_t num_pe);
uint32_t os_p030_entry(uint32_t num_pe);
uint32_t os_p031_entry(uint32_t num_pe);
uint32_t os_p032_entry(uint32_t num_pe);
uint32_t os_p033_entry(uint32_t num_pe);
uint32_t os_p034_entry(uint32_t num_pe);
uint32_t os_p035_entry(uint32_t num_pe);
uint32_t os_p036_entry(uint32_t num_pe);
uint32_t os_p037_entry(uint32_t num_pe);
uint32_t os_p038_entry(uint32_t num_pe);
uint32_t os_p039_entry(uint32_t num_pe);
uint32_t os_p041_entry(uint32_t num_pe);
uint32_t os_p042_entry(uint32_t num_pe);

/* Linux test */
uint32_t os_p061_entry(uint32_t num_pe);
uint32_t os_p062_entry(uint32_t num_pe);
uint32_t os_p063_entry(uint32_t num_pe);
uint32_t os_p064_entry(uint32_t num_pe);

#endif
