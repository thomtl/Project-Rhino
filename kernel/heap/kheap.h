#ifndef KMALLOC_H
#define KMALLOC_H
#include <stdint.h>
#include <stddef.h>
#include "../types/ordered_array.h"
#define KHEAP_START 0xC0000000
#define KHEAP_INITIAL_SIZE 0x100000
#define HEAP_INDEX_SIZE 0x20000
#define HEAP_MAGIC 0x123890AB
#define HEAP_MIN_SIZE 0x70000

typedef struct {
  uint32_t magic;
  uint32_t is_hole;
  uint32_t size;
} header_t;

typedef struct {
  uint32_t magic;
  header_t *header;
} footer_t;

typedef struct{
  ordered_array_t index;
  uint32_t start_address;
  uint32_t end_address;
  uint32_t max_address;
  uint8_t supervisor;
  uint8_t readonly;
} heap_t;

heap_t *create_heap(uint32_t start, uint32_t end, uint32_t max, uint8_t supervisor, uint8_t readonly);
void *alloc(size_t size, uint8_t page_align, heap_t *heap);
void free_int(void *p, heap_t *heap);

extern uint32_t end;
void kfree(void *p);
void* krealloc(void *ptr, size_t size);
void* kmalloc_int(size_t sz, int align, uint32_t *phys);

void* kmalloc_a(size_t sz);

void* kmalloc_p(size_t sz, uint32_t *phys);

void* kmalloc_ap(size_t sz, uint32_t *phys);

void* kmalloc(size_t sz);
#endif
