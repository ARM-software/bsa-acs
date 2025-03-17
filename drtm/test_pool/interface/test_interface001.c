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

#include "val/common/include/acs_val.h"
#include "val/drtm/include/drtm_val_interface.h"

#define TEST_NUM   (ACS_DRTM_INTERFACE_TEST_NUM_BASE  +  1)
#define TEST_RULE  ""
#define TEST_DESC  "Check DRTM Version                    "

static
void
payload(uint32_t num_pe)
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t major, minor;
  uint32_t status = g_drtm_features.version.status;

  if (status >> 31) {
    val_print(ACS_PRINT_ERR, "\n       DRTM get version failed err=%d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  major = DRTM_VERSION_GET_MAJOR(g_drtm_features.version.value);
  if (major != 1) {
    val_print(ACS_PRINT_ERR, "\n       Major Version not as expected, Current version =%d", major);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
    return;
  }

  minor = DRTM_VERSION_GET_MINOR(g_drtm_features.version.value);
  if (minor != 1) {
    val_print(ACS_PRINT_ERR, "\n       Minor Version not as expected, Current version =%d", minor);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
    return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
interface001_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
  /* execute payload, which will execute relevant functions on current and other PEs */
      payload(num_pe);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, ACS_END(TEST_NUM), NULL);

  return status;
}
