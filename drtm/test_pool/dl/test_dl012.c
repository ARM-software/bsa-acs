/** @file
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
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
#include "val/common/include/acs_memory.h"
#include "val/drtm/include/drtm_val_interface.h"

#define TEST_NUM   (ACS_DRTM_DL_TEST_NUM_BASE  +  12)
#define TEST_RULE  "R45242"
#define TEST_DESC  "Check DLME IMG AUTH when requested    "


static
int64_t
check_event_spec_signature(TCG_EFI_SPECID_EVENT *event_spec)
{

  /* Check Event Signature */
  if (val_strncmp((char8_t *)event_spec->signature, "Spec ID Event03", EVENT_SPEC_ID_STR_LEN)) {
    val_print(ACS_PRINT_ERR, " Event Specification mismatch", 0);
    return ACS_STATUS_FAIL;
  }

  return ACS_STATUS_PASS;
}

static
void
payload(uint32_t num_pe)
{
  /* This test will parse Event Log and verify Event Log Header */
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  int64_t  status;
  uint64_t drtm_params_size = DRTM_SIZE_4K;
  int32_t  event_log_size;
  int32_t  event_head_size;
  int32_t  event2_size = 0;
  uint32_t digest_index;
  uint64_t drtm_feature;
  DRTM_PARAMETERS      *drtm_params;
  EVENT_DATA           *event_data;
  DRTM_DLME_DATA_HDR   *dlme_data_head;
  TCG_PCR_EVENT        *event_log_head;
  VENDOR_INFO          *vendor_info;
  TCG_PCR_EVENT2       *event;
  TCG_PCR_EVENT2       *event2;
  TPMT_HA              *digest;
  TCG_EFI_SPECID_EVENT *event_spec;
  uint32_t dlme_image_auth = ACS_STATUS_ERR;

  /* Verify if DLME img auth is supported. If not supported, the test is skipped */
  drtm_feature = val_drtm_get_feature(DRTM_DRTM_FEATURES_DLME_IMG_AUTH);
  if (drtm_feature != DRTM_DLME_IMG_FEAT_DLME_IMG_AUTH_SUPP) {
    val_print(ACS_PRINT_DEBUG,
              "\n       DRTM implementation does not support DLME Img Auth, skip check", 0);
    val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
    return;
  }

  /* Allocate Memory For DRTM Parameters 4KB Aligned */
  drtm_params = (DRTM_PARAMETERS *)((uint64_t)val_aligned_alloc(DRTM_SIZE_4K, drtm_params_size));
  if (!drtm_params) {
    val_print(ACS_PRINT_ERR, "\n    Failed to allocate memory for DRTM Params", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  status = val_drtm_init_drtm_params(drtm_params);
  if (status != ACS_STATUS_PASS) {
    val_print(ACS_PRINT_ERR, "\n       DRTM Init Params failed err=%d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
    goto free_drtm_params;
  }

  /* R312090 : Request DLME Auth and check DLME IMG AUTH in evnt log */
  drtm_params->launch_features = drtm_params->launch_features |
            (DRTM_LAUNCH_FEAT_REQ_DLME_IMG_AUTH << DRTM_LAUNCH_FEAT_SHIFT_DLME_IMG_AUTH);

  /* Invoke DRTM Dynamic Launch, This will return only in case of error */
  status = val_drtm_dynamic_launch(drtm_params);
  /* This will return only in fail*/
  if (status < DRTM_ACS_SUCCESS) {
    val_print(ACS_PRINT_ERR, "\n       DRTM Dynamic Launch failed err=%d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
    goto free_dlme_region;
  }

  /* Call DRTM Unprotect Memory */
  status = val_drtm_unprotect_memory();
  if (status < DRTM_ACS_SUCCESS) {
    val_print(ACS_PRINT_ERR, "\n       Unprotect Memory failed err=%d", status);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
    goto free_dlme_region;
  }

  dlme_data_head = (DRTM_DLME_DATA_HDR *)(drtm_params->dlme_data_offset +
                                        drtm_params->dlme_region_address);

  event_log_size = dlme_data_head->drtm_event_log_size;

  /* Get Event Log Header address from DLME Data */
  event_log_head = (TCG_PCR_EVENT *)((uint8_t *)dlme_data_head + dlme_data_head->size +
                       dlme_data_head->protected_regions_size + dlme_data_head->address_map_size);

  event_spec = (TCG_EFI_SPECID_EVENT *)(event_log_head + 1);

  status = check_event_spec_signature(event_spec);
  if (status != ACS_STATUS_PASS) {
    val_print(ACS_PRINT_ERR, "\n       Event Log signature check failed", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 6));
    goto free_dlme_region;
  }

  vendor_info = (VENDOR_INFO *)(&event_spec->digest_sizes[0] +
                (sizeof(TCG_EFI_SPECID_EVENT_ALGO_SIZE) * event_spec->number_of_algorithms));
  /* Get Event2 Address from Vendor info pointer */
  event = (TCG_PCR_EVENT2 *)((uint8_t *)vendor_info + vendor_info->vendor_info_size
                                             + sizeof(uint8_t));
  event_head_size = (uint8_t *)event - (uint8_t *)event_log_head;

  event_log_size = event_log_size - event_head_size;
  /* Loop untill Event Log size is grater than zero and print event data */
  while (event_log_size > 0) {
    /* Check for zeroed which indicates end of log */
    if ((event->pcr_index == 0) && (event->event_type == 0) && (event->digests.count == 0)) {
      break;
    }
    val_print(ACS_PRINT_DEBUG, "\n       EVENT2 : ", 0);
    val_print(ACS_PRINT_DEBUG, "\n         PCR Index       : %d",    event->pcr_index);
    val_print(ACS_PRINT_DEBUG, "\n         Event Type      : 0x%x",  event->event_type);
    val_print(ACS_PRINT_DEBUG, "\n         Digest Count    : 0x%x",  event->digests.count);

    if (event->event_type == DRTM_EVTYPE_ARM_DLME) {
      dlme_image_auth = ACS_STATUS_PASS;
      break;
    }

    /* Parse Digest values for all hash algorithms */
    digest = (TPMT_HA *)&event->digests.digests[0];
    for (digest_index = 0; digest_index < event->digests.count; digest_index++);

    /* Cornor case if no digest are present */
    if (digest_index == 0) {
      digest_index = 1;
    }

    event_data = (EVENT_DATA *)((uint8_t *)&digest[--digest_index].digest[SHA256_DIGEST_SIZE]);

    /* Gent next event address and continue the loop */
    event2 = (TCG_PCR_EVENT2 *)((uint8_t *)event_data + sizeof(event_data->event_size) +
                              (event_data->event_size * sizeof(uint8_t)));
    event2_size = (uint8_t *)event2 - (uint8_t *) event;
    event_log_size = event_log_size - event2_size;
    event = event2;
  }

  if (dlme_image_auth != ACS_STATUS_PASS) {
    val_print(ACS_PRINT_ERR, "\n       DLME IMG AUTH not found in Event Log", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 7));
    goto free_dlme_region;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));

free_dlme_region:
  val_memory_free_aligned((void *)drtm_params->dlme_region_address);
free_drtm_params:
  val_memory_free_aligned((void *)drtm_params);

  return;
}

uint32_t dl012_entry(uint32_t num_pe)
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
