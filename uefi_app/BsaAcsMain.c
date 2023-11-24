/** @file
 * Copyright (c) 2016-2023, Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/val_interface.h"
#include "val/include/bsa_acs_pe.h"
#include "val/include/bsa_acs_val.h"

#include "BsaAcs.h"

UINT32 g_pcie_p2p;
UINT32 g_pcie_cache_present;

UINT32  g_print_level;
UINT32  g_sw_view[3] = {1, 1, 1}; //Operating System, Hypervisor, Platform Security
UINT32  *g_skip_test_num;
UINT32  g_num_skip;
UINT32  g_bsa_tests_total;
UINT32  g_bsa_tests_pass;
UINT32  g_bsa_tests_fail;
UINT64  g_stack_pointer;
UINT64  g_exception_ret_addr;
UINT64  g_ret_addr;
UINT32  g_wakeup_timeout;
UINT32  g_build_sbsa = 0;
UINT32  g_print_mmio;
UINT32  g_curr_module;
UINT32  g_enable_module;
UINT32  *g_execute_tests;
UINT32  g_num_tests = 0;
UINT32  *g_execute_modules;
UINT32  g_num_modules = 0;
/* VE systems run acs at EL1 and in some systems crash is observed during acess
   of EL1 phy and virt timer, Below command line option is added only for debug
   purpose to complete BSA run on these systems */
UINT32  g_el1physkip = FALSE;

SHELL_FILE_HANDLE g_bsa_log_file_handle;
SHELL_FILE_HANDLE g_dtb_log_file_handle;

STATIC VOID FlushImage (VOID)
{
  EFI_LOADED_IMAGE_PROTOCOL   *ImageInfo;
  EFI_STATUS Status;
  Status = gBS->HandleProtocol (gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&ImageInfo);
  if(EFI_ERROR (Status))
  {
    return;
  }

  val_pe_cache_clean_range((UINT64)ImageInfo->ImageBase, (UINT64)ImageInfo->ImageSize);

}

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

EFI_STATUS
createTimerInfoTable(
)
{
  UINT64   *TimerInfoTable;
  EFI_STATUS Status;

  Status = gBS->AllocatePool (EfiBootServicesData,
                              TIMER_INFO_TBL_SZ,
                              (VOID **) &TimerInfoTable);

  if (EFI_ERROR(Status))
  {
    Print(L"Allocate Pool failed %x\n", Status);
    return Status;
  }
  val_timer_create_info_table(TimerInfoTable);

  return Status;
}

EFI_STATUS
createWatchdogInfoTable(
)
{
  UINT64   *WdInfoTable;
  EFI_STATUS Status;

  Status = gBS->AllocatePool (EfiBootServicesData,
                              WD_INFO_TBL_SZ,
                              (VOID **) &WdInfoTable);

  if (EFI_ERROR(Status))
  {
    Print(L"Allocate Pool failed %x\n", Status);
    return Status;
  }
  val_wd_create_info_table(WdInfoTable);

  return Status;

}


EFI_STATUS
createPcieVirtInfoTable(
)
{
  UINT64   *PcieInfoTable;
  UINT64   *IoVirtInfoTable;

  EFI_STATUS Status;

  Status = gBS->AllocatePool (EfiBootServicesData,
                              PCIE_INFO_TBL_SZ,
                              (VOID **) &PcieInfoTable);

  if (EFI_ERROR(Status))
  {
    Print(L"Allocate Pool failed %x\n", Status);
    return Status;
  }
  val_pcie_create_info_table(PcieInfoTable);

  Status = gBS->AllocatePool (EfiBootServicesData,
                              IOVIRT_INFO_TBL_SZ,
                              (VOID **) &IoVirtInfoTable);

  if (EFI_ERROR(Status))
  {
    Print(L"Allocate Pool failed %x\n", Status);
    return Status;
  }
  val_iovirt_create_info_table(IoVirtInfoTable);

  return Status;
}

