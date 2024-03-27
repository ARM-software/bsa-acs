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

#ifndef __BSA_PAL_INTERFACE_H__
#define __BSA_PAL_INTERFACE_H__

void pal_dump_dtb(void);
uint32_t pal_target_is_dt(void);

uint32_t pal_gic_set_intr_trigger(uint32_t int_id, INTR_TRIGGER_INFO_TYPE_e trigger_type);

uint64_t pal_timer_get_counter_frequency(void);

uint32_t pal_pcie_device_driver_present(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn);
uint32_t pal_pcie_is_devicedma_64bit(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn);
uint32_t pal_pcie_is_device_behind_smmu(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn);

int pal_dma_mem_get_attrs(void *buf, uint32_t *attr, uint32_t *sh);
void pal_dma_scsi_get_dma_addr(void *port, void *dma_addr, uint32_t *dma_len);

uint64_t pal_memory_get_unpopulated_addr(uint64_t *addr, uint32_t instance);
void    *pal_mem_alloc_cacheable(uint32_t bdf, uint32_t size, void **pa);
void     pal_mem_free_cacheable(uint32_t bdf, unsigned int size, void *va, void *pa);

#endif
