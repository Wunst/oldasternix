/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef INITCALL_H
#define INITCALL_H

#define initcall(func) \
    asm(".section .initcall\n" \
        ".long "#func"\n" \
        ".previous")

#endif
