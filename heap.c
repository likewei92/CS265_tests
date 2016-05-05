#include "heap.h"

min_heap *new_heap() {
	min_heap *ret = malloc(sizeof(min_heap));
	ret->data = malloc(sizeof(int) * STARTING_HEAP_SIZE);
	ret->len = 0;
	ret->max = STARTING_HEAP_SIZE;
	return ret;
}


void heap_insert(min_heap *heap, int in) {
	if (heap->len == heap->max) {
		heap->max *= 2;
		heap->data = realloc(heap->data, sizeof(int) * heap->max);
	}

	heap->data[heap->len] = in;
	int cur_i = heap->len;
	int parent_i;
	while (cur_i != 0) {
		parent_i = (cur_i - 1) / 2;
		if (heap->data[parent_i] <= in) break;
		heap->data[cur_i] = heap->data[parent_i];
		heap->data[parent_i] = in;
	}
}


