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
	struct load_info load_info;
  
};

void suppPage_init(void);

bool suppPage_insert (void *upage,
		struct file *file, size_t page_read_bytes,
		size_t page_zero_bytes, bool writable, off_t ofs);

struct suppPage * suppPage_lookup (void *upage);

bool suppPage_load(struct suppPage *p);


#endif
