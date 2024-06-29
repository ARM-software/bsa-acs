## @file
#  Copyright (c) 2023-2024, Arm Limited or its affiliates. All rights reserved.
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


if [ $(uname -m) != "aarch64" ] && [ -v $GCC49_AARCH64_PREFIX ]
then
    echo "GCC49_AARCH64_PREFIX is not set"
    echo "set using export GCC49_AARCH64_PREFIX=<lib_path>/bin/aarch64-linux-gnu-"
    return 0
fi

if [ "$1" == "ENABLE_OOB" ]; then
    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/bsa-acs/baremetal_app/BsaAcs.inf -D ENABLE_OOB
    return 0;
fi

if [ "$1" == "ENABLE_MEMTEST" ]; then
    build -a AARCH64 -t GCC49 -n 1 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/bsa-acs/uefi_app/BsaAcsMem.inf
    return 0;
fi

if [ "$1" == "ENABLE_DRTM" ]; then
    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/bsa-acs/drtm/uefi_app/DrtmAcs.inf
    return 0;
fi

    build -a AARCH64 -t GCC49 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/bsa-acs/uefi_app/BsaAcs.inf
