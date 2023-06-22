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

#ifndef __DBG_H
#define __DBG_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * @brief Prints a debug message.
 * @param msg The message.
 */

static inline void dbglog(
  const char *msg, ...)
{
  va_list args = {0};

  va_start(args, msg);
  printf("debug: ");

  vprintf(msg, args);
  va_end(args);
}

#endif