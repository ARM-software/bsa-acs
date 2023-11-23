/** @file
 * Copyright (c) 2016-2019, 2021-2023, Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/bsa_acs_pe.h"
#include "val/include/val_interface.h"

#include "val/include/bsa_acs_wakeup.h"

#define TEST_NUM1  (ACS_WAKEUP_TEST_NUM_BASE + 1)
#define TEST_RULE1 "B_WAK_01, B_WAK_02, B_WAK_03, B_WAK_04, B_WAK_05 \
                    \n       B_WAK_06, B_WAK_07, B_WAK_10, B_WAK_11"
#define TEST_DESC1 "Wake from EL1 PHY Timer Int           "
#define TEST_NUM2  (ACS_WAKEUP_TEST_NUM_BASE + 2)
#define TEST_RULE2 "B_WAK_01, B_WAK_02, B_WAK_03, B_WAK_04, B_WAK_05 \
                    \n       B_WAK_06, B_WAK_07, B_WAK_10, B_WAK_11"
#define TEST_DESC2 "Wake from EL1 VIR Timer Int           "
#define TEST_NUM3  (ACS_WAKEUP_TEST_NUM_BASE + 3)
#define TEST_RULE3 "B_WAK_01, B_WAK_02, B_WAK_03, B_WAK_04, B_WAK_05 \
                    \n       B_WAK_06, B_WAK_07, B_WAK_10, B_WAK_11"
#define TEST_DESC3 "Wake from EL2 PHY Timer Int           "
#define TEST_NUM4  (ACS_WAKEUP_TEST_NUM_BASE + 4)
#define TEST_RULE4 "B_WAK_01, B_WAK_02, B_WAK_03, B_WAK_04, B_WAK_05 \
                    \n       B_WAK_06, B_WAK_07, B_WAK_10, B_WAK_11"
#define TEST_DESC4 "Wake from Watchdog WS0 Int            "
#define TEST_NUM5  (ACS_WAKEUP_TEST_NUM_BASE + 5)
#define TEST_RULE5 "B_WAK_01, B_WAK_02, B_WAK_03, B_WAK_04, B_WAK_05 \
                    \n       B_WAK_06, B_WAK_07, B_WAK_10, B_WAK_11"
#define TEST_DESC5 "Wake from System Timer Int            "

static uint32_t intid;
static uint32_t failsafe_test_num;
uint64_t timer_num;
static uint32_t g_wd_int_received;
static uint32_t g_failsafe_int_received;
extern uint32_t g_wakeup_timeout;

static
void
isr_failsafe()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  val_timer_set_phy_el1(0);
  val_print(ACS_PRINT_ERR, "       Received Failsafe interrupt\n", 0);
  g_failsafe_int_received = 1;
  val_set_status(index, RESULT_FAIL(failsafe_test_num, 1));
  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  val_gic_end_of_interrupt(intid);
}

static
void
isr1()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  val_timer_set_phy_el1(0);
  val_print(ACS_PRINT_INFO, "       Received EL1 PHY interrupt\n", 0);
  val_set_status(index, RESULT_PASS(TEST_NUM1, 1));
  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  val_gic_end_of_interrupt(intid);
}

static
void
isr2()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  /* We received our interrupt, so disable timer from generating further interrupts */
  val_timer_set_vir_el1(0);
  val_print(ACS_PRINT_INFO, "       Received EL1 VIRT interrupt\n", 0);
  val_set_status(index, RESULT_PASS(TEST_NUM2, 1));
  intid = val_timer_get_info(TIMER_INFO_VIR_EL1_INTID, 0);
  val_gic_end_of_interrupt(intid);
}

static
void
isr3()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  /* We received our interrupt, so disable timer from generating further interrupts */
  val_timer_set_phy_el2(0);
  val_print(ACS_PRINT_INFO, "       Received EL2 Physical interrupt\n", 0);
  val_set_status(index, RESULT_PASS(TEST_NUM3, 1));
  intid = val_timer_get_info(TIMER_INFO_PHY_EL2_INTID, 0);
  val_gic_end_of_interrupt(intid);
}

static
void
isr4()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  val_wd_set_ws0(timer_num, 0);
  val_print(ACS_PRINT_INFO, "       Received WS0 interrupt\n", 0);
  g_wd_int_received = 1;
  val_set_status(index, RESULT_PASS(TEST_NUM4, 1));
  intid = val_wd_get_info(timer_num, WD_INFO_GSIV);
  val_gic_end_of_interrupt(intid);
}

static
void
isr5()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint64_t cnt_base_n = val_timer_get_info(TIMER_INFO_SYS_CNT_BASE_N, timer_num);
  val_timer_disable_system_timer((addr_t)cnt_base_n);
  val_print(ACS_PRINT_INFO, "       Received Sys timer interrupt\n", 0);
  val_set_status(index, RESULT_PASS(TEST_NUM5, 1));
  intid = val_timer_get_info(TIMER_INFO_SYS_INTID, timer_num);
  val_gic_end_of_interrupt(intid);
}