EFI_STATUS
createPeripheralInfoTable(
)
{
  UINT64   *PeripheralInfoTable;
  UINT64   *MemoryInfoTable;

  EFI_STATUS Status;

  Status = gBS->AllocatePool (EfiBootServicesData,
                              PERIPHERAL_INFO_TBL_SZ,
                              (VOID **) &PeripheralInfoTable);

  if (EFI_ERROR(Status))
  {
    Print(L"Allocate Pool failed %x\n", Status);
    return Status;
  }
  val_peripheral_create_info_table(PeripheralInfoTable);

  Status = gBS->AllocatePool (EfiBootServicesData,
                              MEM_INFO_TBL_SZ,
                              (VOID **) &MemoryInfoTable);

  if (EFI_ERROR(Status))
  {
    Print(L"Allocate Pool failed %x\n", Status);
    return Status;
  }

  val_memory_create_info_table(MemoryInfoTable);

  return Status;
}

VOID
freeBsaAcsMem()
{

  val_pe_free_info_table();
  val_gic_free_info_table();
  val_timer_free_info_table();
  val_wd_free_info_table();
  val_pcie_free_info_table();
  val_iovirt_free_info_table();
  val_peripheral_free_info_table();
  val_free_shared_mem();
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
         "              Module numbers are PE 0, MEM 1, GIC 2, SMMU 3, TIMER 4, WAKEUP 5   ...\n"
         "              PERIPHERAL 6, Watchdog 7, PCIe 8, Exerciser 9   ...\n"
         "              E.g., To enable mmio prints for PE and TIMER pass -v 104\n"
         "-mmio   Pass this flag to enable pal_mmio_read/write prints, use with -v 1\n"
         "-f      Name of the log file to record the test results in\n"
         "-skip   Test(s) to be skipped\n"
         "        Refer to section 4 of BSA ACS User Guide\n"
         "        To skip a module, use Module ID as mentioned in user guide\n"
         "        To skip a particular test within a module, use the exact testcase number\n"
         "-t      If Test ID(s) set, will only run the specified test(s), all others will be skipped.\n"
         "-m      If Module ID(s) set, will only run the specified module(s), all others will be skipped.\n"
         "-p2p    Pass this flag to indicate that PCIe Hierarchy Supports Peer-to-Peer\n"
         "-cache  Pass this flag to indicate that if the test system supports PCIe address translation cache\n"
         "-timeout  Set timeout multiple for wakeup tests\n"
         "        1 - min value  5 - max value\n"
         "-os     Enable the execution of operating system tests\n"
         "-hyp    Enable the execution of hypervisor tests\n"
         "-ps     Enable the execution of platform security tests\n"
         "-dtb    Enable the execution of dtb dump\n"
         "-sbsa   Enable sbsa requirements for bsa binary\n"
         "-el1physkip Skips EL1 register checks\n"
  );
}

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
  {L"-v", TypeValue},    // -v    # Verbosity of the Prints. 1 shows all prints, 5 shows Errors
  {L"-f", TypeValue},    // -f    # Name of the log file to record the test results in.
  {L"-skip", TypeValue}, // -skip # test(s) to skip execution
  {L"-t", TypeValue},    // -t    # Test to be run
  {L"-m", TypeValue},    // -m    # Module to be run
  {L"-p2p", TypeFlag},   // -p2p  # Peer-to-Peer is supported
  {L"-cache", TypeFlag}, // -cache# PCIe address translation cache is supported
  {L"-timeout", TypeValue}, // -timeout # Set timeout multiple for wakeup tests
  {L"-help", TypeFlag},  // -help # help : info about commands
  {L"-h", TypeFlag},     // -h    # help : info about commands
  {L"-os", TypeFlag},    // -os   # Binary Flag to enable the execution of operating system tests.
  {L"-hyp", TypeFlag},   // -hyp  # Binary Flag to enable the execution of hypervisor tests.
  {L"-ps", TypeFlag},    // -ps   # Binary Flag to enable the execution of platform security tests.
  {L"-dtb", TypeValue},  // -dtb  # Binary Flag to enable dtb dump
  {L"-sbsa", TypeFlag},  // -sbsa # Enable sbsa requirements for bsa binary\n"
  {L"-mmio", TypeFlag}, // -mmio # Enable pal_mmio prints
  {L"-el1physkip", TypeFlag}, // -el1physkip # Skips EL1 register checks
  {NULL, TypeMax}
  };

