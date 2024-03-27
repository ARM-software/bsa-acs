/** @file
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

/* Settings */
#define PLATFORM_OVERRIDE_SBSA_LEVEL   0x7         //The permissible levels are 3,4,5,6 and 7
#define PLATFORM_OVERRIDE_SBSA_PRINT_LEVEL  0x3    //The permissible levels are 1,2,3,4 and 5

/* System Last-Level cache info
   0 - Unknown
   1 - PPTT PE-side LLC
   2 - HMAT mem-side LLC
*/
#define PLATFORM_OVERRRIDE_SLC 0x0


/* GIC platform config parameters */
#define PLATFORM_OVERRIDE_GICRD_COUNT       0x1

/* Coresight components config parameters*/
#define CS_COMPONENT_COUNT         1

#define CS_COMPONENT_0_IDENTIFIER    "ARMHC97C"
#define CS_COMPONENT_0_DEVICE_NAME   "\\_SB_.ETR0"


/* Cache config and MASKS*/
#define SIZE_MASK           0x1
#define CACHE_TYPE_MASK     0x10
#define CACHE_ID_MASK       0x80
#define CACHE_TYPE_SHIFT    4
#define CACHE_ID_SHIFT      7

#define DEFAULT_CACHE_IDX 0xFFFFFFFF
#define MAX_L1_CACHE_RES 2 /* Generally PE Level 1 have a data and a instruction cache */

#define PLATFORM_OVERRIDE_CACHE_CNT            0x30

#define PLATFORM_CACHE0_FLAGS                  0xFF
#define PLATFORM_CACHE0_OFFSET                 0x68
#define PLATFORM_CACHE0_NEXT_LEVEL_INDEX       1
#define PLATFORM_CACHE0_SIZE                   0x10000
#define PLATFORM_CACHE0_CACHE_ID               0x1
#define PLATFORM_CACHE0_IS_PRIVATE             0x1
#define PLATFORM_CACHE0_TYPE                   0

#define PLATFORM_CACHE1_FLAGS                  0xFF
#define PLATFORM_CACHE1_OFFSET                 0xA0
#define PLATFORM_CACHE1_NEXT_LEVEL_INDEX       -1
#define PLATFORM_CACHE1_SIZE                   0x100000
#define PLATFORM_CACHE1_CACHE_ID               0x3
#define PLATFORM_CACHE1_IS_PRIVATE             0x1
#define PLATFORM_CACHE1_TYPE                   0x2

#define PLATFORM_CACHE2_FLAGS                  0xFF
#define PLATFORM_CACHE2_OFFSET                 0x84
#define PLATFORM_CACHE2_NEXT_LEVEL_INDEX       0x1
#define PLATFORM_CACHE2_SIZE                   0x10000
#define PLATFORM_CACHE2_CACHE_ID               0x2
#define PLATFORM_CACHE2_IS_PRIVATE             0x1
#define PLATFORM_CACHE2_TYPE                   0x1

#define PLATFORM_CACHE3_FLAGS                  0xFF
#define PLATFORM_CACHE3_OFFSET                 0xEC
#define PLATFORM_CACHE3_NEXT_LEVEL_INDEX       4
#define PLATFORM_CACHE3_SIZE                   0x10000
#define PLATFORM_CACHE3_CACHE_ID               0x1001
#define PLATFORM_CACHE3_IS_PRIVATE             0x1
#define PLATFORM_CACHE3_TYPE                   0x0

#define PLATFORM_CACHE4_FLAGS                  0xFF
#define PLATFORM_CACHE4_OFFSET                 0x124
#define PLATFORM_CACHE4_NEXT_LEVEL_INDEX       -1
#define PLATFORM_CACHE4_SIZE                   0x100000
#define PLATFORM_CACHE4_CACHE_ID               0x1003
#define PLATFORM_CACHE4_IS_PRIVATE             0x1
#define PLATFORM_CACHE4_TYPE                   0x2

#define PLATFORM_CACHE5_FLAGS                  0xFF
#define PLATFORM_CACHE5_OFFSET                 0x108
#define PLATFORM_CACHE5_NEXT_LEVEL_INDEX       4
#define PLATFORM_CACHE5_SIZE                   0x10000
#define PLATFORM_CACHE5_CACHE_ID               0x1002
#define PLATFORM_CACHE5_IS_PRIVATE             0x1
#define PLATFORM_CACHE5_TYPE                   0x1

#define PLATFORM_CACHE6_FLAGS                  0xFF
#define PLATFORM_CACHE6_OFFSET                 0x170
#define PLATFORM_CACHE6_NEXT_LEVEL_INDEX       7
#define PLATFORM_CACHE6_SIZE                   0x10000
#define PLATFORM_CACHE6_CACHE_ID               0x2001
#define PLATFORM_CACHE6_IS_PRIVATE             0x1
#define PLATFORM_CACHE6_TYPE                   0x0

