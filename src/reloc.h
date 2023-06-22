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

#ifndef __RELOC_H
#define __RELOC_H

#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "elf.h"

typedef struct _reloc_sym {
  const char *symbol;
  void *ptr;
} reloc_sym_t;

/* This symbol has to be visible since we're storing it in our global symbol table. */
static int __attribute__((noinline)) __libc_start_main_impl(
  int (*entry)(int, char **, char **), int argc, char **argv);

void relocate_data(elf_t *binary, 
  const int size, const bool pltrel);

#endif