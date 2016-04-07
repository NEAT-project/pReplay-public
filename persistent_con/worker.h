#ifndef WORKER_H
#define WORKER_H
#define _GNU_SOURCE             /* See feature_test_macros(7) */

#include <unistd.h>


#define STATUS_READY 0
#define STATUS_WORKING 1

struct memory_chunk {
	char *memory;
	size_t size;
	int enabled;
};



struct worker {
	struct worker *next;
	char *url;
	int status;
	int pid;
    struct memory_chunk chunk;
	CURL *curl;
	CURLcode res;
};

int start_worker(struct worker *w);
int kill_worker(struct worker *w);

#endif
