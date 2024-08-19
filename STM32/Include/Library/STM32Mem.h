/** @file
 *
 *  Copyright (c) 2024, Phan Ba Gia Bao <phanbagiabao2001@gmail.com>
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef STM32_MEM_H__
#define STM32_MEM_H__

#define STM32_MEM_UNMAPPED_REGION 0
#define STM32_MEM_BASIC_REGION    1
#define STM32_MEM_RUNTIME_REGION  2
#define STM32_MEM_RESERVED_REGION 3

typedef struct {
  CONST CHAR16*                 Name;
  UINTN                         Type;
} STM32_MEMORY_REGION_INFO;

VOID
STM32PlatformGetVirtualMemoryInfo (
  IN STM32_MEMORY_REGION_INFO** MemoryInfo
  );

#endif /* STM32_MEM_H__ */
