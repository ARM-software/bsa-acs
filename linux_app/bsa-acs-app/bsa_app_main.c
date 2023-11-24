/** @file
 * Copyright (c) 2016-2018, 2020-2021, 2023, Arm Limited or its affiliates. All rights reserved.
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
#include "include/bsa_app.h"
#include <getopt.h>

int  g_print_level = 3;
unsigned int  g_sw_view[3] = {1, 1, 1}; //Operating System, Hypervisor, Platform Security
unsigned int  *g_skip_test_num;
unsigned int  g_num_skip = 3;
unsigned long int  g_exception_ret_addr;
unsigned int g_print_mmio;
unsigned int g_curr_module;
unsigned int g_enable_module;

int
initialize_test_environment(unsigned int print_level)
{

    return call_drv_init_test_env(print_level);
}

void
cleanup_test_environment()
{

    call_drv_clean_test_env();
}

void print_help(){
  printf ("\nUsage: Bsa [-v <n>] | [--skip <n>]\n"
         "Options:\n"
         "-v      Verbosity of the Prints\n"
         "        1 shows all prints, 5 shows Errors\n"
         "--skip  Test(s) to be skipped\n"
         "        Refer to section 4 of BSA_ACS_User_Guide\n"
         "        To skip a module, use Model_ID as mentioned in user guide\n"
         "        To skip a particular test within a module, use the exact testcase number\n"
  );
}

int
main (int argc, char **argv)
{

    int   c = 0,i=0;
    char *endptr, *pt;
    int   status;
    int   run_exerciser = 0;

    struct option long_opt[] =
    {
      {"skip", required_argument, NULL, 'n'},
      {"help", no_argument, NULL, 'h'},
      {NULL, 0, NULL, 0}
    };

    g_skip_test_num = (unsigned int *) malloc(g_num_skip * sizeof(unsigned int));

    /* Process Command Line arguments */
    while ((c = getopt_long(argc, argv, "hv:e:", long_opt, NULL)) != -1)
    {
       switch (c)
       {
       case 'v':
         g_print_level = strtol(optarg, &endptr, 10);
         break;
       case 'h':
         print_help();
         return 1;
         break;
       case 'n':/*SKIP tests */
         pt = strtok(optarg, ",");
         while ((pt != NULL) && (i < g_num_skip)) {
           int a = atoi(pt);
           g_skip_test_num[i++] = a;
           pt = strtok(NULL, ",");
         }
         break;
       case 'e':
         run_exerciser = 1;
         break;
       case '?':
         if (isprint (optopt))
           fprintf (stderr, "Unknown option `-%c'.\n", optopt);
         else
           fprintf (stderr,
                    "Unknown option character `\\x%x'.\n",
                    optopt);
         return 1;
       default:
         abort ();
       }
    }

    g_sw_view[G_SW_OS]  = 0x1;
    g_sw_view[G_SW_HYP] = 0x0;
    g_sw_view[G_SW_PS]  = 0x0;

    printf("\n ************ BSA Architecture Compliance Suite *********\n");
    printf("                        Version %d.%d.%d\n",
            BSA_APP_VERSION_MAJOR, BSA_APP_VERSION_MINOR, BSA_APP_VERSION_SUBMINOR);


    printf("\n Starting tests (Print level is %2d)\n\n", g_print_level);

    printf(" Gathering system information....\n");
    status = initialize_test_environment(g_print_level);
    if (status) {
        printf("Cannot initialize test environment. Exiting....\n");
        return 0;
    }

    printf("\n      *** Starting Memory Map tests ***\n");
    execute_tests_memory(1, g_print_level);

    printf("\n      *** Starting Peripherals tests ***\n");
    execute_tests_peripheral(1, g_print_level);

    if (run_exerciser) {
        printf("\n      *** PCIe Exerciser tests only runs on UEFI ***\n");
        //execute_tests_exerciser(1, g_print_level);
    } else {
        printf("\n      *** Starting PCIe tests ***\n");
        execute_tests_pcie(1, g_print_level);
    }

    printf("\n                    *** BSA tests complete ***\n\n");

    cleanup_test_environment();

    return 0;
}
