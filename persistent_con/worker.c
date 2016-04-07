#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <curl/curl.h>
#include "worker.h"




static size_t memory_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct memory_chunk *chunk = userp;
	if(!chunk->enabled) return realsize;

	chunk->memory = realloc(chunk->memory, chunk->size + realsize + 1);
	if(chunk->memory == NULL) {
		perror("realloc");
		return 0;
	}

	memcpy(&(chunk->memory[chunk->size]), contents, realsize);
	chunk->size += realsize;
	chunk->memory[chunk->size] = 0;
	return realsize;
}

static int init_worker(struct worker *data)
{
	int res;
	data->curl=curl_easy_init();
	
	if(!data->curl)
		return -1;
	
	if((res = curl_easy_setopt(data->curl, CURLOPT_TCP_NODELAY, 1L)) != CURLE_OK) {
		fprintf(stderr, "cURL option error: %s\n", curl_easy_strerror(res));
	}
	
		/* send all data to this function  */
	if((res = curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, memory_callback)) != CURLE_OK) {
		fprintf(stderr, "cURL option error: %s\n", curl_easy_strerror(res));
	}


	/* we pass our 'chunk' struct to the callback function */
	if((res = curl_easy_setopt(data->curl, CURLOPT_WRITEDATA, (void *)&data->chunk)) != CURLE_OK) {
		fprintf(stderr, "cURL option error: %s\n", curl_easy_strerror(res));
	}


	/* some servers don't like requests that are made without a user-agent
	   field, so we provide one */
	if((res = curl_easy_setopt(data->curl, CURLOPT_USERAGENT, "get-http/0.1")) != CURLE_OK) {
		fprintf(stderr, "cURL option error: %s\n", curl_easy_strerror(res));
	}
	
	data->chunk.memory = NULL;
	data->chunk.size = 0;
	data->chunk.enabled = 0;

	return 0;
}



static int run_worker(struct worker *data)
{
	double bytes;
	long header_bytes;
	double transfer_time;
	int res,i;
	if(init_worker(data)) return -1;
	char *p="http://example.com";
	curl_easy_setopt(data->curl, CURLOPT_URL, p);
	data->chunk.size = 0;
	
	if((res=curl_easy_perform(data->curl)) != CURLE_OK){
		perror("Curl error");
	}
	else{
		if((res = curl_easy_getinfo(data->curl, CURLINFO_SIZE_DOWNLOAD, &bytes)) != CURLE_OK ||
				(res = curl_easy_getinfo(data->curl, CURLINFO_HEADER_SIZE, &header_bytes)) != CURLE_OK ||
				(res = curl_easy_getinfo(data->curl, CURLINFO_TOTAL_TIME, &transfer_time)) != CURLE_OK ) {
				fprintf(stderr, "cURL error: %s\n", curl_easy_strerror(res));
			}
		printf("Time: %f Size: %lu\n", transfer_time, (long)bytes+header_bytes);
		 
	}
	data->status=STATUS_READY;
	return 0;
}


int start_worker(struct worker *w)
{
	int cpid;
	w->next=NULL;
	w->status=STATUS_READY;
	
	cpid=fork();
	if(cpid== -1){
		perror("pipe");
		return EXIT_FAILURE;
	}
	
	if(cpid==0){
		w->status = STATUS_WORKING;
		//struct worker_data wd={};
		_exit(run_worker(w));
	}
	else{
		w->pid=cpid;
		return 0;
	}
	w->status = STATUS_READY;
	return 0;
}

int kill_worker(struct worker *w)
{
	waitpid(w->pid, NULL, 0);
	return 0;
}
