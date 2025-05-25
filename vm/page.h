#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <stdbool.h>
#include <stdint.h>
#include "filesys/file.h"
#include "hash.h"

struct load_info {
	struct file *file;
	size_t page_read_bytes;
	size_t page_zero_bytes;
	bool writable;
	off_t ofs;
};

/* Supplementary page structure to hold pages' information*/
struct suppPage {
	void *upage;
	struct hash_elem hash_elem;
	bool isLoaded;
	bool isFile;
	bool isPinned;
	bool isInSwap;
	size_t swap_slot;
	struct load_info load_info;
  
};

void suppPage_init(void);

bool suppPage_insert (void *upage,
		struct file *file, size_t page_read_bytes,
		size_t page_zero_bytes, bool writable, off_t ofs, bool isFile);

struct suppPage * suppPage_lookup (void *upage);

bool suppPage_load(struct suppPage *p);

void pin_user_pages(const void *uaddr_, size_t size);

void unpin_user_pages(const void *uaddr_, size_t size);
	
#endif