void
wakeup_set_failsafe()
{
  uint64_t timer_expire_val = val_get_counter_frequency() * (g_wakeup_timeout + 1);

  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  val_gic_install_isr(intid, isr_failsafe);
  val_timer_set_phy_el1(timer_expire_val);
}

void
wakeup_clear_failsafe()
{
  val_timer_set_phy_el1(0);

}

static
void
payload1()
{
  uint64_t timer_expire_val = val_get_counter_frequency() * g_wakeup_timeout;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  val_set_status(index, RESULT_FAIL(TEST_NUM1, 1));

  intid = val_timer_get_info(TIMER_INFO_PHY_EL1_INTID, 0);
  if (val_gic_install_isr(intid, isr1)) {
    val_print(ACS_PRINT_WARN, "\n       GIC Install Handler Failed...", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM1, 2));
    return;
  }
  val_timer_set_phy_el1(timer_expire_val);
  val_power_enter_semantic(BSA_POWER_SEM_B);
  return;
}

static
void
payload2()
{
  uint64_t timer_expire_val = val_get_counter_frequency() * g_wakeup_timeout;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  val_set_status(index, RESULT_FAIL(TEST_NUM2, 1));
  intid = val_timer_get_info(TIMER_INFO_VIR_EL1_INTID, 0);
  if (val_gic_install_isr(intid, isr2)) {
    val_print(ACS_PRINT_WARN, "\n       GIC Install Handler Failed...", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM2, 2));
    return;
  }
  failsafe_test_num = TEST_NUM2;
  wakeup_set_failsafe();
  val_timer_set_vir_el1(timer_expire_val);
  val_power_enter_semantic(BSA_POWER_SEM_B);
  wakeup_clear_failsafe();

  return;
}

static
void
payload3()
{
  uint64_t timer_expire_val = val_get_counter_frequency() * g_wakeup_timeout;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  val_set_status(index, RESULT_FAIL(TEST_NUM3, 1));
  intid = val_timer_get_info(TIMER_INFO_PHY_EL2_INTID, 0);
  if (val_gic_install_isr(intid, isr3)) {
    val_print(ACS_PRINT_WARN, "\n       GIC Install Handler Failed...", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM3, 2));
    return;
  }
  failsafe_test_num = TEST_NUM3;
  wakeup_set_failsafe();
  val_timer_set_phy_el2(timer_expire_val);
  val_power_enter_semantic(BSA_POWER_SEM_B);
  wakeup_clear_failsafe();

  return;
}

static
void
payload4()
{
  uint32_t status, ns_wdg = 0;
  uint64_t timer_expire_val = 1 * g_wakeup_timeout;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  timer_num = val_wd_get_info(0, WD_INFO_COUNT);
  if(!timer_num){
      val_print(ACS_PRINT_DEBUG, "\n       No watchdog implemented      ", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM4, 1));
      return;
  }

  while(timer_num--) {
      if(val_wd_get_info(timer_num, WD_INFO_ISSECURE))
          continue;

      ns_wdg++;
      intid = val_wd_get_info(timer_num, WD_INFO_GSIV);
      status = val_gic_install_isr(intid, isr4);

      /* Set Interrupt Type Edge/Level Trigger */
      if (val_wd_get_info(timer_num, WD_INFO_IS_EDGE))
          val_gic_set_intr_trigger(intid, INTR_TRIGGER_INFO_EDGE_RISING);
      else
          val_gic_set_intr_trigger(intid, INTR_TRIGGER_INFO_LEVEL_HIGH);

      if (status == 0) {
          failsafe_test_num = TEST_NUM4;
          wakeup_set_failsafe();
          status = val_wd_set_ws0(timer_num, timer_expire_val);
          if (status) {
              val_print(ACS_PRINT_ERR, "\n       Setting watchdog timeout failed", 0);
              val_set_status(index, RESULT_FAIL(TEST_NUM4, 2));
              return;
          }
          g_wd_int_received = 0;
          g_failsafe_int_received = 0;

          val_power_enter_semantic(BSA_POWER_SEM_B);
          wakeup_clear_failsafe();
          /* If PE wakeup is due to some interrupt other than WD
             or failsafe, test will be consider as PASS(as BSA WAK_10 rule
             Semantic B is satisfied)
             Test will be consider as failure in case WD interrupt
             failed to fire.
          */
          if (!(g_wd_int_received || g_failsafe_int_received)) {
            val_gic_clear_interrupt(intid);
            val_set_status(index, RESULT_PASS(TEST_NUM4, 1));
          }
      } else {
          val_print(ACS_PRINT_WARN, "\n       GIC Install Handler Failed...", 0);
          val_set_status(index, RESULT_FAIL(TEST_NUM4, 1));
      }
  }

  if(!ns_wdg){
      val_print(ACS_PRINT_DEBUG, "       No non-secure watchdog implemented\n", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM4, 2));
      return;
  }

}

