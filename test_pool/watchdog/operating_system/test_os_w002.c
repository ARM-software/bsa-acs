/** @file
 * Copyright (c) 2016-2018,2021, 2023-2024, Arm Limited or its affiliates. All rights reserved.
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


#include "val/common/include/acs_val.h"
#include "val/bsa/include/bsa_val_interface.h"

#include "val/common/include/acs_gic.h"
#include "val/common/sys_arch_src/gic/v3/gic_v3.h"

#include "val/common/include/acs_wd.h"

#define TEST_NUM   (ACS_WD_TEST_NUM_BASE + 2)
#define TEST_RULE  "B_WD_03, S_L3WD_01"
#define TEST_DESC  "Check Watchdog WS0 interrupt          "

static uint32_t int_id;
static uint64_t wd_num;
static volatile uint32_t g_failsafe_int_received;
static volatile uint32_t g_wd_int_received;

extern uint32_t g_wakeup_timeout;

static
void
isr()
{
    val_wd_set_ws0(wd_num, 0);
    g_wd_int_received = 1;
    val_print(ACS_PRINT_DEBUG, "\n       Received WS0 interrupt                ", 0);
    val_gic_end_of_interrupt(int_id);
}

static
void
isr_failsafe()
{
  uint32_t intid;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  val_timer_set_phy_el1(0);
  val_print(ACS_PRINT_ERR, "       Received Failsafe interrupt\n", 0);
  g_failsafe_int_received = 1;
  val_set_status(index, RESULT_FAIL(TEST_NUM, 7));
  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  val_gic_end_of_interrupt(intid);
}

static
void
wakeup_set_failsafe()
{
  uint32_t intid;
  uint64_t timer_expire_val = (val_get_counter_frequency() * 3 * g_wakeup_timeout) / 2;

  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  val_gic_install_isr(intid, isr_failsafe);
  val_timer_set_phy_el1(timer_expire_val);
}

static
void
wakeup_clear_failsafe()
{
  val_timer_set_phy_el1(0);
}

static
void
payload()
{

    uint32_t status, timeout, ns_wdg = 0;
    uint64_t timer_expire_ticks = 1 * g_wakeup_timeout;
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
    wd_num = val_wd_get_info(0, WD_INFO_COUNT);

    if (wd_num == 0) {
        val_print(ACS_PRINT_DEBUG, "\n       No Watchdogs reported          %d  ", wd_num);
        if (g_build_sbsa)
            val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
        else
            val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
        return;
    }

    /* Assume pass until proven otherwise. */
    val_set_status(index, RESULT_PASS(TEST_NUM, 1));

    do {
        wd_num--; /*array index starts from 0, so subtract 1 from count*/

        if (val_wd_get_info(wd_num, WD_INFO_ISSECURE))
            continue;    /*Skip Secure watchdog*/

        ns_wdg++;
        timeout = val_get_counter_frequency() * 2;
        int_id       = val_wd_get_info(wd_num, WD_INFO_GSIV);
        val_print(ACS_PRINT_DEBUG, "\n       WS0 Interrupt id  %d        ", int_id);

        /* Check intid is SPI or ESPI */
        if (!(IsSpi(int_id)) && !(val_gic_is_valid_espi(int_id))) {
            val_print(ACS_PRINT_ERR, "\n       Interrupt-%d is neither SPI nor ESPI", int_id);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
            return;
        }

        /* Install ISR */
        if (val_gic_install_isr(int_id, isr)) {
            val_print(ACS_PRINT_ERR, "\n       GIC Install Handler Failed...", 0);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
            return;
        }

        /* Set Interrupt Type Edge/Level Trigger */
        if (val_wd_get_info(wd_num, WD_INFO_IS_EDGE))
            val_gic_set_intr_trigger(int_id, INTR_TRIGGER_INFO_EDGE_RISING);
        else
            val_gic_set_intr_trigger(int_id, INTR_TRIGGER_INFO_LEVEL_HIGH);

        wakeup_set_failsafe();
        g_wd_int_received = 0;
        status = val_wd_set_ws0(wd_num, timer_expire_ticks);
        if (status) {
            val_print(ACS_PRINT_ERR, "\n       Setting watchdog timeout failed", 0);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
            return;
        }

        timeout = val_get_counter_frequency() * 2;
        while (timeout && (g_wd_int_received == 0) && (g_failsafe_int_received == 0)) {
          timeout--;
        }
        wakeup_clear_failsafe();

        val_wd_set_ws0(wd_num, 0);

        if (g_failsafe_int_received) {
          val_set_status(index, RESULT_FAIL(TEST_NUM, 7));
          return;
        }

        if (timeout == 0) {
            val_print(ACS_PRINT_ERR, "\n       WS0 Interrupt not received on %d   ", int_id);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 5));
            return;
        }

    } while (wd_num);

    if (!ns_wdg) {
        if (g_build_sbsa) {
            val_print(ACS_PRINT_ERR, "\n       No non-secure Watchdogs reported", 0);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 6));
        } else {
            val_print(ACS_PRINT_WARN, "\n       No non-secure Watchdogs reported", 0);
            val_set_status(index, RESULT_SKIP(TEST_NUM, 3));
        }
        return;
    }

}

uint32_t
os_w002_entry(uint32_t num_pe)
{

    uint32_t status = ACS_STATUS_FAIL;

    num_pe = 1;  /*This Timer test is run on single processor*/

    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

    if (status != ACS_STATUS_SKIP)
        val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

    val_report_status(0, ACS_END(TEST_NUM), NULL);
    return status;

}
