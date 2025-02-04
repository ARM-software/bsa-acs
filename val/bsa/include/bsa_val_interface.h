/** @file
 * Copyright (c) 2016-2025, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __BSA_VAL_INTERFACE_H__
#define __BSA_VAL_INTERFACE_H__

#include "../../common/include/pal_interface.h"

/*VAL APIs */
void val_dump_dtb(void);
void view_print_info(uint32_t view);

/* VAL PE APIs */
uint32_t val_bsa_pe_execute_tests(uint32_t num_pe, uint32_t *g_sw_view);
int      val_suspend_pe(uint64_t entry, uint32_t context_id);

/* GIC VAL APIs */
uint32_t val_bsa_gic_execute_tests(uint32_t num_pe, uint32_t *g_sw_view);
uint32_t val_gic_get_interrupt_state(uint32_t int_id);
void val_gic_clear_interrupt(uint32_t int_id);
void val_gic_set_intr_trigger(uint32_t int_id, INTR_TRIGGER_INFO_TYPE_e trigger_type);

uint32_t val_gic_get_espi_intr_trigger_type(uint32_t int_id,
                                                          INTR_TRIGGER_INFO_TYPE_e *trigger_type);

/* GICv2m APIs */
typedef enum {
  V2M_MSI_FRAME_ID = 1,
  V2M_MSI_SPI_BASE,
  V2M_MSI_SPI_NUM,
  V2M_MSI_FRAME_BASE,
  V2M_MSI_FLAGS
} V2M_MSI_INFO_e;

uint32_t val_gic_v2m_parse_info(void);
uint64_t val_gic_v2m_get_info(V2M_MSI_INFO_e type, uint32_t instance);

/*TIMER VAL APIs */
#define BSA_TIMER_FLAG_ALWAYS_ON 0x4

uint32_t val_bsa_timer_execute_tests(uint32_t num_pe, uint32_t *g_sw_view);
void     val_timer_set_phy_el2(uint64_t timeout);
void     val_timer_set_vir_el2(uint64_t timeout);
void     val_timer_set_system_timer(addr_t cnt_base_n, uint32_t timeout);
void     val_timer_disable_system_timer(addr_t cnt_base_n);
uint32_t val_timer_skip_if_cntbase_access_not_allowed(uint64_t index);
uint64_t val_get_phy_el1_timer_count(void);

/* Watchdog VAL APIs */
uint32_t val_bsa_wd_execute_tests(uint32_t num_pe, uint32_t *g_sw_view);
uint32_t val_wd_set_ws0(uint32_t index, uint32_t timeout);
uint64_t val_get_counter_frequency(void);

/* PCIE VAL APIs */
uint32_t val_bsa_pcie_execute_tests(uint32_t num_pe, uint32_t *g_sw_view);
uint32_t val_pcie_is_devicedma_64bit(uint32_t bdf);
uint32_t val_pcie_device_driver_present(uint32_t bdf);
uint8_t val_pcie_parent_is_rootport(uint32_t dsf_bdf, uint32_t *rp_bdf);
void val_pcie_clear_device_status_error(uint32_t bdf);
uint32_t val_pcie_is_device_status_error(uint32_t bdf);
uint32_t val_pcie_is_sig_target_abort(uint32_t bdf);
void val_pcie_clear_sig_target_abort(uint32_t bdf);

/* IO-VIRT APIs */
uint32_t val_bsa_smmu_execute_tests(uint32_t num_pe, uint32_t *g_sw_view);

void     val_dma_device_get_dma_addr(uint32_t ctrl_index, uint64_t *dma_addr, uint32_t *cpu_len);
int      val_dma_mem_get_attrs(void *buf, uint32_t *attr, uint32_t *sh);

/* POWER and WAKEUP APIs */
typedef enum {
    BSA_POWER_SEM_B = 1,
    BSA_POWER_SEM_c,
    BSA_POWER_SEM_D,
    BSA_POWER_SEM_E,
    BSA_POWER_SEM_F,
    BSA_POWER_SEM_G,
    BSA_POWER_SEM_H,
    BSA_POWER_SEM_I
} BSA_POWER_SEM_e;

uint32_t val_power_enter_semantic(BSA_POWER_SEM_e semantic);
uint32_t val_bsa_wakeup_execute_tests(uint32_t num_pe, uint32_t *g_sw_view);

/* Peripheral Tests APIs */
void     val_peripheral_uart_setup(void);
uint32_t val_bsa_peripheral_execute_tests(uint32_t num_pe, uint32_t *g_sw_view);

/* Memory Tests APIs */
typedef enum {
  MEM_TYPE_DEVICE = 0x1000,
  MEM_TYPE_NORMAL,
  MEM_TYPE_RESERVED,
  MEM_TYPE_NOT_POPULATED,
  MEM_TYPE_LAST_ENTRY
} MEMORY_INFO_e;

#define MEM_ATTR_UNCACHED  0x2000
#define MEM_ATTR_CACHED    0x1000

/* Mem Map APIs */
void val_mmap_add_region(uint64_t va_base, uint64_t pa_base,
                uint64_t length, uint64_t attributes);
uint32_t val_setup_mmu(void);
uint32_t val_enable_mmu(void);

/* Identify memory type using MAIR attribute, refer to ARM ARM VMSA for details */

#define MEM_NORMAL_WB_IN_OUT(attr) (((attr & 0xcc) == 0xcc) || \
                                    (((attr & 0x7) >= 5) && (((attr >> 4) & 0x7) >= 5)))
#define MEM_NORMAL_NC_IN_OUT(attr) (attr == 0x44)
#define MEM_DEVICE(attr) ((attr & 0xf0) == 0)
#define MEM_SH_INNER(sh) (sh == 0x3)

uint32_t val_bsa_memory_execute_tests(uint32_t num_pe, uint32_t *g_sw_view);
uint64_t val_memory_get_unpopulated_addr(addr_t *addr, uint32_t instance);
uint64_t val_get_max_memory(void);

/* PCIe Exerciser tests */
uint32_t val_bsa_exerciser_execute_tests(uint32_t *g_sw_view);

#endif
