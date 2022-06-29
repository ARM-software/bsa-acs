/** @file
 * Copyright (c) 2016-2022, Arm Limited or its affiliates. All rights reserved.
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


UINT32  g_print_level;
UINT32  g_sw_view[3] = {1, 1, 1}; //Operating System, Hypervisor, Platform Security
UINT32  g_skip_test_num[9] = {10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000};
UINT32  g_bsa_tests_total;
UINT32  g_bsa_tests_pass;
UINT32  g_bsa_tests_fail;
UINT64  g_stack_pointer;
UINT64  g_exception_ret_addr;
UINT64  g_ret_addr;
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
    Print(L"Allocate Pool failed %x \n", Status);
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
    Print(L"Allocate Pool failed %x \n", Status);
    return Status;
  }

  Status = val_gic_create_info_table(GicInfoTable);

  return Status;

}

EFI_STATUS
configureGicIts (
)
{
  EFI_STATUS Status;

  Status = val_gic_its_configure();

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
    Print(L"Allocate Pool failed %x \n", Status);
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
    Print(L"Allocate Pool failed %x \n", Status);
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
    Print(L"Allocate Pool failed %x \n", Status);
    return Status;
  }
  val_pcie_create_info_table(PcieInfoTable);

  Status = gBS->AllocatePool (EfiBootServicesData,
                              IOVIRT_INFO_TBL_SZ,
                              (VOID **) &IoVirtInfoTable);

  if (EFI_ERROR(Status))
  {
    Print(L"Allocate Pool failed %x \n", Status);
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
    Print(L"Allocate Pool failed %x \n", Status);
    return Status;
  }
  val_peripheral_create_info_table(PeripheralInfoTable);

  Status = gBS->AllocatePool (EfiBootServicesData,
                              MEM_INFO_TBL_SZ,
                              (VOID **) &MemoryInfoTable);

  if (EFI_ERROR(Status))
  {
    Print(L"Allocate Pool failed %x \n", Status);
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
  Print (L"\nUsage: Bsa.efi [-v <n>] | [-f <filename>] | [-skip <n>]\n"
         "Options:\n"
         "-v      Verbosity of the prints\n"
         "        1 prints all, 5 prints only the errors\n"
         "-f      Name of the log file to record the test results in\n"
         "-skip   Test(s) to be skipped\n"
         "        Refer to section 4 of BSA ACS User Guide\n"
         "        To skip a module, use Module ID as mentioned in user guide\n"
         "        To skip a particular test within a module, use the exact testcase number\n"
         "-os     Enable the execution of operating system tests\n"
         "-hyp    Enable the execution of hypervisor tests\n"
         "-ps     Enable the execution of platform security tests\n"
         "-dtb    Enable the execution of dtb dump\n"
  );
}

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
  {L"-v", TypeValue},    // -v    # Verbosity of the Prints. 1 shows all prints, 5 shows Errors
  {L"-f", TypeValue},    // -f    # Name of the log file to record the test results in.
  {L"-skip", TypeValue}, // -skip # test(s) to skip execution
  {L"-help", TypeFlag},  // -help # help : info about commands
  {L"-h", TypeFlag},     // -h    # help : info about commands
  {L"-os", TypeFlag},    // -os   # Binary Flag to enable the execution of operating system tests.
  {L"-hyp", TypeFlag},   // -hyp  # Binary Flag to enable the execution of hypervisor tests.
  {L"-ps", TypeFlag},    // -ps   # Binary Flag to enable the execution of platform security tests.
  {L"-dtb", TypeValue},  // -dtb  # Binary Flag to enable dtb dump
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
  UINT32             i,j=0;
  VOID               *branch_label;


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
      if (CmdLineArg == NULL) {
        Print(L" No valid test number or module number specified for -skip\n");
      } else {
        for (i = 0 ; i < StrLen(CmdLineArg) ; i++) {
          g_skip_test_num[0] = StrDecimalToUintn((CONST CHAR16 *)(CmdLineArg+0));
            if (*(CmdLineArg+i) == L',') {
              g_skip_test_num[++j] = StrDecimalToUintn((CONST CHAR16 *)(CmdLineArg+i+1));
          }
        }
      }
  }

    // Options with Values
  CmdLineArg  = ShellCommandLineGetValue (ParamPackage, L"-v");
  if (CmdLineArg == NULL) {
    g_print_level = G_PRINT_LEVEL;
  } else {
    g_print_level = StrDecimalToUintn(CmdLineArg);
    if (g_print_level > 5) {
      g_print_level = G_PRINT_LEVEL;
    }
  }

  // Options with Values
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

  val_print(ACS_PRINT_TEST, "\n      ***  Starting PE tests ***  ", 0);
  Status = val_pe_execute_tests(val_pe_get_num(), g_sw_view);

  val_print(ACS_PRINT_TEST, "\n      ***  Starting Memory Map tests ***  ", 0);
  val_memory_execute_tests(val_pe_get_num(), g_sw_view);

  /*
   * Configure Gic Redistributor and ITS to support
   * Generation of LPIs.
  */
  configureGicIts();

  val_print(ACS_PRINT_TEST, "\n      ***  Starting GIC tests ***  ", 0);
  Status |= val_gic_execute_tests(val_pe_get_num(), g_sw_view);

  val_print(ACS_PRINT_TEST, "\n      *** Starting System MMU tests ***  ", 0);
  Status |= val_smmu_execute_tests(val_pe_get_num(), g_sw_view);

  val_print(ACS_PRINT_TEST, "\n      *** Starting Timer tests ***  ", 0);
  Status |= val_timer_execute_tests(val_pe_get_num(), g_sw_view);

  val_print(ACS_PRINT_TEST, "\n      *** Starting Power and Wakeup semantic tests ***  ", 0);
  Status |= val_wakeup_execute_tests(val_pe_get_num(), g_sw_view);

  val_print(ACS_PRINT_TEST, "\n      *** Starting Peripheral tests ***  ", 0);
  Status |= val_peripheral_execute_tests(val_pe_get_num(), g_sw_view);

  val_print(ACS_PRINT_TEST, "\n      *** Starting Watchdog tests ***  ", 0);
  Status |= val_wd_execute_tests(val_pe_get_num(), g_sw_view);

  val_print(ACS_PRINT_TEST, "\n      *** Starting PCIe tests ***  ", 0);
  Status |= val_pcie_execute_tests(val_pe_get_num(), g_sw_view);

  val_print(ACS_PRINT_TEST, "\n      *** Starting PCIe Exerciser tests ***  ", 0);
  Status |= val_exerciser_execute_tests(g_sw_view);

print_test_status:
  val_print(ACS_PRINT_TEST, "\n     ------------------------------------------------------- \n", 0);
  val_print(ACS_PRINT_TEST, "     Total Tests run  = %4d", g_bsa_tests_total);
  val_print(ACS_PRINT_TEST, "  Tests Passed  = %4d", g_bsa_tests_pass);
  val_print(ACS_PRINT_TEST, "  Tests Failed = %4d\n", g_bsa_tests_fail);
  val_print(ACS_PRINT_TEST, "     ------------------------------------------------------- \n", 0);

  freeBsaAcsMem();

  if (g_bsa_log_file_handle) {
    ShellCloseFile(&g_bsa_log_file_handle);
  }

  if (g_dtb_log_file_handle) {
    ShellCloseFile(&g_dtb_log_file_handle);
  }

  val_print(ACS_PRINT_TEST, "\n      *** BSA tests complete. Reset the system. *** \n\n", 0);

  val_pe_context_restore(AA64WriteSp(g_stack_pointer));

  return(0);
}
