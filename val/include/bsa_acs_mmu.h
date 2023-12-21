/** @file
 * Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __BSA_AVS_MMU_H__
#define __BSA_AVS_MMU_H__

extern void val_mair_write(uint64_t value, uint64_t el_num);
extern void val_tcr_write(uint64_t value, uint64_t el_num);
extern void val_ttbr0_write(uint64_t value, uint64_t el_num);
extern void val_sctlr_write(uint64_t value, uint64_t el_num);
extern uint64_t val_sctlr_read(uint64_t el_num);
extern uint64_t val_ttbr0_read(uint64_t el_num);
extern uint64_t val_read_current_el(void);

extern uint64_t tt_l0_base[];

#endif
