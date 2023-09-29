
Please Note: The code in the "pal_baremetal" directory is only a reference code for implementation of PAL API's and it has not been verified on any model or SoC's.

The directory pal_baremetal consists of the reference code of the PAL API's specific to a platform.
Description of each directory are as follows:

1. src: The implementation common to all the baremetal platforms for all modules are in the source directory.
2. include:
  -  pcie_enum.h: Implementation needed for enumeration.
  -  pal_common_support.h: Implementation that is common to all platforms.
3. FVP: Contains Platform specific code. The details in this folder need to be modified w.r.t the platform.

## Build Steps

Reference Makefile is present at [Makefile](../../Makefile). To compile BSA, perform following steps.
1. cd bsa-acs
2. export BSA_PATH=pointing to BSA directory
3. export GCC49_AARCH64_PREFIX=GCC toolchain path pointing to /bin/aarch64-linux-gnu-
4. make

The output library files will be generated at <bsa_path>/build/lib/

Note: Any platform specific changes can be done by using TARGET_EMULATION macro defintion. The pal_baremetal reference code is located in [pal_baremetal](.). To customize the bare-metal code for different platforms, create a directory <platform_name> in [pal_baremetal](.) folder and copy the reference code from [source](src) folder and [RDN2](FVP/RDN2) folder to <platform_name>.

For more details on how to port the reference code to a specific platform and for further customisation please refer to the [User Guide](../../docs/Arm_BSA_ACS_Bare-metal_User_Guide.pdf)

-----------------

*Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.*
