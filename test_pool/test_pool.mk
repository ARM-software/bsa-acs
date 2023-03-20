## @file
 # Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
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

BSA_ROOT:= $(BSA_PATH)
BSA_DIR := $(BSA_ROOT)/test_pool
BSA_TEST_DIR := $(BSA_ROOT)/test_pool/exerciser
BSA_TEST_DIR += $(BSA_ROOT)/test_pool/gic
BSA_TEST_DIR += $(BSA_ROOT)/test_pool/memory_map
BSA_TEST_DIR += $(BSA_ROOT)/test_pool/pcie
BSA_TEST_DIR += $(BSA_ROOT)/test_pool/pe
BSA_TEST_DIR += $(BSA_ROOT)/test_pool/peripherals
BSA_TEST_DIR += $(BSA_ROOT)/test_pool/power_wakeup
BSA_TEST_DIR += $(BSA_ROOT)/test_pool/smmu
BSA_TEST_DIR += $(BSA_ROOT)/test_pool/timer
BSA_TEST_DIR += $(BSA_ROOT)/test_pool/watchdog

CFLAGS    += -I$(BSA_ROOT)/val/include
CFLAGS    += -I$(BSA_ROOT)/

CC = $(GCC49_AARCH64_PREFIX)gcc -march=armv8.2-a -DTARGET_EMULATION
AR = $(GCC49_AARCH64_PREFIX)ar
CC_FLAGS = -g -O0 -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wextra -Wmissing-declarations -Wstrict-prototypes -Wno-error=conversion -Wno-error=sign-conversion -Wno-error=strict-overflow -Wno-type-limits

DEPS = $(BSA_ROOT)/platform/pal_baremetal/FVP/RDN2/include/platform_override_fvp.h

OBJ_DIR := $(BSA_ROOT)/build/obj
LIB_DIR := $(BSA_ROOT)/build/lib
OUT_DIR = $(BSA_ROOT)/build

FILES   := $(foreach files,$(BSA_TEST_DIR),$(wildcard $(files)/*.c))
FILE    = `find $(FILES) -type f -exec sh -c 'echo {} $$(basename {})' \; | sort -u --stable -k2,2 | awk '{print $$1}'`
FILE_1  := $(shell echo $(FILE))
XYZ     := $(foreach a,$(FILE_1),$(info $(a)))
PAL_OBJS :=$(addprefix $(OBJ_DIR)/,$(addsuffix .o, $(basename $(notdir $(foreach dirz,$(FILE_1),$(dirz))))))

all: PAL_LIB

create_dirs:
	rm -rf ${OBJ_DIR}
	rm -rf ${LIB_DIR}
	rm -rf ${OUT_DIR}
	@mkdir ${OUT_DIR}
	@mkdir ${OBJ_DIR}
	@mkdir ${LIB_DIR}


$(OBJ_DIR)/%.o: $(DEPS)
	$(CC)  -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1
$(OBJ_DIR)/%.o: $(BSA_DIR)/exerciser/%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1
$(OBJ_DIR)/%.o: $(BSA_DIR)/gic/%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1
$(OBJ_DIR)/%.o: $(BSA_DIR)/memory_map/%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1
$(OBJ_DIR)/%.o: $(BSA_DIR)/pcie/%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1
$(OBJ_DIR)/%.o: $(BSA_DIR)/pe/%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1
$(OBJ_DIR)/%.o: $(BSA_DIR)/peripherals/%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1
$(OBJ_DIR)/%.o: $(BSA_DIR)/power_wakeup/%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1
$(OBJ_DIR)/%.o: $(BSA_DIR)/smmu/%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1
$(OBJ_DIR)/%.o: $(BSA_DIR)/timer/%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1
$(OBJ_DIR)/%.o: $(BSA_DIR)/watchdog/%.c
	$(CC) $(CC_FLAGS) $(CFLAGS) -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1

$(OBJ_DIR)/%.o: %.S$(BSA_DIR)
	$(CC) -c -o $@ $< >> $(OUT_DIR)/compile.log 2>&1

$(LIB_DIR)/lib_testpool.a: $(PAL_OBJS)
	$(AR) $(ARFLAGS) $@ $^ >> $(OUT_DIR)/link.log 2>&1

PAL_LIB: $(LIB_DIR)/lib_testpool.a

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(LIB_DIR)
	rm -rf ${OUT_DIR}

.PHONY: all PAL_LIB

