/** @file
 * Copyright (c) 2024, 2025, Arm Limited or its affiliates. All rights reserved.
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

#define TEST_NUM   (ACS_DRTM_DL_TEST_NUM_BASE  +  6)
#define TEST_RULE  ""
#define TEST_DESC  "Check DRTM event log                  "

static
void
print_dlme_region(DRTM_PARAMETERS *drtm_params)
{
  val_print(ACS_PRINT_DEBUG, "\n       DLME Region : ", 0);
  val_print(ACS_PRINT_DEBUG, "\n         Alloc address      : 0x%lx",
                            drtm_params->dlme_region_address);
  val_print(ACS_PRINT_DEBUG, "\n         Region size        : 0x%lx",
                            drtm_params->dlme_region_size);
  val_print(ACS_PRINT_DEBUG, "\n         Free space 1 size  : 0x%lx",
                            drtm_params->dlme_image_start);
  val_print(ACS_PRINT_DEBUG, "\n         Image size         : 0x%lx",
                            drtm_params->dlme_image_size);
  val_print(ACS_PRINT_DEBUG, "\n         Data size          : 0x%lx\n",
                           (drtm_params->dlme_region_size - drtm_params->dlme_data_offset));
}

static
void
print_dlme_data_header(DRTM_DLME_DATA_HDR *dlme_data_header)
{
  val_print(ACS_PRINT_DEBUG, "\n       DLME Data Header :", 0);
  val_print(ACS_PRINT_DEBUG, "\n         Revision                   : 0x%08lx",
                                                dlme_data_header->revision);
  val_print(ACS_PRINT_DEBUG, "\n         Size                       : %d Bytes",
                                                dlme_data_header->size);
  val_print(ACS_PRINT_DEBUG, "\n         DLME_data_size             : %d Bytes",
                                                dlme_data_header->dlme_data_size);
  val_print(ACS_PRINT_DEBUG, "\n         Protected Regions Size     : %d Bytes",
                                                dlme_data_header->protected_regions_size);
  val_print(ACS_PRINT_DEBUG, "\n         Address Map Size           : %d Bytes",
                                                dlme_data_header->address_map_size);
  val_print(ACS_PRINT_DEBUG, "\n         DRTM Event Log Size        : %d Bytes",
                                                dlme_data_header->drtm_event_log_size);
  val_print(ACS_PRINT_DEBUG, "\n         TCB Hash Table Size        : %d Bytes",
                                                dlme_data_header->tcb_hash_table_size);
  val_print(ACS_PRINT_DEBUG, "\n         ACPI Table Region Size     : %d Bytes",
                                                dlme_data_header->acpi_table_region_size);
  val_print(ACS_PRINT_DEBUG, "\n        Implementation Region Size : %d Bytes\n",
                                                dlme_data_header->implementation_region_size);
}

static
int64_t
print_event_spec(TCG_EFI_SPECID_EVENT *event_spec)
{
  uint32_t index;
  TCG_EFI_SPECID_EVENT_ALGO_SIZE *digest_algo;
  VENDOR_INFO *vendor_info;
  uint8_t *vendor_data;

  val_print(ACS_PRINT_DEBUG, "\n       TCG Spec ID :", 0);
  val_print(ACS_PRINT_DEBUG, "\n         Signature            : %a",
                                         (uint64_t)event_spec->signature);
  val_print(ACS_PRINT_DEBUG, "\n         Platform Class       : 0x%x", event_spec->platform_class);
  val_print(ACS_PRINT_DEBUG, "\n         Spec Version Minor   : %d",
                                         event_spec->spec_version_minor);
  val_print(ACS_PRINT_DEBUG, "\n         Spec Version Major   : %d",
                                         event_spec->spec_version_major);
  val_print(ACS_PRINT_DEBUG, "\n         Spec Errata          : %d", event_spec->spec_errata);
  val_print(ACS_PRINT_DEBUG, "\n         Uintn Size           : %d", event_spec->uintn_size);
  val_print(ACS_PRINT_DEBUG, "\n         Number Of Algorithms : %d\n",
                              event_spec->number_of_algorithms);

  /* Check Event Signature */
  if (val_strncmp((char8_t *)event_spec->signature, "Spec ID Event03", EVENT_SPEC_ID_STR_LEN)) {
    val_print(ACS_PRINT_ERR, " Event Specification mismatch", 0);
    return ACS_STATUS_FAIL;
  }

  digest_algo = (TCG_EFI_SPECID_EVENT_ALGO_SIZE *)event_spec->digest_sizes;
  for (index = 0; index < event_spec->number_of_algorithms; index++) {
    val_print(ACS_PRINT_DEBUG, "\n         Digest(%d)",   index);
    val_print(ACS_PRINT_DEBUG, "\n           Algorithm Id       : %d",
                                           digest_algo[index].algorithm_id);
    val_print(ACS_PRINT_DEBUG, "\n           Digest Size        : %d\n",
                                      digest_algo[index].digest_size);
  }

  vendor_info = (VENDOR_INFO *)(&digest_algo[event_spec->number_of_algorithms]);

  val_print(ACS_PRINT_DEBUG, "\n         Vendor Info Size     : %d",
                                         vendor_info->vendor_info_size);

  vendor_data = (uint8_t *)&vendor_info->vendor_info[0];
  val_print(ACS_PRINT_DEBUG, "\n         Vendor Info          : ", 0);
  for (index = 0; index < vendor_info->vendor_info_size; index++) {
    val_print(ACS_PRINT_DEBUG, "%c",   vendor_data[index]);
  }
  val_print(ACS_PRINT_DEBUG, "\n", 0);

  return ACS_STATUS_PASS;
}

