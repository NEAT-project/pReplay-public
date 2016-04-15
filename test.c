/* <DESC>
 * Multiplexed HTTP/2 downloads over a single connection
 * adapated from oroginal example given in wwww.haxx.se
 * </DESC>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* somewhat unix-specific */
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* curl stuff */
#include <curl/curl.h>

#ifndef CURLPIPE_MULTIPLEX
/* This little trick will just make sure that we don't enable pipelining for
   libcurls old enough to not have this symbol. It is _not_ defined to zero in
   a recent libcurl header. */
#define CURLPIPE_MULTIPLEX 0
#endif

#define NUM_HANDLES 1000

struct stat st = {0};

void *curl_hnd[NUM_HANDLES];
int num_transfers;

/* a handle to number lookup, highly ineffective when we do many
   transfers... */
static int hnd2num(CURL *hnd)
{
  int i;
  for(i=0; i< num_transfers; i++) {
    if(curl_hnd[i] == hnd)
      return i;
  }
  return 0; /* weird, but just a fail-safe */
}

void usage(int argc, char* argv[])
{
	printf("Usage: %s url_file\n", argv[0]);
	exit(1);
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    printf ("Got it\n");
    return 1;
}

static void setup(CURL *hnd, const char *url, int num)
{
 printf("URL: %s \n", url);

 /* send all data to this function  */
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_data);
    
  /* set the  URL */
  curl_easy_setopt(hnd, CURLOPT_URL, url);


  /* HTTP/2 please */
  curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

  /* we use a self-signed test server, skip verification during debugging */
  curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);

#if (CURLPIPE_MULTIPLEX > 0)
  /* wait for pipe connection to confirm */
  curl_easy_setopt(hnd, CURLOPT_PIPEWAIT, 1L);
#endif

  curl_hnd[num] = hnd;
}

/*
 * Download all the files over HTTP/2, using the same physical connection!
 */
int main(int argc, char **argv)
{

  if (argc<2)
	usage(argc,argv);

  struct timeval start, stop;
  gettimeofday(&start, NULL);

  CURL *easy[NUM_HANDLES];
  CURLM *multi_handle;
  int i=0,num_transfers=0;
  int still_running; /* keep number of running handles */

  /* init a multi stack */
  multi_handle = curl_multi_init();

  FILE *file;
  char line[256], orig_line[256];
  char * strt[100];

  file=fopen(argv[1], "r");
  if(file==NULL){
	perror("Error opening file");
	return -1;
   }

  while (fgets(line, sizeof(line), file)) {
	strcpy(orig_line,line);
	strt[0]=strtok(line,"/");
	if(strt!=NULL && strcmp(strt[0],"https:")==0){
    		easy[i] = curl_easy_init();
                /* set options */
		setup(easy[i],strtok(orig_line,"\n"),num_transfers);
    		/* add the individual transfer */
    		curl_multi_add_handle(multi_handle, easy[i]);
		num_transfers++;
		i++;
        }
  }



  curl_multi_setopt(multi_handle, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);

  
  /* we start some action by calling perform right away */ 
  curl_multi_perform(multi_handle, &still_running);
 do {
    struct timeval timeout;
    int rc; /* select() return code */ 
    CURLMcode mc; /* curl_multi_fdset() return code */ 
 
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd = -1;
 
    long curl_timeo = -1;
 
    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);
 
    /* set a suitable timeout to play around with */ 
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
 
    curl_multi_timeout(multi_handle, &curl_timeo);
    if(curl_timeo >= 0) {
      timeout.tv_sec = curl_timeo / 1000;
      if(timeout.tv_sec > 1)
        timeout.tv_sec = 1;
      else
        timeout.tv_usec = (curl_timeo % 1000) * 1000;
    }
 
    /* get file descriptors from the transfers */ 
    mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
 
    if(mc != CURLM_OK) {
      fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
      break;
    }
 
    /* On success the value of maxfd is guaranteed to be >= -1. We call
       select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
       no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
       to sleep 100ms, which is the minimum suggested value in the
       curl_multi_fdset() doc. */ 
 
    if(maxfd == -1) {
#ifdef _WIN32
      Sleep(100);
      rc = 0;
#else
      /* Portable sleep for platforms other than Windows. */ 
      struct timeval wait = { 0, 100 * 1000 }; /* 100ms */ 
      rc = select(0, NULL, NULL, NULL, &wait);
#endif
    }
    else {
      /* Note that on some platforms 'timeout' may be modified by select().
         If you need access to the original value save a copy beforehand. */ 
      rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
    }
 
    switch(rc) {
    case -1:
      /* select error */ 
      break;
    case 0:
    default:
      /* timeout or readable/writable sockets */ 
      curl_multi_perform(multi_handle, &still_running);
      break;
    }
  
  } while(still_running);
 
 
  for(i=0; i<num_transfers; i++){
    curl_easy_cleanup(easy[i]);
   /* remove an easy handle */
	curl_multi_remove_handle(multi_handle, easy[i]);}

    // another request
    easy[0] = curl_easy_init();
   /* set options */
    setup(easy[0],"https://xxx/xxxx/xxx.js",0);
   /* add the individual transfer */
    curl_multi_add_handle(multi_handle, easy[0]);
   	
   	

   int still_running_2; /* keep number of running handles */
  /* we start some action by calling perform right away */ 
  curl_multi_perform(multi_handle, &still_running_2);
  
  
 do {
    struct timeval timeout;
    int rc; /* select() return code */ 
    CURLMcode mc; /* curl_multi_fdset() return code */ 
    
   
 
    fd_set fdread2;
    fd_set fdwrite2;
    fd_set fdexcep2;
    int maxfd = -1;
 
    long curl_timeo = -1;
 
    FD_ZERO(&fdread2);
    FD_ZERO(&fdwrite2);
    FD_ZERO(&fdexcep2);
 
    /* set a suitable timeout to play around with */ 
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
 
    curl_multi_timeout(multi_handle, &curl_timeo);
    if(curl_timeo >= 0) {
      timeout.tv_sec = curl_timeo / 1000;
      if(timeout.tv_sec > 1)
        timeout.tv_sec = 1;
      else
        timeout.tv_usec = (curl_timeo % 1000) * 1000;
    }
 
    /* get file descriptors from the transfers */ 
    mc = curl_multi_fdset(multi_handle, &fdread2, &fdwrite2, &fdexcep2, &maxfd);
 
    if(mc != CURLM_OK) {
      fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
      break;
    }
 
    /* On success the value of maxfd is guaranteed to be >= -1. We call
       select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
       no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
       to sleep 100ms, which is the minimum suggested value in the
       curl_multi_fdset() doc. */ 
 
    if(maxfd == -1) {
#ifdef _WIN32
      Sleep(100);
      rc = 0;
#else
      /* Portable sleep for platforms other than Windows. */ 
      struct timeval wait = { 0, 100 * 1000 }; /* 100ms */ 
      rc = select(0, NULL, NULL, NULL, &wait);
#endif
    }
    else {
      /* Note that on some platforms 'timeout' may be modified by select().
         If you need access to the original value save a copy beforehand. */ 
      rc = select(maxfd+1, &fdread2, &fdwrite2, &fdexcep2, &timeout);
    }
 
    switch(rc) {
    case -1:
      /* select error */ 
      break;
    case 0:
    default:
      /* timeout or readable/writable sockets */ 
      curl_multi_perform(multi_handle, &still_running_2);
      break;
    }
 
  } while(still_running_2);
  
   
  curl_multi_cleanup(multi_handle);
 
  curl_easy_cleanup(easy[0]);

  return 0;
}

