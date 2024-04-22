/** @file
 * Copyright (c) 2023-2024, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __BSA_ACS_LITMUS_H__
#define __BSA_ACS_LITMUS_H__

int _X2_2B_2W_2B_dmb_2E_sys(int argc, char **argv);
int CO_2D_MIXED_2D_20cc_2B_H(int argc, char **argv);
int CoRR(int argc, char **argv);
int CoRW1(int argc, char **argv);
int CoRW2_2B_posb1b0_2B_h0(int argc, char **argv);
int CoRW2(int argc, char **argv);
int CoWR(int argc, char **argv);
int CoWW(int argc, char **argv);
int LB_2B_BEQ4(int argc, char **argv);
int LB_2B_CSEL_2D_addr_2D_po_2B_DMB(int argc, char **argv);
int LB_2B_CSEL_2D_rfi_2D_data_2B_DMB(int argc, char **argv);
int LB_2B_dmb_2E_sy_2B_data_2D_wsi_2D_wsi_2B_MIXED_2B_H(int argc, char **argv);
int LB_2B_dmb_2E_sys(int argc, char **argv);
int LB_2B_rel_2B_BEQ2(int argc, char **argv);
int LB_2B_rel_2B_CSEL_2D_CSEL(int argc, char **argv);
int LB_2B_rel_2B_data(int argc, char **argv);
int MP_2B_dmb_2E_sys(int argc, char **argv);
int MP_2D_Koeln(int argc, char **argv);
int R_2B_dmb_2E_sys(int argc, char **argv);
int S_2B_dmb_2E_sys(int argc, char **argv);
int S_2B_rel_2B_CSEL_2D_data(int argc, char **argv);
int S_2B_rel_2B_CSEL_2D_rf_2D_reg(int argc, char **argv);
int SB_2B_dmb_2E_sys(int argc, char **argv);
int T10B(int argc, char **argv);
int T10C(int argc, char **argv);
int T15_2D_corrected(int argc, char **argv);
int T15_2D_datadep_2D_corrected(int argc, char **argv);
int T3_2D_bis(int argc, char **argv);
int T3(int argc, char **argv);
int T7(int argc, char **argv);
int T7dep(int argc, char **argv);
int T8_2B_BIS(int argc, char **argv);
int T9B(int argc, char **argv);

#endif
