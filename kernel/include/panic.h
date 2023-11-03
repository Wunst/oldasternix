/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef PANIC_H
#define PANIC_H

void panic(const char *format, ...);

int panic_unwind(void **unwind_ptr);

#endif