#define PLATFORM_CACHE7_FLAGS                  0xFF
#define PLATFORM_CACHE7_OFFSET                 0x1A8
#define PLATFORM_CACHE7_NEXT_LEVEL_INDEX       -1
#define PLATFORM_CACHE7_SIZE                   0x100000
#define PLATFORM_CACHE7_CACHE_ID               0x2003
#define PLATFORM_CACHE7_IS_PRIVATE             0x1
#define PLATFORM_CACHE7_TYPE                   0x2

#define PLATFORM_CACHE8_FLAGS                  0xFF
#define PLATFORM_CACHE8_OFFSET                 0x18C
#define PLATFORM_CACHE8_NEXT_LEVEL_INDEX       7
#define PLATFORM_CACHE8_SIZE                   0x10000
#define PLATFORM_CACHE8_CACHE_ID               0x2002
#define PLATFORM_CACHE8_IS_PRIVATE             0x1
#define PLATFORM_CACHE8_TYPE                   0x1

#define PLATFORM_CACHE9_FLAGS                  0xFF
#define PLATFORM_CACHE9_OFFSET                 0x1f4
#define PLATFORM_CACHE9_NEXT_LEVEL_INDEX       10
#define PLATFORM_CACHE9_SIZE                   0x10000
#define PLATFORM_CACHE9_CACHE_ID               0x3001
#define PLATFORM_CACHE9_IS_PRIVATE             0x1
#define PLATFORM_CACHE9_TYPE                   0x0

#define PLATFORM_CACHE10_FLAGS                 0xFF
#define PLATFORM_CACHE10_OFFSET                0x22c
#define PLATFORM_CACHE10_NEXT_LEVEL_INDEX     -1
#define PLATFORM_CACHE10_SIZE                  0x100000
#define PLATFORM_CACHE10_CACHE_ID              0x3003
#define PLATFORM_CACHE10_IS_PRIVATE            0x1
#define PLATFORM_CACHE10_TYPE                  0x2

#define PLATFORM_CACHE11_FLAGS                 0xFF
#define PLATFORM_CACHE11_OFFSET                0x210
#define PLATFORM_CACHE11_NEXT_LEVEL_INDEX      10
#define PLATFORM_CACHE11_SIZE                  0x10000
#define PLATFORM_CACHE11_CACHE_ID              0x3002
#define PLATFORM_CACHE11_IS_PRIVATE            0x1
#define PLATFORM_CACHE11_TYPE                  0x1

#define PLATFORM_CACHE12_FLAGS                 0xFF
#define PLATFORM_CACHE12_OFFSET                0x278
#define PLATFORM_CACHE12_NEXT_LEVEL_INDEX      13
#define PLATFORM_CACHE12_SIZE                  0x10000
#define PLATFORM_CACHE12_CACHE_ID              0x4001
#define PLATFORM_CACHE12_IS_PRIVATE            0x1
#define PLATFORM_CACHE12_TYPE                  0x0

#define PLATFORM_CACHE13_FLAGS                 0xFF
#define PLATFORM_CACHE13_OFFSET                0x2b0
#define PLATFORM_CACHE13_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE13_SIZE                  0x100000
#define PLATFORM_CACHE13_CACHE_ID              0x4003
#define PLATFORM_CACHE13_IS_PRIVATE            0x1
#define PLATFORM_CACHE13_TYPE                  0x2

#define PLATFORM_CACHE14_FLAGS                 0xFF
#define PLATFORM_CACHE14_OFFSET                0x294
#define PLATFORM_CACHE14_NEXT_LEVEL_INDEX      13
#define PLATFORM_CACHE14_SIZE                  0x10000
#define PLATFORM_CACHE14_CACHE_ID              0x4002
#define PLATFORM_CACHE14_IS_PRIVATE            0x1
#define PLATFORM_CACHE14_TYPE                  0x1

#define PLATFORM_CACHE15_FLAGS                 0xFF
#define PLATFORM_CACHE15_OFFSET                0x2fc
#define PLATFORM_CACHE15_NEXT_LEVEL_INDEX      16
#define PLATFORM_CACHE15_SIZE                  0x10000
#define PLATFORM_CACHE15_CACHE_ID              0x5001
#define PLATFORM_CACHE15_IS_PRIVATE            0x1
#define PLATFORM_CACHE15_TYPE                  0x0

#define PLATFORM_CACHE16_FLAGS                 0xFF
#define PLATFORM_CACHE16_OFFSET                0x334
#define PLATFORM_CACHE16_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE16_SIZE                  0x100000
#define PLATFORM_CACHE16_CACHE_ID              0x5003
#define PLATFORM_CACHE16_IS_PRIVATE            0x1
#define PLATFORM_CACHE16_TYPE                  0x2

