#include "vm/frame.h"
#include <hash.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "vm/page.h"
#include "vm/swap.h"

static struct hash frames;
static struct lock lock_evict;

unsigned frame_hash (const struct hash_elem *f_, void *aux UNUSED);
bool frame_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED);
void evict(void);


/* Returns a hash value for frame f. */
unsigned
frame_hash (const struct hash_elem *f_, void *aux UNUSED)
{
	const struct frame *f = hash_entry (f_, struct frame, hash_elem);
	return hash_bytes (&f->kaddr, sizeof f->kaddr);
}

/* Returns true if frame a precedes frame b. */
bool
frame_less (const struct hash_elem *a_, const struct hash_elem *b_,
		           void *aux UNUSED)
{
	const struct frame *a = hash_entry (a_, struct frame, hash_elem);
	const struct frame *b = hash_entry (b_, struct frame, hash_elem);
	return a->kaddr < b->kaddr;
}

void
frame_init (void) {
	hash_init (&frames, frame_hash, frame_less, NULL);
	lock_init(&lock_evict);
}

void *
frame_get_page(void *upage, bool zero) {
	struct frame *new_frame = malloc(sizeof(struct frame));

	if(zero) {
		new_frame->kaddr = palloc_get_page(PAL_USER | PAL_ZERO);
	} else {
		new_frame->kaddr = palloc_get_page(PAL_USER );
	}
	if(new_frame->kaddr == NULL){
		lock_acquire(&lock_evict);
		evict();
		lock_release(&lock_evict);
		if(zero) {
			new_frame->kaddr = palloc_get_page(PAL_USER | PAL_ASSERT | PAL_ZERO);
		} else {
			new_frame->kaddr = palloc_get_page(PAL_USER | PAL_ASSERT);
		}
	}
	new_frame->uaddr = upage;
	new_frame->t = thread_current();
	hash_insert (&frames, &new_frame->hash_elem);
	return new_frame->kaddr;

}

void
evict(void){
	// printf("start eviction\n");
	/* choose victim: enhanced second-chance algorithm */

	// to be saved: representatives of the four categories
	struct frame *v_0_0, *v_0_1, *v_1_0, *v_1_1;
	v_0_0 = NULL; v_0_1 = NULL;
	v_1_0 = NULL; v_1_1 = NULL;

	struct hash_iterator i;
	hash_first(&i, &frames);

	// First pass: categorize
	while (hash_next(&i)) {
		struct frame *f = hash_entry(hash_cur(&i), struct frame, hash_elem);
		bool accessed = pagedir_is_accessed(f->t->pagedir, f->kaddr);
		bool dirty = pagedir_is_dirty(f->t->pagedir, f->kaddr);
		struct suppPage *p = suppPage_lookup(f->uaddr);
		if (p->isPinned)continue;

		if (!accessed && !dirty && v_0_0 == NULL) v_0_0 = f;
		else if (!accessed && dirty && v_0_1 == NULL) v_0_1 = f;
		else if (accessed && !dirty && v_1_0 == NULL) v_1_0 = f;
		else if (accessed && dirty && v_1_1 == NULL) v_1_1 = f;
	}

	// If nothing good, clear accessed bits and retry
	if (v_0_0 == NULL && v_0_1 == NULL) {
		hash_first(&i, &frames);
		while (hash_next(&i)) {
			struct frame *f = hash_entry(hash_cur(&i), struct frame, hash_elem);
			struct suppPage *p = suppPage_lookup(f->uaddr);
			if (p->isPinned)continue;
			pagedir_set_accessed(f->t->pagedir, f->kaddr, false);  // give second chance
		}
		// retry after giving second chance
		v_0_0 = v_0_1 = v_1_0 = v_1_1 = NULL;
		hash_first(&i, &frames);
		while (hash_next(&i)) {
			struct frame *f = hash_entry(hash_cur(&i), struct frame, hash_elem);
			bool accessed = pagedir_is_accessed(f->t->pagedir, f->kaddr);
			bool dirty = pagedir_is_dirty(f->t->pagedir, f->kaddr);
			struct suppPage *p = suppPage_lookup(f->uaddr);
			if (p->isPinned)continue;

			if (!accessed && !dirty && v_0_0 == NULL) v_0_0 = f;
			else if (!accessed && dirty && v_0_1 == NULL) v_0_1 = f;
			else if (accessed && !dirty && v_1_0 == NULL) v_1_0 = f;
			else if (accessed && dirty && v_1_1 == NULL) v_1_1 = f;
		}
	}

	// Choose best available
	struct frame *victim = NULL;
	bool need_swapping = false;
	if (v_0_0)      { victim = v_0_0; /*printf("00\n");*/ }
	else if (v_0_1) { victim = v_0_1; need_swapping = true; /*printf("01\n");*/ }
	else if (v_1_0) { victim = v_1_0; /*printf("10\n"); */}
	else if (v_1_1) { victim = v_1_1; need_swapping = true; /*printf("11\n");*/ }

	if (!victim) {
		PANIC("No victim found for eviction.");
	}


	/* kick out that victim */

	// swap out the page if necessary
	if (need_swapping){
		size_t swap_slot = swap_out(victim->kaddr);
		struct suppPage *victim_suppp = suppPage_lookup(victim->uaddr);
		victim_suppp->swap_slot = swap_slot;
		victim_suppp->isInSwap = true;
	}
	
	// kick out from the page dir
	pagedir_clear_page (victim->t->pagedir, victim->uaddr);
	palloc_free_page(victim->kaddr);

	// kick out from the frame table
	hash_delete(&frames, &victim->hash_elem);

	// printf("evction done\n");
}


