
# MPAM System Component Specification - Architecture Compliance Suite

## Memory System Resource Partitioning and Monitoring
**Memory System Resource Partitioning and Monitoring** (MPAM) System Component specification describes propagation of a Partition ID (PARTID) and Performance Monitoring Group (PMG) through the memory system, a framework for memory-system component (MSC) controls that partition one or more of the performance resources of the component.

For more information, download the [MPAM System Component Specification](https://developer.arm.com/documentation/ihi0099/latest/).

## Release details
 - Code Quality: Alpha
 - The tests are written for version A.a of the MPAM Memory System Component Specification.
 - For more details on tests implemented in this release, Please refer [MPAM Test Scenario Document](docs/arm_mpam_architecture_compliance_test_scenario.pdf).

## Downloading MPAM ACS

MPAM ACS code is present in a sub-directory in bsa-acs repository.

 - git clone https://github.com/ARM-software/bsa-acs.git <br/>
 - cd bsa-acs <br/>

## Building MPAM ACS
### UEFI application
#### Prerequisites

ACS build requires that the following requirements are met, Please skip this if you are using [MPAM Application Build Script](scripts/build_mpam_uefi.sh).

- Any mainstream Linux based OS distribution.
- git clone EDK2 tree.
- git clone EDK2-libc tree.
- Install GCC-ARM 13.2 [toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).
- Install the build prerequisite packages to build EDK2. The details of the packages are beyond the scope of this document.

#### Build Steps

 - cd /path/to/bsa-acs/<br/>
 - source mpam/scripts/build_mpam_uefi.sh

#### Build Output

The following output file is created in /path/to/bsa-acs/workspace/output/:

- Mpam.efi

#### Note : Steps to get toolchain
- wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz
- tar -xf arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz
- export GCC49_AARCH64_PREFIX= GCC 13.2 toolchain path pointing to arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

## Test suite execution in UEFI

### Post-Silicon

On a system where a USB port is available and functional, perform the following steps:

1. Copy 'Mpam.efi' to a USB Flash drive.
2. Plug in the USB Flash drive to one of the functional USB ports on the system.
3. Boot the system to UEFI shell.
4. To determine the file system number of the plugged in USB drive, execute 'map -r' command.
5. Type 'fsx' where 'x' is replaced by the number determined in step 4.
6. To start the compliance tests, run the executable Mpam.efi with the appropriate arguments.

### Emulation environment with secondary storage

On an emulation environment with secondary storage, perform the following steps:

1. Create an image file which contains the 'Mpam.efi' file. For Example: <br/>
 - mkfs.vfat -C -n HD0 hda.img 31457280 <br/>
 - sudo mount hda.img /mnt/mpam <br/>
 - cd /path/to/bsa-acs/workspace/output/ <br/>
 - sudo cp Mpam.efi /mnt/mpam/ <br/>
 - sudo umount /mnt/mpam
2. Load the image file to the secondary storage using a backdoor. The steps followed to load the image file are Emulation environment specific and beyond the scope of this document.
3. Boot the system to UEFI shell.
4. To determine the file system number of the secondary storage, execute 'map -r' command.
5. Type 'fsx' where 'x' is replaced by the number determined in step 4.
6. To start the compliance tests, run the executable Mpam.efi with the appropriate arguments.

## Application arguments

Command line arguments are similar for uefi application, with some exceptions.

### UEFI

Shell> Mpam.efi [-v &lt;verbosity&gt;] [-skip &lt;test_id&gt;] [-f &lt;filename&gt;]

#### -v
Choose the verbosity level.

- 1 - INFO and above
- 2 - DEBUG and above
- 3 - TEST and above
- 4 - WARN and ERROR
- 5 - ERROR

#### -skip
Overrides the suite to skip the execution of a particular
test. For example, <i>-skip 10</i> skips test 10.

#### -t
If Test ID(s) set, will only run the specified test(s), all others will be skipped
For example, <i>-t 10</i> Run test 10.

#### -m
If Module ID(s) set, will only run the specified module(s), all others will be skipped
For example, <i>-m 100</i> Run Cache Module.

#### -f (Only for UEFI application)
Save the test output into a file in secondary storage. For example <i>-f mpam.log</i> creates a file mpam.log with test output.

### UEFI example

Shell> Mpam.efi -v 1 -skip 15,20,30 -f mpam_uefi.log

Runs MPAM ACS with verbosity INFO, skips test 15, 20 and 30 and saves the test results in <i>mpam_uefi.log</i>.

## Limitations

 - Since this is a Alpha quality release, contains limited number of tests based on MPAM MSC Specification.
 - Few of the tests related to Errors in MSC have been verified on limited platforms. Please reach out to us in case of any help.
 - Memory Bandwidth Partitioning tests have been implemented but have not yet been verified on any platform. If you encounter any failures or errors during the ACS run, Please raise an issue.

## License
MPAM ACS is distributed under Apache v2.0 License.

## Feedback, contributions, and support

 - For feedback, use the GitHub Issue Tracker that is associated with this repository.
 - For support, send an email to "support-systemready-acs@arm.com" with details.
 - Arm licensees may contact Arm directly through their partner managers.
 - Arm welcomes code contributions through GitHub pull requests. See the GitHub documentation on how to raise pull requests.

--------------

*Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.*