#define PLATFORM_CACHE17_FLAGS                 0xFF
#define PLATFORM_CACHE17_OFFSET                0x318
#define PLATFORM_CACHE17_NEXT_LEVEL_INDEX      16
#define PLATFORM_CACHE17_SIZE                  0x10000
#define PLATFORM_CACHE17_CACHE_ID              0x5002
#define PLATFORM_CACHE17_IS_PRIVATE            0x1
#define PLATFORM_CACHE17_TYPE                  0x1

#define PLATFORM_CACHE18_FLAGS                 0xFF
#define PLATFORM_CACHE18_OFFSET                0x380
#define PLATFORM_CACHE18_NEXT_LEVEL_INDEX      19
#define PLATFORM_CACHE18_SIZE                  0x10000
#define PLATFORM_CACHE18_CACHE_ID              0x6001
#define PLATFORM_CACHE18_IS_PRIVATE            0x1
#define PLATFORM_CACHE18_TYPE                  0x0

#define PLATFORM_CACHE19_FLAGS                 0xFF
#define PLATFORM_CACHE19_OFFSET                0x3b8
#define PLATFORM_CACHE19_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE19_SIZE                  0x100000
#define PLATFORM_CACHE19_CACHE_ID              0x6003
#define PLATFORM_CACHE19_IS_PRIVATE            0x1
#define PLATFORM_CACHE19_TYPE                  0x2

#define PLATFORM_CACHE20_FLAGS                 0xFF
#define PLATFORM_CACHE20_OFFSET                0x39c
#define PLATFORM_CACHE20_NEXT_LEVEL_INDEX      19
#define PLATFORM_CACHE20_SIZE                  0x10000
#define PLATFORM_CACHE20_CACHE_ID              0x6002
#define PLATFORM_CACHE20_IS_PRIVATE            0x1
#define PLATFORM_CACHE20_TYPE                  0x1

#define PLATFORM_CACHE21_FLAGS                 0xFF
#define PLATFORM_CACHE21_OFFSET                0x404
#define PLATFORM_CACHE21_NEXT_LEVEL_INDEX      22
#define PLATFORM_CACHE21_SIZE                  0x10000
#define PLATFORM_CACHE21_CACHE_ID              0x7001
#define PLATFORM_CACHE21_IS_PRIVATE            0x1
#define PLATFORM_CACHE21_TYPE                  0x0

#define PLATFORM_CACHE22_FLAGS                 0xFF
#define PLATFORM_CACHE22_OFFSET                0x43c
#define PLATFORM_CACHE22_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE22_SIZE                  0x100000
#define PLATFORM_CACHE22_CACHE_ID              0x7003
#define PLATFORM_CACHE22_IS_PRIVATE            0x1
#define PLATFORM_CACHE22_TYPE                  0x2

#define PLATFORM_CACHE23_FLAGS                 0xFF
#define PLATFORM_CACHE23_OFFSET                0x420
#define PLATFORM_CACHE23_NEXT_LEVEL_INDEX      22
#define PLATFORM_CACHE23_SIZE                  0x10000
#define PLATFORM_CACHE23_CACHE_ID              0x7002
#define PLATFORM_CACHE23_IS_PRIVATE            0x1
#define PLATFORM_CACHE23_TYPE                  0x1

#define PLATFORM_CACHE24_FLAGS                 0xFF
#define PLATFORM_CACHE24_OFFSET                0x488
#define PLATFORM_CACHE24_NEXT_LEVEL_INDEX      25
#define PLATFORM_CACHE24_SIZE                  0x10000
#define PLATFORM_CACHE24_CACHE_ID              0x8001
#define PLATFORM_CACHE24_IS_PRIVATE            0x1
#define PLATFORM_CACHE24_TYPE                  0x0

#define PLATFORM_CACHE25_FLAGS                 0xFF
#define PLATFORM_CACHE25_OFFSET                0x4c0
#define PLATFORM_CACHE25_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE25_SIZE                  0x100000
#define PLATFORM_CACHE25_CACHE_ID              0x8003
#define PLATFORM_CACHE25_IS_PRIVATE            0x1
#define PLATFORM_CACHE25_TYPE                  0x2

#define PLATFORM_CACHE26_FLAGS                 0xFF
#define PLATFORM_CACHE26_OFFSET                0x4a4
#define PLATFORM_CACHE26_NEXT_LEVEL_INDEX      25
#define PLATFORM_CACHE26_SIZE                  0x10000
#define PLATFORM_CACHE26_CACHE_ID              0x8002
#define PLATFORM_CACHE26_IS_PRIVATE            0x1
#define PLATFORM_CACHE26_TYPE                  0x1

