/**
    Frostbyte kernel and operating system
    Copyright (C) 2023  Amol Dhamale <amoldhamale1105@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef KERNEL_H
#define KERNEL_H

/* Invalidate any previous definitions if found */
#ifdef offsetof
#undef offsetof
#endif
#define offsetof(TYPE, MEMBER) (uint64_t)(&(((TYPE*)(0))->MEMBER))

#ifdef container_of
#undef container_of
#endif
#define container_of(ptr, type, member) \
                ((type *) ((char *)(ptr) - offsetof(type, member)))

#endif