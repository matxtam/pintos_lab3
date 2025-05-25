#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <hash.h>
#include <stdbool.h>


void swap_init(void);
size_t swap_out(uint8_t *);
void swap_in(uint8_t *, size_t);

#endif
