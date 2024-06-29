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

#ifndef __BSA_ACS_GIC_H__
#define __BSA_ACS_GIC_H__

/* GICv2m MSI Frame */
#define GICv2m_MSI_TYPER           0x008
#define GICv2m_MSI_SETSPI          0x040
#define GICv2m_MSI_IIDR            0xFCC

/**
  @brief  structure instance for MSI Frame Entry
**/
typedef struct {
  uint64_t base;
  uint32_t entry_id;
  uint32_t flags;
  uint32_t spi_count;
  uint32_t spi_base;
} MSI_FRAME_ENTRY;

typedef struct {
  uint32_t          num_msi_frame;
  MSI_FRAME_ENTRY   msi_info[];
} GICv2m_MSI_FRAME_INFO;


uint32_t
os_g001_entry(uint32_t num_pe);
uint32_t
os_g002_entry(uint32_t num_pe);
uint32_t
os_g003_entry(uint32_t num_pe);
uint32_t
os_g004_entry(uint32_t num_pe);
uint32_t
os_g005_entry(uint32_t num_pe);
uint32_t
os_g006_entry(uint32_t num_pe);
uint32_t
os_g007_entry(uint32_t num_pe);
uint32_t
hyp_g001_entry(uint32_t num_pe);
uint32_t
hyp_g002_entry(uint32_t num_pe);
uint32_t
hyp_g003_entry(uint32_t num_pe);


uint32_t os_v2m001_entry(uint32_t num_pe);
uint32_t os_v2m002_entry(uint32_t num_pe);
uint32_t os_v2m003_entry(uint32_t num_pe);
uint32_t os_v2m004_entry(uint32_t num_pe);

/* ITS tests */
uint32_t os_its001_entry(uint32_t num_pe);
uint32_t os_its002_entry(uint32_t num_pe);
uint32_t os_its003_entry(uint32_t num_pe);
uint32_t os_its004_entry(uint32_t num_pe);

#endif
