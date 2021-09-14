/** @file
 * Copyright (c) 2016-2018,2021 Arm Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef __BSA_ACS_TIMER_H__
#define __BSA_ACS_TIMER_H__

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


uint32_t os_t001_entry(uint32_t num_pe);
uint32_t os_t002_entry(uint32_t num_pe);
uint32_t os_t003_entry(uint32_t num_pe);
uint32_t os_t004_entry(uint32_t num_pe);
uint32_t os_t005_entry(uint32_t num_pe);
#endif // __BSA_ACS_TIMER_H__
