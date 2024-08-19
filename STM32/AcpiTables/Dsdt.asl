/** @file
 *
 *  Differentiated System Definition Table (DSDT)
 *
 *  Copyright (c) 2024, Phan Ba Gia Bao <phanbagiabao2001@gmail.com>
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Copyright (c) 2021, ARM Limited. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include "AcpiTables.h"

DefinitionBlock ("Dsdt.aml", "DSDT", 2, "STM32", "MP25", 2)
{
  Scope (\_SB_)
  {

    include ("Cpu.asl")
    include ("Uart.asl")
  
  }
}
