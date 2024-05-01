******************************************************
Memory Model Consistency Test Scenario and User Guide
******************************************************
.. raw:: pdf

   PageBreak

.. section-numbering::

.. contents::
      :depth: 4

Introduction
============

This document provides detailed information on the test scenarios for memory model consistency tests.
It includes instructions for analyzing the log output of litmus tests to help users infer the memory model.
Additionally, an overview of the source code directory structure is provided, explaining how the source code
is structured.

Test Suite Overview
===================

.. list-table::
  :widths: 5 25 75

  * - Test number
    - Test name
    - Description
  * - 1
    - 2+2W+dmb.sys
    - | Tests the effect of coherence order per location between two PEs.
      | Validated if there is an execution where: P0 performs a STR of 2 to x
      | and a STR of 1 to y with a DMB SY between them;  P1 performs a STR of 2
      | to y and a STR of 1 to y, with a DMB SY between them. At the end, x=2
      | and y=2.

  * - 2
    - CO-MIXED-20cc+H
    - | A test with reads and writes of different sizes to the same location.
      | Validated if there is an execution where: P0 performs a 8bit STR of 1 to
      | x and then a 16bit LDR from x reading 0x0201; P1 performs a 16bit STR of
      | 0x0202 to x. At the end, location x has the value 0x0202.

  * - 3
    - CoRR
    - | Tests whether two reads from the same location in program order can be
      | reordered. Validated if there is an execution where: P0 performs a STR
      | of 1 to x;  P1 performs a LDR reading 1 and then a LDR x from x reading 0.

  * - 4
    - CoRW1
    - | Tests whether a read has to get its value from a write to the same
      | location that follows in program order. Validated if there is an
      | execution where: P0 performs a LDR from x, reading 1 and then a STR of 1
      | to x.

  * - 5
    - CoRW2+posb1b0+h0
    - | A test with reads and writes of different sizes to the same location.
      | Validated if there is an execution where: P0 performs a 16bit STR of
      | 0x101 to x; P1 performs a 8bit LDR from the address x + 1 reading 0 or
      | 1, then a 8bit STR of 2 to x, and then a 16bit LDR from x reading 0x102.
      | At the end, location x has the value 0x101.

  * - 6
    - CoRW2
    - | A test with a read and writes to the same location. Validated if there
      | is an execution where: P0 performs a STR of 1 to x; P1 performs a LDR
      | from x reading 1, and then a STR of 2 to x. At the end, location x has
      | the value 1.

  * - 7
    - CoWR
    - | Single PE test that shows that a read that is in program order after a
      | STR to the same location gets its value from that write. Validated if
      | there is an execution where: P0 performs a STR of 1 to x and then a LDR
      | from x reading 0.

  * - 8
    - CoWW
    - | Single PE test that shows that two writes to the same location cannot be
      | observed out of order. Validated if there is an execution where: P0
      | performs a STR of 1 to x and then a STR of 2 to x. At the end, location
      | x has the value 1.

  * - 9
    - LB+BEQ4
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from y  reading 1 and a STR
      | release of 1 to x; P1 performs a LDR from x reading 1 and a STR of 1 to
      | y with a control dependency in between.

  * - 10
    - LB+CSEL-addr-po+DMB
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1 and a STR of 1
      | to a which has a pick address dependency from the preceding LDR, and
      | finally a STR of 1 to y; P1 performs a LDR from y reading 1, then a dmb
      | sy, and then a STR of 1 to x.

  * - 11
    - LB+CSEL-rfi-data+DMB
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1, then a STR to
      | z which has a pick data dependency from the preceding LDR, then a LDR
      | from z, and finally a STR of 1 to y which has a data dependency from the
      | preceding LDR; P1 performs a LDR from y reading 1, then dmb sy, and then
      | a STR of 1 to x.

  * - 12
    - LB+dmb.sy+data-wsi-wsi+MIXED+H
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs an 8bit LDR from x reading 2, then a
      | dmb sy, and finally a 16bit STR of 1 to y; P1 performs a 8bit LDR from y
      | reading 1, then an 8bit STR of 1 to the address x+1, then a 16bit STR of
      | 2 to x, and finally a 16bit STR of 3 to x. At the end, location x has
      | the value 3.

  * - 13
    - LB+dmb.sys
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1 and a STR of 1
      | to y, with a DMB SY between them; P1 performs a LDR from y reading 1 and
      | a STR of 1 to x, with a DMB SY between them.

  * - 14
    - LB+rel+BEQ2
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from y reading 1 and a STR
      | release of 1 to x; P1 performs a LDR from x reading 1 and a STR of 1 to
      | y with a data dependency in between.

  * - 15
    - LB+rel+CSEL-CSEL
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1 and a STR
      | release of 1 to y; P1 performs a LDR from y reading 1, and finally a STR
      | of 1 to x which has a pick data dependency from the preceding LDR.

  * - 16
    - LB+rel+data
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1 and a STR
      | release of 1 to y; P1 performs a LDR from y reading 1, and finally a STR
      | of 1 to x which has a data dependency from the preceding LDR.

  * - 17
    - MP+dmb.sys
    - | Tests the effect of message passing between two PEs. Validated if there
      | is an execution where: P0 performs a STR of 1 to x (message) and a STR
      | of 1 to y (flag) with a DMB SY between them; P1 performs a LDR from y
      | (flag) reading 1 and a LDR from x (message) reading 0 with a DMB SY
      | between them.

  * - 18
    - MP-Koeln
    - | Tests the effect of message passing between two PEs. Validated if there
      | is an execution where: P0 performs a 16bit STR of 0x101 to x, a 8bit STR
      | release of 1 to y, and a 16bit STR of 0x202 to y; P1 performs a 8bit LDR
      | from the address y+1 reading 2, and then a 16 bit LDR from x reading 0.
      | At the end, location y has the value 0x202.

  * - 19
    - R+dmb.sys
    - | Tests the effect of communication between two PEs. Validated if there is
      | an execution where: P0 performs a STR of 1 to x and a STR of 1 to y with
      | a DMB SY between them; P1 performs a STR of 2 to y and a LDR from x
      | reading 0 with a DMB SY between them. At the end, y=2.

  * - 20
    - S+dmb.sys
    - | Tests the effect of communication between two PEs. Validated if there is
      | an execution where: P0 performs a STR of 1 to x and a STR of 2 to y with
      | a DMB SY between them; P1 performs a LDR from y reading 1 and a STR of 1
      | to x with a DMB SY between them. At the end, y=2.

  * - 21
    - S+rel+CSEL-data
    - | Tests the effect of message passing between two PEs. Validated if there
      | is an execution where: P0 performs a  STR of 1 to x, and a STR release
      | of 1 to y; P1 performs a LDR from the address y reading 1, then a STR of
      | 0 to z which has a pick data dependency from the preceding LDR, then a
      | LDR from z reading 0, and then a STR of 2 to x which has a data
      | dependency from the preceding LDR. At the end, location x has the value 1.

  * - 22
    - S+rel+CSEL-rf-reg
    - | Tests the effect of message passing between two PEs. Validated if there
      | is an execution where: P0 performs a  STR of 1 to x, and a STR release
      | of 1 to y; P1 performs a LDR from the y reading 1, then a STR of 0 to x
      | which has a pick data dependency from the preceding LDR. At the end,
      | location x has the value 1.

  * - 23
    - SB+dmb.sys
    - | Tests the effect of store buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from y reading 1 and a STR of 1
      | to x, ordered via a DMB SY; P1 performs a LDR reading 1 from x STR of 1
      | to y, ordered via a DMB SY.

  * - 24
    - T10B
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1 and a STR
      | release of 1 to y; P1 performs a LDR from y reading 1, then a STR of 0
      | to za, then a LDR from za reading 0, then a STR of 0 to zb which has a
      | data dependecy from the predecing LDR, then a LDR from zb and finally a
      | STR of 1 to x which has an address dependency from the preceding LDR.

  * - 25
    - T10C
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1 and a STR
      | release of 1 to y; P1 performs a LDR from y reading 1, then a STR of 0
      | to za which has a pick data dependency from the preceding LDR, then a
      | LDR from za reading 0, and finally, a STR of 1 to x which has an address
      | dependency from the preceding LDR.

  * - 26
    - T15-corrected
    - | Tests the effect of message passing between two PEs. Validated if there
      | is an execution where: P0 performs a STR of 1 to x and a STR release of
      | 1 to y; P1 performs a LDR from y reading 1, then a STR of 0 to z which
      | has a pick data dependency from the preceding LDR, then a LDR from z
      | reading 0, and finally a LDR from x reading 0 which has a control
      | dependecy followed by an isb from the preceding LDR.

  * - 27
    - T15-datadep-corrected
    - | Tests the effect of message passing between two PEs. Validated if there
      | is an execution where: P0 performs a STR of 1 to x and a STR release of
      | 1 to y; P1 performs a LDR from y reading 1, then a STR of 0 to z, then a
      | LDR from z reading 0, and finally a LDR from x reading 0 which has a
      | control dependecy followed by an isb from the preceding LDR.

  * - 28
    - T3-bis
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1, a STR of 0 to
      | za which has a pick address dependency from the preceding LDR, a LDR of
      | 0 from za and a STR of 1 to y; P1 performs a LDR acquire from y reading
      | 1, then a STR of 0 to x.

  * - 29
    - T3
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1, a LDR of 0
      | from za which has a pick address dependency from the preceding LDR, and
      | a STR of 1 to y; P1 performs a LDR acquire from y reading 1, then a STR
      | of 0 to x.

  * - 30
    - T7
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1, a STR of 0 to
      | z which has a pick data dependency from the preceding LDR, a LDR acquire
      | from z reading 0, a LDR from a reading 0, and finally, a STR of 1 to
      | y; P1 performs a LDR acquire from y reading 1, then a STR of 0 to x.

  * - 31
    - T7dep
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1, a STR of 0 to
      | z which has a pick data dependency from the preceding LDR, a LDR from z
      | reading 0, a LDR from a reading 0 which has an address dependecy from
      | the preceding LDR, and finally, a STR of 1 to y; P1 performs a LDR
      | acquire from y reading 1, then a STR of 0 to x.

  * - 32
    - T8+BIS
    - | Tests the effect of load buffering between two PEs. Validated if there
      | is an execution where: P0 performs a LDR from x reading 1, a STR of 0 to
      | z which has a pick data dependency from the preceding LDR, a LDR
      | exclusive from z reading 0, a LDR from z reading 0, and finally, a STR
      | of 1 to y which has an address dependecy from the preceding LDR; P1
      | performs a LDR acquire from y reading 1, then a STR of 0 to x.

  * - 33
    - T9B
    - | Tests the effect of message passing between two PEs. Validated if there
      | is an execution where: P0 performs a STR of 1 to x and a STR release of
      | 1 to y; P1 performs a LDR from y reading 1, then a LDR from x reading 0
      | which has a pick control dependency followed by an isb from the
      | preceding LDR.

