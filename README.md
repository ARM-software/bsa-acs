# Base System Architecture - Architecture Compliance Suite

[![BSA-ACS UEFI Build](https://github.com/ARM-software/bsa-acs/actions/workflows/build-bsa-uefi.yml/badge.svg?event=schedule)](https://github.com/ARM-software/bsa-acs/actions/workflows/build-bsa-uefi.yml)

## Base System Architecture
**Base System Architecture** (BSA) specification describes a hardware system architecture based on the Arm 64-bit architecture. System software such as operating systems, hypervisors, and firmware rely on this. It addresses PE features and key aspects of system architecture.

For more information, download the [BSA specification](https://developer.arm.com/documentation/den0094/c/?lang=en)


## BSA - Architecture Compliance Suite

BSA **Architecture Compliance Suite** (ACS) is a collection of self-checking, portable C-based tests.
This suite includes a set of examples of the invariant behaviors that are provided by the [BSA](https://developer.arm.com/documentation/den0094/c/?lang=en) specification, so that you can verify if these behaviour have been interpreted correctly.

Most of the tests are executed from UEFI (Unified Extensible Firmware Interface) Shell by executing the BSA UEFI shell application.
A few tests are executed by running the BSA ACS Linux application which in turn depends on the BSA ACS Linux kernel module.
The tests can also be executed in a Bare-metal environment. The initialization of the Bare-metal environment is specific to the environment and is out of scope of this document.

## Release details
 - Code quality: v1.0.7
 - The tests are written for version 1.0 (c) of the BSA specification.
 - The tests can be run at both the Pre-Silicon and Silicon level.
 - For complete coverage of the BSA rules, availability of an Exerciser is required for Exerciser tests to be run during verficiation at Pre-Silicon level.
 - The compliance suite is not a substitute for design verification.
 - To review the BSA ACS logs, Arm licensees can contact Arm directly through their partner managers.
 - To know about the BSA rules not implemented in this release, see the [Test Scenario Document](docs/arm_bsa_architecture_compliance_test_scenario.pdf).

## GitHub branch
  - To pick up the release version of the code, checkout the corresponding tag from the main branch.
  - To get the latest version of the code with bug fixes and new features, use the main branch.

## Additional reading
  - For information about the implementable BSA rules test algorithm and for unimplemented BSA rules, see the [arm BSA Test Scenario Document](docs/arm_bsa_architecture_compliance_test_scenario.pdf).
  - For information on test category(UEFI, Linux, Bare-metal) and applicable systems(IR,ES,SR,Pre-Silicon), see the [arm BSA Test Checklist](docs/arm_bsa_testcase_checklist.rst).
  - For details on the design of the BSA ACS, see the [arm BSA Validation Methodology Document](docs/arm_bsa_architecture_compliance_validation_methodology.pdf).
  - For details on the BSA ACS UEFI Shell Application and Linux Application see the [arm BSA ACS User Guide](docs/arm_bsa_architecture_compliance_user_guide.pdf).
  - For details on the BSA ACS Bare-metal support, see the
    - [arm BSA ACS Bare-metal User Guide](docs/arm_bsa_architecture_compliance_bare-metal_user_guide.pdf).
    - [Bare-metal Code](platform/pal_baremetal/). <br />
Note: The Bare-metal PCIe enumeration code provided as part of the BSA ACS should be used and should not be replaced. This code is vital in analyzing of the test result.

### Running Exerciser tests for complete coverage

Exerciser is a client device wrapped up by PCIe Endpoint. This device is created to meet BSA requirements for various PCIe capability validation tests. Running the Exerciser tests provides additional test coverage on the platform.

Note: To run the exerciser tests on a UEFI Based platform with Exerciser, the Exerciser PAL API's need to be implemented. For details on the reference Exerciser implementation and support, see the [Exerciser.md](docs/PCIe_Exerciser/Exerciser.md) and [Exerciser_API_porting_guide.md](docs/PCIe_Exerciser/Exerciser_API_porting_guide.md)

## ACS build steps - UEFI Shell application

### Prebuilt images
Prebuilt images for each release are available in the prebuilt_images folder of the main branch. You can choose to use these images or build your own image by following the steps below. If you choose to use the prebuilt image, see the Test suite execution section below for details on how to run the application.

### 1. Building from source
    Before you start the ACS build, ensure that the following requirements are met.

- Any mainstream Linux-based OS distribution running on a x86 or AArch64 machine.
- git clone the [EDK2 tree](https://github.com/tianocore/edk2). Recommended edk2 tag is edk2-stable202302
- git clone the [EDK2 port of libc](https://github.com/tianocore/edk2-libc) to local <edk2_path>.
- Install GCC-ARM 10.3 [toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads).
- Install the build prerequisite packages to build EDK2.<br />
Note:<br />
- The details of the packages are beyond the scope of this document.

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
1.  export GCC49_AARCH64_PREFIX= GCC10.3 toolchain path pointing to /bin/aarch64-linux-gnu- in case of x86 machine.<br /> For an AArch64 build it should point to /usr/bin/
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

  - For information on the BSA uefi shell application parameters, see the [User Guide](docs/arm_bsa_architecture_compliance_user_guide.pdf).


#### 2.3 Emulation environment without secondary storage

On an emulation platform where secondary storage is not available, perform the following steps:

1. Add the path to 'Bsa.efi' file in the UEFI FD file.
2. Build UEFI image including the UEFI Shell.
3. Boot the system to UEFI shell.
4. Run the executable 'Bsa.efi' to start the compliance tests. For details about the parameters,see the [User Guide](docs/arm_bsa_architecture_compliance_user_guide.pdf).
5. Copy the UART console output to a log file for analysis and certification.


## ACS build steps - Linux application

Certain Peripheral, PCIe and Memory map tests require Linux operating system.This chapter provides information on building and executing these tests from the Linux application.

### 1. Build steps and environment setup
This section lists the porting and build steps for the kernel module.
The patch for the kernel tree and the Linux PAL are hosted separately on [linux-acs](https://git.gitlab.arm.com/linux-arm/linux-acs.git) repo

### 1.1 Building the kernel module
#### Prerequisites
- Linux kernel source version 5.11, 5.13, 5.15, 6.0, 6.4.
- Install GCC-ARM 10.3 [toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads).
- Build environment for AArch64 Linux kernel.<br />
NOTE: <br />
- Linux version 6.4 is recommened version.

#### Porting steps for Linux kernel
1. git clone https://git.gitlab.arm.com/linux-arm/linux-acs.git bsa-acs-drv
2. git clone https://github.com/ARM-software/bsa-acs.git bsa-acs
3. git clone https://github.com/torvalds/linux.git -b v6.4
4. export CROSS_COMPILE=<GCC10.3 toolchain path> pointing to /bin/aarch64-linux-gnu-
5. git apply <local_dir>/bsa-acs-drv/kernel/src/0001-BSA-ACS-Linux-6.4.patch to your kernel source tree.
6. make ARCH=arm64 defconfig && make -j $(nproc) ARCH=arm64

NOTE: The steps mentions Linux version 6.4, as it is latest version which is verified at ACS end.

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
  - For information on the BSA Linux application parameters, see the [User Guide](docs/arm_bsa_architecture_compliance_user_guide.pdf).

## ACS build steps - Bare-metal abstraction

The Bare-metal build environment is platform specific.

To provide a baseline, the build steps to integrate and run the Bare-metal tests from UEFI shell are provided in [README.md](platform/pal_baremetal/RDN2/README.md).

For details on generating the binaries to run on Bare-metal environment, refer [README.md](platform/pal_baremetal/README.md)

## Security implication
The Arm SystemReady ACS test suite may run at a higher privilege level. An attacker may utilize these tests to elevate the privilege which can potentially reveal the platform security assets. To prevent the leakage of Secure information, Arm strongly recommends that you run the ACS test suite only on development platforms. If it is run on production systems, the system should be scrubbed after running the test suite.

## Limitations

 - For systems that present firmware compliant to SBBR, BSA depends on SPCR acpi table to get UART information.
   UEFI console setting must be set to "serial" on these systems.
 - ITS test are available only for systems that present firmware compliant to SBBR.
 - Some PCIe and Exerciser test are dependent on PCIe features supported by the test system.
   Please fill the required API's with test system information.
   - pal_pcie_p2p_support : If the test system PCIe supports peer to peer transaction.
   - pal_pcie_is_cache_present : If the test system supports PCIe address translation cache.
   - pal_pcie_get_legacy_irq_map : Fill system legacy irq map

   Below exerciser capabilities are required by exerciser test.
   - MSI-X interrupt generation.
   - Incoming Transaction Monitoring(order, type).
   - Initiating transacions from and to the exerciser.
   - Ability to check on BDF and register address seen for each configuration address along with access type.

### BSA ACS version mapping
--------------------------------------------------------------------------------------------
|    BSA Spec Version   |   BSA ACS Version   |      BSA Tag ID     |    Pre-Si Support    |
|-----------------------|:-------------------:|:-------------------:|:--------------------:|
|       BSA v1.0(c)     |        v1.0.7       |   v23.12_REL1.0.7   |       Yes            |
|       BSA v1.0(c)     |        v1.0.6       |v23.11_BootFramework |       Yes            |
|       BSA v1.0(c)     |        v1.0.6       |   v23.09_REL1.0.6   |       Yes            |
|       BSA v1.0(c)     |        v1.0.5       |   v23.07_REL1.0.5   |       Yes            |
|       BSA v1.0(c)     |        v1.0.4       |   v23.03_REL1.0.4   |       Yes            |
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
