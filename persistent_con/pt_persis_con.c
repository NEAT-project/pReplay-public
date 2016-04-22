/* <DESC>
 * 6 persistent connections
 * </DESC>
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <curl/curl.h>



int download=0;

//static const char *urls[]={
char *urls[]={
	"http://example.com/",
	"http://example.com/",
	"http://example.com/",
	"http://example.com/",
	"http://example.com/",
	"http://example.com/",
	"http://example.com/",
	"http://example.com/",
	"http://example.com/",
	"http://example.com/"
};

struct memory_chunk {
        char *memory;
        size_t size;
        int enabled;
};


static struct worker *workers = NULL;




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


void *run_worker(void *url)
{
	double bytes;
	long header_bytes;
	double transfer_time;
	int res,i;
	
	CURL *curl;
	curl=curl_easy_init();
	
	struct memory_chunk chunk;
	
	
	/*if(!curl)
		return -1;*/
	
	if((res = curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1L)) != CURLE_OK) {
		fprintf(stderr, "cURL option error: %s\n", curl_easy_strerror(res));
	}
	
		/* send all data to this function  */
	if((res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, memory_callback)) != CURLE_OK) {
		fprintf(stderr, "cURL option error: %s\n", curl_easy_strerror(res));
	}


	/* we pass our 'chunk' struct to the callback function */
	if((res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk)) != CURLE_OK) {
		fprintf(stderr, "cURL option error: %s\n", curl_easy_strerror(res));
	}


	/* some servers don't like requests that are made without a user-agent
	   field, so we provide one */
	if((res = curl_easy_setopt(curl, CURLOPT_USERAGENT, "get-http/0.1")) != CURLE_OK) {
		fprintf(stderr, "cURL option error: %s\n", curl_easy_strerror(res));
	}
	
	chunk.memory = NULL;
	chunk.size = 0;
	chunk.enabled = 0;
	//char *p="http://example.com";
	printf("URL: %s \n",(char *)url);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	chunk.size = 0;
	
	if((res=curl_easy_perform(curl)) != CURLE_OK){
		perror("Curl error");
	}
	else{
		if((res = curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &bytes)) != CURLE_OK ||
				(res = curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &header_bytes)) != CURLE_OK ||
				(res = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &transfer_time)) != CURLE_OK ) {
				fprintf(stderr, "cURL error: %s\n", curl_easy_strerror(res));
			}
		printf("Time: %f Size: %lu\n", transfer_time, (long)bytes+header_bytes);
		 
	}
	//data->status=STATUS_READY;
	if (download>0){
	    printf("download complete\n");
		download --;
	}
	return NULL;
}


int main()
{
	
	int i;
	

    int used_node=0;
    pthread_t tid;
    int error; 
	
	for(i=0;i<9;i++){
		if (i>used_node) i--;
	
			  if (download<6){
				used_node++;
				error = pthread_create(&tid,
                           NULL, /* default attributes please */
                           run_worker,
                           (void *)urls[i]);
				if(0 != error)
					fprintf(stderr, "Couldn't run thread number %d, errno %d\n", i, error);
				download++;
				printf ("I am %d download\n", download);
			}
		
	    }
    
    sleep(2);

	return 0;
}
