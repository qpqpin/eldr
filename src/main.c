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

#include <stdio.h>
#include <stdlib.h>

#include "elf.h"

int    cmdline_argc; /* Argument count. */
char **cmdline_argv; /* Argument vector. */

/**
 * @brief Program entry point.
 * @param argc The argument count.
 * @param argv The argument vector.
 * @returns EXIT_SUCCESS if the program executed successfully, EXIT_FAILURE if otherwise.
 */

int main(int argc, char **argv) {
  fprintf(stderr, "Usage: %s <elf> <args>\n\n", *argv);

  if (argc < 2)
    return EXIT_SUCCESS;

  cmdline_argc = argc - 1;
  cmdline_argv = argv + 1;

  elf_manual_map(argv[1]);
  return EXIT_SUCCESS;
}