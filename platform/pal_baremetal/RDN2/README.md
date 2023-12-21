# Base System Architecture - Architecture Compliance Suite


## Base System Architecture
**Base System Architecture** (BSA) specification specifies a hardware system architecture based on the Arm 64-bit architecture. Server system software such as operating systems, hypervisors, and firmware rely on this. It addresses PE features and key aspects of system architecture.

For more information, download the [BSA specification](https://developer.arm.com/documentation/den0094/c/?lang=en)


## BSA - Architecture Compliance Suite

BSA **Architecture Compliance Suite** (ACS) is a collection of self-checking, portable C-based tests.
This suite includes a set of examples of the invariant behaviors that are provided by the [BSA](https://developer.arm.com/documentation/den0094/c/?lang=en) specification, so that implementers can verify if these behaviours have been interpreted correctly.
The tests are executed in a baremetal environment. The initialization of the baremetal environment is specific to the environment and is out of scope of this document.

## Release details
 - Code Quality: REL v1.0.7
 - The tests are written for version 1.0 (c) of the BSA specification.
 - The compliance suite is not a substitute for design verification.
 - To review the BSA ACS logs, Arm licensees can contact Arm directly through their partner managers.

### EDA vendors
Contact your EDA vendor and ask if they include these tests as part of their verificatoin IP package.

## GitHub branch
  - To pick up the release version of the code, checkout the corresponding tag from the main branch.
  - To get the latest version of the code with bug fixes and new features, use the main branch.

## Additional reading
  - For details on the BSA ACS test execution, see the [Arm BSA ACS User Guide](../../../docs/arm_bsa_architecture_compliance_bare-metal_user_guide.pdf).
  - For details on the Design of the BSA ACS, see the [Arm BSA Validation Methodology Document](../../../docs/arm_bsa_architecture_compliance_validation_methodology.pdf).
Note: The Baremetal PCIe enumeration code provided as part of the BSA ACS should be used and should not be replaced. This code is vital in analyzing of the test result.

## Target platforms
  Any 64-bit Arm based Server design presented as a full chip Emulation or Simulation environment

## ACS build steps - UEFI Shell application

The baremetal build environment is platform specific. To provide a baseline, the build steps to integrate and run the tests from UEFI shell are provided here.


### Prebuilt images
Prebuilt images for each release are available in the prebuilt_images folder of the release branch. You can choose to use these images or build your own image by following the steps below. If you choose to use the prebuilt image, jump to the test suite execution section below for details on how to run the application.

### Prerequisites
    Before starting the ACS build, ensure that the following requirements are met.

- Any mainstream Linux based OS distribution running on a x86 or AArch64 machine.
- git clone the edk2-stable202208 branch of [EDK2 tree](https://github.com/tianocore/edk2).
- Install GCC 7.5 or later toolchain for Linux from [here](https://releases.linaro.org/components/toolchain/binaries/).
- Install the build prerequisite packages to build EDK2.


To start the ACS build, perform the following steps:
1.  git clone -b edk2-stable202208 https://github.com/tianocore/edk2.git
2.  cd edk2
3.  git clone https://github.com/tianocore/edk2-libc
4.  git submodule update --init --recursive
5.  git clone https://github.com/ARM-software/bsa-acs.git ShellPkg/Application/bsa-acs
6.  Add the following to the [LibraryClasses.common] section in ShellPkg/ShellPkg.dsc
   - UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
   - BsaValBaremetalLib|ShellPkg/Application/bsa-acs/val/BsaValBaremetalLib.inf
   - BsaPalBaremetalLib|ShellPkg/Application/bsa-acs/platform/pal_baremetal/BsaPalBaremetalLib.inf
   - BsaPalFVPLib|ShellPkg/Application/bsa-acs/platform/pal_baremetal/RDN2/BsaPalFVPLib.inf
7.  Add ShellPkg/Application/bsa-acs/baremetal_app/BsaAcs.inf in the [components] section of ShellPkg/ShellPkg.dsc
8.  Modify CC Flags in the [BuildOptions] section of ShellPkg/ShellPkg.dsc
```
      *_*_*_CC_FLAGS = -DENABLE_OOB -I${WORKSPACE}/ShellPkg/Application/bsa-acs/platform/pal_baremetal/common/include -I${WORKSPACE}/ShellPkg/Application/bsa-acs/platform/pal_baremetal/RDN2/include/

      !include StdLib/StdLib.inc
```
9.  Modify the following in the edk2-libc/StdLib/LibC/Main/Main.c
```
      -extern int main( int, char**);
      +extern int ShellAppMainbsa( int, char**);
```
10.  Modify the following in ShellAppMain() of edk2-libc/StdLib/LibC/Main/Main.c
```
      -ExitVal = (INTN)main( (int)Argc, gMD->NArgV);
      +ExitVal = (INTN)ShellAppMainbsa( (int)Argc, gMD->NArgV);
```
11.  Comment the map[] variable in edk2-libc/StdLib/LibC/Main/Arm/flt_rounds.c to avoid -werror=unused-variable
```
          +#if 0
          static const int map[] = {
          1,  /* round to nearest */
          2,  /* round to positive infinity */
          3,  /* round to negative infinity */
          0   /* round to zero */
          };
          +#endif
```


### Linux build environment
If the build environment is Linux, perform the following steps:
1.  export GCC49_AARCH64_PREFIX= GCC10.3 toolchain path pointing to /bin/aarch64-linux-gnu- in case of x86 machine. For AArch64 build it should point to /usr/bin/
2.  export PACKAGES_PATH= path pointing to edk2-libc
3.  source edksetup.sh
4.  make -C BaseTools/Source/C
5.  source ShellPkg/Application/bsa-acs/tools/scripts/acsbuild.sh ENABLE_OOB



### Build output

The EFI executable file is generated at <edk2_path>/Build/Shell/DEBUG_GCC49/AARCH64/Bsa.efi


## Test suite execution

The execution of the compliance suite varies depending on the test environment. These steps assume that the test suite is invoked through the ACS UEFI shell application.

For details about the BSA ACS UEFI Shell application, see the [BSA ACS USER Guide](../../../docs/arm_bsa_architecture_compliance_user_guide.pdf)

### On-Silicon

Executing the tests in a baremetal environment is platform specific. As a baseline, the following steps are provided to run the tests from UEFI shell on silicon.

On a system where a USB port is available and functional, perform the following steps:

1. Copy 'Bsa.efi' to a USB Flash drive.
2. Plug in the USB Flash drive to one of the functional USB ports on the system.
3. Boot the system to UEFI shell.
4. To determine the file system number of the plugged in USB drive, execute 'map -r' command.
5. Type 'fsx' where 'x' is replaced by the number determined in step 4.
6. To start the compliance tests, run the executable Bsa.efi with the appropriate parameters.
   For details on the parameters, refer to [BSA ACS User Guide](../../../docs/arm_bsa_architecture_compliance_user_guide.pdf)

## Limitations
Validating the compliance of certain PCIe rules defined in the BSA specification requires the PCIe end-point to generate specific stimulus during the runtime of the test. Examples of such stimulus are  P2P, PASID, ATC, etc. The tests that requires these stimuli are grouped together in the exerciser module. The exerciser layer is an abstraction layer that enables the integration of hardware capable of generating such stimuli to the test framework.
The details of the hardware or Verification IP which enable these exerciser tests are platform specific and are beyond the scope of this document.

## License
BSA ACS is distributed under Apache v2.0 License.


## Feedback, contributions and support

 - For feedback, use the GitHub Issue Tracker that is associated with this repository.
 - For support, send an email to "support-systemready-acs@arm.com" with details.
 - Arm licensees may contact Arm directly through their partner managers.
 - Arm welcomes code contributions through GitHub pull requests. See GitHub documentation on how to raise pull requests.

--------------

*Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.*
