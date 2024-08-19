/** @file
 *
 *  STM32 defines for constructing ACPI tables
 *
 *  Copyright (c) 2024, Phan Ba Gia Bao <phanbagiabao2001@gmail.com>
 *  Copyright (c) 2019, ARM Ltd. All rights reserved.
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef __ACPITABLES_H__
#define __ACPITABLES_H__

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h>
#include <Library/PcdLib.h>

#define NUM_PCIE_CONTROLLER 2
// The ASL compiler can't perform arithmetic on MEMORY32FIXED ()
// parameters so you can't pass a constant like BASE + OFFSET.
// We therefore define a macro that can perform arithmetic base
// address update with an offset.


#define EFI_ACPI_OEM_ID                       {'S','T','M','3','2',' '}
#define EFI_ACPI_OEM_TABLE_ID                 SIGNATURE_64 ('M','P','2','5',' ',' ',' ',' ')

#define EFI_ACPI_OEM_REVISION                 0x00000200
#define EFI_ACPI_CREATOR_ID                   SIGNATURE_32 ('E','D','K','2')
#define EFI_ACPI_CREATOR_REVISION             0x00000300

#define EFI_ACPI_VENDOR_ID                    SIGNATURE_32 ('S','T','M',' ')

// A macro to initialise the common header part of EFI ACPI tables as defined by
// EFI_ACPI_DESCRIPTION_HEADER structure.
#define ACPI_HEADER(Signature, Type, Revision) {                  \
    Signature,                      /* UINT32  Signature */       \
    sizeof (Type),                  /* UINT32  Length */          \
    Revision,                       /* UINT8   Revision */        \
    0,                              /* UINT8   Checksum */        \
    EFI_ACPI_OEM_ID,                /* UINT8   OemId[6] */        \
    EFI_ACPI_OEM_TABLE_ID,          /* UINT64  OemTableId */      \
    EFI_ACPI_OEM_REVISION,          /* UINT32  OemRevision */     \
    EFI_ACPI_CREATOR_ID,            /* UINT32  CreatorId */       \
    EFI_ACPI_CREATOR_REVISION       /* UINT32  CreatorRevision */ \
  }

//
// Device resource helpers
//
#define QWORDMEMORY_BUF(Index, ResourceType)                    \
  QWordMemory (ResourceType,,                                   \
    MinFixed, MaxFixed, NonCacheable, ReadWrite,                \
    0x0, 0x0, 0x0, 0x0, 0x1,,, RB ## Index)

#define QWORDMEMORY_SET(Index, Minimum, Length)                 \
  CreateQwordField (RBUF, RB ## Index._MIN, MI ## Index)        \
  CreateQwordField (RBUF, RB ## Index._MAX, MA ## Index)        \
  CreateQwordField (RBUF, RB ## Index._LEN, LE ## Index)        \
  LE ## Index = Length                                          \
  MI ## Index = Minimum                                         \
  MA ## Index = MI ## Index + LE ## Index - 1

#pragma pack(push, 1)
typedef struct {
  EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER Header;
  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE MainEntries[NUM_PCIE_CONTROLLER];
  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE RootPortEntries[NUM_PCIE_CONTROLLER];
} STM32_MCFG_TABLE;
#pragma pack(pop)

#endif // __ACPITABLES_H__
