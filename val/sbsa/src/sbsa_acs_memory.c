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

#include "common/include/val_interface.h"
#include "sbsa/include/sbsa_acs_memory.h"

extern MEMORY_INFO_TABLE  *g_memory_info_table;

#ifndef TARGET_LINUX

/**
  @brief  Checks for presence of persistent memory.

  @param  none

  @return 1 - persistent memory exists.
          0 - persistent memory does not exist.
**/
uint32_t
val_memory_check_for_persistent_mem(void)
{

  uint32_t index = 0;

  while (g_memory_info_table->info[index].type != MEMORY_TYPE_LAST_ENTRY) {
      if (g_memory_info_table->info[index].type == MEMORY_TYPE_PERSISTENT)
          return 1;
      index++;
  }
  return 0;
}

#endif
