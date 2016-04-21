/* <DESC>
 * Multiplexed HTTP/2 downloads over a single connection
 * Using thread but not structure as argument
 * </DESC>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


/* somewhat unix-specific */
#include <sys/time.h>
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

int request_count=0;

void *curl_hnd[NUM_HANDLES];
CURL *easy[NUM_HANDLES];
CURLM *multi_handle;
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
/*
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    printf ("Got it\n");
    return 1;
}
*/

static
void dump(const char *text, int num, unsigned char *ptr, size_t size,
          char nohex)
{
  size_t i;
  size_t c;

  unsigned int width=0x10;

  if(nohex)
    // without the hex output, we can fit more on screen 
    width = 0x40;

  fprintf(stderr, "%d %s, %ld bytes (0x%lx)\n",
          num, text, (long)size, (long)size);

  for(i=0; i<size; i+= width) {

    fprintf(stderr, "%4.4lx: ", (long)i);

    if(!nohex) {
      // hex not disabled, show it 
      for(c = 0; c < width; c++)
        if(i+c < size)
          fprintf(stderr, "%02x ", ptr[i+c]);
        else
          fputs("   ", stderr);
    }

    for(c = 0; (c < width) && (i+c < size); c++) {
      // check for 0D0A; if found, skip past and start a new line of output 
      if(nohex && (i+c+1 < size) && ptr[i+c]==0x0D && ptr[i+c+1]==0x0A) {
        i+=(c+2-width);
        break;
      }
      fprintf(stderr, "%c",
              (ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.');
      // check again for 0D0A, to avoid an extra \n if it's at width 
      if(nohex && (i+c+2 < size) && ptr[i+c+1]==0x0D && ptr[i+c+2]==0x0A) {
        i+=(c+3-width);
        break;
      }
    }
    fputc('\n', stderr); // newline 
  }
}

static
int my_trace(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp)
{
  const char *text;
  int num = hnd2num(handle);
  (void)handle; // prevent compiler warning 
  (void)userp;
  switch (type) {
  case CURLINFO_TEXT:
    fprintf(stderr, "== %d Info: %s", num, data);
  default: // in case a new one is introduced to shock us 
    return 0;

  case CURLINFO_HEADER_OUT:
    text = "=> Send header";
    break;
  case CURLINFO_DATA_OUT:
    text = "=> Send data";
    break;
  case CURLINFO_SSL_DATA_OUT:
    text = "=> Send SSL data";
    break;
  case CURLINFO_HEADER_IN:
    text = "<= Recv header";
    break;
  case CURLINFO_DATA_IN:
    text = "<= Recv data";
    break;
  case CURLINFO_SSL_DATA_IN:
    text = "<= Recv SSL data";
    break;
  }

  dump(text, num, (unsigned char *)data, size, 1);
  return 0;
}






//void  request_url(char *url, int i)
void  *request_url(void *arg)
{

    request_count++;
    //struct arg_struct *args=(struct arg_struct *)arguments;
    char *url= (char *)arg;
    int still_running; /* keep number of running handles */

    printf("COUNT=%d, URL=%s\n",request_count,url);
    /* set options */
    //setup(easy[args->count], args->count, args->uri);

 FILE *out;
 char filename[128];

 snprintf(filename, 128, "dl-%d", request_count);

 out = fopen(filename, "wb");

 // write to this file
 curl_easy_setopt(easy[request_count], CURLOPT_WRITEDATA, out);

 /* send all data to this function  */
 //curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_data);


 /* set the same URL */
 curl_easy_setopt(easy[request_count], CURLOPT_URL, url);

 /* send it verbose for max debuggaility */
 //curl_easy_setopt(easy[args->count], CURLOPT_VERBOSE, 1L);
 //curl_easy_setopt(easy[args->count], CURLOPT_DEBUGFUNCTION, my_trace);

 /* HTTP/2 please */
 curl_easy_setopt(easy[request_count], CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

 /* we use a self-signed test server, skip verification during debugging */
 curl_easy_setopt(easy[request_count], CURLOPT_SSL_VERIFYPEER, 0L);
 curl_easy_setopt(easy[request_count], CURLOPT_SSL_VERIFYHOST, 0L);

#if (CURLPIPE_MULTIPLEX > 0)
 /* wait for pipe connection to confirm */
 curl_easy_setopt(easy[request_count], CURLOPT_PIPEWAIT, 1L);
#endif

 curl_hnd[request_count] = easy[request_count];

    /* add the individual transfer */
    curl_multi_add_handle(multi_handle, easy[request_count]);
 

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
  
  
}

int main(int argc, char **argv)
{
  
  int i=0;

  pthread_t http2_thread;
    //1, http2_thread2, http2_thread3;

printf("Cnt=%d\n",request_count);

 /* init a multi stack */
  multi_handle = curl_multi_init();

  easy[request_count] = curl_easy_init();
  if(pthread_create(&http2_thread, NULL,&request_url, (void *)"https://193.10.227.23:8000/dn_files/thente66.jpg"))
  {
   printf("Uh-oh!\n");
   return -1;
  }


    //request_count++;

    printf("Cnt=%d\n",request_count);


  easy[request_count] = curl_easy_init();

    if(pthread_create(&http2_thread, NULL,&request_url, (void *)"https://193.10.227.23:8000/dn_files/ullgren66.png"))
    {
    printf("Uh-oh!\n");
    return -1;
  }

   // request_count++;

    printf("Cnt=%d\n",request_count);

    easy[request_count] = curl_easy_init();
  if(pthread_create(&http2_thread, NULL,&request_url, (void *)"https://193.10.227.23:8000/dn_files/widget2.0.js"))
    {
     printf("Uh-oh!\n");
     return -1;
    }


    sleep(5);

 /* i++;
  
   easy[i] = curl_easy_init();
  request_url("https://193.10.227.23:8000/dn_files/thente66.jpg",i);
  i++;
  
  easy[i] = curl_easy_init();
  request_url("https://193.10.227.23:8000/dn_files/ullgren66.png",i);
  i++;
  
  easy[i] = curl_easy_init();
  request_url("https://193.10.227.23:8000/dn_files/widget2.0.js",i);
  i++;
 */
  /*curl_multi_cleanup(multi_handle);

  int j;
  for(j=0; j<i; i++)
    curl_easy_cleanup(easy[j]);*/
    

  return 0;
  
}
