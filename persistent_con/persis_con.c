/* <DESC>
 * 6 persistent connections
 * </DESC>
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "worker.h"



static struct worker *workers = NULL;

void kill_workers()
{
	struct worker *w;
	for(w = workers; w; w = w->next) kill_worker(w);
}

/*int get_started (int num_worker)
{
	struct worker *w;
	int i;
	for(i=0; i< num_worker; i++){
		w=malloc(sizeof(*w));
		if(w==NULL){
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		if(start_worker(w)!=0) exit(EXIT_FAILURE);
		w->next=workers;
		workers=w;
	}
	kill_workers();
	return 0;	
}*/

int get_started (int num_worker)
{
	struct worker *w;
	int i;
	for(i=0; i< num_worker; i++){
		w=malloc(sizeof(*w));
		if(w==NULL){
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		w->status=STATUS_READY;
		w->next=workers;
		workers=w;
	}
	return 0;	
}

int main()
{
	if(get_started(6)) perror("get_started failure");
	
	struct worker *w;
	for(w=workers;w;w=w->next){
		if (w->status==STATUS_READY){
			if(start_worker(w)!=0) exit(EXIT_FAILURE);
			kill_worker(w);
			break;
		}
	}
	
	return 0;
}
