#ifndef SELECT_H__
#define SELECT_H__
#include <sys/types.h>

int load_column(int **ret);

typedef enum select_t {
	MAX,
	MIN,
	BOTH
} select_t;

typedef struct minmax_t {
	select_t stype;
	int min;
	int max;
} minmax_t;

int load_selects(minmax_t **select_queries);

int basic_scan(int *col, int col_len, minmax_t mm, int **ret);

int sub_prev_scan(int *col, int col_len, int **prevs, int *prev_lens, int n_prevs, minmax_t mm, int **ret);

int super_prev_scan(int *col, int col_len, int *prev, int prev_len, minmax_t mm, int **ret);

int merge_pos(int **pos, int *pos_lens, int **ret);

const uint masks[32] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536,
						131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432,
						67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648};

#endif