#define PLATFORM_CACHE27_FLAGS                 0xFF
#define PLATFORM_CACHE27_OFFSET                0x50c
#define PLATFORM_CACHE27_NEXT_LEVEL_INDEX      28
#define PLATFORM_CACHE27_SIZE                  0x10000
#define PLATFORM_CACHE27_CACHE_ID              0x9001
#define PLATFORM_CACHE27_IS_PRIVATE            0x1
#define PLATFORM_CACHE27_TYPE                  0x0

#define PLATFORM_CACHE28_FLAGS                 0xFF
#define PLATFORM_CACHE28_OFFSET                0x544
#define PLATFORM_CACHE28_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE28_SIZE                  0x100000
#define PLATFORM_CACHE28_CACHE_ID              0x9003
#define PLATFORM_CACHE28_IS_PRIVATE            0x1
#define PLATFORM_CACHE28_TYPE                  0x2

#define PLATFORM_CACHE29_FLAGS                 0xFF
#define PLATFORM_CACHE29_OFFSET                0x528
#define PLATFORM_CACHE29_NEXT_LEVEL_INDEX      28
#define PLATFORM_CACHE29_SIZE                  0x10000
#define PLATFORM_CACHE29_CACHE_ID              0x9002
#define PLATFORM_CACHE29_IS_PRIVATE            0x1
#define PLATFORM_CACHE29_TYPE                  0x1

#define PLATFORM_CACHE30_FLAGS                 0xFF
#define PLATFORM_CACHE30_OFFSET                0x590
#define PLATFORM_CACHE30_NEXT_LEVEL_INDEX      31
#define PLATFORM_CACHE30_SIZE                  0x10000
#define PLATFORM_CACHE30_CACHE_ID              0xa001
#define PLATFORM_CACHE30_IS_PRIVATE            0x1
#define PLATFORM_CACHE30_TYPE                  0x0

#define PLATFORM_CACHE31_FLAGS                 0xFF
#define PLATFORM_CACHE31_OFFSET                0x5c8
#define PLATFORM_CACHE31_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE31_SIZE                  0x100000
#define PLATFORM_CACHE31_CACHE_ID              0xa003
#define PLATFORM_CACHE31_IS_PRIVATE            0x1
#define PLATFORM_CACHE31_TYPE                  0x2

#define PLATFORM_CACHE32_FLAGS                 0xFF
#define PLATFORM_CACHE32_OFFSET                0x5ac
#define PLATFORM_CACHE32_NEXT_LEVEL_INDEX      31
#define PLATFORM_CACHE32_SIZE                  0x10000
#define PLATFORM_CACHE32_CACHE_ID              0xa002
#define PLATFORM_CACHE32_IS_PRIVATE            0x1
#define PLATFORM_CACHE32_TYPE                  0x1

#define PLATFORM_CACHE33_FLAGS                 0xFF
#define PLATFORM_CACHE33_OFFSET                0x614
#define PLATFORM_CACHE33_NEXT_LEVEL_INDEX      34
#define PLATFORM_CACHE33_SIZE                  0x10000
#define PLATFORM_CACHE33_CACHE_ID              0xb001
#define PLATFORM_CACHE33_IS_PRIVATE            0x1
#define PLATFORM_CACHE33_TYPE                  0x0

#define PLATFORM_CACHE34_FLAGS                 0xFF
#define PLATFORM_CACHE34_OFFSET                0x64c
#define PLATFORM_CACHE34_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE34_SIZE                  0x100000
#define PLATFORM_CACHE34_CACHE_ID              0xb003
#define PLATFORM_CACHE34_IS_PRIVATE            0x1
#define PLATFORM_CACHE34_TYPE                  0x2

#define PLATFORM_CACHE35_FLAGS                 0xFF
#define PLATFORM_CACHE35_OFFSET                0x630
#define PLATFORM_CACHE35_NEXT_LEVEL_INDEX      34
#define PLATFORM_CACHE35_SIZE                  0x10000
#define PLATFORM_CACHE35_CACHE_ID              0xb002
#define PLATFORM_CACHE35_IS_PRIVATE            0x1
#define PLATFORM_CACHE35_TYPE                  0x1

#define PLATFORM_CACHE36_FLAGS                 0xFF
#define PLATFORM_CACHE36_OFFSET                0x698
#define PLATFORM_CACHE36_NEXT_LEVEL_INDEX      37
#define PLATFORM_CACHE36_SIZE                  0x10000
#define PLATFORM_CACHE36_CACHE_ID              0xc001
#define PLATFORM_CACHE36_IS_PRIVATE            0x1
#define PLATFORM_CACHE36_TYPE                  0x0