/***
  BSA Compliance Suite Entry Point.

  Call the Entry points of individual modules.

  @retval  0         The application exited normally.
  @retval  Other     An error occurred.
***/
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

  // Options with Values
  CmdLineArg  = ShellCommandLineGetValue (ParamPackage, L"-timeout");
  if (CmdLineArg == NULL) {
    g_wakeup_timeout = 1;
  } else {
    g_wakeup_timeout = StrDecimalToUintn(CmdLineArg);
    Print(L"Wakeup timeout multiple %d.\n", g_wakeup_timeout);
    if (g_wakeup_timeout > 5)
        g_wakeup_timeout = 5;
    }

    // Options with Values
  CmdLineArg  = ShellCommandLineGetValue (ParamPackage, L"-v");
  if (CmdLineArg == NULL) {
    g_print_level = G_PRINT_LEVEL;
  } else {
    ReadVerbosity = StrDecimalToUintn(CmdLineArg);
    while (ReadVerbosity/10) {
      g_enable_module |= (1 << ReadVerbosity%10);
      ReadVerbosity /= 10;
    }
    g_print_level = ReadVerbosity;
    if (g_print_level > 5) {
      g_print_level = G_PRINT_LEVEL;
    }
  }

  if (ShellCommandLineGetFlag (ParamPackage, L"-mmio")) {
    g_print_mmio = TRUE;
  } else {
    g_print_mmio = FALSE;
  }

  // Options with Flags
   if (ShellCommandLineGetFlag (ParamPackage, L"-os")
       || ShellCommandLineGetFlag (ParamPackage, L"-hyp")
       || ShellCommandLineGetFlag (ParamPackage, L"-ps")) {
       g_sw_view[G_SW_OS]  = 0;
       g_sw_view[G_SW_HYP] = 0;
       g_sw_view[G_SW_PS]  = 0;

       if (ShellCommandLineGetFlag (ParamPackage, L"-os"))
           g_sw_view[G_SW_OS] = 1;

       if (ShellCommandLineGetFlag (ParamPackage, L"-hyp"))
           g_sw_view[G_SW_HYP] = 1;

       if (ShellCommandLineGetFlag (ParamPackage, L"-ps"))
           g_sw_view[G_SW_PS] = 1;
  }

  // Options with Flags
  if (ShellCommandLineGetFlag (ParamPackage, L"-sbsa"))
      g_build_sbsa  = 1;

    // Options with Values
  CmdLineArg  = ShellCommandLineGetValue (ParamPackage, L"-f");
  if (CmdLineArg == NULL) {
    g_bsa_log_file_handle = NULL;
  } else {
    Status = ShellOpenFileByName(CmdLineArg, &g_bsa_log_file_handle,
             EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0x0);
    if(EFI_ERROR(Status)) {
         Print(L"Failed to open log file %s\n", CmdLineArg);
         g_bsa_log_file_handle = NULL;
    }
  }

    // If user has pass dtb flag, then dump the dtb in file
  CmdLineArg  = ShellCommandLineGetValue(ParamPackage, L"-dtb");
  if (CmdLineArg == NULL) {
    g_dtb_log_file_handle = NULL;
  } else {
    Status = ShellOpenFileByName(CmdLineArg, &g_dtb_log_file_handle,
             EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0x0);
    if (EFI_ERROR(Status)) {
         Print(L"Failed to open file for dtb dump %s\n", CmdLineArg);
         g_dtb_log_file_handle = NULL;
    } else {
        val_dump_dtb();
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

  if (ShellCommandLineGetFlag (ParamPackage, L"-p2p")) {
    g_pcie_p2p = TRUE;
  } else {
    g_pcie_p2p = FALSE;
  }

  if (ShellCommandLineGetFlag (ParamPackage, L"-cache")) {
    g_pcie_cache_present = TRUE;
  } else {
    g_pcie_cache_present = FALSE;
  }

  if (ShellCommandLineGetFlag (ParamPackage, L"-el1physkip")) {
    g_el1physkip = TRUE;
  }
  //
  // Initialize global counters
  //
  g_bsa_tests_total = 0;
  g_bsa_tests_pass  = 0;
  g_bsa_tests_fail  = 0;

  val_print(ACS_PRINT_TEST, "\n\n BSA Architecture Compliance Suite", 0);
  val_print(ACS_PRINT_TEST, "\n          Version %d.", BSA_ACS_MAJOR_VER);
  val_print(ACS_PRINT_TEST, "%d.", BSA_ACS_MINOR_VER);
  val_print(ACS_PRINT_TEST, "%d\n", BSA_ACS_SUBMINOR_VER);

  val_print(ACS_PRINT_TEST, "\n Starting tests with print level : %2d\n\n", g_print_level);
  val_print(ACS_PRINT_TEST, "\n Creating Platform Information Tables\n", 0);





  Status = createPeInfoTable();
  if (Status)
    return Status;

  Status = createGicInfoTable();
  if (Status)
    return Status;

  /* Initialise exception vector, so any unexpected exception gets handled by default
     BSA exception handler */
  branch_label = &&print_test_status;
  val_pe_context_save(AA64ReadSp(), (uint64_t)branch_label);
  val_pe_initialize_default_exception_handler(val_pe_default_esr);

  createTimerInfoTable();
  createWatchdogInfoTable();
  createPcieVirtInfoTable();
  createPeripheralInfoTable();

  val_allocate_shared_mem();

  FlushImage();

  /***  Starting PE tests             ***/
  Status = val_pe_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting Memory Map tests     ***/
  Status |= val_memory_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting GIC tests            ***/
  Status |= val_gic_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting System MMU tests     ***/
  Status |= val_smmu_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting Timer tests          ***/
  Status |= val_timer_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting Wakeup semantic tests ***/
  Status |= val_wakeup_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting Peripheral tests     ***/
  Status |= val_peripheral_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting Watchdog tests       ***/
  Status |= val_wd_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting PCIe tests           ***/
  Status |= val_pcie_execute_tests(val_pe_get_num(), g_sw_view);

  /***  Starting PCIe Exerciser tests ***/
  Status |= val_exerciser_execute_tests(g_sw_view);

print_test_status:
  val_print(ACS_PRINT_TEST, "\n     -------------------------------------------------------\n", 0);
  val_print(ACS_PRINT_TEST, "     Total Tests run  = %4d", g_bsa_tests_total);
  val_print(ACS_PRINT_TEST, "  Tests Passed  = %4d", g_bsa_tests_pass);
  val_print(ACS_PRINT_TEST, "  Tests Failed = %4d\n", g_bsa_tests_fail);
  val_print(ACS_PRINT_TEST, "     -------------------------------------------------------\n", 0);

  freeBsaAcsMem();

  if (g_dtb_log_file_handle) {
    ShellCloseFile(&g_dtb_log_file_handle);
  }

  val_print(ACS_PRINT_TEST, "\n      *** BSA tests complete. Reset the system. ***\n\n", 0);

  if (g_bsa_log_file_handle) {
    ShellCloseFile(&g_bsa_log_file_handle);
  }

  val_pe_context_restore(AA64WriteSp(g_stack_pointer));

  return(0);
}
