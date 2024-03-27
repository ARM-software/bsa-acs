# Base System Architecture - Architecture Compliance Suite


## Base System Architecture
**Base System Architecture** (BSA) specification specifies a hardware system architecture based on the Arm 64-bit architecture. Server system software such as operating systems, hypervisors, and firmware rely on this. It addresses PE features and key aspects of system architecture.

For more information, download the [BSA specification](https://developer.arm.com/documentation/den0094/c/?lang=en)


## BSA - Architecture Compliance Suite

BSA **Architecture Compliance Suite** (ACS) is a collection of self-checking, portable C-based tests.
This suite includes a set of examples of the invariant behaviors that are provided by the [BSA](https://developer.arm.com/documentation/den0094/c/?lang=en) specification, so that implementers can verify if these behaviours have been interpreted correctly.
The tests are executed in a baremetal environment. The initialization of the baremetal environment is specific to the environment and is out of scope of this document.

## Release details
 - Code Quality: REL v1.0.8
 - The tests are written for version 1.0 (c) of the BSA specification.
 - The compliance suite is not a substitute for design verification.
 - To review the BSA ACS logs, Arm licensees can contact Arm directly through their partner managers.

### EDA vendors
Contact your EDA vendor and ask if they include these tests as part of their verificatoin IP package.

## GitHub branch
  - To pick up the release version of the code, checkout the corresponding tag from the main branch.
  - To get the latest version of the code with bug fixes and new features, use the main branch.

## Additional reading
  - For details on the BSA ACS test execution, see the [Arm BSA ACS User Guide](../../../docs/arm_bsa_architecture_compliance_bare-metal_user_guide.pdf).
  - For details on the Design of the BSA ACS, see the [Arm BSA Validation Methodology Document](../../../docs/arm_bsa_architecture_compliance_validation_methodology.pdf).
Note: The Baremetal PCIe enumeration code provided as part of the BSA ACS should be used and should not be replaced. This code is vital in analyzing of the test result.

## Target platforms
  Any 64-bit Arm based Server design presented as a full chip Emulation or Simulation environment

## ACS build steps - UEFI Shell application

The baremetal build environment is platform specific. To provide a baseline, the build steps to integrate and run the tests from UEFI shell are provided [here](https://github.com/ARM-software/bsa-acs/blob/v23.12_REL1.0.7/platform/pal_baremetal/RDN2/README.md).

## License
BSA ACS is distributed under Apache v2.0 License.

## Feedback, contributions and support

 - For feedback, use the GitHub Issue Tracker that is associated with this repository.
 - For support, send an email to "support-systemready-acs@arm.com" with details.
 - Arm licensees may contact Arm directly through their partner managers.
 - Arm welcomes code contributions through GitHub pull requests. See GitHub documentation on how to raise pull requests.

--------------

*Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.*
