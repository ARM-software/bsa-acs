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

#include "common/include/acs_val.h"
#include "common/include/acs_pe.h"
#include "common/include/acs_common.h"
#include "common/sys_arch_src/gic/acs_exception.h"

#include "common/include/val_interface.h"
#include "bsa/include/bsa_val_interface.h"
#include "bsa/include/bsa_pal_interface.h"

/**
   Calls pal API to dump dtb

   @param none

   @return none

**/
void
val_dump_dtb(void)
{
  pal_dump_dtb();
}
