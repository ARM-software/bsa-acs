# Base System Architecture - Architecture Compliance Suite


## Base System Architecture
**Base System Architecture** (BSA) specification describes a hardware system architecture based on the Arm 64-bit architecture. System software such as operating systems, hypervisors, and firmware rely on this. It addresses PE features and key aspects of system architecture.

For more information, download the [BSA specification](https://developer.arm.com/documentation/den0094/latest)


## BSA - Architecture Compliance Suite

BSA **Architecture Compliance Suite** (ACS) is a collection of self-checking, portable C-based tests.
This suite includes a set of examples of the invariant behaviors that are provided by the [BSA](https://developer.arm.com/documentation/den0094/latest) specification, so that you can verify if these behaviour have been interpreted correctly.
Most of the tests are executed from UEFI Shell by executing the BSA UEFI shell application.
A few tests are executed by running the BSA ACS Linux application which in turn depends on the BSA ACS Linux kernel module.


## Release details
 - Code quality: v0.5 Alpha
 - The tests are written for version 1.0 of the BSA specification.
 - The compliance suite is not a substitute for design verification.
 - To review the BSA ACS logs, Arm licensees can contact Arm directly through their partner managers.


## GitHub branch
  - To pick up the release version of the code, checkout the coresponding tag from main branch.
  - To get the latest version of the code with bug fixes and new features, use the main branch.

## Additional reading
  - For information on the test scenarios currently implemented for IR, see [Scenario Document](docs/Arm_Base_System_Architecture_Scenario_IR.pdf).

## ACS build steps - UEFI Shell application

### Prebuilt images
Prebuilt images for each release are available in the prebuilt_images folder of the release branch. You can choose to use these images or build your own image by following the steps below. If you choose to use the prebuilt image, see the Test suite execution section below for details on how to run the application.

### Prerequisites
    Before you start the ACS build, ensure that the following requirements are met.

- Any mainstream Linux-based OS distribution running on a x86 or AArch64 machine.
- git clone the edk2-stable202102 tag of [EDK2 tree](https://github.com/tianocore/edk2).
- git clone the [EDK2 port of libc](https://github.com/tianocore/edk2-libc) to local <edk2_path>.
- Install GCC 7.5 or a later toolchain for Linux from [here](https://releases.linaro.org/components/toolchain/binaries/).
- Install the build prerequisite packages to build EDK2.
Note: The details of the packages are beyond the scope of this document.

To start the ACS build for IR, perform the following steps:

1.  cd local_edk2_path
2.  git submodule update --init --recursive
3.  git clone https://github.com/ARM-software/bsa-acs.git ShellPkg/Application/bsa-acs
4.  Add the following to the [LibraryClasses.common] section in ShellPkg/ShellPkg.dsc
>          FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf
>          BsaValLib|ShellPkg/Application/bsa-acs/val/BsaValLib.inf
>          BsaPalLib|ShellPkg/Application/bsa-acs/platform/pal_uefi_dt/BsaPalLib.inf

5.  Add the following in the [components] section of ShellPkg/ShellPkg.dsc
>          ShellPkg/Application/bsa-acs/uefi_app/BsaAcs.inf

6.  In IR systems, ACS efi application runs on top of efi shell which runs on u-boot as efi payload.
   - Below change in edk2 code MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.c is required before compiling for IR system.
>          -Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &gHiiConfigRouting);
>          -ASSERT_EFI_ERROR (Status);
>          +//Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &gHiiConfigRouting);
>          +//ASSERT_EFI_ERROR (Status);

### Linux build environment
If the build environment is Linux, perform the following steps:
1.  export GCC49_AARCH64_PREFIX= GCC7.5 toolchain path pointing to /bin/aarch64-linux-gnu- in case of x86 machine. For an AArch64 build it should point to /usr/bin/
2.  export PACKAGES_PATH= path pointing to edk2-libc
3.  source edksetup.sh
4.  make -C BaseTools/Source/C
5.  source ShellPkg/Application/bsa-acs/tools/scripts/acsbuild.sh

### Build output

The EFI executable file is generated at <edk2_path>/Build/Shell/DEBUG_GCC49/AARCH64/Bsa.efi


## Test suite execution

The execution of the compliance suite varies depending on the test environment. The below steps assume that the test suite is invoked through the ACS UEFI shell application.


### Prerequisites
- If the system supports LPIs (Interrupt ID > 8192) then Firmware should support installation of handler for LPI interrupts.
    - If you are using edk2, change the ArmGic driver in the ArmPkg to support installation of handler for LPIs.
    - Add the following in edk2/ArmPkg/Drivers/ArmGic/GicV3/ArmGicV3Dxe.c
>        - After [#define ARM_GIC_DEFAULT_PRIORITY  0x80]
>          +#define ARM_GIC_MAX_NUM_INTERRUPT 16384
>        - Change this in GicV3DxeInitialize function.
>          -mGicNumInterrupts      = ArmGicGetMaxNumInterrupts (mGicDistributorBase);
>          +mGicNumInterrupts      = ARM_GIC_MAX_NUM_INTERRUPT;

### Silicon
On a system where a USB port is available and functional, perform the following steps:

#### For IR Systems:
1. Copy 'Bsa.efi' and 'Shell.efi' to a USB Flash drive.
2. Boot the system to U-Boot shell.
3. Plug in the USB flash drive to one of the functional USB ports on the system.
4. To determine the file system number of the plugged-in USB drive, execute command <br />
   `usb start`
5. Copy the 'Shell.efi' to memory location using the command <br />
   `Syntax: fatload usb <dev_num> ${kernel_addr_r} Shell.efi` <br />
   `Eg: fatload usb 0 ${kernel_addr_r} Shell.efi` <br />
   This boots the system to UEFI Shell.
6. To determine the file system number of the plugged-in USB drive, execute 'map -r' command.
7. Type 'fs<x>' where '<x>' is replaced by the number determined in step 5.
8. To start the compliance tests, run the executable Bsa.efi with the appropriate parameters.
9. Copy the UART console output to a log file.
Note: 'Shell.efi' is available in the [pebuilt_images/IR](prebuilt_images)

### Emulation environment with secondary storage
On an emulation environment with secondary storage, perform the following steps:

1. Create an image file which contains the 'Bsa.efi' file. For example:
  - mkfs.vfat -C -n HD0 hda.img 2097152
  - sudo mount -o rw,loop=/dev/loop0,uid=`whoami`,gid=`whoami` hda.img /mnt/bsa
  - sudo cp  "<path to application>/Bsa.efi" /mnt/bsa/
  - sudo umount /mnt/bsa
2. Load the image file to the secondary storage using a backdoor. The steps to load the image file are emulation environment-specific and beyond the scope of this document.
3. Boot the system to UEFI shell.
4. To determine the file system number of the secondary storage, execute 'map -r' command.
5. Type 'fsx' where 'x' is replaced by the number determined in step 4.
6. To start the compliance tests, run the executable Bsa.efi with the appropriate parameters.
7. Copy the UART console output to a log file for analysis and certification.


### Emulation environment without secondary storage

On an emulation platform where secondary storage is not available, perform the following steps:

1. Add the path to 'Bsa.efi' file in the UEFI FD file.
2. Build UEFI image including the UEFI Shell.
3. Boot the system to UEFI shell.
4. Run the executable 'Bsa.efi' to start the compliance tests. For details about the parameters,
5. Copy the UART console output to a log file for analysis and certification.


## Linux OS-based tests
Certain Peripheral, PCIe and Memory map tests require Linux operating system with kernel version 5.10 or above.
This chapter provides information on executing tests from the Linux application.

### Build steps and environment setup
This section lists the porting and build steps for the kernel module.
The patch for the kernel tree and the Linux PAL are hosted separately on [linux-acs](https://gitlab.arm.com/linux-arm/linux-acs) repo

### Building the kernel module
#### Prerequisites
- Linux kernel source version 5.10.
- Linaro GCC tool chain 7.5 or above.
- Build environment for AArch64 Linux kernel.

#### Porting steps for Linux kernel
1. git clone https://git.gitlab.arm.com/linux-arm/linux-acs.git bsa-acs-drv
2. git clone https://github.com/ARM-software/bsa-acs.git bsa-acs
3. git clone https://github.com/torvalds/linux.git -b v5.10
4. export CROSS_COMPILE=<GCC7.5 toolchain path> pointing to /bin/aarch64-linux-gnu-
5. git apply <local_dir>/bsa-acs-drv/kernel/src/0001-BSA-SBSA-ACS-Linux-5.10.patch to your kernel source tree.
6. make ARCH=arm64 defconfig && make -j $(nproc) ARCH=arm64

#### Build steps for BSA kernel module
1. cd <local_dir>/bsa-acs-drv/files
2. export CROSS_COMPILE=<ARM64 toolchain path>/bin/aarch64-linux-gnu-
3. export KERNEL_SRC=<linux kernel path>
4. ./setup.sh <local_dir/bsa-acs>
5. ./linux_bsa_acs.sh
bsa_acs.ko file is generated.

#### BSA Linux application build
1. cd <bsa-acs path>/linux_app/bsa-acs-app
2. export CROSS_COMPILE=<ARM64 toolchain path>/bin/aarch64-linux-gnu-
3. make
The executable file bsa is generated.

## Linux application arguments
Run the Linux application with the following set of arguments
```sh
shell> bsa [--v <n>] [--skip <x,y,z>]
```

| Argument | Description |
| ------ | ------ |
| v | Print level |
|| 1.  INFO and above|
|| 2.  DEBUG and above|
|| 3.  TEST and above|
|| 4.  WARN and ERROR|
|| 5.  ERROR|
|||
| skip | Overrides the suite to skip the execution of a particular test.
|| For example, 53 skips test case with ID 53.|

### Example
```sh
shell> bsa --v 3 --skip 53
```
This set of parameters tests for compliance against BSA with print verbosity set to 3, and skips test number 53.

### Loading the kernel module
Before the BSA ACS Linux application can be run, load the BSA ACS kernel module using the insmod command.
```sh
shell> insmod bsa_acs.ko
```

### Running BSA ACS
```sh
shell> ./bsa
```

## Security implication
The Arm System Ready ACS test suite may run at a higher privilege level. An attacker may utilize these tests to elevate the privilege which can potentially reveal the platform security assets. To prevent the leakage of secure information, Arm strongly recommends that you run the ACS test suite only on development platforms. If it is run on production systems, the system should be scrubbed after running the test suite.

## Limitations
 No known limitations.

## License
BSA ACS is distributed under Apache v2.0 License.


## Feedback, contributions and support

 - For feedback, use the GitHub Issue Tracker that is associated with this repository.
 - For support, please send an email to "support-systemready-acs@arm.com" with details.
 - Arm licensees may contact Arm directly through their partner managers.
 - Arm welcomes code contributions through GitHub pull requests. See GitHub documentation on how to raise pull requests.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
