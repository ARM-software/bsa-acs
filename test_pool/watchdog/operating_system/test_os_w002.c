/** @file
 * Copyright (c) 2016-2018,2021, 2023 Arm Limited or its affiliates. All rights reserved.
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


#include "val/include/bsa_acs_val.h"
#include "val/include/val_interface.h"

#include "val/include/bsa_acs_wd.h"

#define TEST_NUM   (ACS_WD_TEST_NUM_BASE + 2)
#define TEST_RULE  "B_WD_03, S_L3WD_01"
#define TEST_DESC  "Check Watchdog WS0 interrupt          "

static uint32_t int_id;
static uint64_t wd_num;

static
void
isr()
{
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
    val_wd_set_ws0(wd_num, 0);
    val_print(ACS_PRINT_DEBUG, "\n       Received WS0 interrupt                ", 0);
    val_set_status(index, RESULT_PASS(TEST_NUM, 1));
    val_gic_end_of_interrupt(int_id);
}


static
void
payload()
{

    uint32_t status, timeout, ns_wdg = 0;
    uint64_t timer_expire_ticks = 1;
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

    do {
        wd_num--; /*array index starts from 0, so subtract 1 from count*/

        if (val_wd_get_info(wd_num, WD_INFO_ISSECURE))
            continue;    /*Skip Secure watchdog*/

        ns_wdg++;
        timeout = val_get_counter_frequency() * 2;
        val_set_status(index, RESULT_PENDING(TEST_NUM));     /* Set the initial result to pending*/

        int_id       = val_wd_get_info(wd_num, WD_INFO_GSIV);
        val_print(ACS_PRINT_DEBUG, "\n       WS0 Interrupt id  %d        ", int_id);

        if (val_gic_install_isr(int_id, isr)) {
            val_print(ACS_PRINT_ERR, "\n       GIC Install Handler Failed...", 0);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
            return;
        }

        /* Set Interrupt Type Edge/Level Trigger */
        if (val_wd_get_info(wd_num, WD_INFO_IS_EDGE))
            val_gic_set_intr_trigger(int_id, INTR_TRIGGER_INFO_EDGE_RISING);
        else
            val_gic_set_intr_trigger(int_id, INTR_TRIGGER_INFO_LEVEL_HIGH);

        status = val_wd_set_ws0(wd_num, timer_expire_ticks);
        if (status) {
            val_print(ACS_PRINT_ERR, "\n       Setting watchdog timeout failed", 0);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
            return;
        }

        while ((--timeout > 0) && (IS_RESULT_PENDING(val_get_status(index))));

        if (timeout == 0) {
            val_print(ACS_PRINT_ERR, "\n       WS0 Interrupt not received on %d   ", int_id);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
            return;
        }

    } while (wd_num);

    if (!ns_wdg) {
        if (g_build_sbsa) {
            val_print(ACS_PRINT_ERR, "\n       No non-secure Watchdogs reported", 0);
            val_set_status(index, RESULT_FAIL(TEST_NUM, 5));
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

    val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);
    return status;

}
