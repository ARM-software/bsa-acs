# Baremetal README
**Please Note**: The code in the "baremetal" directory is only a reference code for implementation of PAL API's and it has not been verified on any model or SoC's.
The directory baremetal consists of the reference code of the PAL API's specific to a platform.
Description of each directory are as follows:

## Directory Structure
&emsp; 1. **common**: The implementation common to both BSA and SBSA ACS for all modules are in this directory.\
&emsp; &emsp; 1.1 **include**: Consists of the include files common to both BSA and SBSA ACS \
&emsp; &emsp; 1.2. **src**: Source files common to both BSA and SBSA ACS for all modules which do not require user modification.\
&emsp; &emsp; &emsp; Eg: Info tables parsing, PCIe enumeration code, etc.

&emsp; 2. **bsa**   : The implementation specific to BSA ACS for all modules are in this directory \
&emsp; 3. **sbsa**  : The implementation specific to SBSA ACS for all modules are in this directory \
&emsp; 4. **target**: Contains Platform specific code. The details in this folder need to be modified w.r.t the platform

Contains Platform specific code. The details in this folder need to be modified w.r.t the platform.

## Build Steps

1. To compile BSA, perform the following steps \
&emsp; 1.1 cd bsa-acs \
&emsp; 1.2 export CROSS_COMPILE=<path_to_the_toolchain>/bin/aarch64-none-elf- \
&emsp; 1.3 mkdir build \
&emsp; 1.4 cd build \
&emsp; 1.5 cmake ../ -G"Unix Makefiles" -DCROSS_COMPILE=$CROSS_COMPILE -DTARGET="Target platform" \
&emsp; 1.6 make

Note: Reference Cmake file for BSA is present at [CMakeLists.txt](../../CMakeLists.txt).

2. To compile SBSA from BSA, perform the following steps \
&emsp; 2.1 cd bsa-acs \
&emsp; 2.2 export CROSS_COMPILE=<path_to_the_toolchain>/bin/aarch64-none-elf- \
&emsp; 2.3 mkdir build \
&emsp; 2.4 cd build \
&emsp; 2.5 cmake ../ -G"Unix Makefiles" -DCROSS_COMPILE=$CROSS_COMPILE -DTARGET="Target platform" -DACS=sbsa -DSBSA_DIR=<sbsa-acs_path> \
&emsp; 2.6 make

*Recommended*: CMake v3.17, GCC v12.2
```
CMake Command Line Options:
 -DARM_ARCH_MAJOR = Arch major version. Default value is 9.
 -DARM_ARCH_MINOR = Arch minor version. Default value is 0.
 -DCROSS_COMPILE  = Cross compiler path
 -DTARGET         = Target platform. Should be same as folder under baremetal/target/
 -DACS            = To compile SBSA ACS
 -DSBSA_DIR       = SBSA path for SBSA compilation
```

On a successful build, *.bin, *.elf, *.img and debug binaries are generated at *build/output* directory. The output library files will be generated at *build/tools/cmake/* of the bsa-acs directory.

## Running ACS with Bootwrapper on RDN2

**1. In RDN2 software stack make following change:**

  In <rdn2_path>/build-scripts/build-target-bins.sh - replace uefi.bin with acs_latest.bin

```
  if [ "${!tfa_tbbr_enabled}" == "1" ]; then
      $TOP_DIR/$TF_A_PATH/tools/cert_create/cert_create  \
      ${cert_tool_param} \
-     ${bl33_param_id} ${OUTDIR}/${!uefi_out}/uefi.bin
+     ${bl33_param_id} ${OUTDIR}/${!uefi_out}/acs_latest.bin
  fi

  ${fip_tool} update \
  ${fip_param} \
- ${bl33_param_id} ${OUTDIR}/${!uefi_out}/uefi.bin \
+ ${bl33_param_id} ${OUTDIR}/${!uefi_out}/acs_latest.bin \
  ${PLATDIR}/${!target_name}/fip-uefi.bin

```

**2. Repackage the FIP image with this new binary**
- cp <bsa_acs>/build/output/<acs>.bin <rdn2_path>/output/rdn2/components/css-common/acs_latest.bin

- cd <rdn2_path>

- ./build-scripts/rdinfra/build-test-acs.sh -p rdn2 package

- export MODEL=<path_to_FVP_RDN2_model>

- cd <rdn2>/model-scripts/rdinfra/platforms/rdn2

- ./run_model.sh

**Note:** <acs>.bin stands for either bsa.bin or sbsa.bin. Any platform specific changes can be done by using TARGET_BM_BOOT macro defintion. The baremetal reference code is located in [baremetal](.). To customize the bare-metal code for different platforms, create a directory <platform_name> in [target](target/) folder and copy the reference code from [include](target/RDN2/include) and [source](target/RDN2/src) folders from [RDN2](target/RDN2) to <platform_name>.


For more details on how to port the reference code to a specific platform and for further customisation please refer to the [User Guide](../../docs/arm_bsa_architecture_compliance_bare-metal_user_guide.pdf)

-----------------

*Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.*
