#include "vm/swap.h"
#include <stdio.h>
#include "devices/block.h"
#include "threads/vaddr.h"
#include "lib/kernel/bitmap.h"

static struct block *swap_block;
static struct bitmap *swap_bitmap;
static size_t sector_per_pg;

void
swap_init(void){
	swap_block = block_get_role(BLOCK_SWAP);
	block_sector_t n_sector = block_size(swap_block);
	sector_per_pg = PGSIZE / BLOCK_SECTOR_SIZE;
	swap_bitmap = bitmap_create(n_sector / sector_per_pg);
	ASSERT(swap_bitmap);
}

/* swap victim frames into the swap block. */

size_t
swap_out(uint8_t *kpage){
	// select a null swap slot
	size_t swap_slot = bitmap_scan_and_flip (swap_bitmap, 0, 1, false);
	if(swap_slot == BITMAP_ERROR) PANIC("Swap is full!\n");

	// write to swap block
	for(size_t i=0; i<sector_per_pg; i++){
		block_write (swap_block, swap_slot*sector_per_pg + i, kpage + i*BLOCK_SECTOR_SIZE);
	}
	return swap_slot;
}

/* swap memory from swap block to physical page frame */
void
swap_in(uint8_t *kpage, size_t swap_slot){
	// printf("swap in\n");

	// read from the swap block
	for(size_t i=0; i<sector_per_pg; i++){
		block_read (swap_block, swap_slot*sector_per_pg + i, kpage + i*BLOCK_SECTOR_SIZE);
	}

	// free the swap block
	bitmap_set (swap_bitmap, swap_slot, false);
}
