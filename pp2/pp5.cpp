#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
using namespace std;
#define err_exit(code, str) { cerr << str << ": " << strerror(code) \
<< endl; exit(EXIT_FAILURE); }
pthread_mutex_t mutex;
bool state = false;
bool state2=true;
void signal()
{
    state=true;
}
void wait()
{
    int err;
    err = pthread_mutex_unlock(&mutex);
    if(err != 0)
        err_exit(err, "Cannot unlock mutex");
    while(!state) ;
    state=false;
}
void *thread_job(void *arg)
{
    int err;
    
    // Захватываем мьютекс 
    err = pthread_mutex_lock(&mutex);
    if(err != 0)
        err_exit(err, "Cannot lock mutex");
    state2=false;
    cout<<"созданный поток "<< pthread_self()<<" ждет сигнала "<<endl;
    wait();
    cout<<"Поток "<< pthread_self()<<" получил сигнал и продолжил исполнение"<<endl;
    err = pthread_mutex_unlock(&mutex);
    if(err != 0)
        err_exit(err, "Cannot unlock mutex");
    
    
}
void *thread_job2(void *arg)
{
    while(state2);
    int err;
    // Захватываем мьютекс 
    err = pthread_mutex_lock(&mutex);
    if(err != 0)
        err_exit(err, "Cannot lock mutex");
    cout<<"Подаем сигнал"<<endl;    
    signal();
        err = pthread_mutex_unlock(&mutex);
    if(err != 0)
        err_exit(err, "Cannot unlock mutex");
}
int main()
{
int err;
//
pthread_t thread1, thread2; // Идентификаторы потоков
// Инициализируем мьютекс
err = pthread_mutex_init(&mutex, NULL);
if(err != 0)
err_exit(err, "Cannot initialize mutex");
// Создаём потоки
err = pthread_create(&thread1, NULL, thread_job, NULL);
if(err != 0)
err_exit(err, "Cannot create thread 1");

err = pthread_create(&thread2, NULL, thread_job2, NULL);
if(err != 0)
err_exit(err, "Cannot create thread 2");

// Дожидаемся завершения потоков
pthread_join(thread1, NULL);
pthread_join(thread2, NULL);
// Освобождаем ресурсы, связанные с мьютексом
pthread_mutex_destroy(&mutex);
}