#define PLATFORM_CACHE37_FLAGS                 0xFF
#define PLATFORM_CACHE37_OFFSET                0x6d0
#define PLATFORM_CACHE37_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE37_SIZE                  0x100000
#define PLATFORM_CACHE37_CACHE_ID              0xc003
#define PLATFORM_CACHE37_IS_PRIVATE            0x1
#define PLATFORM_CACHE37_TYPE                  0x2

#define PLATFORM_CACHE38_FLAGS                 0xFF
#define PLATFORM_CACHE38_OFFSET                0x6b4
#define PLATFORM_CACHE38_NEXT_LEVEL_INDEX      37
#define PLATFORM_CACHE38_SIZE                  0x10000
#define PLATFORM_CACHE38_CACHE_ID              0xc002
#define PLATFORM_CACHE38_IS_PRIVATE            0x1
#define PLATFORM_CACHE38_TYPE                  0x1

#define PLATFORM_CACHE39_FLAGS                 0xFF
#define PLATFORM_CACHE39_OFFSET                0x71c
#define PLATFORM_CACHE39_NEXT_LEVEL_INDEX      40
#define PLATFORM_CACHE39_SIZE                  0x10000
#define PLATFORM_CACHE39_CACHE_ID              0xd001
#define PLATFORM_CACHE39_IS_PRIVATE            0x1
#define PLATFORM_CACHE39_TYPE                  0x0

#define PLATFORM_CACHE40_FLAGS                 0xFF
#define PLATFORM_CACHE40_OFFSET                0x754
#define PLATFORM_CACHE40_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE40_SIZE                  0x100000
#define PLATFORM_CACHE40_CACHE_ID              0xd003
#define PLATFORM_CACHE40_IS_PRIVATE            0x1
#define PLATFORM_CACHE40_TYPE                  0x2

#define PLATFORM_CACHE41_FLAGS                 0xFF
#define PLATFORM_CACHE41_OFFSET                0x738
#define PLATFORM_CACHE41_NEXT_LEVEL_INDEX      40
#define PLATFORM_CACHE41_SIZE                  0x10000
#define PLATFORM_CACHE41_CACHE_ID              0xd002
#define PLATFORM_CACHE41_IS_PRIVATE            0x1
#define PLATFORM_CACHE41_TYPE                  0x1

#define PLATFORM_CACHE42_FLAGS                 0xFF
#define PLATFORM_CACHE42_OFFSET                0x7a0
#define PLATFORM_CACHE42_NEXT_LEVEL_INDEX      43
#define PLATFORM_CACHE42_SIZE                  0x10000
#define PLATFORM_CACHE42_CACHE_ID              0xe001
#define PLATFORM_CACHE42_IS_PRIVATE            0x1
#define PLATFORM_CACHE42_TYPE                  0x0

#define PLATFORM_CACHE43_FLAGS                 0xFF
#define PLATFORM_CACHE43_OFFSET                0x7d8
#define PLATFORM_CACHE43_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE43_SIZE                  0x100000
#define PLATFORM_CACHE43_CACHE_ID              0xe003
#define PLATFORM_CACHE43_IS_PRIVATE            0x1
#define PLATFORM_CACHE43_TYPE                  0x2

#define PLATFORM_CACHE44_FLAGS                 0xFF
#define PLATFORM_CACHE44_OFFSET                0x7bc
#define PLATFORM_CACHE44_NEXT_LEVEL_INDEX      43
#define PLATFORM_CACHE44_SIZE                  0x10000
#define PLATFORM_CACHE44_CACHE_ID              0xe002
#define PLATFORM_CACHE44_IS_PRIVATE            0x1
#define PLATFORM_CACHE44_TYPE                  0x1

#define PLATFORM_CACHE45_FLAGS                 0xFF
#define PLATFORM_CACHE45_OFFSET                0x824
#define PLATFORM_CACHE45_NEXT_LEVEL_INDEX      46
#define PLATFORM_CACHE45_SIZE                  0x10000
#define PLATFORM_CACHE45_CACHE_ID              0xf001
#define PLATFORM_CACHE45_IS_PRIVATE            0x1
#define PLATFORM_CACHE45_TYPE                  0x0

#define PLATFORM_CACHE46_FLAGS                 0xFF
#define PLATFORM_CACHE46_OFFSET                0x85c
#define PLATFORM_CACHE46_NEXT_LEVEL_INDEX      -1
#define PLATFORM_CACHE46_SIZE                  0x100000
#define PLATFORM_CACHE46_CACHE_ID              0xf003
#define PLATFORM_CACHE46_IS_PRIVATE            0x1
#define PLATFORM_CACHE46_TYPE                  0x2

