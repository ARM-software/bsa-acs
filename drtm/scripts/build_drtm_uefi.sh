## @file
#  Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
#  SPDX-License-Identifier : Apache-2.0
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
##

#!/bin/bash

WORK_DIR=${PWD}/workspace
HOME_DIR=${PWD}

if [ ! -d workspace ]
then
    mkdir workspace
fi
cd ${WORK_DIR}
if [ ! -d arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-linux-gnu ]
then
    rm -f arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz
    wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz
    tar -xf arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz
fi

if [ ! -d output ]
then
    mkdir output
fi

echo "Building DRTM ACS for UEFI"
if [ ! -d edk2 ]
then
    git clone --recursive --branch edk2-stable202402 https://github.com/tianocore/edk2.git
    git clone https://github.com/tianocore/edk2-libc edk2/edk2-libc
fi
cd ${WORK_DIR}/edk2
make -C BaseTools

if ! grep -q DrtmPalLib "ShellPkg/ShellPkg.dsc"
then
    sed -i '/LibraryClasses.common/ a \ \ DrtmPalLib|ShellPkg/Application/bsa-acs/pal/uefi_acpi/DrtmPalLib.inf' ShellPkg/ShellPkg.dsc
fi
if ! grep -q DrtmValLib "ShellPkg/ShellPkg.dsc"
then
    sed -i '/LibraryClasses.common/ a \ \ DrtmValLib|ShellPkg/Application/bsa-acs/val/DrtmValLib.inf' ShellPkg/ShellPkg.dsc
fi
if ! grep -q DrtmAcs "ShellPkg/ShellPkg.dsc"
then
    sed -i '/Components/ a \ \ ShellPkg/Application/bsa-acs/drtm/uefi_app/DrtmAcs.inf' ShellPkg/ShellPkg.dsc
fi


rm -f ShellPkg/Application/bsa-acs
rm -rf Build/Shell/DEBUG_GCC49/*

ln -s ${HOME_DIR} ShellPkg/Application/bsa-acs
export GCC49_AARCH64_PREFIX=${WORK_DIR}/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
export PACKAGES_PATH=${WORK_DIR}/edk2/edk2-libc

source edksetup.sh
source ShellPkg/Application/bsa-acs/tools/scripts/acsbuild.sh ENABLE_DRTM
cp Build/Shell/DEBUG_GCC49/AARCH64/Drtm.efi ${WORK_DIR}/output

cd ${HOME_DIR}

echo "Drtm EFI Application is available here : ${WORK_DIR}/output/Drtm.efi"
