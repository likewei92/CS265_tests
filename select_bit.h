#ifndef SELECT_H__
#define SELECT_H__

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

#endif
