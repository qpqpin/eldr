/*
 * Copyright (C) 2023 qpqpin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __ELF_UTILS_H
#define __ELF_UTILS_H

#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define ELF_BASE_ADDR_VAL 0x10000000 /* ELF base address value. */

/* Structure meant for storing the parsed attributes of a given ELF file. */
typedef struct _elf {
  int fd;

  Elf64_Ehdr *header;
  Elf64_Phdr *program_header_table;

  Elf64_Rela *relocations;
  Elf64_Dyn  *dynamic_section;
  Elf64_Sym  *dynamic_syms;

  off_t size, dynamic_section_size;

  uint8_t *memory;
  char *dynamic_strtab;
} elf_t;

void elf_manual_map(const char *pathname);

#endif