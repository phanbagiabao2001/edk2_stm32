/** @file
 *
 *  STM32MP25 CPU devices.
 *  Copyright (c) 2024, Phan Ba Gia Bao <phanbagiabao2001@gmail.com>
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Copyright (c) 2021, ARM Limited. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/


#include "AcpiTables.h"

Device (CPU0)
{
  Name (_HID, "ACPI0007")
  Name (_UID, 0x0)
  Method (_STA)
  {
    Return (0xf)
  }
}

Device (CPU1)
{
  Name (_HID, "ACPI0007")
  Name (_UID, 0x1)
  Method (_STA)
  {
    Return (0xf)
  }
}