static
int64_t
print_tcg_event_header(TCG_PCR_EVENT *event_head)
{
  uint32_t index;

  val_print(ACS_PRINT_DEBUG, "\n       EVENT LOG HEADER:", 0);
  val_print(ACS_PRINT_DEBUG, "\n         PCR Index         : %d",    event_head->pcr_index);
  /* Check Event Log Header PCRIndex should be zero */
  if (event_head->pcr_index != 0) {
    val_print(ACS_PRINT_ERR, "\n       Event Log Header should have PCRIndex as zero", 0);
    return ACS_STATUS_FAIL;
  }

  val_print(ACS_PRINT_DEBUG, "\n         Event Type        : 0x%x",  event_head->event_type);
  /* Check Event Log Header EventType should be EV_NO_ACTION */
  if (event_head->event_type != EV_NO_ACTION) {
    val_print(ACS_PRINT_ERR, "\n       Event Log Header should have EventType as EV_NO_ACTION", 0);
    return ACS_STATUS_FAIL;
  }

  val_print(ACS_PRINT_DEBUG, "\n         Digest - ", 0);
  for (index = 0; index < sizeof(TCG_DIGEST); index++) {
    val_print(ACS_PRINT_DEBUG, "%02x ",  event_head->digest.digest[index]);
  }
  val_print(ACS_PRINT_DEBUG, "\n         Event Size        : 0x%x\n",  event_head->event_size);

  return ACS_STATUS_PASS;
}

static DRTM_PARAMETERS *drtm_params;

