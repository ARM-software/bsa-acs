/** @file
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __BSA_ACS_IOVIRT_H__
#define __BSA_ACS_IOVIRT_H__

int val_iovirt_get_its_info(
  uint32_t type, uint32_t group_index, uint32_t param, uint32_t *return_value);
uint32_t val_iovirt_check_unique_ctx_intid(uint32_t smmu_index);
#endif