#define PLATFORM_CACHE47_FLAGS                 0xFF
#define PLATFORM_CACHE47_OFFSET                0x840
#define PLATFORM_CACHE47_NEXT_LEVEL_INDEX      46
#define PLATFORM_CACHE47_SIZE                  0x10000
#define PLATFORM_CACHE47_CACHE_ID              0xf002
#define PLATFORM_CACHE47_IS_PRIVATE            0x1
#define PLATFORM_CACHE47_TYPE                  0x1


#define PLATFORM_PPTT0_CACHEID0  0x1
#define PLATFORM_PPTT0_CACHEID1  0x2

#define PLATFORM_PPTT1_CACHEID0  0x1001
#define PLATFORM_PPTT1_CACHEID1  0x1002

#define PLATFORM_PPTT2_CACHEID0  0x2001
#define PLATFORM_PPTT2_CACHEID1  0x2002

#define PLATFORM_PPTT3_CACHEID0  0x3001
#define PLATFORM_PPTT3_CACHEID1  0x3002

#define PLATFORM_PPTT4_CACHEID0  0x4001
#define PLATFORM_PPTT4_CACHEID1  0x4002

#define PLATFORM_PPTT5_CACHEID0  0x5001
#define PLATFORM_PPTT5_CACHEID1  0x5002

#define PLATFORM_PPTT6_CACHEID0  0x6001
#define PLATFORM_PPTT6_CACHEID1  0x6002

#define PLATFORM_PPTT7_CACHEID0  0x7001
#define PLATFORM_PPTT7_CACHEID1  0x7002

#define PLATFORM_PPTT8_CACHEID0  0x8001
#define PLATFORM_PPTT8_CACHEID1  0x8002

#define PLATFORM_PPTT9_CACHEID0  0x9001
#define PLATFORM_PPTT9_CACHEID1  0x9002

#define PLATFORM_PPTT10_CACHEID0 0xa001
#define PLATFORM_PPTT10_CACHEID1 0xa002

#define PLATFORM_PPTT11_CACHEID0 0xb001
#define PLATFORM_PPTT11_CACHEID1 0xb002

#define PLATFORM_PPTT12_CACHEID0 0xc001
#define PLATFORM_PPTT12_CACHEID1 0xc002

#define PLATFORM_PPTT13_CACHEID0 0xd001
#define PLATFORM_PPTT13_CACHEID1 0xd002

#define PLATFORM_PPTT14_CACHEID0 0xe001
#define PLATFORM_PPTT14_CACHEID1 0xe002

#define PLATFORM_PPTT15_CACHEID0 0xf001
#define PLATFORM_PPTT15_CACHEID1 0xf002

/* SRAT config */
#define PLATFORM_OVERRIDE_NUM_SRAT_ENTRIES  0
#define PLATFORM_OVERRIDE_MEM_AFF_CNT       0
#define PLATFORM_OVERRIDE_GICC_AFF_CNT      0

#define PLATFORM_SRAT_MEM0_PROX_DOMAIN      0x0
#define PLATFORM_SRAT_MEM0_FLAGS            0x1
#define PLATFORM_SRAT_MEM0_ADDR_BASE        0x8080000000
#define PLATFORM_SRAT_MEM0_ADDR_LEN         0x3F7F7FFFFFF

#define PLATFORM_SRAT_GICC0_PROX_DOMAIN     0x0
#define PLATFORM_SRAT_GICC0_PROC_UID        0x0
#define PLATFORM_SRAT_GICC0_FLAGS           0x1
#define PLATFORM_SRAT_GICC0_CLK_DOMAIN      0x0

#define PLATFORM_SRAT_GICC1_PROX_DOMAIN     0x0
#define PLATFORM_SRAT_GICC1_PROC_UID        0x1
#define PLATFORM_SRAT_GICC1_FLAGS           0x1
#define PLATFORM_SRAT_GICC1_CLK_DOMAIN      0x0

#define PLATFORM_SRAT_GICC2_PROX_DOMAIN     0x0
#define PLATFORM_SRAT_GICC2_PROC_UID        0x2
#define PLATFORM_SRAT_GICC2_FLAGS           0x1
#define PLATFORM_SRAT_GICC2_CLK_DOMAIN      0x0

#define PLATFORM_SRAT_GICC3_PROX_DOMAIN     0x0
#define PLATFORM_SRAT_GICC3_PROC_UID        0x3
#define PLATFORM_SRAT_GICC3_FLAGS           0x1
#define PLATFORM_SRAT_GICC3_CLK_DOMAIN      0x0

#define PLATFORM_SRAT_GICC4_PROX_DOMAIN     0x0
#define PLATFORM_SRAT_GICC4_PROC_UID        0x4
#define PLATFORM_SRAT_GICC4_FLAGS           0x1
#define PLATFORM_SRAT_GICC4_CLK_DOMAIN      0x0

