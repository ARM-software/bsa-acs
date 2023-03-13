/** @file
 * Copyright (c) 2021, 2023 Arm Limited or its affiliates. All rights reserved.
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

#ifndef __PAL_DT_SPEC_H__
#define __PAL_DT_SPEC_H__

#define PPI_OFFSET 16
#define SPI_OFFSET 32

#define INTERRUPT_CELLS_MAX 4
#define INTERRUPT_CELLS_MIN 1

#define INTERRUPT_TYPE_SPI  0
#define INTERRUPT_TYPE_PPI  1

/*  PE related */
#define PROPERTY_MASK_PE_AFF3  0xFF  /* Affinity bits 3 mask MPIDR_EL1[39:32] */
#define PROPERTY_MASK_PE_AFF0_AFF2  0xFFFFFF  /* Affinity bits 0, 1, 2 mask MPIDR_EL1[23:0] */
#define PMU_COMPATIBLE_STR_LEN      24

/* PSCI related */
#define PSCI_COMPATIBLE_STR_LEN 15

/*  WD related */
/* Interrupt Flags for WD*/
#define INTERRUPT_IS_LEVEL_TRIGGERED 0x0
#define INTERRUPT_IS_EDGE_TRIGGERED  0x1
#define INTERRUPT_IS_ACTIVE_HIGH     0x0
#define INTERRUPT_IS_ACTIVE_LOW      0x1

#define WD_COMPATIBLE_STR_LEN        16

/* Timer related */
#define MEMTIMER_COMPATIBLE_STR_LEN  24
#define SYSTIMER_COMPATIBLE_STR_LEN  16

/* USB related */
#define USB_COMPATIBLE_STR_LEN         16

/* SATA related */
#define SATA_COMPATIBLE_STR_LEN        16

/* UART related */
#define UART_COMPATIBLE_STR_LEN        16


/* Interrupt Flags from DT*/
#define IRQ_TYPE_NONE             0x00000000         /* Default, unspecified type */
#define IRQ_TYPE_EDGE_RISING      0x00000001         /* Edge rising type */
#define IRQ_TYPE_EDGE_FALLING     0x00000002         /* Edge falling type */
#define IRQ_TYPE_LEVEL_HIGH       0x00000004         /* Level high type */
#define IRQ_TYPE_LEVEL_LOW        0x00000008         /* Level low type */

/* Interrupt Type from DT*/
#define GIC_SPI                   0
#define GIC_PPI                   1

#define GIC_COMPATIBLE_STR_LEN         24

/* SMMU related */
#define SMMU_COMPATIBLE_STR_LEN 16

/* PCI related */
#define PCI_COMPATIBLE_STR_LEN  26

/* Peripheral Related */
#define COMPATIBLE_FULL_16550     0x0
#define COMPATIBLE_SUBSET_16550   0x1
#define ARM_PL011_UART            0x3
#define ARM_SBSA_GENERIC_UART     0xE
#define COMPATIBLE_GENERIC_16550  0x12

#define USB_TYPE_OHCI             0x1
#define USB_TYPE_EHCI             0x2
#define USB_TYPE_XHCI             0x3

#define SATA_TYPE_AHCI            0x1

#endif
