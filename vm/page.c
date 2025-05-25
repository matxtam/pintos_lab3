#include "threads/thread.h"
#include "threads/palloc.h"
#include "filesys/file.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "vm/frame.h"
#include "vm/page.h"
#include <hash.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

static bool install_page (void *upage, void *kpage, bool writable);
unsigned suppPage_hash (const struct hash_elem *f_, void *aux UNUSED);
bool suppPage_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED);


/* Returns a hash value for suppPage f. */
unsigned
suppPage_hash(const struct hash_elem *f_, void *aux UNUSED){
	const struct suppPage *p = hash_entry (f_, struct suppPage, hash_elem);
	return hash_bytes (&p->upage, sizeof p->upage);
}

/* Returns true if suppPage a precedes suppPage b. */
bool
suppPage_less (const struct hash_elem *a_, const struct hash_elem *b_,
		           void *aux UNUSED)
{
	const struct suppPage *a = hash_entry (a_, struct suppPage, hash_elem);
	const struct suppPage *b = hash_entry (b_, struct suppPage, hash_elem);
	return a->upage < b->upage;
}

void
suppPage_init (void) {
	hash_init (&thread_current()->suppPages, suppPage_hash, suppPage_less, NULL);
}

bool
suppPage_insert (void *upage,
		struct file *file, size_t page_read_bytes,
		size_t page_zero_bytes, bool writable, off_t ofs,
		bool isFile){
	// printf("spt: inserting...\n");

	struct load_info li = {
		.file = file,
		.page_read_bytes = page_read_bytes,
		.page_zero_bytes = page_zero_bytes,
		.writable = writable,
		.ofs = ofs,
	};

	struct suppPage *p = malloc(sizeof(struct suppPage));
  if (p == NULL)
    return false;

  p->upage = upage;
  p->load_info = li;
	p->isLoaded = false;
	p->isFile = isFile;
	p->isPinned = false;
	p->isInSwap = false;

  hash_insert(&thread_current()->suppPages, &p->hash_elem);
	// printf("insert successfully\n");
  return true;
}


struct suppPage *
suppPage_lookup (void *upage)
{
  struct suppPage p;
  struct hash_elem *e;

  p.upage = upage;
  e = hash_find (&thread_current()->suppPages, &p.hash_elem);
  return e != NULL ? hash_entry (e, struct suppPage, hash_elem) : NULL;
}

bool
suppPage_load(struct suppPage *p) {
	if (p == NULL){
		printf("Error: spt: p is null!\n");
		return false;
	}

	// load file
	void *upage = p->upage;
	struct file *file = p->load_info.file;
	size_t page_read_bytes = p->load_info.page_read_bytes;
	size_t page_zero_bytes = p->load_info.page_zero_bytes;
	bool writable = p->load_info.writable;
	off_t ofs = p->load_info.ofs;
	p->isPinned = true;


	/* Get a page of memory. */
	uint8_t *kpage;
	if(page_read_bytes == 0)kpage = frame_get_page(upage, true);
	else kpage = frame_get_page(upage, false);
	if (kpage == NULL) {
		return false;
	}

	/* Load this page. */
	if (p->isFile && (!p->isLoaded || !p->isInSwap)) {
		file_seek (file, ofs);
		// printf("loading: is file\n");
		if (file_read (file, kpage, page_read_bytes) != (int) page_read_bytes)
			{
				palloc_free_page (kpage);
				return false; 
			}
		memset (kpage + page_read_bytes, 0, page_zero_bytes);

	} else if(p->isInSwap){
		swap_in(kpage, p->swap_slot);
	}

	/* Add the page to the process's address space. */
	if (!install_page (upage, kpage, writable)) 
		{
			palloc_free_page (kpage);
			return false; 
		}

	/* loaded successfully */
	p->isPinned = false;
	p->isLoaded = true;
	p->isInSwap = false;
	// printf("load success\n");
	return true;
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}


void pin_user_pages(const void *uaddr_, size_t size) {
	printf("pin user page\n");
  uint8_t *uaddr = pg_round_down(uaddr_);
  for (size_t ofs = 0; ofs < size; ofs += PGSIZE) {
    struct suppPage *p = suppPage_lookup(uaddr + ofs);
		ASSERT(p);
			    if (!p->isLoaded) {
						      suppPage_load(p); 
			}
			p->isPinned = true;
  }
}

void unpin_user_pages(const void *uaddr_, size_t size) {
  uint8_t *uaddr = pg_round_down(uaddr_);
  for (size_t ofs = 0; ofs < size; ofs += PGSIZE) {
    struct suppPage *p = suppPage_lookup(uaddr + ofs);
    if (p) p->isPinned = false;
  }
}
void spt_destructor(struct hash_elem *e, void *aux) {
	struct suppPage *p = hash_entry(e, struct suppPage, hash_elem);
	free(p);
}
void
suppPage_destroy(void){
	hash_destroy(&thread_current()->suppPages, spt_destructor);
}
