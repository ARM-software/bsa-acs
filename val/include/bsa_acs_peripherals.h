/** @file
 * Copyright (c) 2016-2018, 2021 Arm Limited or its affiliates. All rights reserved.
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

#ifndef __BSA_ACS_PERIPHERALS_H__
#define __BSA_ACS_PERIPHERALS_H__



uint32_t os_d001_entry(uint32_t num_pe);
uint32_t os_d002_entry(uint32_t num_pe);
uint32_t os_d003_entry(uint32_t num_pe);
uint32_t os_d004_entry(uint32_t num_pe);
uint32_t os_d005_entry(uint32_t num_pe);


#define WIDTH_BIT8     0x1
#define WIDTH_BIT16    0x2
#define WIDTH_BIT32    0x4

#define IER                 0x1
#define LCR                 0x3
#define MCR                 0x4
#define MSR                 0x6
#define CTS_DCD_EN          0x90
#define LCR_DATA_BIT_MASK   0x3
#define LCR_DATA_BIT_SHIFT  0x0
#define LCR_STOP_BIT_MASK   0x4
#define LCR_STOP_BIT_SHIFT  0x2
#define MCR_LOOP            0x10

#define DIVISOR_LATCH_EN    0x80
#define DIVISOR_LATCH_DIS   0x0
#define DIVISOR_LATCH_BYTE1 0x0
#define DIVISOR_LATCH_BYTE2 0x1

#define BAUDRATE_1200       1200
#define BAUDRATE_9600       9200
#define BAUDRATE_115200     115200

#define BSA_UARTDR    0x0
#define BSA_UARTRSR   0x4
#define BSA_UARTFR    0x18
#define BSA_UARTLCR_H 0x2C
#define BSA_UARTCR    0x30
#define BSA_UARTIMSC  0x38
#define BSA_UARTRIS   0x3C
#define BSA_UARTMIS   0x40
#define BSA_UARTICR   0x44

#define COMPATIBLE_FULL_16550     0x0
#define COMPATIBLE_SUBSET_16550   0x1
#define ARM_PL011_UART            0x3
#define ARM_SBSA_GENERIC_UART     0xE
#define COMPATIBLE_GENERIC_16550  0x12

#define USB_TYPE_OHCI             0x1
#define USB_TYPE_EHCI             0x2
#define USB_TYPE_XHCI             0x3

#define SATA_TYPE_AHCI            0x1

#endif // __BSA_ACS_PERIPHERAL_H__
