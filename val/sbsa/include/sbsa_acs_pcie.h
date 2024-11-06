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

#ifndef __SBSA_ACS_PCIE_H__
#define __SBSA_ACS_PCIE_H__

#include "../../common/include/acs_pcie_spec.h"
#include "../../common/include/acs_pcie.h"

uint32_t
val_pcie_multifunction_support(uint32_t bdf);

uint32_t
val_pcie_get_rp_transaction_frwd_support(uint32_t bdf);

uint32_t
val_pcie_is_cache_present(uint32_t bdf);

uint32_t
val_pcie_link_cap_support(uint32_t bdf);

uint32_t
val_pcie_scan_bridge_devices_and_check_memtype(uint32_t bdf);

uint32_t
val_pcie_get_atomicop_requester_capable(uint32_t bdf);

uint32_t
p001_entry(uint32_t num_pe);

uint32_t
p002_entry(uint32_t num_pe);

uint32_t
p003_entry(uint32_t num_pe);

uint32_t
p005_entry(uint32_t num_pe);

uint32_t
p006_entry(uint32_t num_pe);

uint32_t
p007_entry(uint32_t num_pe);

uint32_t
p008_entry (uint32_t num_pe);

uint32_t
p009_entry (uint32_t num_pe);

uint32_t
p010_entry (uint32_t num_pe);

uint32_t
p011_entry (uint32_t num_pe);

uint32_t
p012_entry (uint32_t num_pe);

uint32_t
p013_entry (uint32_t num_pe);

uint32_t
p014_entry (uint32_t num_pe);

uint32_t
p015_entry (uint32_t num_pe);

uint32_t
p016_entry (uint32_t num_pe);

uint32_t
p017_entry (uint32_t num_pe);

uint32_t
p018_entry (uint32_t num_pe);

uint32_t
p019_entry (uint32_t num_pe);

uint32_t
p020_entry(uint32_t num_pe);

uint32_t
p021_entry(uint32_t num_pe);

uint32_t
p022_entry(uint32_t num_pe);

uint32_t
p023_entry(uint32_t num_pe);

uint32_t
p024_entry(uint32_t num_pe);

uint32_t
p025_entry(uint32_t num_pe);

uint32_t
p026_entry(uint32_t num_pe);

uint32_t
p027_entry(uint32_t num_pe);

uint32_t
p028_entry(uint32_t num_pe);

uint32_t
p029_entry(uint32_t num_pe);

uint32_t
p030_entry(uint32_t num_pe);

uint32_t
p031_entry(uint32_t num_pe);

uint32_t
p032_entry(uint32_t num_pe);

uint32_t
p033_entry(uint32_t num_pe);

uint32_t
p034_entry(uint32_t num_pe);

uint32_t
p035_entry(uint32_t num_pe);

uint32_t
p036_entry(uint32_t num_pe);

uint32_t
p037_entry(uint32_t num_pe);

uint32_t
p038_entry(uint32_t num_pe);

uint32_t
p039_entry(uint32_t num_pe);

uint32_t
p040_entry(uint32_t num_pe);

uint32_t
p041_entry(uint32_t num_pe);

uint32_t
p042_entry(uint32_t num_pe);

uint32_t
p043_entry(uint32_t num_pe);

uint32_t
p044_entry(uint32_t num_pe);

uint32_t
p045_entry(uint32_t num_pe);

uint32_t
p046_entry(uint32_t num_pe);

uint32_t
p047_entry(uint32_t num_pe);

uint32_t
p048_entry(uint32_t num_pe);

uint32_t
p049_entry(uint32_t num_pe);

uint32_t
p050_entry(uint32_t num_pe);

uint32_t
p051_entry(uint32_t num_pe);

uint32_t
p052_entry(uint32_t num_pe);

uint32_t
p053_entry(uint32_t num_pe);

uint32_t
p054_entry(uint32_t num_pe);

uint32_t
p055_entry(uint32_t num_pe);

uint32_t
p056_entry(uint32_t num_pe);

uint32_t
p057_entry(uint32_t num_pe);

uint32_t
p058_entry(uint32_t num_pe);

uint32_t
p059_entry(uint32_t num_pe);

uint32_t
p060_entry(uint32_t num_pe);

uint32_t
p061_entry(uint32_t num_pe);

uint32_t
p062_entry(uint32_t num_pe);

uint32_t
p063_entry(uint32_t num_pe);

uint32_t
p064_entry(uint32_t num_pe);

uint32_t
p065_entry(uint32_t num_pe);

uint32_t
p066_entry(uint32_t num_pe);

uint32_t
p067_entry(uint32_t num_pe);

#endif
