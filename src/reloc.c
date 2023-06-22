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

#include "dbg.h"
#include "reloc.h"

extern int    cmdline_argc; /* Argument count. */
extern char **cmdline_argv; /* Argument vector. */

/* Map of relocation symbols. */
static reloc_sym_t symbols[] = {
  {"stdin",  NULL},
  {"stdout", NULL},
  {"stderr", NULL},
  {"__libc_start_main", __libc_start_main_impl}
};

/**
 * @brief Implementation of __libc_start_main.
 * @param entry The entry point function for the ELF executable.
 * @param argc  The argument count.
 * @param argv  The argument vector.
 * @returns The return value of the entry point function.
 */

static int __attribute__((noinline)) __libc_start_main_impl(
  int (*entry)(int, char **, char **), int argc, char **argv)
{
  argc = cmdline_argc;
  argv = cmdline_argv;

  exit(entry(argc, argv, 0x00));
}

/**
 * @brief Resolves a symbol within our own symbol table. (symbols[])
 * @param symbol The symbol.
 * @returns A pointer to the symbol.
 */

static inline void *resolve_reloc_symbol(const char *symbol) {
  for (int i = 0; i < (sizeof(symbols) / sizeof(*symbols)); ++i)

    if (!strncmp(symbols[i].symbol, symbol, strlen(symbols[i].symbol)))
      return symbols[i].ptr;

  return NULL;
}

/**
 * @brief Applies changes to the data referenced in the given relocation table entry.
 * @param relocation Entry within the relocation table.
 * @param ptr        A pointer to a symbol.
 * @param address    Address thats calculated from the offset in the relocation entry + base address value.
 */

static inline void apply_relocation(const Elf64_Rela *relocation,
  const void *ptr, uintptr_t *address)
{
  switch (ELF64_R_TYPE(relocation->r_info)) {
    case R_X86_64_RELATIVE:
      *address += (uintptr_t)(ELF_BASE_ADDR_VAL + relocation->r_addend);
      break;
    case R_X86_64_COPY:
      *address = (uintptr_t)ptr;
      break;
    case R_X86_64_GLOB_DAT:
      *address = (uintptr_t)ptr;
      break;
    case R_X86_64_JUMP_SLOT:
      if (ptr)
        *address = (uintptr_t)ptr;
      else
        *address = (uintptr_t)0x00;
      break;
  }
}

/**
 * @brief Performs relocations for a given parsed ELF executable.
 * @param binary A structure containing parsed ELF data.
 * @param size   Size of relocation table.
 * @param pltrel Boolean argument dictating how we'll be relocating the data.
 */

void relocate_data(elf_t *binary, 
  const int size, const bool pltrel)
{
  Elf64_Rela *relocations = binary->relocations;

  if (pltrel)
    relocations = (relocations + (size / sizeof(*relocations)));

  symbols[0].ptr = stdin, symbols[1].ptr = stdout;
  symbols[2].ptr = stderr;

  for (int i = 0; i < (size / sizeof(Elf64_Rela)); ++i) {
    uintptr_t *address = (uintptr_t *)((char *)relocations[i].r_offset + ELF_BASE_ADDR_VAL);

    const char *symbol_name = (binary->dynamic_strtab + 
      (binary->dynamic_syms + ELF64_R_SYM(relocations[i].r_info))->st_name);

    if (!(*symbol_name))
      continue;

    void *symbol_ptr = resolve_reloc_symbol(symbol_name);

    if (!symbol_ptr)
      symbol_ptr = dlsym(RTLD_DEFAULT, symbol_name);

#ifdef DEBUG
  dbglog("Relocating [%s] [off: 0x%lx] -> [0x%lx]\n", symbol_name, 
    ((binary->dynamic_syms + ELF64_R_SYM(relocations[i].r_info))->st_value), symbol_ptr);
#endif

    apply_relocation(&relocations[i], symbol_ptr, address);
  }
}
