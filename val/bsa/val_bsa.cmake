## @file
 # Copyright (c) 2023-2024, Arm Limited or its affiliates. All rights reserved.
 # SPDX-License-Identifier : Apache-2.0
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #  http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
 ##

 file(GLOB VAL_SRC
 "${ROOT_DIR}/val/common/src/AArch64/*.S"
 "${ROOT_DIR}/val/common/src/AArch64/*.s"
 "${ROOT_DIR}/val/common/src/AArch64/*.h"
 "${ROOT_DIR}/val/common/include/*.h"
 "${ROOT_DIR}/val/bsa/src/*.c"
 "${ROOT_DIR}/val/bsa/src/AArch64/*.S"
 "${ROOT_DIR}/val/common/src/*.c"
 "${ROOT_DIR}/val/common/sys_arch_src/pcie/*.h"
 "${ROOT_DIR}/val/common/sys_arch_src/pcie/*.c"
 "${ROOT_DIR}/val/common/sys_arch_src/smmu_v3/*.h"
 "${ROOT_DIR}/val/common/sys_arch_src/smmu_v3/*.c"
 "${ROOT_DIR}/val/common/sys_arch_src/gic/*.h"
 "${ROOT_DIR}/val/common/sys_arch_src/gic/*.c"
 "${ROOT_DIR}/val/common/sys_arch_src/gic/AArch64/*.S"
 "${ROOT_DIR}/val/common/sys_arch_src/gic/its/*.h"
 "${ROOT_DIR}/val/common/sys_arch_src/gic/its/*.c"
 "${ROOT_DIR}/val/common/sys_arch_src/gic/v2/*.h"
 "${ROOT_DIR}/val/common/sys_arch_src/gic/v2/*.c"
 "${ROOT_DIR}/val/common/sys_arch_src/gic/v3/*.h"
 "${ROOT_DIR}/val/common/sys_arch_src/gic/v3/*.c"
 "${ROOT_DIR}/val/common/sys_arch_src/gic/v3/AArch64/*.S"
 "${ROOT_DIR}/baremetal_app/*.h"
 "${ROOT_DIR}/baremetal_app/*.c"

)

#Create compile list files
list(APPEND COMPILE_LIST ${VAL_SRC})
set(COMPILE_LIST ${COMPILE_LIST} PARENT_SCOPE)

# Create VAL library
add_library(${VAL_LIB} STATIC ${VAL_SRC})

target_include_directories(${VAL_LIB} PRIVATE
 ${CMAKE_CURRENT_BINARY_DIR}
 ${ROOT_DIR}/
 ${ROOT_DIR}/val
 ${ROOT_DIR}/val/include/
 ${ROOT_DIR}/val/common/
 ${ROOT_DIR}/val/common/include/
 ${ROOT_DIR}/val/bsa/
 ${ROOT_DIR}/val/bsa/include/
 ${ROOT_DIR}/val/common/src/AArch64/
 ${ROOT_DIR}/val/common/sys_arch_src/smmu_v3/
 ${ROOT_DIR}/val/common/sys_arch_src/gic/
 ${ROOT_DIR}/val/common/sys_arch_src/gic/its/
 ${ROOT_DIR}/val/common/sys_arch_src/gic/v2/
 ${ROOT_DIR}/val/common/sys_arch_src/gic/v3/
 ${ROOT_DIR}/baremetal_app/
 ${ROOT_DIR}/pal/baremetal/common/include/
 ${ROOT_DIR}/pal/baremetal/target/${TARGET}/common/include/
 ${ROOT_DIR}/pal/baremetal/common/src/AArch64/
)

unset(VAL_SRC)
