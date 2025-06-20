### 📢 Repository Notice — Project Restructure

> **Status Update (June 2025)**
> The **BSA-ACS** and **SBSA-ACS** repositories are now ***read-only***.

> Development has moved to the consolidated **[`sysarch-acs`](https://github.com/ARM-software/sysarch-acs)** repository, which hosts the test suites for BSA, SBSA, and future system-standard compliance suites.

| What changed?                      | Where to contribute now?                               |
| ---------------------------------- | ------------------------------------------------------ |
| **Code updates**                   | Open pull requests in **`sysarch-acs`**                |
| **Bug reports / feature requests** | Create GitHub issues in **`sysarch-acs`**              |
| **Open PRs & issues here**         | The ACS team will migrate or close them as appropriate |

We appreciate your cooperation as we streamline our codebase.
For questions, please contact the ACS maintainers or open an issue in **`sysarch-acs`**.

-------------------------------------------------------------------------------------------------------------------

# Base System Architecture - Architecture Compliance Suite

[![BSA-ACS Build Check](https://github.com/ARM-software/bsa-acs/actions/workflows/bsa-acs_build_check.yml/badge.svg)](https://github.com/ARM-software/bsa-acs/actions/workflows/bsa-acs_build_check.yml)

## Base System Architecture
**Base System Architecture** (BSA) specification describes a hardware system architecture based on the Arm 64-bit architecture. System software such as operating systems, hypervisors, and firmware rely on this. It addresses PE features and key aspects of system architecture.

For more information, download the [BSA specification](https://developer.arm.com/documentation/den0094/d/?lang=en)


## BSA - Architecture Compliance Suite

BSA **Architecture Compliance Suite** (ACS) is a collection of self-checking, portable C-based tests.
This suite includes a set of examples of the invariant behaviors that are provided by the [BSA](https://developer.arm.com/documentation/den0094/d/?lang=en) specification, so that you can verify if these behaviour have been interpreted correctly.

Most of the tests are executed from UEFI (Unified Extensible Firmware Interface) Shell by executing the BSA UEFI shell application.
A few tests are executed by running the BSA ACS Linux application which in turn depends on the BSA ACS Linux kernel module.
The tests can also be executed in a Bare-metal environment. The initialization of the Bare-metal environment is specific to the environment and is out of scope of this document.

## Release details
 - Code quality: v1.1.0
 - The tests are written for version 1.1 of the BSA specification.
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
    - [Bare-metal Code](pal/baremetal/). <br />
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
- git clone the [EDK2 tree](https://github.com/tianocore/edk2). Recommended edk2 commit is 836942fbadb629050b866a8052e6af755bcdf623
- git clone the [EDK2 port of libc](https://github.com/tianocore/edk2-libc) to local <edk2_path>.
- Install GCC-ARM 13.2 [toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).
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
>          BsaPalLib|ShellPkg/Application/bsa-acs/pal/uefi_acpi/BsaPalLib.inf

5.  Add the following in the [components] section of ShellPkg/ShellPkg.dsc
>          ShellPkg/Application/bsa-acs/uefi_app/BsaAcs.inf

##### To start the ACS build for platform using Device tree, perform the following steps:

1.  cd local_edk2_path
2.  git submodule update --init --recursive
3.  git clone https://github.com/ARM-software/bsa-acs.git ShellPkg/Application/bsa-acs
4.  Add the following to the [LibraryClasses.common] section in ShellPkg/ShellPkg.dsc
>          FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf
>          BsaValLib|ShellPkg/Application/bsa-acs/val/BsaValLib.inf
>          BsaPalLib|ShellPkg/Application/bsa-acs/pal/uefi_dt/BsaPalLib.inf

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
1.  export GCC49_AARCH64_PREFIX= GCC13.2 toolchain path pointing to /bin/aarch64-linux-gnu- in case of x86 machine.<br /> For an AArch64 build it should point to /usr/bin/
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

#### Prerequisites
ACS build requires that the following requirements are met, Please skip this if you are using [Linux Build Script](https://gitlab.arm.com/linux-arm/linux-acs/-/blob/master/acs-drv/files/build.sh?ref_type=heads)
- Linux kernel source version 5.11, 5.13, 5.15, 6.0, 6.4, 6.7, 6.8
- Install GCC-ARM 13.2 [toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).
- Build environment for AArch64 Linux kernel.<br />
NOTE: <br />
- Linux version 6.8 is recommended version.


#### 1.1 Building the Kernel Module, App (Script)

The following steps describe how to build the BSA kernel module and application using the build.sh script.
The build.sh script supports both native builds and cross-compilation.
- For Native Builds, run the script directly on the target machine.
- For Cross-Compilation, pass the Linux version and GCC tool version as script arguments. 

##### Linux Build Steps (Script)

1. wget https://gitlab.arm.com/linux-arm/linux-acs/-/raw/master/acs-drv/files/build.sh
2. chmod +x build.sh
3. source build.sh

##### Build Output

The following output folder is created in __build__ folder:
 - bsa_acs.ko
 - bsa_app

#### 1.2 Building the Kernel Module, App (Manual)

The following steps describe how to build the BSA kernel module and application for the system manually.

##### Build steps for BSA kernel module
1. git clone https://git.gitlab.arm.com/linux-arm/linux-acs.git linux-acs
2. git clone https://github.com/ARM-software/bsa-acs.git bsa-acs
3. cd <local_dir>/linux-acs/files
4. export CROSS_COMPILE=<ARM64 toolchain path>/bin/aarch64-linux-gnu-
5. export KERNEL_SRC=/lib/modules/$(uname -r)/build
6. ./bsa_setup.sh <local_dir/bsa-acs>
7. ./linux_bsa_acs.sh

__Note:__
- If the path /lib/modules/$(uname -r)/build does not exist on the native system, install the kernel headers using:
```sh
shell> sudo apt-get install linux-headers-$(uname -r)

```
- In case of cross-compilation, the __KERNEL_SRC__ variable must be set to point to the Linux kernel build output directory for the target architecture.


Successful completion of above steps will generate bsa_acs.ko

##### BSA Linux application build
1. cd <bsa-acs path>/linux_app/bsa-acs-app
2. export CROSS_COMPILE=<ARM64 toolchain path>/bin/aarch64-linux-gnu-
3. make

Successful completion of above steps will generate executable file bsa

### 2. Steps for Running BSA Tests in Linux

#### 2.1 Loading the kernel module
Before the BSA ACS Linux application can be run, load the BSA ACS kernel module using the insmod command.
```sh
shell>sudo insmod bsa_acs.ko
```

#### 2.2 Running BSA ACS
```sh
shell> ./bsa_app or ./bsa

```

  - For information on the BSA Linux application parameters, see the [User Guide](docs/arm_bsa_architecture_compliance_user_guide.pdf).

#### 2.3 BSA Linux Test Log View
```sh
shell> sudo dmesg | tail -500 # print last 500 kernel logs

```

#### 2.4 Remove the BSA module

After the run is complete, you can remove the BSA module from the system if it is no longer needed.

```sh
shell> sudo rmmod bsa_acs

```

### Build Script aguments
The following arguments can be used when running the build.sh script:

- __-v or --version__ \- Specifies the Linux kernel version to be used for cross-compilation.
                    If not provided, the default version is 6.8.

- __--GCC_TOOLS__     \-  Specifies the GCC toolchain version for cross-compilation.
                    The default version is 13.2.rel1.

- __--help__          \-  Displays information about the ACS build environment, including default values,
                    usage instructions, and additional notes.

- __--clean__         \-  Removes the output folder build, which contains the resulting modules
                    and applications from the build.

- __--clean_all__     \-  Removes all downloaded repositories and build-related files,
                    including the output directory.


### Limitations
⚠️ **Note:**: DMA-related tests have not been verified.

For cross-compilation platforms, if you want compatibility with the target system, ensure that the Linux source version matches the version running on the target device.

Example:
 - Linux source version: 5.15
 - Target AArch64 machine kernel version: 5.15.0-139-generic

-  If the versions do not match exactly, the module may fail to load due to an invalid module format.

- ✅ If both versions are identical (e.g., both are 5.15), the build will work correctly — similar to how it works for a SystemReady image.

## ACS build steps - Bare-metal abstraction

The Bare-metal build environment is platform specific.

To provide a baseline, the build steps to integrate and run the Bare-metal tests from UEFI shell are provided in [README.md](pal/baremetal/target/RDN2/README.md).

For details on generating the binaries to run on Bare-metal environment, refer [README.md](pal/baremetal/README.md)

## ACS build steps - Memory model consistency tests
To evaluate the correctness and consistency of a system's memory model, memory model consistency tests (litmus tests) can be optionally built into BSA UEFI application,
the build and run steps are provided in [mem_test/README.md](mem_test/README.md).

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
|       BSA v1.1        |        v1.1.0       |   v25.03_REL1.1.0   |       Yes            |
|       BSA v1.1        |        v1.0.9       |   v24.11_REL1.0.9   |       Yes            |
|       BSA v1.0(c)     |        v1.0.8       |   v24.03_REL1.0.8   |       Yes            |
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

*Copyright (c) 2021-2025, Arm Limited and Contributors. All rights reserved.*