static
void
payload(uint32_t num_pe)
{
  /* This test will parse Event Log and verify Event Log Header */
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  int64_t  status;
  uint64_t drtm_params_size = DRTM_SIZE_4K;
  uint64_t dlme_image_addr;
  int32_t  event_log_size;
  int32_t  event_head_size;
  int32_t  event2_size = 0;
  uint32_t digest_index, i;
  uint8_t  *digest_buffer;
  uint8_t  *data;
  EVENT_DATA           *event_data;
  DRTM_DLME_DATA_HDR   *dlme_data_head;
  TCG_PCR_EVENT        *event_log_head;
  VENDOR_INFO          *vendor_info;
  TCG_PCR_EVENT2       *event;
  TCG_PCR_EVENT2       *event2;
  TPMT_HA              *digest;
  TCG_EFI_SPECID_EVENT *event_spec;

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

  /* DLME Image address is sum of region start and DLME image offset */
  dlme_image_addr = drtm_params->dlme_region_address + drtm_params->dlme_image_start;
  val_memcpy((void *)dlme_image_addr, (void *)g_drtm_acs_dlme, g_drtm_acs_dlme_size);

 /* Invoke DRTM Dynamic Launch, This will return only in case of error */
  status = val_drtm_dynamic_launch(drtm_params);
  /* This will return only in fail*/
  if (status < 0) {
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

  print_dlme_region(drtm_params);

  dlme_data_head = (DRTM_DLME_DATA_HDR *)(drtm_params->dlme_data_offset +
                                        drtm_params->dlme_region_address);

  print_dlme_data_header(dlme_data_head);
  event_log_size = dlme_data_head->drtm_event_log_size;

  /* Get Event Log Header address from DLME Data */
  event_log_head = (TCG_PCR_EVENT *)((uint8_t *)dlme_data_head + dlme_data_head->size +
                       dlme_data_head->protected_regions_size + dlme_data_head->address_map_size);

  status = print_tcg_event_header(event_log_head);
  if (status != ACS_STATUS_PASS) {
    val_print(ACS_PRINT_ERR, "\n       Event Log Header Checks failed", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 5));
    goto free_dlme_region;
  }

  event_spec = (TCG_EFI_SPECID_EVENT *)(event_log_head + 1);

  status = print_event_spec(event_spec);
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

    /* Print Digest values for all hash algorithms */
    digest = (TPMT_HA *)&event->digests.digests[0];
    for (digest_index = 0; digest_index < event->digests.count; digest_index++) {
      val_print(ACS_PRINT_DEBUG, "\n         Hash Alg        : 0x%x",
                                             digest[digest_index].hashalg);
      val_print(ACS_PRINT_DEBUG, "\n         Digest(%02d)      :",  digest_index);
      digest_buffer = (uint8_t *)(digest[digest_index].digest);
      for (i = 0; i < SHA256_DIGEST_SIZE; i++) {
        if ((!(i % 8)) && (i != 0))
          val_print(ACS_PRINT_DEBUG, "\n                          ", 0);
        val_print(ACS_PRINT_DEBUG, " %02x",  digest_buffer[i]);
      }
    }

    /* Cornor case if no digest are present */
    if (digest_index == 0) {
      digest_index = 1;
    }

    event_data = (EVENT_DATA *)((uint8_t *)&digest[--digest_index].digest[SHA256_DIGEST_SIZE]);
    val_print(ACS_PRINT_DEBUG, "\n         Event Size      : 0x%x",  event_data->event_size);
    val_print(ACS_PRINT_DEBUG, "\n         Event Data      : ",  0);
    data = (uint8_t *)((uint8_t *)event_data + sizeof(uint32_t));
    for (i = 0; i < event_data->event_size; i++) {
      val_print(ACS_PRINT_DEBUG, " %x",  data[i]);
    }
    val_print(ACS_PRINT_DEBUG, "\n", 0);

    /* Gent next event address and continue the loop */
    event2 = (TCG_PCR_EVENT2 *)((uint8_t *)event_data + sizeof(event_data->event_size) +
                              (event_data->event_size * sizeof(uint8_t)));
    event2_size = (uint8_t *)event2 - (uint8_t *) event;
    event_log_size = event_log_size - event2_size;
    event = event2;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));

free_dlme_region:
  val_memory_free_aligned((void *)drtm_params->dlme_region_address);
free_drtm_params:
  val_memory_free_aligned((void *)drtm_params);

  return;
}

uint32_t dl006_entry(uint32_t num_pe)
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
