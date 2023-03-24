#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <regex.h>
#include <queue>
#include <unordered_set>
#include <string>
#include <iostream>
#define BUFSIZE 16384
#define err_exit(code, str) { cerr << str << ": " << strerror(code) \
    << endl; exit(EXIT_FAILURE); }
using namespace std;
pthread_mutex_t mutex;
pthread_cond_t cond;
size_t lr = 0;
queue<char *> sq;
unordered_set<char*> check;
bool state=true;
size_t filterit(void *ptr, size_t size, size_t nmemb, char *stream)
{
  if ( (lr + size*nmemb) > BUFSIZE ) return BUFSIZE;
  memcpy(stream+lr, ptr, size*nmemb);
  lr += size*nmemb;
  return size*nmemb;
}
void write(char* cur)
{
  cout<<cur<<" "<<pthread_self()<<endl;
  char buffer[BUFSIZE];
  CURL *curlHandle;
  curlHandle = curl_easy_init();
  curl_easy_setopt(curlHandle, CURLOPT_URL, cur);
  curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, filterit);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, buffer);
  int success = curl_easy_perform(curlHandle);
  curl_easy_cleanup(curlHandle);
  buffer[lr] = 0;
  regmatch_t amatch;
  regex_t cregex;
  regcomp(&cregex, "https://*", REG_NEWLINE);
  int eflag = 0;
  char *ps=buffer;
  while(regexec(&cregex, ps, 1, &amatch, eflag)==0){
  {int bi = amatch.rm_so;
  char buf[50];
  for (int i=0;i<50;i++)
  {
    if(ps[bi]=='\"') break;
    buf[i]=ps[bi];
    bi++;
    
  }
  if(check.count(buf)==0)
        sq.push(buf);
  ps+= amatch.rm_eo;
  eflag = REG_NOTBOL;
  //printf("%s\n", ps);
  }}
  regfree(&cregex);
  check.insert(cur);
    int err = pthread_cond_signal(&cond);
    if(err != 0)
    err_exit(err, "Cannot send signal");
  
}

void* thread_job(void* arg)
{
  int err;
  while(!sq.empty()){
    while(!sq.empty())
    {
      state=true;         
      err = pthread_mutex_lock(&mutex);
      if(err != 0)
        err_exit(err, "Cannot lock mutex");
      char* cur=sq.front();
      sq.pop();
      err = pthread_mutex_unlock(&mutex);
      if(err != 0)
        err_exit(err, "Cannot unlock mutex");
      write(cur);
  }
  state=false;
  if(state){
  err = pthread_cond_wait(&cond, &mutex);
  if(err != 0)
    err_exit(err, "Cannot wait on condition variable");
  }
  }
  
}
int main()
{
  sq.push("https://www.stroustrup.com");
  int err, nthreads=5;
  pthread_t* threads = new pthread_t[nthreads];
  err = pthread_mutex_init(&mutex, NULL);
  if(err != 0)
  {   
    cout<< "Cannot initialize mutex";
    return 0;
  }
  err = pthread_cond_init(&cond, NULL);
  if(err != 0)
      err_exit(err, "Cannot initialize condition variable");
  for(int i = 0; i < nthreads; i++)
    {
      
        // Создание потока
        err = pthread_create(&threads[i], NULL,thread_job , NULL);
        if(err != 0)
        {
            cout << "Cannot create a thread: " << strerror(err) << endl;
        exit(-1);
        }

    }  
    for(int i = 0; i < nthreads; ++i)
    {
        pthread_join(threads[i], NULL);
    }
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
  return 0;
}
