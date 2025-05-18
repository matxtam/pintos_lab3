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
};

/* Supplementary page structure to hold pages' information*/
struct suppPage {
	uint8_t *upage;
	struct hash_elem hash_elem;
	bool isLoaded;
	struct load_info load_info;
  
};

void suppPage_init(void);

bool suppPage_insert (uint8_t *upage,
		struct file *file, size_t page_read_bytes,
		size_t page_zero_bytes, bool writable);

struct suppPage * suppPage_lookup (uint8_t *upage);

bool suppPage_load(struct suppPage *p);


#endif
