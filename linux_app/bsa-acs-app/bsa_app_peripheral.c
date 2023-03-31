/** @file
 * Copyright (c) 2021, 2023 Arm Limited or its affiliates. All rights reserved.
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


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "include/bsa_app.h"
#include "include/bsa_drv_intf.h"


extern unsigned int  *g_skip_test_num;
extern unsigned int  g_sw_view[3];

/**
  This function calls the BSA Kernel Module in a loop to execute all the PCIe tests
**/
int
execute_tests_peripheral(int num_pe, unsigned int print_level)
{

    int status;
    call_update_sw_view(BSA_UPDATE_SW_VIEW, g_sw_view);
    call_update_skip_list(BSA_UPDATE_SKIP_LIST, g_skip_test_num);
    call_drv_execute_test(BSA_PER_EXECUTE_TEST, num_pe, print_level, 0);
    status  = call_drv_wait_for_completion();
    return status;
}
