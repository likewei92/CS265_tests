#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include "select_pos.h"

int main() {
	int *col;
	int col_len = load_column(&col);

	minmax_t *select_queries;
	int queries_count = load_selects(&select_queries);

	clock_t start, end;
	double cpu_time_used;

	int **basic_rets = malloc(sizeof(int*) * queries_count);
	int *basic_ret_lens = malloc(sizeof(int) * queries_count);
	int temp = queries_count - 1;
	for (int i = 0; i < temp; i++) {
		basic_ret_lens[i] = basic_scan(col, col_len, select_queries[i], &(basic_rets[i]));
	}
	start = clock();
	basic_ret_lens[temp] = basic_scan(col, col_len, select_queries[temp], &(basic_rets[temp]));
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("%f\n", cpu_time_used);

	int **recycle_rets = malloc(sizeof(int*) * queries_count);
	int *recycle_ret_lens = malloc(sizeof(int) * queries_count);

	for (int i = 0; i < temp; i++) {
		recycle_ret_lens[i] = basic_scan(col, col_len, select_queries[i], &(recycle_rets[i]));
	}

	start = clock();
	recycle_ret_lens[temp] = sub_prev_scan(col, col_len, recycle_rets, recycle_ret_lens, temp, select_queries[temp], &(recycle_rets[temp]));
//	recycle_ret_lens[temp] = super_prev_scan(col, col_len, recycle_rets[1], recycle_ret_lens[1], select_queries[temp], &(recycle_rets[temp]));
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

int sub_prev_scan(int *col, int col_len, int **prevs, int *prev_lens, int n_prevs, minmax_t mm, int **ret) {
	int ret_len = 0;
	int cur_pos[n_prevs];
	memset(cur_pos, 0, sizeof(int) * n_prevs);
	*ret = malloc(sizeof(int) * col_len);
	uint fins = 0;
	uint quals = 0;
	uint advs;
	int adv_ind;
	int red_adv_ind;
	int mask_cache = masks[n_prevs] - 1;

//	int small_n_prevs = n_prevs - 1;

	switch (mm.stype) {
	case BOTH:
		while (fins == 0) {
			quals = 0;
			for (int i = 0; i < n_prevs; i++) { quals |= ((mm.min <= col[prevs[i][cur_pos[i]]]) & (col[prevs[i][cur_pos[i]]] < mm.max)) * masks[i]; }
			advs = mask_cache;
			for (int i = 0; i < n_prevs; i++) {
				for (int j = 0; j < n_prevs; j++) {
					advs &= (mask_cache - (!(prevs[i][cur_pos[i]] <= prevs[j][cur_pos[j]]) << i));
				}
			}
			adv_ind = __builtin_ffs(quals & advs);
			red_adv_ind = adv_ind - 1;
			(*ret)[ret_len] = (adv_ind) ? prevs[red_adv_ind][cur_pos[red_adv_ind]] : adv_ind;
			ret_len += ((quals & advs) > 0);
			for (int i = 0; i < n_prevs; i++) {
				cur_pos[i] += ((advs & masks[i]) != 0);
				fins |= ((cur_pos[i] == prev_lens[i]) * masks[i]);
			}
		}
		if (fins == 1){
			for (int j = cur_pos[1]; j < prev_lens[1]; j++) {
				quals = ((mm.min <= col[prevs[1][j]]) & (col[prevs[1][j]] < mm.max));
				(*ret)[ret_len] = prevs[1][j];
				ret_len += quals;
			}
		} else if (fins == 2) {
			for (int j = cur_pos[0]; j < prev_lens[0]; j++) {
				quals = ((mm.min <= col[prevs[0][j]]) & (col[prevs[0][j]] < mm.max));
				(*ret)[ret_len] = prevs[0][j];
				ret_len += quals;
			}
		}
		break;
	case MIN:

		break;
	case MAX:

		break;
	}
	*ret = realloc(*ret, sizeof(int) * ret_len);
	return ret_len;
}


int super_prev_scan(int *col, int col_len, int *prev, int prev_len, minmax_t mm, int **ret) {
	int ret_len = 0;
	int cur_pos = 0;
	*ret = malloc(sizeof(int) * col_len);

	int test;
	int skip;
	(void) prev_len;

	switch (mm.stype) {
	case BOTH:
		for (int i = 0; i < col_len; i++) {
			skip = (i == prev[cur_pos]);
			test = skip ? 1 : ((mm.min <= col[i]) & (col[i] < mm.max));
			(*ret)[ret_len] = i;
			cur_pos += skip;
			ret_len += test;
		}
		break;
	case MIN:
		for (int i = 0; i < col_len; i++) {
			skip = (i == prev[cur_pos]);
			test = skip ? 1 : (mm.min <= col[i]);
			(*ret)[ret_len] = i;
			cur_pos += skip;
			ret_len += test;
		}
		break;
	case MAX:
		for (int i = 0; i < col_len; i++) {
			skip = (i == prev[cur_pos]);
			test = skip ? 1 : (col[i] < mm.max);
			(*ret)[ret_len] = i;
			cur_pos += skip;
			ret_len += test;
		}
		break;
	}
	*ret = realloc(*ret, sizeof(int) * ret_len);
	return ret_len;
}

int basic_scan(int *col, int col_len, minmax_t mm, int **ret) {
	*ret = malloc(sizeof(int) * col_len);
	int ret_len = 0;
	int test;

	switch (mm.stype) {
	case BOTH:
		for (int i = 0; i < col_len; i++) {
			test = (mm.min <= col[i]) & (col[i] < mm.max);
			(*ret)[ret_len] = i;
			ret_len += test;
		}
		break;
	case MIN:
		for (int i = 0; i < col_len; i++) {
			test = (mm.min <= col[i]);
			(*ret)[ret_len] = i;
			ret_len += test;
		}
		break;
	case MAX:
		for (int i = 0; i < col_len; i++) {
			test = (col[i] < mm.max);
			(*ret)[ret_len] = i;
			ret_len += test;
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
