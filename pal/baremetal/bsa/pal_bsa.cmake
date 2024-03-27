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

 file(GLOB PAL_SRC
 "${ROOT_DIR}/pal/baremetal/target/${TARGET}/common/src/*.c"
 "${ROOT_DIR}/pal/baremetal/target/${TARGET}/bsa/src/*.c"
 "${ROOT_DIR}/pal/baremetal/bsa/src/*.c"
 "${ROOT_DIR}/pal/baremetal/common/src/*.c"
 "${ROOT_DIR}/pal/baremetal/common/src/AArch64/*.S"
)

#Create compile list files
list(APPEND COMPILE_LIST ${PAL_SRC})
set(COMPILE_LIST ${COMPILE_LIST} PARENT_SCOPE)

# Create PAL library
add_library(${PAL_LIB} STATIC ${PAL_SRC})

target_include_directories(${PAL_LIB} PRIVATE
 ${CMAKE_CURRENT_BINARY_DIR}
 ${ROOT_DIR}/
 ${ROOT_DIR}/baremetal_app/
 ${ROOT_DIR}/pal/baremetal/
 ${ROOT_DIR}/pal/baremetal/common/include/
 ${ROOT_DIR}/pal/baremetal/common/src/AArch64/
 ${ROOT_DIR}/pal/baremetal/target/${TARGET}/common/include/
)

unset(PAL_SRC)
