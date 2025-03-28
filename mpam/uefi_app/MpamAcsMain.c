/** @file
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
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

#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/ShellCEntryLib.h>
#include  <Library/ShellLib.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/CacheMaintenanceLib.h>
#include  <Protocol/LoadedImage.h>

#include "../val/common/include/val_interface.h"
#include "../val/common/include/acs_pe.h"
#include "../val/common/include/acs_val.h"
#include "../val/common/include/acs_mpam.h"
#include "../val/common/include/acs_memory.h"
#include "../val/mpam/include/mpam_val_interface.h"

#include "MpamAcs.h"


UINT32  g_print_level;
UINT32  g_execute_secure;
UINT32  *g_skip_test_num;
UINT32  g_num_skip;
UINT32  g_acs_tests_total;
UINT32  g_acs_tests_pass;
UINT32  g_acs_tests_fail;
UINT64  g_stack_pointer;
UINT64  g_exception_ret_addr;
UINT64  g_ret_addr;
UINT32  g_print_mmio;
UINT32  g_curr_module;
UINT32  g_enable_module;
UINT32  *g_execute_tests;
UINT32  g_num_tests = 0;
UINT32  *g_execute_modules;
UINT32  g_num_modules = 0;

SHELL_FILE_HANDLE g_acs_log_file_handle;

STATIC
VOID
FlushImage (
    VOID
    )
{

    EFI_LOADED_IMAGE_PROTOCOL   *ImageInfo;
    EFI_STATUS Status;
    Status = gBS->HandleProtocol (gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&ImageInfo);
    if(EFI_ERROR (Status)) {
        return;
    }

    val_pe_cache_clean_range((UINT64)ImageInfo->ImageBase, (UINT64)ImageInfo->ImageSize);
}

UINT32
createPeInfoTable (
)
{
  UINT32 Status;
  UINT64 *PeInfoTable;

  PeInfoTable = val_aligned_alloc(SIZE_4K, PE_INFO_TBL_SZ);

  Status = val_pe_create_info_table(PeInfoTable);

  return Status;
}

UINT32
createGicInfoTable (
)
{
  UINT32 Status;
  UINT64 *GicInfoTable;

  GicInfoTable = val_aligned_alloc(SIZE_4K, GIC_INFO_TBL_SZ);

  Status = val_gic_create_info_table(GicInfoTable);

  return Status;
}

VOID
createMpamInfoTable(
)
{
  UINT64 *MpamInfoTable;

  MpamInfoTable = val_aligned_alloc(SIZE_4K, MPAM_INFO_TBL_SZ);

  val_mpam_create_info_table(MpamInfoTable);
}

VOID
createHmatInfoTable(
)
{
  UINT64 *HmatInfoTable;

  HmatInfoTable = val_aligned_alloc(SIZE_4K, HMAT_INFO_TBL_SZ);

  val_hmat_create_info_table(HmatInfoTable);
}

VOID
createSratInfoTable(
)
{
  UINT64 *SratInfoTable;

  SratInfoTable = val_aligned_alloc(SIZE_4K, SRAT_INFO_TBL_SZ);

  val_srat_create_info_table(SratInfoTable);
}

VOID
createPccInfoTable(
)
{
  UINT64 *PccInfoTable;

  PccInfoTable = val_aligned_alloc(SIZE_4K, PCC_INFO_TBL_SZ);

  val_pcc_create_info_table(PccInfoTable);
}

VOID
createCacheInfoTable(
)
{
  UINT64 *CacheInfoTable;

  CacheInfoTable = val_aligned_alloc(SIZE_4K, CACHE_INFO_TBL_SZ);

  val_cache_create_info_table(CacheInfoTable);
}

VOID
FreeMpamAcsMem (
)
{
    val_pe_free_info_table();
    val_gic_free_info_table();
    val_mpam_free_info_table();
    val_hmat_free_info_table();
    val_srat_free_info_table();
    val_pcc_free_info_table();
    val_free_shared_mem();
}

VOID
HelpMsg (
    VOID
    )
{

    Print (L"\nUsage: Mpam.efi [-v <n>] | [-f <filename>] | [-s] | [-skip <n>]| [-t <n>] | [-m <n>]\n"
             "Options:\n"
             "-v      Verbosity of the Prints\n"
             "        1 shows all prints, 5 shows Errors\n"
             "-f      Name of the log file to record the test results in\n"
             "-skip   Test(s) to be skipped\n"
             "        To skip a module, use Model_ID as mentioned in user guide\n"
             "        To skip a particular test within a module, use the exact testcase number\n"
             "-t      If Test ID(s) set, will only run the specified test(s), all others will be skipped.\n"
             "-m      If Module ID(s) set, will only run the specified module(s), all others will be skipped.\n"
             );
}

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
    {L"-v"    , TypeValue},    // -v    # Verbosity of the Prints. 1 shows all prints, 5 shows Errors
    {L"-f"    , TypeValue},    // -f    # Name of the log file to record the test results in.
    {L"-skip" , TypeValue},    // -skip # test(s) to skip execution
    {L"-help" , TypeFlag},     // -help # help : info about commands
    {L"-h"    , TypeFlag},     // -h    # help : info about commands
    {L"-t"    , TypeValue},    // -t    # Test to be run
    {L"-m"    , TypeValue},    // -m    # Module to be run
    {NULL     , TypeMax}
};

/**
 * @brief   MPAM Compliance Suite Entry Point.
 *
 * Call the Entry points of individual modules.
 *
 * @retval  0       The application exited normally.
 * @retval  Other   An error occurred.
 */