#define PLATFORM_SRAT_GICC5_PROX_DOMAIN     0x0
#define PLATFORM_SRAT_GICC5_PROC_UID        0x5
#define PLATFORM_SRAT_GICC5_FLAGS           0x1
#define PLATFORM_SRAT_GICC5_CLK_DOMAIN      0x0

#define PLATFORM_SRAT_GICC6_PROX_DOMAIN     0x0
#define PLATFORM_SRAT_GICC6_PROC_UID        0x6
#define PLATFORM_SRAT_GICC6_FLAGS           0x1
#define PLATFORM_SRAT_GICC6_CLK_DOMAIN      0x0

#define PLATFORM_SRAT_GICC7_PROX_DOMAIN     0x0
#define PLATFORM_SRAT_GICC7_PROC_UID        0x7
#define PLATFORM_SRAT_GICC7_FLAGS           0x1
#define PLATFORM_SRAT_GICC7_CLK_DOMAIN      0x0

#define PLATFORM_SRAT_GICC8_PROX_DOMAIN     0x0
#define PLATFORM_SRAT_GICC8_PROC_UID        0x8
#define PLATFORM_SRAT_GICC8_FLAGS           0x1
#define PLATFORM_SRAT_GICC8_CLK_DOMAIN      0x0

#define PLATFORM_SRAT_GICC9_PROX_DOMAIN     0x0
#define PLATFORM_SRAT_GICC9_PROC_UID        0x9
#define PLATFORM_SRAT_GICC9_FLAGS           0x1
#define PLATFORM_SRAT_GICC9_CLK_DOMAIN      0x0

#define PLATFORM_SRAT_GICC10_PROX_DOMAIN    0x0
#define PLATFORM_SRAT_GICC10_PROC_UID       0xA
#define PLATFORM_SRAT_GICC10_FLAGS          0x1
#define PLATFORM_SRAT_GICC10_CLK_DOMAIN     0x0

#define PLATFORM_SRAT_GICC11_PROX_DOMAIN    0x0
#define PLATFORM_SRAT_GICC11_PROC_UID       0xB
#define PLATFORM_SRAT_GICC11_FLAGS          0x1
#define PLATFORM_SRAT_GICC11_CLK_DOMAIN     0x0

#define PLATFORM_SRAT_GICC12_PROX_DOMAIN    0x0
#define PLATFORM_SRAT_GICC12_PROC_UID       0xC
#define PLATFORM_SRAT_GICC12_FLAGS          0x1
#define PLATFORM_SRAT_GICC12_CLK_DOMAIN     0x0

#define PLATFORM_SRAT_GICC13_PROX_DOMAIN    0x0
#define PLATFORM_SRAT_GICC13_PROC_UID       0xD
#define PLATFORM_SRAT_GICC13_FLAGS          0x1
#define PLATFORM_SRAT_GICC13_CLK_DOMAIN     0x0

#define PLATFORM_SRAT_GICC14_PROX_DOMAIN    0x0
#define PLATFORM_SRAT_GICC14_PROC_UID       0xE
#define PLATFORM_SRAT_GICC14_FLAGS          0x1
#define PLATFORM_SRAT_GICC14_CLK_DOMAIN     0x0

#define PLATFORM_SRAT_GICC15_PROX_DOMAIN    0x0
#define PLATFORM_SRAT_GICC15_PROC_UID       0xF
#define PLATFORM_SRAT_GICC15_FLAGS          0x1
#define PLATFORM_SRAT_GICC15_CLK_DOMAIN     0x0

#define PLATFORM_OVERRIDE_NUM_OF_HMAT_PROX_DOMAIN 0
#define PLATFORM_OVERRIDE_HMAT_MEM_ENTRIES    0x0

#define HMAT_NODE_MEM_SLLBIC                  0x1
#define HMAT_NODE_MEM_SLLBIC_DATA_TYPE        0x3
#define HMAT_NODE_MEM_SLLBIC_FLAGS            0x0
#define HMAT_NODE_MEM_SLLBIC_ENTRY_BASE_UNIT  0x64

#define PLATFORM_HMAT_MEM0_PROX_DOMAIN        0x0
#define PLATFORM_HMAT_MEM0_MAX_WRITE_BW       0x82
#define PLATFORM_HMAT_MEM0_MAX_READ_BW        0x82

#define PLATFORM_HMAT_MEM1_PROX_DOMAIN        0x1
#define PLATFORM_HMAT_MEM1_MAX_WRITE_BW       0x8c
#define PLATFORM_HMAT_MEM1_MAX_READ_BW        0x8c

#define PLATFORM_HMAT_MEM2_PROX_DOMAIN        0x2
#define PLATFORM_HMAT_MEM2_MAX_WRITE_BW       0x96
#define PLATFORM_HMAT_MEM2_MAX_READ_BW        0x96

