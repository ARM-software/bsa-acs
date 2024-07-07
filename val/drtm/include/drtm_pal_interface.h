/** @file
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
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

#ifndef __PAL_INTERFACE_H
#define __PAL_INTERFACE_H

long pal_invoke_drtm_fn(unsigned long function_id, unsigned long arg1,
            unsigned long arg2, unsigned long arg3,
            unsigned long arg4, unsigned long arg5,
            unsigned long *ret1, unsigned long *ret2,
            unsigned long *ret3);

typedef struct drtm_log_control {
    int print_level;
    void *log_file_handle;
} drtm_log_control;

int32_t pal_invoke_psci_fn(uint64_t function_id, uint64_t arg0,
                                    uint64_t arg1, uint64_t arg2);
#endif
