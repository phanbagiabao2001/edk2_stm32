/** @file
 *
 *  Copyright (c) 2024, Phan Ba Gia Bao <phanbagiabao2001@gmail.com>
 *  Copyright (c) 2014-2016, Linaro Limited. All rights reserved.
 *  Copyright (c) 2014, Red Hat, Inc.
 *  Copyright (c) 2011-2013, ARM Limited. All rights reserved.
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
**/

#include <Library/IoLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

#include <Ppi/ArmMpCoreInfo.h>

/**
  Return the current Boot Mode

  This function returns the boot reason on the platform

  @return   Return the current Boot Mode of the platform

**/
EFI_BOOT_MODE
ArmPlatformGetBootMode (
  VOID
  )
{
  return BOOT_WITH_FULL_CONFIGURATION;
}

/**
  This function is called by PrePeiCore, in the SEC phase.
**/
RETURN_STATUS
ArmPlatformInitialize (
  IN  UINTN  MpId
  )
{
  return RETURN_SUCCESS;
}

VOID
ArmPlatformInitializeSystemMemory (
  VOID
  )
{
}

STATIC ARM_CORE_INFO mSTM32MP25InfoTable[] = {
  { 0x0, }, // Cluster 0, Core 0
  { 0x1, }, // Cluster 0, Core 1
  { 0x2, }, // Cluster 0, Core 2
  { 0x3, }, // Cluster 0, Core 3
};

STATIC
EFI_STATUS
PrePeiCoreGetMpCoreInfo (
  OUT UINTN          *CoreCount,
  OUT ARM_CORE_INFO  **ArmCoreTable
  )
{
  // Only support one cluster
  *CoreCount = sizeof (mSTM32MP25InfoTable) / sizeof (ARM_CORE_INFO);
  *ArmCoreTable = mSTM32MP25InfoTable;

  return EFI_SUCCESS;
}

STATIC ARM_MP_CORE_INFO_PPI mMpCoreInfoPpi = {
  PrePeiCoreGetMpCoreInfo
};
STATIC EFI_PEI_PPI_DESCRIPTOR mPlatformPpiTable[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gArmMpCoreInfoPpiGuid,
    &mMpCoreInfoPpi
  }
};

VOID
ArmPlatformGetPlatformPpiList (
  OUT UINTN                   *PpiListSize,
  OUT EFI_PEI_PPI_DESCRIPTOR  **PpiList
  )
{
  *PpiListSize = sizeof (mPlatformPpiTable);
  *PpiList = mPlatformPpiTable;
}

UINTN
ArmPlatformGetCorePosition (
  IN UINTN MpId
  )
{
  return 1;
}

UINTN
ArmPlatformGetPrimaryCoreMpId(
    VOID
  )
{
  return 1;
}

UINTN
ArmPlatformIsPrimaryCore (
  IN UINTN  MpId
  )
{
  return 1;
}

VOID
ArmPlatformPeiBootAction (
  VOID
  )
{

}