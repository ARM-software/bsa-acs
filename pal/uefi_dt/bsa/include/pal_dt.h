/** @file
 * Copyright (c) 2021,2024-2025, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __PAL_DT_H__
#define __PAL_DT_H__

extern VOID *g_dtb_log_file_handle;

UINT64
pal_get_dt_ptr();

VOID
pal_pe_create_info_table_dt(PE_INFO_TABLE *PeTable);

VOID
pal_wd_create_info_table_dt(WD_INFO_TABLE *WdTable);

VOID
pal_timer_create_info_table_dt(TIMER_INFO_TABLE *TimerTable);

VOID
pal_gic_create_info_table_dt(GIC_INFO_TABLE *GicTable);

VOID
pal_pcie_create_info_table_dt(PCIE_INFO_TABLE *PcieTable);

VOID
pal_iovirt_create_info_table_dt(IOVIRT_INFO_TABLE *IoVirtTable);

VOID
pal_peripheral_usb_create_info_table_dt(PERIPHERAL_INFO_TABLE *peripheralInfoTable);

VOID
pal_peripheral_sata_create_info_table_dt(PERIPHERAL_INFO_TABLE *peripheralInfoTable);

VOID
pal_peripheral_uart_create_info_table_dt(PERIPHERAL_INFO_TABLE *peripheralInfoTable);

VOID
pal_smbios_create_info_table(PE_SMBIOS_PROCESSOR_INFO_TABLE *SmbiosTable);

int
fdt_node_offset_by_prop_name(const void *fdt, int startoffset, const char *p_name, int p_len);

int
fdt_frame_number(const void *fdt, int nodeoffset);

int
fdt_interrupt_cells(const void *fdt, int nodeoffset);



/*-----------------DEBUG FUNCTION----------------*/

VOID
dt_dump_pe_table(PE_INFO_TABLE *PeTable);

VOID
dt_dump_gic_table(GIC_INFO_TABLE *GicTable);

VOID
dt_dump_pcie_table(PCIE_INFO_TABLE *PcieTable);

VOID
dt_dump_timer_table(TIMER_INFO_TABLE *TimerTable);

VOID
dt_dump_wd_table(WD_INFO_TABLE *WdTable);

VOID
dt_dump_timer_table(TIMER_INFO_TABLE *TimerTable);

VOID
dt_dump_memory_table(MEMORY_INFO_TABLE *MemoryInfoTable);

VOID
dt_dump_iovirt_table(IOVIRT_INFO_TABLE *IoVirtTable);

VOID
dt_dump_peripheral_table(PERIPHERAL_INFO_TABLE *peripheralInfoTable);

VOID
pal_pe_info_table_gmaint_gsiv_dt(PE_INFO_TABLE *PeTable);
#endif
