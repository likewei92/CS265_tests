#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "select_headers.h"

int main() {
	int *col;
	int col_len = load_column(&col);
	(void) col_len;

	minmax_t *select_queries;
	int queries_count = load_selects(&select_queries);

	clock_t start, end;
	double cpu_time_used;

	int **basic_rets = malloc(sizeof(int*) * queries_count);
	int *basic_ret_lens = malloc(sizeof(int) * queries_count);
	start = clock();
	for (int i = 0; i < queries_count; i++) {
		basic_ret_lens[i] = basic_scan(col, col_len, select_queries[i], &(basic_rets[i]));
	}
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("%f\n", cpu_time_used);

	int **recycle_rets = malloc(sizeof(int*) * queries_count);
	int *recycle_ret_lens = malloc(sizeof(int) * queries_count);
	start = clock();

	for (int i = 0; i < queries_count - 1; i++) {
		recycle_ret_lens[i] = basic_scan(col, col_len, select_queries[i], &(recycle_rets[i]));
	}

	int temp = queries_count - 1;
	recycle_ret_lens[temp] = sub_prev_scan(col, recycle_rets, recycle_ret_lens, temp, select_queries[temp], &(recycle_rets[temp]));

	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("%f\n", cpu_time_used);


	for (int i = 0; i < queries_count; i++) assert(recycle_ret_lens[i] == basic_ret_lens[i]);
	for (int i = 0; i < queries_count; i++) {
		for (int j = 0; j < basic_ret_lens[i]; j++) assert(recycle_rets[i][j] == basic_rets[i][j]);
	}

	for (int i = 0; i < queries_count; i++) {
		free(basic_rets[i]);
		free(recycle_rets[i]);
	}
	free(basic_rets);
	free(basic_ret_lens);
	free(recycle_rets);
	free(recycle_ret_lens);
	free(col);
	free(select_queries);
}

int merge_pos(int **pos, int *pos_lens, int **ret) {
	int cur_pos[2];
	memset(cur_pos, 0, sizeof(int) * 2);
	int merged_pos = 0;
	int total_len = pos_lens[0] + pos_lens[1];
	*ret = malloc(sizeof(int) * total_len);
	for (int i = 0; i < total_len; i++) {
		if (cur_pos[0] == pos_lens[0]) {
			int left = pos_lens[1] - cur_pos[1];
			memcpy(&((*ret)[merged_pos]), &(pos[1][cur_pos[1]]), sizeof(int) * left);
			merged_pos += left;
			break;
		} else if (cur_pos[1] == pos_lens[1]) {
			int left = pos_lens[0] - cur_pos[0];
			memcpy(&((*ret)[merged_pos]), &(pos[0][cur_pos[0]]), sizeof(int) * left);
			merged_pos += left;
			break;
		} else if (pos[0][cur_pos[0]] < pos[1][cur_pos[1]]) {
			(*ret)[merged_pos] = pos[0][cur_pos[0]];
			cur_pos[0]++;
			merged_pos++;
		} else if (pos[0][cur_pos[0]] == pos[1][cur_pos[1]]) {
			(*ret)[merged_pos] = pos[0][cur_pos[0]];
			cur_pos[0]++;
			cur_pos[1]++;
			merged_pos++;
		} else {
			(*ret)[merged_pos] = pos[1][cur_pos[1]];
			cur_pos[1]++;
			merged_pos++;
		}
	}
	*ret = realloc(*ret, sizeof(int) * merged_pos);
	return merged_pos;
}

int neg_merge_pos(int **pos, int *pos_lens, int col_len, int **ret) {
	int cur_pos[2];
	memset(cur_pos, 0, sizeof(int) * 2);
	int merged_pos = 0;
	*ret = malloc(sizeof(int) * col_len);
	for (int i = 0; i < col_len; i++) {
		if (cur_pos[0] < pos_lens[0] && pos[0][cur_pos[0]] == i) {
			cur_pos[0]++;
		} else if (cur_pos[1] < pos_lens[1] && pos[1][cur_pos[1]] == i) {
			cur_pos[1]++;
		} else {
			(*ret)[merged_pos] = i;
			merged_pos++;
		}
	}
	*ret = realloc(*ret, sizeof(int) * merged_pos);
	return merged_pos;
}