static
void
payload5()
{
  uint64_t cnt_base_n;
  uint64_t timer_expire_val = val_get_counter_frequency() * g_wakeup_timeout;
  uint32_t status, ns_timer = 0;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  timer_num = val_timer_get_info(TIMER_INFO_NUM_PLATFORM_TIMERS, 0);
  if(!timer_num){
      val_print(ACS_PRINT_DEBUG, "\n       No system timers implemented", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM5, 1));
      return;
  }

  while(timer_num--) {

      if(val_timer_get_info(TIMER_INFO_IS_PLATFORM_TIMER_SECURE, timer_num))
          continue;

      ns_timer++;

      //Read CNTACR to determine whether access permission from NS state is permitted
      status = val_timer_skip_if_cntbase_access_not_allowed(timer_num);
      if(status == ACS_STATUS_SKIP){
          val_print(ACS_PRINT_DEBUG, "       Timer cntbase can't accessed\n", 0);
          val_set_status(index, RESULT_SKIP(TEST_NUM5, 3));
          return;
      }

      cnt_base_n = val_timer_get_info(TIMER_INFO_SYS_CNT_BASE_N, timer_num);
      if(cnt_base_n == 0){
          val_print(ACS_PRINT_DEBUG, "       Timer cntbase is invalid\n", 0);
          val_set_status(index, RESULT_SKIP(TEST_NUM5, 4));
          return;
      }

      intid = val_timer_get_info(TIMER_INFO_SYS_INTID, timer_num);
      status = val_gic_install_isr(intid, isr5);

      if(status == 0) {
          failsafe_test_num = TEST_NUM5;
          wakeup_set_failsafe();
          /* enable System timer */
          val_timer_set_system_timer((addr_t)cnt_base_n, timer_expire_val);
          val_power_enter_semantic(BSA_POWER_SEM_B);
          wakeup_clear_failsafe();
      } else{
          val_print(ACS_PRINT_WARN, "\n       GIC Install Handler Failed...", 0);
          val_set_status(index, RESULT_FAIL(TEST_NUM5, 1));
          return;
      }
  }

  if(!ns_timer){
      val_print(ACS_PRINT_WARN, "       No non-secure systimer implemented\n", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM5, 3));
      return;
  }
}

uint32_t
os_u001_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL, status_test = ACS_STATUS_FAIL;

  num_pe = 1;  //This Timer test is run on single processor

  if (!g_el1physkip) {
      /* EL1 PHY */
      status_test = val_initialize_test(TEST_NUM1, TEST_DESC1, num_pe);
      if (status_test != ACS_STATUS_SKIP)
          val_run_test_payload(TEST_NUM1, num_pe, payload1, 0);
      status |= val_check_for_error(TEST_NUM1, num_pe, TEST_RULE1);
      val_report_status(0, BSA_ACS_END(TEST_NUM1), NULL);

      /* EL1 VIR */
      status_test = val_initialize_test(TEST_NUM2, TEST_DESC2, num_pe);
      if (status_test != ACS_STATUS_SKIP)
          val_run_test_payload(TEST_NUM2, num_pe, payload2, 0);
      status |= val_check_for_error(TEST_NUM2, num_pe, TEST_RULE2);
      val_report_status(0, BSA_ACS_END(TEST_NUM2), NULL);
  }

  /* EL2 PHY */
  /* Run this test if current exception level is EL2 */
  if (val_pe_reg_read(CurrentEL) == AARCH64_EL2) {
      status_test = val_initialize_test(TEST_NUM3, TEST_DESC3, num_pe);
      if (status_test != ACS_STATUS_SKIP)
          val_run_test_payload(TEST_NUM3, num_pe, payload3, 0);
      status |= val_check_for_error(TEST_NUM3, num_pe, TEST_RULE3);
      val_report_status(0, BSA_ACS_END(TEST_NUM3), NULL);
  }

  /* Watchdog */
  status_test = val_initialize_test(TEST_NUM4, TEST_DESC4, num_pe);
  if (status_test != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM4, num_pe, payload4, 0);
  status = val_check_for_error(TEST_NUM4, num_pe, TEST_RULE4);
  val_report_status(0, BSA_ACS_END(TEST_NUM4), NULL);

  /* System Timer */
  status_test = val_initialize_test(TEST_NUM5, TEST_DESC5, num_pe);
  if (status_test != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM5, num_pe, payload5, 0);
  status |= val_check_for_error(TEST_NUM5, num_pe, TEST_RULE5);
  val_report_status(0, BSA_ACS_END(TEST_NUM5), NULL);

  return status;
}
