/** @file
 *
 *  Copyright (c) 2024, Phan Ba Gia Bao <phanbagiabao2001@gmail.com>
 *  Copyright (c) 2019 - 2020, ARM Limited. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <IndustryStandard/SerialPortConsoleRedirectionTable.h>
#include <Library/AcpiLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/IoLib.h>
#include <Library/NetLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Protocol/AcpiTable.h>
#include <STM32MP25.h>
#include <ConfigVars.h>
#include "ConfigDxeFormSetGuid.h"
#include "ConfigDxe.h"
#include <IndustryStandard/DebugPort2Table.h>

STATIC CONST EFI_GUID mAcpiTableFile = {
  0xAF6AB56B, 0xEB7F, 0x400D, {0x8C, 0x2B, 0xD8, 0x62, 0x9A, 0x40, 0xE8, 0xC8}
};


STATIC
EFI_STATUS
EFIAPI
ApplyDsdtFixups (
  VOID
  )
{
  EFI_STATUS                         Status;
  EFI_ACPI_SDT_PROTOCOL              *AcpiSdtProtocol;
  EFI_ACPI_DESCRIPTION_HEADER        *Table;
  UINTN                              TableKey;
  UINTN                              TableIndex;
  EFI_ACPI_HANDLE                    TableHandle;

  Status = gBS->LocateProtocol (
                  &gEfiAcpiSdtProtocolGuid,
                  NULL,
                  (VOID **)&AcpiSdtProtocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Couldn't locate gEfiAcpiSdtProtocolGuid!\n", __func__));
    return Status;
  }

  TableIndex = 0;
  Status = AcpiLocateTableBySignature (
             AcpiSdtProtocol,
             EFI_ACPI_6_3_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE,
             &TableIndex,
             &Table,
             &TableKey);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Couldn't locate ACPI DSDT table!\n", __func__));
    return Status;
  }

  Status = AcpiSdtProtocol->OpenSdt (TableKey, &TableHandle);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Couldn't open ACPI DSDT table!\n", __func__));
    AcpiSdtProtocol->Close (TableHandle);
    return Status;
  }

  AcpiSdtProtocol->Close (TableHandle);
  AcpiUpdateChecksum ((UINT8 *)Table, Table->Length);

  return EFI_SUCCESS;
}

VOID
EFIAPI
ApplyConfigTableVariables (
  VOID
  )
{
  EFI_STATUS Status;

  Status = ApplyDsdtFixups ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Failed to apply ACPI DSDT fixups!\n"));
  }
}


EFI_STATUS
EFIAPI
ConfigInitialize (
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{

  UINTN i = 0;
  EFI_STATUS                      Status;

  EFI_CONFIGURATION_TABLE *ConfigTable = SystemTable->ConfigurationTable;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut = SystemTable->ConOut;
  DEBUG ((DEBUG_INFO, "Begin ConfigInitialize\n"));
  DEBUG ((DEBUG_INFO, "Number of Configuration Table Entries before adding ACPI: %d\n", SystemTable->NumberOfTableEntries));

  for (i = 0; i < SystemTable->NumberOfTableEntries; i++) {
    DEBUG ((DEBUG_INFO, "Configuration Table Entry %d: VendorGuid %g, Address 0x%p\r\n",
                  i, &ConfigTable[i].VendorGuid, ConfigTable[i].VendorTable));
  }

  DEBUG((DEBUG_INFO, "OutputString function pointer value: %p\n", ConOut->OutputString));

  Status = gBS->InstallProtocolInterface (&ImageHandle,
                  &gSTM32ConfigAppliedProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL);
  ASSERT_EFI_ERROR (Status);

  if (PcdGet32 (PcdSystemTableMode) == SYSTEM_TABLE_MODE_ACPI ||
      PcdGet32 (PcdSystemTableMode) == SYSTEM_TABLE_MODE_BOTH) {
      Status = LocateAndInstallAcpiFromFvConditional (&mAcpiTableFile, 
                                                     NULL);
      ASSERT_EFI_ERROR (Status);
  }

  DEBUG ((DEBUG_INFO, "Number of Configuration Table Entries after adding ACPI: %d\n", SystemTable->NumberOfTableEntries));
  for (i = 0; i < SystemTable->NumberOfTableEntries; i++) {
    DEBUG ((DEBUG_INFO, "Configuration Table Entry %d: VendorGuid %g, Address 0x%p\r\n",
                  i, &ConfigTable[i].VendorGuid, ConfigTable[i].VendorTable));
  }

  return EFI_SUCCESS;
}