INTN
EFIAPI
ShellAppMain (
    IN UINTN Argc,
    IN CHAR16 **Argv
    )
{

    LIST_ENTRY         *ParamPackage;
    CONST CHAR16       *CmdLineArg;
    CHAR16             *ProbParam;
    UINT32             Status;
    UINT32             i;
    VOID               *branch_label;
    UINT32             msc_node_cnt;

    /* Process Command Line arguments */
    Status = ShellInitialize();
    Status = ShellCommandLineParse (ParamList, &ParamPackage, &ProbParam, TRUE);
    if (Status) {
        Print(L"Shell command line parse error %x\n", Status);
        Print(L"Unrecognized option %s passed\n", ProbParam);
        HelpMsg();
        return SHELL_INVALID_PARAMETER;
    }

  // Options with Values
    if (ShellCommandLineGetFlag (ParamPackage, L"-skip")) {
      CmdLineArg  = ShellCommandLineGetValue (ParamPackage, L"-skip");
      if (CmdLineArg == NULL)
      {
        Print(L"Invalid parameter passed for -skip\n", 0);
        HelpMsg();
        return SHELL_INVALID_PARAMETER;
      }
      else
      {
        Status = gBS->AllocatePool(EfiBootServicesData,
                                   StrLen(CmdLineArg),
                                   (VOID **) &g_skip_test_num);
        if (EFI_ERROR(Status))
        {
          Print(L"Allocate memory for -skip failed\n", 0);
          return 0;
        }

        g_skip_test_num[0] = StrDecimalToUintn((CONST CHAR16 *)(CmdLineArg+0));
        for (i = 0; i < StrLen(CmdLineArg); i++) {
        if(*(CmdLineArg+i) == L',') {
            g_skip_test_num[++g_num_skip] = StrDecimalToUintn((CONST CHAR16 *)(CmdLineArg+i+1));
          }
        }

        g_num_skip++;
      }
  }

    /* Options with Values */
    CmdLineArg  = ShellCommandLineGetValue (ParamPackage, L"-v");
    if (CmdLineArg == NULL) {
        g_print_level = G_PRINT_LEVEL;
    } else {
        g_print_level = StrDecimalToUintn(CmdLineArg);
        if (g_print_level > 5) {
            g_print_level = G_PRINT_LEVEL;
        }
    }

    /* Options with Values */
    CmdLineArg  = ShellCommandLineGetValue (ParamPackage, L"-f");
    if (CmdLineArg == NULL) {
        g_acs_log_file_handle = NULL;
    } else {
        Status = ShellOpenFileByName(CmdLineArg, &g_acs_log_file_handle,
                     EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0x0);
        if (EFI_ERROR(Status)) {
            Print(L"Failed to open log file %s\n", CmdLineArg);
            g_acs_log_file_handle = NULL;
        }
    }

    // Options with Values
    if (ShellCommandLineGetFlag (ParamPackage, L"-t")) {
      CmdLineArg  = ShellCommandLineGetValue (ParamPackage, L"-t");
      if (CmdLineArg == NULL)
      {
          Print(L"Invalid parameter passed for -t\n", 0);
          HelpMsg();
          return SHELL_INVALID_PARAMETER;
      }
      else
      {
          Status = gBS->AllocatePool(EfiBootServicesData,
                                     StrLen(CmdLineArg),
                                     (VOID **) &g_execute_tests);
          if (EFI_ERROR(Status))
          {
              Print(L"Allocate memory for -t failed\n", 0);
              return 0;
          }

          /* Check if the first value to -t is a decimal character. */
          if (!ShellIsDecimalDigitCharacter(*CmdLineArg)) {
              Print(L"Invalid parameter passed for -t\n", 0);
              HelpMsg();
              return SHELL_INVALID_PARAMETER;
          }

          g_execute_tests[0] = StrDecimalToUintn((CONST CHAR16 *)(CmdLineArg + 0));
          for (i = 0; i < StrLen(CmdLineArg); i++) {
              if (*(CmdLineArg + i) == L',') {
                  g_execute_tests[++g_num_tests] = StrDecimalToUintn(
                                                          (CONST CHAR16 *)(CmdLineArg + i + 1));
              }
          }

          g_num_tests++;
        }
    }

    if (ShellCommandLineGetFlag (ParamPackage, L"-m")) {
        CmdLineArg  = ShellCommandLineGetValue (ParamPackage, L"-m");
        if (CmdLineArg == NULL)
        {
            Print(L"Invalid parameter passed for -m\n", 0);
            HelpMsg();
            return SHELL_INVALID_PARAMETER;
        }
        else
        {
            Status = gBS->AllocatePool(EfiBootServicesData,
                                       StrLen(CmdLineArg),
                                       (VOID **) &g_execute_modules);
            if (EFI_ERROR(Status))
            {
                Print(L"Allocate memory for -m failed\n", 0);
                return 0;
            }

            /* Check if the first value to -m is a decimal character. */
            if (!ShellIsDecimalDigitCharacter(*CmdLineArg)) {
                Print(L"Invalid parameter passed for -m\n", 0);
                HelpMsg();
                return SHELL_INVALID_PARAMETER;
            }

            g_execute_modules[0] = StrDecimalToUintn((CONST CHAR16 *)(CmdLineArg + 0));
            for (i = 0; i < StrLen(CmdLineArg); i++) {
                if (*(CmdLineArg + i) == L',') {
                    g_execute_modules[++g_num_modules] = StrDecimalToUintn(
                                                            (CONST CHAR16 *)(CmdLineArg + i + 1));
                }
            }

            g_num_modules++;
        }
    }

    /* Options with Values */
    if ((ShellCommandLineGetFlag (ParamPackage, L"-help")) || (ShellCommandLineGetFlag (ParamPackage, L"-h"))) {
        HelpMsg();
        return 0;
    }

    /* Initialize global counters */
    g_acs_tests_total = 0;
    g_acs_tests_pass  = 0;
    g_acs_tests_fail  = 0;

    Print(L"\n\n MPAM System Architecture Compliance Suite \n");
    Print(L"    Version %d.%d  \n", MPAM_ACS_MAJOR_VER, MPAM_ACS_MINOR_VER);

    Print(L"\n Starting tests for Print level %2d\n\n", g_print_level);

    Print(L" Creating Platform Information Tables \n");
    Status = createPeInfoTable();
    if (Status)
        return Status;

    /* check if PE supports MPAM extension, else skip all MPAM tests */
    if (val_pe_feat_check(PE_FEAT_MPAM)) {
        val_print(ACS_PRINT_TEST,
                  "\n       PE MPAM extension unimplemented. Skipping all MPAM tests\n", 0);
        goto print_test_status;
    }

    Status = createGicInfoTable();
    if (Status)
        return Status;

    createCacheInfoTable();

    /* required before calling createMpamInfoTable() */
    createPccInfoTable();

    createHmatInfoTable();

    createSratInfoTable();

    createMpamInfoTable();

    /* Get total number of MSCs reported by MPAM ACPI table */
    msc_node_cnt = val_mpam_get_msc_count();
    if (msc_node_cnt == 0) {
        val_print(ACS_PRINT_TEST, "\n      *** Exiting suite - No MPAM nodes *** \n", 0);
        goto print_test_status;
    }

    val_allocate_shared_mem();

    /*
     * Initialise exception vector, so any unexpected exception gets handled
     * by default MPAM exception handler
     */
    branch_label = &&print_test_status;
    val_pe_context_save(AA64ReadSp(), (uint64_t)branch_label);
    val_pe_initialize_default_exception_handler(val_pe_default_esr);
    FlushImage();

    Status |= val_mpam_execute_register_tests(val_pe_get_num());

    Status |= val_mpam_execute_cache_tests(val_pe_get_num());

    Status |= val_mpam_execute_error_tests(val_pe_get_num());

    Status |= val_mpam_execute_membw_tests(val_pe_get_num());

print_test_status:
    val_print(ACS_PRINT_TEST, "\n     ------------------------------------------------------- \n", 0);
    val_print(ACS_PRINT_TEST, "     Total Tests run  = %4d;", g_acs_tests_total);
    val_print(ACS_PRINT_TEST, "  Tests Passed  = %4d", g_acs_tests_pass);
    val_print(ACS_PRINT_TEST, "  Tests Failed = %4d\n", g_acs_tests_fail);
    val_print(ACS_PRINT_TEST, "     --------------------------------------------------------- \n", 0);

    FreeMpamAcsMem();

    if (g_acs_log_file_handle) {
        ShellCloseFile(&g_acs_log_file_handle);
    }

    Print(L"\n      *** MPAM tests complete. Reset the system. *** \n\n");

    val_pe_context_restore(AA64WriteSp(g_stack_pointer));

    return(0);
}