int sub_prev_scan(int *col, int **prevs, int *prev_lens, int n_prevs, minmax_t mm, int **ret) {
	int ret_len = 0;
	int cur_pos[2];
	memset(cur_pos, 0, sizeof(int) * 2);
	int total_len = 0;
	for (int i = 0; i < n_prevs; i++) total_len += prev_lens[i];
	*ret = malloc(sizeof(int) * total_len);

	switch (mm.stype) {
	case BOTH:
		for (int i = 0; i < total_len; i++) {
			if (cur_pos[0] == prev_lens[0]) {
				for (int j = cur_pos[1]; j < prev_lens[1]; j++) {
					if ((mm.min <= col[prevs[1][j]]) & (col[prevs[1][j]] < mm.max)) {
						(*ret)[ret_len] = prevs[1][j];
						ret_len++;
					}
				}
				break;
			} else if (cur_pos[1] == prev_lens[1]) {
				for (int j = cur_pos[0]; j < prev_lens[0]; j++) {
					if ((mm.min <= col[prevs[0][j]]) & (col[prevs[0][j]] < mm.max)) {
						(*ret)[ret_len] = prevs[0][j];
						ret_len++;
					}
				}
				break;
			} else if (prevs[0][cur_pos[0]] < prevs[1][cur_pos[1]]) {
				if ((mm.min <= col[prevs[0][cur_pos[0]]]) & (col[prevs[0][cur_pos[0]]] < mm.max)) {
					(*ret)[ret_len] = prevs[0][cur_pos[0]];
					ret_len++;
				}
				cur_pos[0]++;
			} else if (prevs[0][cur_pos[0]] == prevs[1][cur_pos[1]]) {
				if ((mm.min <= col[prevs[0][cur_pos[0]]]) & (col[prevs[0][cur_pos[0]]] < mm.max)) {
					(*ret)[ret_len] = prevs[0][cur_pos[0]];
					ret_len++;
				}
				cur_pos[0]++;
				cur_pos[1]++;
			} else {
				if ((mm.min <= col[prevs[1][cur_pos[1]]]) & (col[prevs[1][cur_pos[1]]] < mm.max)) {
					(*ret)[ret_len] = prevs[1][cur_pos[1]];
					ret_len++;
				}
				cur_pos[1]++;
			}
		}
		break;
	case MIN:
		for (int i = 0; i < total_len; i++) {
			if (cur_pos[0] == prev_lens[0]) {
				for (int j = cur_pos[1]; j < prev_lens[1]; j++) {
					if (mm.min <= col[prevs[1][j]]) {
						(*ret)[ret_len] = prevs[1][j];
						ret_len++;
					}
				}
				break;
			} else if (cur_pos[1] == prev_lens[1]) {
				for (int j = cur_pos[0]; j < prev_lens[0]; j++) {
					if (mm.min <= col[prevs[0][j]]) {
						(*ret)[ret_len] = prevs[0][j];
						ret_len++;
					}
				}
				break;
			} else if (prevs[0][cur_pos[0]] < prevs[1][cur_pos[1]]) {
				if (mm.min <= col[prevs[0][cur_pos[0]]]) {
					(*ret)[ret_len] = prevs[0][cur_pos[0]];
					ret_len++;
				}
				cur_pos[0]++;
			} else if (prevs[0][cur_pos[0]] == prevs[1][cur_pos[1]]) {
				if (mm.min <= col[prevs[0][cur_pos[0]]]) {
					(*ret)[ret_len] = prevs[0][cur_pos[0]];
					ret_len++;
				}
				cur_pos[0]++;
				cur_pos[1]++;
			} else {
				if (mm.min <= col[prevs[1][cur_pos[1]]]) {
					(*ret)[ret_len] = prevs[1][cur_pos[1]];
					ret_len++;
				}
				cur_pos[1]++;
			}
		}
		break;
	case MAX:
		for (int i = 0; i < total_len; i++) {
			if (cur_pos[0] == prev_lens[0]) {
				for (int j = cur_pos[1]; j < prev_lens[1]; j++) {
					if (col[prevs[1][j]] < mm.max) {
						(*ret)[ret_len] = prevs[1][j];
						ret_len++;
					}
				}
				break;
			} else if (cur_pos[1] == prev_lens[1]) {
				for (int j = cur_pos[0]; j < prev_lens[0]; j++) {
					if (col[prevs[0][j]] < mm.max) {
						(*ret)[ret_len] = prevs[0][j];
						ret_len++;
					}
				}
				break;
			} else if (prevs[0][cur_pos[0]] < prevs[1][cur_pos[1]]) {
				if (col[prevs[0][cur_pos[0]]] < mm.max) {
					(*ret)[ret_len] = prevs[0][cur_pos[0]];
					ret_len++;
				}
				cur_pos[0]++;
			} else if (prevs[0][cur_pos[0]] == prevs[1][cur_pos[1]]) {
				if (col[prevs[0][cur_pos[0]]] < mm.max) {
					(*ret)[ret_len] = prevs[0][cur_pos[0]];
					ret_len++;
				}
				cur_pos[0]++;
				cur_pos[1]++;
			} else {
				if (col[prevs[1][cur_pos[1]]] < mm.max) {
					(*ret)[ret_len] = prevs[1][cur_pos[1]];
					ret_len++;
				}
				cur_pos[1]++;
			}
		}
		break;
	}
	*ret = realloc(*ret, sizeof(int) * ret_len);
	return ret_len;
}

