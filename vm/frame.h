#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <hash.h>
#include <stdbool.h>


/* The frame table entry phat contains a user page */
struct frame {
	struct hash_elem hash_elem;
	void *uaddr;
	uint8_t *kaddr;
	struct thread *t;
};

void frame_init(void);
void *frame_get_page(void *upage, bool zero);

#endif
