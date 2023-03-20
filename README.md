# Base System Architecture - Architecture Compliance Suite


## Base System Architecture
**Base System Architecture** (BSA) specification describes a hardware system architecture based on the Arm 64-bit architecture. System software such as operating systems, hypervisors, and firmware rely on this. It addresses PE features and key aspects of system architecture.

For more information, see [BSA specification](https://developer.arm.com/documentation/den0094/c/?lang=en)


## BSA - Architecture Compliance Suite

BSA **Architecture Compliance Suite** (ACS) is a collection of self-checking, portable C-based tests.
This suite includes a set of examples of the invariant behaviors that are provided by the [BSA](https://developer.arm.com/documentation/den0094/c/?lang=en) specification, so that you can verify if these behaviour have been interpreted correctly.
Most of the tests are executed from UEFI (Unified Extensible Firmware Interface) Shell by executing the BSA UEFI shell application.
A few tests are executed by running the BSA ACS Linux application which in turn depends on the BSA ACS Linux kernel module.


## Release details
 - Code quality: v1.0.4
 - The tests are written for version 1.0 of the BSA specification.
 - The compliance suite is not a substitute for design verification.
 - To review the BSA ACS logs, Arm licensees can contact Arm directly through their partner managers.


## GitHub branch
  - To pick up the release version of the code, checkout the corresponding tag from the main branch.
  - To get the latest version of the code with bug fixes and new features, use the main branch.

## Additional reading
  - For information on the test scenarios currently implemented for platform using Device tree, see [Scenario Document](docs/Arm_Base_System_Architecture_Scenario_IR.pdf).
  - For information on the test scenarios currently implemented for platform using ACPI table, see [Scenario Document](docs/Arm_Base_System_Architecture_Scenario_ES.pdf).

## BSA ACS Baremetal Reference Code
Bare-metal reference code is added as part of this release. For more information, see
  - [Arm BSA ACS Bare-metal User Guide](platform/pal_baremetal/docs/Arm_BSA_ACS_Bare-metal_User_Guide.pdf).
  - [Bare-metal Code](platform/pal_baremetal/). <br />
Note: The Baremetal PCIe enumeration code provided as part of the BSA ACS should be used and should not be replaced. This code is vital in analyzing of the test result.

## ACS build steps - UEFI Shell application

### Prebuilt images
Prebuilt images for each release are available in the prebuilt_images folder of the main branch. You can choose to use these images or build your own image by following the steps below. If you choose to use the prebuilt image, see the Test suite execution section below for details on how to run the application.

### 1. Building from source
    Before you start the ACS build, ensure that the following requirements are met.

- Any mainstream Linux-based OS distribution running on a x86 or AArch64 machine.
- git clone the [EDK2 tree](https://github.com/tianocore/edk2). Recommended edk2 tag is edk2-stable202208
- git clone the [EDK2 port of libc](https://github.com/tianocore/edk2-libc) to local <edk2_path>.
- GCC 7.5 or a later toolchain for Linux from [here](https://releases.linaro.org/components/toolchain/binaries/).
- Install the build prerequisite packages to build EDK2.<br />
Note:<br />
- The details of the packages are beyond the scope of this document.
- GCC 7.5 is recommended toolchain, build issues are observed with toolchain version 10.xx and above.

#### 1.1 Target Platform
##### To start the ACS build for platform using ACPI table, perform the following steps:

1.  cd local_edk2_path
2.  git submodule update --init --recursive
3.  git clone https://github.com/ARM-software/bsa-acs.git ShellPkg/Application/bsa-acs
4.  Add the following to the [LibraryClasses.common] section in ShellPkg/ShellPkg.dsc
>          BsaValLib|ShellPkg/Application/bsa-acs/val/BsaValLib.inf
>          BsaPalLib|ShellPkg/Application/bsa-acs/platform/pal_uefi_acpi/BsaPalLib.inf

5.  Add the following in the [components] section of ShellPkg/ShellPkg.dsc
>          ShellPkg/Application/bsa-acs/uefi_app/BsaAcs.inf

##### To start the ACS build for platform using Device tree, perform the following steps:

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

####    1.2 Build environment
##### If the build environment is Linux, perform the following steps:
1.  export GCC49_AARCH64_PREFIX= GCC7.5 toolchain path pointing to /bin/aarch64-linux-gnu- in case of x86 machine.<br /> For an AArch64 build it should point to /usr/bin/
2.  export PACKAGES_PATH= path pointing to edk2-libc
3.  source edksetup.sh
4.  make -C BaseTools/Source/C
5.  source ShellPkg/Application/bsa-acs/tools/scripts/acsbuild.sh

#### 1.3 Build output

The EFI executable file is generated at <edk2_path>/Build/Shell/DEBUG_GCC49/AARCH64/Bsa.efi


### 2. Test suite execution


#### Prerequisites
- If the system supports LPIs (Interrupt ID > 8192) then Firmware should support installation of handler for LPI interrupts.
    - If you are using edk2, change the ArmGic driver in the ArmPkg to support installation of handler for LPIs.
    - Add the following in edk2/ArmPkg/Drivers/ArmGic/GicV3/ArmGicV3Dxe.c
>        - After [#define ARM_GIC_DEFAULT_PRIORITY  0x80]
>          +#define ARM_GIC_MAX_NUM_INTERRUPT 16384
>        - Change this in GicV3DxeInitialize function.
>          -mGicNumInterrupts      = ArmGicGetMaxNumInterrupts (mGicDistributorBase);
>          +mGicNumInterrupts      = ARM_GIC_MAX_NUM_INTERRUPT;

The execution of the compliance suite varies depending on the test environment. The following steps assume that the test suite is invoked through the ACS UEFI shell application.

#### 2.1 Silicon

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

#### 2.2 Emulation environment with secondary storage
On an emulation environment with secondary storage, perform the following steps:

1. Create an image file which contains the 'Bsa.efi' file. For example:
  - mkfs.vfat -C -n HD0 hda.img 2097152
  - sudo mount -o rw,loop=/dev/loop0,uid=\`whoami\`,gid=\`whoami\` hda.img /mnt/bsa
    In case loop0 is busy, please specify the one that is free.
  - sudo cp  "<path to application>/Bsa.efi" /mnt/bsa/
  - sudo umount /mnt/bsa
2. Load the image file to the secondary storage using a backdoor. The steps to load the image file are emulation environment-specific and beyond the scope of this document.
3. Boot the system to UEFI shell.
4. To determine the file system number of the secondary storage, execute 'map -r' command.
5. Type 'fs<x>' where '<x>' is replaced by the number determined in step 4.
6. To start the compliance tests, run the executable Bsa.efi with the appropriate parameters.
7. Copy the UART console output to a log file for analysis and certification.

  - For information on the BSA uefi shell application parameters, see the [User Guide](docs/Arm_Base_System_Architecture_Compliance_User_Guide.pdf).


#### 2.3 Emulation environment without secondary storage

On an emulation platform where secondary storage is not available, perform the following steps:

1. Add the path to 'Bsa.efi' file in the UEFI FD file.
2. Build UEFI image including the UEFI Shell.
3. Boot the system to UEFI shell.
4. Run the executable 'Bsa.efi' to start the compliance tests. For details about the parameters,see the [User Guide](docs/Arm_Base_System_Architecture_Compliance_User_Guide.pdf).
5. Copy the UART console output to a log file for analysis and certification.


## ACS build steps - Linux application

Certain Peripheral, PCIe and Memory map tests require Linux operating system.This chapter provides information on building and executing these tests from the Linux application.

### 1. Build steps and environment setup
This section lists the porting and build steps for the kernel module.
The patch for the kernel tree and the Linux PAL are hosted separately on [linux-acs](https://git.gitlab.arm.com/linux-arm/linux-acs.git) repo

### 1.1 Building the kernel module
#### Prerequisites
- Linux kernel source version 5.11, 5.13, 5.15, 6.0.
- Linaro GCC tool chain 7.5 or above.
- Build environment for AArch64 Linux kernel.<br />
NOTE: <br />
- Linux version 6.0 is recommened version.
- GCC 7.5 is recommended toolchain, build issues are observed with toolchain version 10.xx and above.

#### Porting steps for Linux kernel
1. git clone https://git.gitlab.arm.com/linux-arm/linux-acs.git bsa-acs-drv
2. git clone https://github.com/ARM-software/bsa-acs.git bsa-acs
3. git clone https://github.com/torvalds/linux.git -b v6.0
4. export CROSS_COMPILE=<GCC7.5 toolchain path> pointing to /bin/aarch64-linux-gnu-
5. git apply <local_dir>/bsa-acs-drv/kernel/src/0001-BSA-ACS-Linux-6.0.patch to your kernel source tree.
6. make ARCH=arm64 defconfig && make -j $(nproc) ARCH=arm64

NOTE: The steps mentions Linux version 6.0, as it is latest version which is verified at ACS end.

#### 1.2 Build steps for BSA kernel module
1. cd <local_dir>/bsa-acs-drv/files
2. export CROSS_COMPILE=<ARM64 toolchain path>/bin/aarch64-linux-gnu-
3. export KERNEL_SRC=<linux kernel path>
4. ./setup.sh <local_dir/bsa-acs>
5. ./linux_bsa_acs.sh

Successful completion of above steps will generate bsa_acs.ko

#### 1.3 BSA Linux application build
1. cd <bsa-acs path>/linux_app/bsa-acs-app
2. export CROSS_COMPILE=<ARM64 toolchain path>/bin/aarch64-linux-gnu-
3. make

Successful completion of above steps will generate executable file bsa

### 2. Loading the kernel module
Before the BSA ACS Linux application can be run, load the BSA ACS kernel module using the insmod command.
```sh
shell> insmod bsa_acs.ko
```

### 3. Running BSA ACS
```sh
shell> ./bsa
```
  - For information on the BSA Linux application parameters, see the [User Guide](docs/Arm_Base_System_Architecture_Compliance_User_Guide.pdf).

## Security implication
The Arm SystemReady ACS test suite may run at a higher privilege level. An attacker may utilize these tests to elevate the privilege which can potentially reveal the platform security assets. To prevent the leakage of Secure information, Arm strongly recommends that you run the ACS test suite only on development platforms. If it is run on production systems, the system should be scrubbed after running the test suite.

## Limitations

 - ITS rules are available only for systems that present firmware compliant to SBBR.
 - Some PCIe and Exerciser test are dependent on PCIe features supported by the test system.
   Please fill the required API's with test system information.
   - pal_pcie_p2p_support : If the test system PCIe supports peer to peer transaction.
   - pal_pcie_is_cache_present : If the test system supports PCIe address translation cache.
   - pal_pcie_get_legacy_ir_map : Fill system legacy ir map
   Below exerciser capabilities are required by exerciser test.
   - MSI-X interrupt generation.
   - Incoming Transaction Monitoring(order, type).
   - Initiating transacions from and to the exerciser.
   - Ability to check on BDF and register address seen for each configuration address along with access type.

### BSA ACS version mapping
--------------------------------------------------------------------------------------------
|    BSA Spec Version   |   BSA ACS Version   |      BSA Tag ID     |    Pre-Si Support    |
|-----------------------|:-------------------:|:-------------------:|:--------------------:|
|       BSA v1.0        |        v1.0.4       |   v23.03_REL1.0.4   |       Yes            |
|       BSA v1.0        |        v1.0.3       |   v23.01_REL1.0.3   |       No             |
|       BSA v1.0        |        v1.0.2       |   v22.10_REL1.0.2   |       No             |
|       BSA v1.0        |        v1.0.1       |   v22.06_REL1.0.1   |       No             |
|       BSA v1.0        |        v1.0         |   v21.09_REL1.0     |       No             |
--------------------------------------------------------------------------------------------

## License
BSA ACS is distributed under Apache v2.0 License.


## Feedback, contributions, and support

 - For feedback, use the GitHub Issue Tracker that is associated with this repository.
 - For support, send an email to "support-systemready-acs@arm.com" with details.
 - Arm licensees may contact Arm directly through their partner managers.
 - Arm welcomes code contributions through GitHub pull requests. See the GitHub documentation on how to raise pull requests.

--------------

*Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.*