Analysing litmus tests log ouput
================================

Each litmus test outputs a result log in a format similar to the following snippet. The log of the 2+2W+dmb.sys test is taken as an example in the interest
of providing guidance for analyzing the results.

.. code-block:: text

    1  Test 2+2W+dmb.sys Forbidden
    2  Histogram (3 states)
    3  277246:>[x]=1; [y]=2;
    4  468606:>[x]=2; [y]=1;
    5  1254148:>[x]=1; [y]=1;
    6  Ok
    7  Witnesses
    8  Positive: 2000000, Negative: 0
    9  Condition ~exists ([x]=2 /\ [y]=2) is validated
    10 Hash=9495f1f810a4f034c732242a8a2c3eed
    11 Cycle=Wse DMB.SYdWW Wse DMB.SYdWW
    12 Generator=diycross7 (version 7.54+01(dev))
    13 Com=Ws Ws
    14 Orig=DMB.SYdWW Wse DMB.SYdWW Wse
    15 Observation 2+2W+dmb.sys Never 0 2000000
    16 Time 2+2W+dmb.sys 3.64

Each litmus test validates a post-condition, and Line 9 echoes this post-condition and states whether the condition was validated or not on the target memory model.
(For example, in the above scenario, it is validating the post condition whether not in their final state  both the memory location x and y had the value 2.

Line 1 provides the test name and specifies the type of test it is. The format is 'Test <name> <kind>', where <kind> can be 'Allowed', 'Forbidden', or 'Required'.
'Allowed' indicates that there can be an execution where the post-condition is validated. 'Forbidden' means that there cannot be any executions where the post-condition
is validated, and 'Required' indicates that all executions must validate the post-condition. If the user hasn’t provided any information, then the default is 'Allowed'.

Line 2-5 provides histogram of observations, each of the line captures the values of the memory locations and registers that the post-condition contains and the number
of executions that each state was observed. (For example, in the above scenario, out of 277246 executions, it was found that in their final state, location x had the value
1 and location 2 had the value 2.)

Line 6 provides the outcome of the test. 'Ok' if the expected outcome is observed. (For example, for an allowed post condition, test will print 'Ok' if there is at least
one execution that validates the post-condition), otherwise it prints 'No'.

Line 7-8 provides the observations of post condition, number of executions that validate the post-condition and number of executions that didn’t validate the post-condition.

Line 9 provides the hash of the litmus test which includes the initial state, the code and the post-condition. Two different files with potentially different names can have
the same signature which means they are the same test.

Line 11-14 contain metadata for other memory model tools, which are not relevent here.

Line 16 provides wall clock time of the execution of the test.

Source Code Directory Structure
===============================

The following structure shows the source code directory of memory model consistency tests
inside bsa-acs repository and infra required to build them into BSA ACS EFI application.

::

    📂 bsa-acs
    ├── .
    ├── 📂 uefi_app
    │  ├── BsaAcsMem.inf
    ├── 📂 mem_test
    │  ├── LibCFlat.inf
    |  ├── README.md
    |  ├── bsa_acs_litmus.h
    |  ├── efi_bsa_entry.c
    |  ├── 📂 litmus-tests
    |  ├── memTestBsaSupport.S
    |  ├── 📂 patches
    |  └── memory_model_tests_scenario_user_guide.rst
    ├── .
    └── .

.. list-table::
  :widths: 25 75

  * - Directory/File name
    - Description
  * - BsaAcsMem.inf
    - | EDK-II INF file describing memory model consistency tests source files,
      | libraries, and compiler flags.
  * - LibCFlat.inf
    - EDK-II INF file describing kvm-unit-tests source files and compiler flags.
  * - README.md
    - README file provides with build and run steps.
  * - bsa_acs_litmus.h
    - Contains test entry prototypes.
  * - efi_bsa_entry.c
    - Contains test entry function calls and initialization of test infrastructure.
  * - litmus-tests
    - Contains memory model consistency test source files.
  * - memTestBsaSupport.S
    - assembly function definitions
  * - patches
    - Contains edk2 repository patches.
  * - memory_model_tests_scenario_user_guide.rst
    - This document captures test scenario and instructions for analyzing the log output of litmus tests.


.. _footer:

.. container:: footer

  Copyright © 2023-2024, Arm Limited and Contributors. All rights reserved.