#define PLATFORM_HMAT_MEM3_PROX_DOMAIN        0x3
#define PLATFORM_HMAT_MEM3_MAX_WRITE_BW       0xa0
#define PLATFORM_HMAT_MEM3_MAX_READ_BW        0xa0

/* APMT config */

#define MAX_NUM_OF_PMU_SUPPORTED              512
#define PLATFORM_OVERRIDE_PMU_NODE_CNT        0x0

#define PLATFORM_PMU_NODE0_BASE0              0x1010028000
#define PLATFORM_PMU_NODE0_BASE1              0x0
#define PLATFORM_PMU_NODE0_TYPE               0x2
#define PLATFORM_PMU_NODE0_PRI_INSTANCE       0x0
#define PLATFORM_PMU_NODE0_SEC_INSTANCE       0x0
#define PLATFORM_PMU_NODE0_DUAL_PAGE_EXT      0x0

/* RAS Config */
#define RAS_MAX_NUM_NODES                     140
#define RAS_MAX_INTR_TYPE                     0x2
#define PLATFORM_OVERRIDE_NUM_RAS_NODES       0x0
#define PLATFORM_OVERRIDE_NUM_PE_RAS_NODES    0x0
#define PLATFORM_OVERRIDE_NUM_MC_RAS_NODES    0x0

/* RAS Node Data */
#define PLATFORM_RAS_NODE0_LENGTH             0x0
#define PLATFORM_RAS_NODE0_NUM_INTR_ENTRY     0x0

#define PLATFORM_RAS_NODE0_PE_PROCESSOR_ID    0x0
#define PLATFORM_RAS_NODE0_PE_RES_TYPE        0x0
#define PLATFORM_RAS_NODE0_PE_FLAGS           0x0
#define PLATFORM_RAS_NODE0_PE_AFF             0x0
#define PLATFORM_RAS_NODE0_PE_RES_DATA        0x0

#define PLATFORM_RAS_NODE0_INTF_TYPE          0x0
#define PLATFORM_RAS_NODE0_INTF_FLAGS         0x0
#define PLATFORM_RAS_NODE0_INTF_BASE          0x0
#define PLATFORM_RAS_NODE0_INTF_START_REC     0x1
#define PLATFORM_RAS_NODE0_INTF_NUM_REC       0x1
#define PLATFORM_RAS_NODE0_INTF_ERR_REC_IMP   0x0
#define PLATFORM_RAS_NODE0_INTF_ERR_STATUS    0x0
#define PLATFORM_RAS_NODE0_INTF_ADDR_MODE     0x0

#define PLATFORM_RAS_NODE0_INTR0_TYPE         0x0
#define PLATFORM_RAS_NODE0_INTR0_FLAG         0x1
#define PLATFORM_RAS_NODE0_INTR0_GSIV         0x11
#define PLATFORM_RAS_NODE0_INTR0_ITS_ID       0x0

#define RAS2_MAX_NUM_BLOCKS                   0x0
#define PLATFORM_OVERRIDE_NUM_RAS2_BLOCK      0x0
#define PLATFORM_OVERRIDE_NUM_RAS2_MEM_BLOCK  0x0

#define PLATFORM_OVERRIDE_RAS2_BLOCK0_PROXIMITY             0x0
#define PLATFORM_OVERRIDE_RAS2_BLOCK0_PATROL_SCRUB_SUPPORT  0x1
#define PLATFORM_OVERRIDE_RAS2_BLOCK1_PROXIMITY             0x1
#define PLATFORM_OVERRIDE_RAS2_BLOCK1_PATROL_SCRUB_SUPPORT  0x1
#define PLATFORM_OVERRIDE_RAS2_BLOCK2_PROXIMITY             0x2
#define PLATFORM_OVERRIDE_RAS2_BLOCK2_PATROL_SCRUB_SUPPORT  0x1

/*MPAM Config*/
#define MPAM_MAX_MSC_NODE                     0x0
#define MPAM_MAX_RSRC_NODE                    0x0
#define PLATFORM_MPAM_MSC_COUNT               0x0

#define PLATFORM_MPAM_MSC0_BASE_ADDR          0x1010028000
#define PLATFORM_MPAM_MSC0_ADDR_LEN           0x2004
#define PLATFORM_MPAM_MSC0_MAX_NRDY           10000000
#define PLATFORM_MPAM_MSC0_RSRC_COUNT         0x1

#define PLATFORM_MPAM_MSC0_RSRC0_RIS_INDEX    0x0
#define PLATFORM_MPAM_MSC0_RSRC0_LOCATOR_TYPE 0x1
#define PLATFORM_MPAM_MSC0_RSRC0_DESCRIPTOR1  0x0
#define PLATFORM_MPAM_MSC0_RSRC0_DESCRIPTOR2  0x0

/** End config **/
