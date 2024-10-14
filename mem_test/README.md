# Memory model consistency tests
Memory model consistency tests (litmus tests) are small parallel programs designed to evaluate the correctness and
consistency of a computer system's memory model, particularly in the context of concurrent or parallel programming.
These parallel payloads with specific memory access patterns are run for several iterations to observe how the system
handles memory synchronization, caching, and reordering. By analyzing the outcomes of these tests, developers can gain
insights into the reliability of the memory model implementation and identify any subtle bugs or inconsistencies that
might lead to unpredictable behavior in concurrent programs.

 - NOTE: Running and passing Memory Model consistency tests are not required for SystemReady Certifications and they are not part of BSA Certification Image binary.

## Release details
 - Code quality: v1.0.0 EAC
 - The tests can be run at Silicon level.
 - The tests checks for forbidden behaviors that memory model should not exhibit.
 - For litmus tests scenario and instructions to analyze litmus tests log output, refer the [Memory model tests Scenario & User Guide](../mem_test/memory_model_tests_scenario_user_guide.rst).

## Steps to build litmus tests into bsa-acs
1. Setup edk2 build directory
>          git clone --branch  edk2-stable202402 https://github.com/tianocore/edk2.git
>          cd edk2
>          git clone https://github.com/tianocore/edk2-libc.git
>          git submodule update --init --recursive

2. Download source files and apply edk2 patch
>          git clone https://github.com/ARM-software/bsa-acs.git ShellPkg/Application/bsa-acs
>          git clone https://github.com/relokin/kvm-unit-tests.git ShellPkg/Application/bsa-acs/mem_test/kvm-unit-tests
>          git -C ShellPkg/Application/bsa-acs/mem_test/kvm-unit-tests checkout target-efi-bsa
>          git apply ShellPkg/Application/bsa-acs/mem_test/patches/mem_test_edk2.patch

3. Build bsa-acs UEFI app <br>
Note :  Install GCC-ARM 13.2 [toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
>          export GCC49_AARCH64_PREFIX=<path to CC>arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
>          export PACKAGES_PATH=`pwd`/edk2-libc
>          source edksetup.sh
>          make -C BaseTools/Source/C
>          source ShellPkg/Application/bsa-acs/tools/scripts/acsbuild.sh ENABLE_MEMTEST

4. BSA EFI application path
- The EFI executable file is generated at <edk2-path>/Build/Shell/DEBUG_GCC49/AARCH64/Bsa.efi

## Test suite execution on QEMU platform
1. Fetch uefi-firmware
>          wget https://releases.linaro.org/components/kernel/uefi-linaro/16.02/release/qemu64/QEMU_EFI.fd

2. Run QEMU model <br>
**Note** : <br> 1. Follow steps provided in [Emulation environment with secondary-storage](../README.md#22-emulation-environment-with-secondary-storage) to create .img file containing Bsa.efi executable. <br> 2. Follow instructions from https://www.qemu.org/download/#source to obtain QEMU model.

>          IMG_PATH=<path to .img containing Bsa.efi>
>          QEMU_PATH=<path to QEMU model>
>                 e.g. /data_sda/user01/qemu/qemu-7.2.0/build/qemu-system-aarch64
>          BIOS_PATH=<pat to QEMU_EFI.fd>

>          $QEMU_PATH -nodefaults -machine virt -accel tcg -cpu cortex-a57 -device pci-testdev -display none -serial stdio -bios $BIOS_PATH -drive file=$IMG_PATH,if=virtio,format=raw -smp 8 -m 512 -machine virtualization=on

3. Press Esc to enter the shell prompt, and navigate to disk containing Bsa.efi, and run BSA app.
>          Bsa.efi

## Limitations
 - The kvm-unit-tests print function depends on SPCR ACPI table for UART base address and UEFI console setting must be set to "serial". In case of non-availability of SPCR,
   set `CONFIG_UART_EARLY_BASE` in `bsa-acs/mem_test/kvm-unit-tests/lib/arm/io.c` to UART base address of system under test, after step 2 in [build steps](#steps-to-build-litmus-tests-into-bsa-acs).
 - Initializing memory model tests infra and transitioning from EL1 to EL2 is observed to be time-consuming (approximately 30 minutes) in systems with large PE caches. The duration of each test run varies, ranging from 1 minute in silicon to 40 minutes in certain emulated platforms like Arm FVP models.
 - The printf implementation in kvm-unit-tests works only with a serial interface. For HDMI or other outputs, a console splitter or similar device is required.

## Feedback, contributions, and support

 - For feedback, use the GitHub Issue Tracker that is associated with this repository.
 - For support, send an email to "support-systemready-acs@arm.com" with details.
 - Arm licensees may contact Arm directly through their partner managers.
 - Arm welcomes code contributions through GitHub pull requests. See the GitHub documentation on how to raise pull requests.

*Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.*
