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

#include "platform_override_struct.h"
#include "pal_common_support.h"

/**
  @brief  This API gets the counter frequency value from user

  @param  None

  @return Counter frequency value
**/
uint64_t
pal_timer_get_counter_frequency(void)
{
  return PLATFORM_BM_TIMER_CNTFRQ;
}

