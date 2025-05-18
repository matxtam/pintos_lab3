#include "vm/frame.h"
#include <hash.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "threads/palloc.h"
#include "threads/thread.h"

static struct hash frames;

unsigned frame_hash (const struct hash_elem *f_, void *aux UNUSED);
bool frame_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED);


/* Returns a hash value for frame f. */
unsigned
frame_hash (const struct hash_elem *f_, void *aux UNUSED)
{
	const struct frame *f = hash_entry (f_, struct frame, hash_elem);
	return hash_bytes (&f->addr, sizeof f->addr);
}

/* Returns true if frame a precedes frame b. */
bool
frame_less (const struct hash_elem *a_, const struct hash_elem *b_,
		           void *aux UNUSED)
{
	const struct frame *a = hash_entry (a_, struct frame, hash_elem);
	const struct frame *b = hash_entry (b_, struct frame, hash_elem);
	return a->addr < b->addr;
}

void
frame_init (void) {
	hash_init (&frames, frame_hash, frame_less, NULL);
}

void *
frame_get_page(bool zero) {
	struct frame *new_frame = malloc(sizeof(struct frame));

	if(zero) {
		new_frame->addr = palloc_get_page(PAL_USER | PAL_ASSERT | PAL_ZERO);
	} else {
		new_frame->addr = palloc_get_page(PAL_USER | PAL_ASSERT);
	}
	if(new_frame->addr == NULL){
		// todo: eviction
		return NULL;
	}
	hash_insert (&frames, &new_frame->hash_elem);
	return new_frame->addr;

}
