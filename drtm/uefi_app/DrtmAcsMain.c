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

#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/ShellCEntryLib.h>
#include  <Library/ShellLib.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/CacheMaintenanceLib.h>
#include  <Protocol/LoadedImage.h>

#include "DrtmAcs.h"
#include "val/drtm/include/drtm_val_interface.h"
#include "val/common/include/val_interface.h"
#include "val/common/include/acs_val.h"

UINT32  g_print_level;
UINT32  g_acs_tests_pass;
UINT32  g_acs_tests_fail;
UINT32  *g_skip_test_num;
UINT32  g_num_skip;
UINT64  g_stack_pointer;
UINT64  g_exception_ret_addr;
UINT64  g_ret_addr;
UINT32  g_wakeup_timeout;
UINT32  g_print_mmio;
UINT32  g_curr_module;
UINT32  g_enable_module;
UINT32  *g_execute_tests;
UINT32  g_num_tests = 0;
UINT32  *g_execute_modules;
UINT32  g_num_modules = 0;
UINT32  g_acs_tests_total;

SHELL_FILE_HANDLE g_acs_log_file_handle;


EFI_STATUS
createPeInfoTable (
)
{

  EFI_STATUS Status;

  UINT64   *PeInfoTable;

/* allowing room for growth, at present each entry is 16 bytes, so we can support upto 511 PEs with 8192 bytes*/
  Status = gBS->AllocatePool ( EfiBootServicesData,
                               PE_INFO_TBL_SZ,
                               (VOID **) &PeInfoTable );

  if (EFI_ERROR(Status))
  {
    Print(L"Allocate Pool failed %x\n", Status);
    return Status;
  }

  Status = val_pe_create_info_table(PeInfoTable);

  return Status;

}

EFI_STATUS
createGicInfoTable (
)
{
  EFI_STATUS Status;
  UINT64     *GicInfoTable;

  Status = gBS->AllocatePool (EfiBootServicesData,
                               GIC_INFO_TBL_SZ,
                               (VOID **) &GicInfoTable);

  if (EFI_ERROR(Status))
  {
    Print(L"Allocate Pool failed %x\n", Status);
    return Status;
  }

  Status = val_gic_create_info_table(GicInfoTable);

  return Status;

}

VOID
HelpMsg (
  VOID
  )
{
  Print (L"\nUsage: Bsa.efi [-v <n>] | [-f <filename>] | [-skip <n>] | [-t <n>] | [-m <n>]\n"
         "Options:\n"
         "-v      Verbosity of the prints\n"
         "        1 prints all, 5 prints only the errors\n"
         "        Note: pal_mmio prints can be enabled for specific modules by passing\n"
         "              module numbers along with global verbosity level 1\n"
         "              Module numbers are Interface 0, Dynamic Launch 1   ...\n"
         "              E.g., To enable mmio prints \n"
         "-f      Name of the log file to record the test results in\n"
         "-skip   Test(s) to be skipped\n"
         "        Refer to section DRTM ACS User Guide\n"
         "        To skip a module, use Module ID as mentioned in user guide\n"
         "        To skip a particular test within a module, use the exact testcase number\n"
         "-t      If Test ID(s) set, will only run the specified test(s), all others will be skipped.\n"
         "-m      If Module ID(s) set, will only run the specified module(s), all others will be skipped.\n"
  );
}

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
  {L"-v", TypeValue},    // -v    # Verbosity of the Prints. 1 shows all prints, 5 shows Errors
  {L"-f", TypeValue},    // -f    # Name of the log file to record the test results in.
  {L"-skip", TypeValue}, // -skip # test(s) to skip execution
  {L"-t", TypeValue},    // -t    # Test to be run
  {L"-m", TypeValue},    // -m    # Module to be run
  {L"-help", TypeFlag},  // -help # help : info about commands
  {L"-h", TypeFlag},     // -h    # help : info about commands
  {NULL, TypeMax}
  };

/***
  DRTM Compliance Suite Entry Point.

  Call the Entry points of individual modules.

  @retval  0         The application exited normally.
  @retval  Other     An error occurred.
***/
INTN
EFIAPI
ShellAppMainDrtm (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{

  LIST_ENTRY         *ParamPackage;
  CONST CHAR16       *CmdLineArg;
  CHAR16             *ProbParam;
  UINT32             Status;
  UINT32             i;
  UINT32             ReadVerbosity;

  //
  // Process Command Line arguments
  //
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
        Print(L"Allocate memory for -skip failed \n", 0);
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

    // Options with Values
  CmdLineArg  = ShellCommandLineGetValue (ParamPackage, L"-v");
  if (CmdLineArg == NULL) {
    g_print_level = 3;
  } else {
    ReadVerbosity = StrDecimalToUintn(CmdLineArg);
    while (ReadVerbosity/10) {
      g_enable_module |= (1 << ReadVerbosity%10);
      ReadVerbosity /= 10;
    }
    g_print_level = ReadVerbosity;
    if (g_print_level > 5) {
      g_print_level = 3;
    }
  }

    // Options with Values
  CmdLineArg  = ShellCommandLineGetValue (ParamPackage, L"-f");
  if (CmdLineArg == NULL) {
    g_acs_log_file_handle = NULL;
  } else {
    Status = ShellOpenFileByName(CmdLineArg, &g_acs_log_file_handle,
             EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0x0);
    if(EFI_ERROR(Status)) {
         Print(L"Failed to open log file %s\n", CmdLineArg);
         g_acs_log_file_handle = NULL;
    }
  }

  // Options with Flags
  if ((ShellCommandLineGetFlag (ParamPackage, L"-help")) || (ShellCommandLineGetFlag (ParamPackage, L"-h"))){
     HelpMsg();
     return 0;
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
              Print(L"Allocate memory for -t failed \n", 0);
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

  // Options with Values
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
              Print(L"Allocate memory for -m failed \n", 0);
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

  val_print(ACS_PRINT_TEST, "\n\n DRTM Architecture Compliance Suite", 0);
  val_print(ACS_PRINT_TEST, "\n          Version %d.", DRTM_ACS_MAJOR_VER);
  val_print(ACS_PRINT_TEST, "%d\n", DRTM_ACS_MINOR_VER);

  val_print(ACS_PRINT_TEST, "\n Starting tests with print level : %2d\n\n", g_print_level);
  val_print(ACS_PRINT_TEST, "\n Creating Platform Information Tables\n", 0);

  Status = createPeInfoTable();
  if (Status)
    return Status;

  Status = createGicInfoTable();
  if (Status)
    return Status;

  val_allocate_shared_mem();
  g_acs_tests_pass  = 0;
  g_acs_tests_fail  = 0;

  Status = val_drtm_create_info_table();

  /* Starting Interface Tests */
  Status |= val_drtm_execute_interface_tests(val_pe_get_num());

  /* Starting Dynamic Launch Tests */
  Status |= val_drtm_execute_dl_tests(val_pe_get_num());

  /* Print Summary */
  val_print(ACS_PRINT_TEST, "\n     -------------------------------------------------------\n", 0);
  val_print(ACS_PRINT_TEST, "     Total Tests Run  = %4d", g_acs_tests_total);
  val_print(ACS_PRINT_TEST, "  Passed  = %4d", g_acs_tests_pass);
  val_print(ACS_PRINT_TEST, "  Failed = %4d\n", g_acs_tests_fail);
  val_print(ACS_PRINT_TEST, "     -------------------------------------------------------\n", 0);

  val_print(ACS_PRINT_TEST, "\n      *** DRTM tests complete. *** \n\n", 0);

  return(0);
}
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  return ShellAppMainDrtm(Argc, Argv);
}
