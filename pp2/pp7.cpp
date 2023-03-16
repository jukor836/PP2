#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <regex.h>
#include <queue>
#include <string>
#include <regex>
#include <iostream>
#define BUFSIZE 16384
using namespace std;
pthread_mutex_t mutex;
size_t lr = 0;
queue<string> sq;
string cur;
size_t filterit(void *ptr, size_t size, size_t nmemb, char *stream)
{
  if ( (lr + size*nmemb) > BUFSIZE ) return BUFSIZE;
  memcpy(stream+lr, ptr, size*nmemb);
  lr += size*nmemb;
  return size*nmemb;
}
void* job(void* arg)
{
  FILE *fp;
  char name[] = "/home/juja/pp2/my.txt";
  (fp = fopen(name, "w"));
  if (fp == NULL)
  {
    printf("Не удалось открыть файл");
    getchar();
    exit(EXIT_FAILURE);
  }
  CURL *curlHandle;
  char buffer[BUFSIZE];
  regmatch_t amatch;
  regex_t cregex;

  curlHandle = curl_easy_init();
  curl_easy_setopt(curlHandle, CURLOPT_URL, cur);
  curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, filterit);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, buffer);
  int success = curl_easy_perform(curlHandle);
  curl_easy_cleanup(curlHandle);

  buffer[lr] = 0;
  
  regcomp(&cregex, " UTC", REG_NEWLINE);
  regexec(&cregex, buffer, 1, &amatch, 0);
  int bi = amatch.rm_so;
  while ( bi-- > 0 )
    if ( memcmp(&buffer[bi], "<BR>", 4) == 0 ) break;

  buffer[amatch.rm_eo] = 0;
  printf("%s\n", &buffer[bi+4]);
  fprintf(fp,"%s\n", &buffer[bi+4]);
  fclose(fp);
  regfree(&cregex);
  
}
regex rgx("((http|https|ftp):\\/\\/)?(([A-Z0-9][A-Z0-9_-]*)(\\.[A-Z0-9][A-Z0-9_-]*)+)");

void read()
{
  string s;
  FILE *fp;
  char name[] = "/home/juja/pp2/my.txt";
  (fp = fopen(name, "r"));
  if (fp == NULL)
  {
    printf("Не удалось открыть файл");
    getchar();
    return ;
  }
  while(fscanf(fp, "%s", &s))
  {
    std::smatch m;
    regex_search(s, m, rgx);
    if(!m.empty())
      sq.push(m[0]);

  }

}
void* thread_job(void* arg)
{
  int err;
  while(!sq.empty())
  {    
    err = pthread_mutex_lock(&mutex);
    if(err != 0)
     { 
      cout<<(err, "Cannot unlock mutex");
      exit(EXIT_FAILURE);
     }
    cur=sq.front();
    sq.pop();
    job(NULL);
    read();
    err = pthread_mutex_unlock(&mutex);
    
    if(err != 0)
     { 
      cout<<(err, "Cannot unlock mutex");
      exit(EXIT_FAILURE);
     }  
  }

}
int main()
{
  sq.push("http://example.com");
  int err, nthreads=3;
  pthread_t* threads = new pthread_t[nthreads];
  err = pthread_mutex_init(&mutex, NULL);
  if(err != 0)
  {   
    cout<< "Cannot initialize mutex";
    return 0;
  }
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
  return 0;
}