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

#include "elf.h"
#include "reloc.h"

#ifdef DEBUG
  #include "dbg.h"
#endif

/**
 * @brief Deallocates a given structure of type elf_t.
 * @param binary A structure containing parsed ELF data.
 * @returns NULL.
 */

static inline __always_inline void *free_elf(elf_t *binary) {
  close(binary->fd);
  free(binary);

  return NULL;
}

/**
 * @brief Returns the size of an ELF file.
 * @param pathname The path to the ELF file.
 * @returns The size of the ELF file.
 */

static inline __always_inline off_t return_elf_size(
  const char *pathname)
{
  struct stat st_stat;

  if (stat(pathname, &st_stat) < 0)
    return 0x00;

  return st_stat.st_size;
}

/**
 * @brief Checks if the ELF header has a valid magic value.
 * @param e_ident The ELF header.
 * @returns True if the header magic value is valid, false if otherwise.
 */

static inline __always_inline bool check_elf_header(
  const uint8_t *e_ident)
{
  return (e_ident[EI_MAG0] == ELFMAG0 && e_ident[EI_MAG1] == ELFMAG1
    && e_ident[EI_MAG2] == ELFMAG2 && e_ident[EI_MAG3] == ELFMAG3);
}

/**
 * @brief Resolves a dynamic section entry.
 * @param binary A structure containing parsed ELF data.
 * @param type   The type of section to be resolved.
 * @returns The resolved dynamic section entry.
 */

static Elf64_Dyn *resolve_elf_dyn(const elf_t *binary,
  const Elf64_Sxword type)
{
  for (int i = 0; i < (binary->dynamic_section_size / sizeof(Elf64_Dyn)); ++i)

    if (binary->dynamic_section[i].d_tag != DT_NULL 
      && binary->dynamic_section[i].d_tag == type)
    {
      return &binary->dynamic_section[i];
    }

  return NULL;
}

/**
 * @brief Resolves an entry within the program header table.
 * @param binary A structure containing parsed ELF data.
 * @param type   The type of program segment to be resolved.
 * @returns The resolved program segment.
 */

static Elf64_Phdr *resolve_elf_phdr(const elf_t *binary, 
  const uint32_t type)
{
  for (int i = 0; i < binary->header->e_phnum; ++i)

    if (binary->program_header_table[i].p_type == type)
      return &binary->program_header_table[i];

  return NULL;
}

/**
 * @brief Parses an ELF file.
 * @param pathname The path to the ELF file.
 * @returns A structure containing parsed ELF data.
 */

static elf_t *parse_elf(const char *pathname) {
  if (access(pathname, F_OK) == -1)
    return NULL;

  const int binfd = open(pathname, O_RDONLY);
  if (binfd < 0)
    return NULL;

  elf_t *binary = malloc(sizeof(elf_t));
  
  if (!binary)
    return free_elf(binary);

  binary->fd = binfd;

  if (!(binary->memory = mmap(NULL, (binary->size = return_elf_size(pathname)), 
    (PROT_READ | PROT_WRITE | PROT_EXEC), MAP_PRIVATE, binfd, 0)))
  {
    return free_elf(binary);
  }

  binary->header = (Elf64_Ehdr *)binary->memory;

  if (!check_elf_header(binary->header->e_ident))
    return free_elf(binary);
  if (!binary->header->e_phoff)
    return free_elf(binary);

  binary->program_header_table = (Elf64_Phdr *)(binary->memory + binary->header->e_phoff);

  const Elf64_Phdr *phdr_dynamic = resolve_elf_phdr(binary, PT_DYNAMIC);
  if (!phdr_dynamic)
    return free_elf(binary);

  binary->dynamic_section = (Elf64_Dyn *)(binary->memory + phdr_dynamic->p_offset);
  binary->dynamic_section_size = phdr_dynamic->p_memsz;

  const Elf64_Dyn *dyn_dtstrtab = resolve_elf_dyn(binary, DT_STRTAB);
  const Elf64_Dyn *dyn_dtsymtab = resolve_elf_dyn(binary, DT_SYMTAB);

  if (!dyn_dtstrtab || !dyn_dtsymtab)
    return free_elf(binary);

  binary->dynamic_strtab = (binary->memory + (dyn_dtstrtab->d_un.d_ptr));
  binary->dynamic_syms   = (Elf64_Sym *)(binary->memory + (dyn_dtsymtab->d_un.d_ptr));

  Elf64_Dyn *dyn_rel = resolve_elf_dyn(binary, DT_REL);

  if (!dyn_rel) {
    if (!(dyn_rel = resolve_elf_dyn(binary, DT_RELA)))
      return free_elf(binary);
  }

  binary->relocations = (Elf64_Rela *)(dyn_rel->d_un.d_ptr + ELF_BASE_ADDR_VAL);
  return binary;
}

/**
 * @brief Maps a loadable ELF program segment into memory.
 * @param fd      The file descriptor referencing an open ELF file.
 * @param segment The loadable ELF program segment.
 * @returns A pointer to the address of the loaded program segment.
 */

static void *elf_map_segment(const int fd,
  const Elf64_Phdr *segment)
{
  int protections = 0;
  const uintptr_t address = (segment->p_vaddr + ELF_BASE_ADDR_VAL);

  if (segment->p_flags & PF_X)
    protections |= PROT_EXEC;
  if (segment->p_flags & PF_W)
    protections |= PROT_WRITE;
  if (segment->p_flags & PF_R)
    protections |= PROT_READ;

  if (mmap((void *)(address & ~0xfff), ((segment->p_filesz + address) & ~0xfff), protections, 
    (MAP_FILE | MAP_PRIVATE | MAP_FIXED), fd, (segment->p_offset - (address & 0xfff))) == MAP_FAILED)
  {
    return NULL;
  }

  return (void *)(address & ~0xfff);
}

/**
 * @brief Manual maps and executes an ELF file in memory.
 * @param pathname The path to the ELF file.
 */

void elf_manual_map(const char *pathname) {
  elf_t *binary = parse_elf(pathname);
  if (!binary)
    return;

  for (int i = 0; i < binary->header->e_phnum; ++i) {
    if (binary->program_header_table[i].p_type == PT_LOAD) {
      const void *mapped_segment = elf_map_segment(binary->fd,
        &binary->program_header_table[i]);

      if (!mapped_segment) {
        free_elf(binary);

        return;
      }

#ifdef DEBUG
  dbglog("Mapped PT_LOAD segment @ [%p]\n", 
    mapped_segment);
#endif
    }
  }

  const Elf64_Dyn *dyn_pltrelsz = resolve_elf_dyn(binary, DT_PLTRELSZ);

  Elf64_Dyn *dyn_relsz = resolve_elf_dyn(binary, DT_RELSZ);

  if (!dyn_relsz) {
    if (!(dyn_relsz = resolve_elf_dyn(binary, DT_RELASZ))) {
      free_elf(binary);

      return;
    }
  }

  relocate_data(binary, dyn_relsz->d_un.d_val, false);
  relocate_data(binary, dyn_relsz->d_un.d_val, true);

  const uintptr_t elf_main = (binary->header->e_entry + ELF_BASE_ADDR_VAL);

#ifdef DEBUG
  dbglog("Executing ELF entry point @ [0x%lx]\n\n", elf_main);
#endif

  free_elf(binary);
  ((int (*)())elf_main)();
}