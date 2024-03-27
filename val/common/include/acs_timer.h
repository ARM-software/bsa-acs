
/** @file
 * Copyright (c) 2016-2018, 2021, 2024, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __ACS_TIMER_H__
#define __ACS_TIMER_H__

#define ARM_ARCH_TIMER_ENABLE           (1 << 0)
#define ARM_ARCH_TIMER_IMASK            (1 << 1)
#define ARM_ARCH_TIMER_ISTATUS          (1 << 2)

/* CNTCTLBase register offset */
#define CNTTIDR          0x8
#define CNTACR           0x40

/* CNTBaseN register offset*/
#define CNTPCT_LOWER     0x00
#define CNTPCT_HIGHER    0x04
#define CNTVCT_LOWER     0x08
#define CNTVCT_HIGHER    0x0C
#define CNTBaseN_CNTFRQ  0x10
#define CNTP_CVAL_LOWER  0x20
#define CNTP_CVAL_HIGHER 0x24
#define CNTP_TVAL        0x28
#define CNTP_CTL         0x2C
#define COUNTER_ID       0xFD0

#endif // __ACS_TIMER_H__
