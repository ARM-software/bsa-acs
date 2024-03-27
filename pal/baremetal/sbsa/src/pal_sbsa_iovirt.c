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

#include "pal_common_support.h"
#include "pal_pcie_enum.h"
#include "platform_override_fvp.h"
#include "platform_override_sbsa_struct.h"

extern PLATFORM_OVERRIDE_CS_COMP_NODE_DATA platform_cs_comp_node_data;

/**
  @brief  Check the hid and copy the full path of hid

  @param  hid      hardware ID to get the path for
  @param  hid_path 2D array in which the path is copied

  @return 1 if test fails, 0 if test passes
**/
uint32_t
pal_get_device_path(const char *hid, char hid_path[][MAX_NAMED_COMP_LENGTH])
{
  uint32_t cmp;
  int32_t i;
  uint32_t status = 1;

  /* Iterate through components and add device name of the component to the array
     if hid of the component is matched */
  for (i = 0; i < CS_COMPONENT_COUNT; i++) {
      cmp = pal_strncmp(hid, platform_cs_comp_node_data.component[i].identifier, MAX_CS_COMP_LENGTH);
      if (!cmp) {
          status = 0;
          pal_strncpy(hid_path[i],
                  platform_cs_comp_node_data.component[i].dev_name, MAX_CS_COMP_LENGTH);
      }
  }

  if (status)
      return 1;  // return 1 if there's no entry in hid_path

  return 0;
}