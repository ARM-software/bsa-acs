/** @file
 * Copyright (c) 2019-2021,2023 Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_pcie.h"

/**
* The test table covers bit-field entries for device capabilities 2 register
* belonging to capability id 10h (PCIe capability structure)
**/

pcie_cfgreg_bitfield_entry bf_info_table26[] = {

    // Bit-field entry 1: Device Capabilities Register 2, bit[5] ARI Forwarding Support
    {
       PCIE_CAP,                                // Part of PCIe capability register
       0x10,                                    // Capability id
       0,                                       // Not applicable
       0x24,                                    // Offset from capability id base
       EP,                                      // Applicable to Endpoints and RCEC
       5,                                       // Start bit position
       5,                                       // End bit position
       0,                                       // Hardwired to 0b
       READ_ONLY,                               // Attribute is Read-only
       "AFS value mismatch",                    // AFS invalid configured value
       "AFS attribute mismatch"                 // AFS invalid attribute
    },

    // Bit-field entry 2: Device Capabilities Register 2, bit[10] No RO-enabled PR-PR passing
    {
       PCIE_CAP,                                // Part of PCIe capability register
       0x10,                                    // Capability id
       0,                                       // Not applicable
       0x24,                                    // Offset from capability id base
       EP,                                      // Applicable to integrated endpoint pair
       10,                                      // Start bit position
       10,                                      // End bit position
       0,                                       // Hardwired to 0b
       HW_INIT,                                 // Attribute is HW INIT
       "NREPP value mismatch",                  // NREPP invalid configured value
       "NREPP attribute mismatch"               // NREPP invalid attribute
    },


    // Bit-field entry 3: Device Capabilities Register 2, bit[14:15] LN System CLS
    {
       PCIE_CAP,                                // Part of PCIe capability register
       0x10,                                    // Capability id
       0,                                       // Not applicable
       0x24,                                    // Offset from capability id base
       EP,                                      // Applicable to RCiEP, iEP_EP and EP
       14,                                      // Start bit position
       15,                                      // End bit position
       0,                                       // Hardwired to 0b
       HW_INIT,                                 // Attribute is HW INIT
       "LSC value mismatch",                    // LSC invalid configured value
       "LSC attribute mismatch"                 // LSC invalid attribute
    },
};
