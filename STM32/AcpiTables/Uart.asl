/** @file
 *
 *  [DSDT] Serial devices (UART).
 *
 *  Copyright (c) 2024, Phan Ba Gia Bao <phanbagiabao2001@gmail.com>
 *  Copyright (c) 2021, ARM Limited. All rights reserved.
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include "AcpiTables.h"

Device(URT0) {
  Name (_HID, "ARMH9999")
  Name (_UID, 0x0)
  Name (_CCA, 0x0)

  Method (_CRS, 0x0, Serialized) {
    Name (RBUF, ResourceTemplate () {
      QWORDMEMORY_BUF (00, ResourceConsumer)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x1C }
    })
    QWORDMEMORY_SET (00, 0x400e0000, 0x400) 
    Return (RBUF)
  }

  Name (_DSD, Package () {
    ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"), 
    Package () {
      Package () { "clock-frequency", 12000000 },
    }
  })
}


// https://uefi.org/sites/default/files/resources/_DSD-device-properties-UUID.pdf
