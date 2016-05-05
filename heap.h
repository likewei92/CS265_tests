#ifndef HEAP_H__
#define HEAP_H__

#define STARTING_HEAP_SIZE 4

typedef struct min_heap {
	int *data;
	int len;
	int max;
} min_heap;

min_heap *new_heap();

void heap_insert(min_heap *heap, int in);

#endif
