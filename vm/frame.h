#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <hash.h>
#include <stdbool.h>


/* The frame table entry that contains a user page */
struct frame {
	struct hash_elem hash_elem;
	void *addr;
};

void frame_init(void);
void *frame_get_page(bool zero);

#endif