int basic_scan(int *col, int col_len, minmax_t mm, int **ret) {
	*ret = malloc(sizeof(int) * col_len);
	int ret_len = 0;

	switch (mm.stype) {
	case BOTH:
		for (int i = 0; i < col_len; i++) {
			if (mm.min <= col[i] && col[i] < mm.max) {
				(*ret)[ret_len] = i;
				ret_len++;
			}
		}
		break;
	case MIN:
		for (int i = 0; i < col_len; i++) {
			if (mm.min <= col[i]) {
				(*ret)[ret_len] = i;
				ret_len++;
			}
		}
		break;
	case MAX:
		for (int i = 0; i < col_len; i++) {
			if (col[i] < mm.max) {
				(*ret)[ret_len] = i;
				ret_len++;
			}
		}
		break;
	}

	*ret = realloc(*ret, sizeof(int) * ret_len);
	return ret_len;
}

int load_selects(minmax_t **select_queries) {
	FILE *fp = fopen("select_queries.txt","r");
	assert(fp != NULL);
	char chr;
	int query_count = 0;
	while ((chr = getc(fp)) != EOF) {
		if (chr == '\n') query_count++;
	}
	rewind(fp);

	*select_queries = malloc(sizeof(minmax_t) * query_count);

	char *number;
	char *line = NULL;
	size_t buf_len = 0;
	ssize_t len_read;
	bool high_exist;
	bool low_exist;

	for (int i = 0; i < query_count; i++) {
		len_read = getline(&line, &buf_len, fp);
		(void) len_read;
		number = strtok(line, ",");
		low_exist = (strcmp(number, "null") == 0) ? false : true;
		if (low_exist) (*select_queries)[i].min = atoi(number);

		number = strtok(NULL, "\n");
		high_exist = (strcmp(number, "null") == 0) ? false : true;
		if (high_exist) (*select_queries)[i].max = atoi(number);

		assert(low_exist || high_exist);
		if (low_exist && high_exist) (*select_queries)[i].stype = BOTH;
		else (*select_queries)[i].stype = low_exist ? MIN : MAX;
	}

	if (line != NULL) free(line);
	fclose(fp);
	return query_count;
}

int load_column(int **ret){
	FILE *fp = fopen("data.txt","r");
	assert(fp != NULL);
	char chr;
	int data_count = 0;
	while ((chr = getc(fp)) != EOF) {
		if (chr == '\n') data_count++;
	}
	rewind(fp);

	*ret = malloc(sizeof(int) * data_count);

	char *line = NULL;
	size_t buf_len = 0;
	ssize_t len_read;
	for (int i = 0; i < data_count; i++) {
		len_read = getline(&line, &buf_len, fp);
		(void) len_read;
		(*ret)[i] = atoi(line);
	}
	if (line != NULL) free(line);
	fclose(fp);
	return data_count;
}
