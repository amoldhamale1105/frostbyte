#ifndef KERNEL_H
#define KERNEL_H

/* Invalidate any previous definitions if found */
#undef offsetof
#define offsetof(TYPE, MEMBER) (uint64_t)(&(((TYPE*)(0))->MEMBER))

#define container_of(ptr, type, member) \
                ((type *) ((char *)(ptr) - offsetof(type, member)))

#endif