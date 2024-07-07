/** @file
 * Copyright (c) 2016-2018, 2021, 2023-2024, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __ACS_GIC_H__
#define __ACS_GIC_H__

#define GICD_CTLR           0x0000
#define GICD_TYPER          0x0004
#define GICD_ISENABLER      0x100
#define GICD_ICENABLER      0x180
#define GICD_ISPENDR        0x200
#define GICD_ISACTIVER0     0x300
#define GICD_ICPENDR0       0x280
#define GICD_ICACTIVER0     0x380
#define GICD_ICFGR          0xC00
#define GICD_IROUTER        0x6000
#define GICD_PIDR2          0xFFE8

#define GICD_ICENABLERE     0x1400
#define GICD_ICPENDRE0      0x1800
#define GICD_ICACTIVERE0    0x1C00
#define GICD_IPRIORITYRE    0x2000
#define GICD_ICFGRE         0x3000
#define GICD_IROUTERnE      0x8000

#define GICR_ISENABLER      0x100

#define RD_FRAME_SIZE       0x10000

#define GITS_TRANSLATER     0x10040

#define GICD_ICFGR_INTR_STRIDE          16 /* (32/2) Interrupt per Register */
 /* Bit Config[2x+1] for config type level/edge */
#define GICD_ICFGR_INTR_CONFIG1(intid)  ((1+int_id*2) % 32)

uint32_t
val_get_max_intid(void);

addr_t
val_get_gicd_base(void);

addr_t
val_gic_get_pe_rdbase(uint64_t mpidr);

addr_t
val_get_gicr_base(uint32_t *rdbase_len, uint32_t gicr_rd_index);


addr_t
val_get_gich_base(void);

addr_t
val_get_cpuif_base(void);

uint32_t
val_gic_espi_supported(void);

uint32_t
val_gic_max_espi_val(void);

uint32_t
val_gic_max_eppi_val(void);

uint32_t
val_gic_is_valid_espi(uint32_t int_id);

uint32_t
val_gic_is_valid_eppi(uint32_t int_id);

uint32_t val_gic_is_valid_ppi(uint32_t int_id);
#endif
