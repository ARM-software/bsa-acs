/*
 * EFI-related functions to set up and run test cases in EFI
 *
 * Copyright (c) 2021, SUSE, Varad Gautam <varad.gautam@suse.com>
 * Copyright (c) 2021, Google Inc, Zixuan Wang <zixuanwang@google.com>
 * Copyright (c) 2023-2024, Arm Ltd.
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "efi.h"
#include <argv.h>
#include <stdlib.h>
#include <ctype.h>
#include <libcflat.h>
#include <asm/setup.h>
#include "bsa_acs_litmus.h"

/* From lib/argv.c */
extern int __argc, __envc;
extern char *__argv[100];
extern char *__environ[200];

efi_status_t mem_model_execute_tests(efi_handle_t handle, efi_system_table_t *sys_tab);
efi_system_table_t *efi_system_table = NULL;

static void efi_free_pool(void *ptr)
{
    efi_bs_call(free_pool, ptr);
}

efi_status_t efi_get_memory_map(struct efi_boot_memmap *map)
{
    efi_memory_desc_t *m = NULL;
    efi_status_t status;
    unsigned long key = 0, map_size = 0, desc_size = 0;
    u32 desc_ver;

    status = efi_bs_call(get_memory_map, &map_size,
                 NULL, &key, &desc_size, &desc_ver);
    if (status != EFI_BUFFER_TOO_SMALL || map_size == 0)
        goto out;

    /*
     * Pad map_size with additional descriptors so we don't need to
     * retry.
     */
    map_size += 4 * desc_size;
    *map->buff_size = map_size;
    status = efi_bs_call(allocate_pool, EFI_LOADER_DATA,
                 map_size, (void **)&m);
    if (status != EFI_SUCCESS)
        goto out;

    /* Get the map. */
    status = efi_bs_call(get_memory_map, &map_size,
                 m, &key, &desc_size, &desc_ver);
    if (status != EFI_SUCCESS) {
        efi_free_pool(m);
        goto out;
    }

    *map->desc_ver = desc_ver;
    *map->desc_size = desc_size;
    *map->map_size = map_size;
    *map->key_ptr = key;
out:
    *map->map = m;
    return status;
}

efi_status_t efi_exit_boot_services(void *handle, struct efi_boot_memmap *map)
{
    return efi_bs_call(exit_boot_services, handle, *map->key_ptr);
}

efi_status_t efi_get_system_config_table(efi_guid_t table_guid, void **table)
{
    size_t i;
    efi_config_table_t *tables;

    tables = (efi_config_table_t *)efi_system_table->tables;
    for (i = 0; i < efi_system_table->nr_tables; i++) {
        if (!memcmp(&table_guid, &tables[i].guid, sizeof(efi_guid_t))) {
            *table = tables[i].table;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}

// /* Adapted from drivers/firmware/efi/libstub/efi-stub.c */
// static char *efi_convert_cmdline(struct efi_loaded_image_64 *image, int *cmd_line_len)
// {
//     const u16 *s2;
//     unsigned long cmdline_addr = 0;
//     int options_chars = image->load_options_size;
//     const u16 *options = image->load_options;
//     int options_bytes = 0, safe_options_bytes = 0;  /* UTF-8 bytes */
//     bool in_quote = false;
//     efi_status_t status;
//     const int COMMAND_LINE_SIZE = 2048;

//     if (options) {
//         s2 = options;
//         while (options_bytes < COMMAND_LINE_SIZE && options_chars--) {
//             u16 c = *s2++;

//             if (c < 0x80) {
//                 if (c == L'\0' || c == L'\n')
//                     break;
//                 if (c == L'"')
//                     in_quote = !in_quote;
//                 else if (!in_quote && isspace((char)c))
//                     safe_options_bytes = options_bytes;

//                 options_bytes++;
//                 continue;
//             }

//             /*
//              * Get the number of UTF-8 bytes corresponding to a
//              * UTF-16 character.
//              * The first part handles everything in the BMP.
//              */
//             options_bytes += 2 + (c >= 0x800);
//             /*
//              * Add one more byte for valid surrogate pairs. Invalid
//              * surrogates will be replaced with 0xfffd and take up
//              * only 3 bytes.
//              */
//             if ((c & 0xfc00) == 0xd800) {
//                 /*
//                  * If the very last word is a high surrogate,
//                  * we must ignore it since we can't access the
//                  * low surrogate.
//                  */
//                 if (!options_chars) {
//                     options_bytes -= 3;
//                 } else if ((*s2 & 0xfc00) == 0xdc00) {
//                     options_bytes++;
//                     options_chars--;
//                     s2++;
//                 }
//             }
//         }
//         if (options_bytes >= COMMAND_LINE_SIZE) {
//             options_bytes = safe_options_bytes;
//             printf("Command line is too long: truncated to %d bytes\n",
//                    options_bytes);
//         }
//     }

//     options_bytes++;        /* NUL termination */

//     status = efi_bs_call(allocate_pool, EFI_LOADER_DATA, options_bytes, (void **)&cmdline_addr);
//     if (status != EFI_SUCCESS)
//         return NULL;

//     snprintf((char *)cmdline_addr, options_bytes, "%.*ls", options_bytes - 1, options);

//     *cmd_line_len = options_bytes;
//     return (char *)cmdline_addr;
// }

efi_status_t mem_model_execute_tests(efi_handle_t handle, efi_system_table_t *sys_tab)
{
    efi_status_t status;
    efi_bootinfo_t efi_bootinfo;

    efi_system_table = sys_tab;

    /* Memory map struct values */
    efi_memory_desc_t *map = NULL;
    unsigned long map_size = 0, desc_size = 0, key = 0, buff_size = 0;
    u32 desc_ver;

    /* Helper variables needed to get the cmdline */
    struct efi_loaded_image_64 *image;
    efi_guid_t loaded_image_proto = LOADED_IMAGE_PROTOCOL_GUID;
    char *cmdline_ptr = NULL;
    //int cmdline_size = 0;

    /*
     * Get a handle to the loaded image protocol.  This is used to get
     * information about the running image, such as size and the command
     * line.
     */
    status = efi_bs_call(handle_protocol, handle, &loaded_image_proto, (void *)&image);
    if (status != EFI_SUCCESS) {
        printf("Failed to get loaded image protocol\n");
        goto efi_main_error;
    }

    // cmdline_ptr = efi_convert_cmdline(image, &cmdline_size);
    // if (!cmdline_ptr) {
    //     printf("getting command line via LOADED_IMAGE_PROTOCOL\n");
    //     status = EFI_OUT_OF_RESOURCES;
    //     goto efi_main_error;
    // }
    setup_args(cmdline_ptr);

    /* Set up efi_bootinfo */
    efi_bootinfo.mem_map.map = &map;
    efi_bootinfo.mem_map.map_size = &map_size;
    efi_bootinfo.mem_map.desc_size = &desc_size;
    efi_bootinfo.mem_map.desc_ver = &desc_ver;
    efi_bootinfo.mem_map.key_ptr = &key;
    efi_bootinfo.mem_map.buff_size = &buff_size;

    /* Get EFI memory map */
    status = efi_get_memory_map(&efi_bootinfo.mem_map);
    if (status != EFI_SUCCESS) {
        printf("Failed to get memory map\n");
        goto efi_main_error;
    }

    /* Set up arch-specific resources */
    status = setup_efi(&efi_bootinfo);
    if (status != EFI_SUCCESS) {
        printf("Failed to set up arch-specific resources\n");
        goto efi_main_error;
    }

      printf("\nRunning tests ...\n\n");
      printf("\n*********************************************\n");
      _X2_2B_2W_2B_dmb_2E_sys(__argc, __argv);
      printf("\n*********************************************\n");
      CO_2D_MIXED_2D_20cc_2B_H(__argc, __argv);
      printf("\n*********************************************\n");
      CoRR(__argc, __argv);
      printf("\n*********************************************\n");
      CoRW1(__argc, __argv);
      printf("\n*********************************************\n");
      CoRW2_2B_posb1b0_2B_h0(__argc, __argv);
      printf("\n*********************************************\n");
      CoRW2(__argc, __argv);
      printf("\n*********************************************\n");
      CoWR(__argc, __argv);
      printf("\n*********************************************\n");
      CoWW(__argc, __argv);
      printf("\n*********************************************\n");
      LB_2B_BEQ4(__argc, __argv);
      printf("\n*********************************************\n");
      LB_2B_CSEL_2D_addr_2D_po_2B_DMB(__argc, __argv);
      printf("\n*********************************************\n");
      LB_2B_CSEL_2D_rfi_2D_data_2B_DMB(__argc, __argv);
      printf("\n*********************************************\n");
      LB_2B_dmb_2E_sy_2B_data_2D_wsi_2D_wsi_2B_MIXED_2B_H(__argc, __argv);
      printf("\n*********************************************\n");
      LB_2B_dmb_2E_sys(__argc, __argv);
      printf("\n*********************************************\n");
      LB_2B_rel_2B_BEQ2(__argc, __argv);
      printf("\n*********************************************\n");
      LB_2B_rel_2B_CSEL_2D_CSEL(__argc, __argv);
      printf("\n*********************************************\n");
      LB_2B_rel_2B_data(__argc, __argv);
      printf("\n*********************************************\n");
      MP_2B_dmb_2E_sys(__argc, __argv);
      printf("\n*********************************************\n");
      MP_2D_Koeln(__argc, __argv);
      printf("\n*********************************************\n");
      R_2B_dmb_2E_sys(__argc, __argv);
      printf("\n*********************************************\n");
      S_2B_dmb_2E_sys(__argc, __argv);
      printf("\n*********************************************\n");
      S_2B_rel_2B_CSEL_2D_data(__argc, __argv);
      printf("\n*********************************************\n");
      S_2B_rel_2B_CSEL_2D_rf_2D_reg(__argc, __argv);
      printf("\n*********************************************\n");
      SB_2B_dmb_2E_sys(__argc, __argv);
      printf("\n*********************************************\n");
      T10B(__argc, __argv);
      printf("\n*********************************************\n");
      T10C(__argc, __argv);
      printf("\n*********************************************\n");
      T15_2D_corrected(__argc, __argv);
      printf("\n*********************************************\n");
      T15_2D_datadep_2D_corrected(__argc, __argv);
      printf("\n*********************************************\n");
      T3_2D_bis(__argc, __argv);
      printf("\n*********************************************\n");
      T3(__argc, __argv);
      printf("\n*********************************************\n");
      T7(__argc, __argv);
      printf("\n*********************************************\n");
      T7dep(__argc, __argv);
      printf("\n*********************************************\n");
      T8_2B_BIS(__argc, __argv);
      printf("\n*********************************************\n");
      T9B(__argc, __argv);
      printf("\n\n      *** Memory model consistency tests run complete. Reset the system. ***  ");
efi_main_error:
    return 0;
}
