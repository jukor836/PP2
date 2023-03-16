#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
using namespace std;
#define err_exit(code, str) { cerr << str << ": " << strerror(code) \
<< endl; exit(EXIT_FAILURE); }
pthread_mutex_t mutex;
pthread_cond_t cond;
bool state = false;
void *thread_job(void *arg)
{
    int err;

  
        // Захватываем мьютекс 
        err = pthread_mutex_lock(&mutex);
    if(err != 0)
        err_exit(err, "Cannot send signal");
        if(err != 0)
            err_exit(err, "Cannot lock mutex");
        while(!state) {
            cout<<"созданный поток"<< pthread_self()<<" ждет сигнала "<<endl;
            err = pthread_cond_wait(&cond, &mutex);
            if(err != 0)
                err_exit(err, "Cannot wait on condition variable");
            state=true;
        }
    cout<<"Поток "<< pthread_self()<<" получил сигнал и продолжил исполнение"<<endl;
    
    // Посылаем сигнал
    
    err = pthread_mutex_unlock(&mutex);
    if(err != 0)
        err_exit(err, "Cannot unlock mutex");
    

}

int main()
{
int err;
//
pthread_t thread1, thread2; // Идентификаторы потоков
// Инициализируем мьютекс и условную переменную
err = pthread_cond_init(&cond, NULL);
if(err != 0)
err_exit(err, "Cannot initialize condition variable");
err = pthread_mutex_init(&mutex, NULL);
if(err != 0)
err_exit(err, "Cannot initialize mutex");
// Создаём потоки
err = pthread_create(&thread1, NULL, thread_job, NULL);
if(err != 0)
err_exit(err, "Cannot create thread 1");
err = pthread_create(&thread2, NULL, thread_job, NULL);
if(err != 0)
err_exit(err, "Cannot create thread 1");
while(!state)
    err = pthread_cond_signal(&cond);
// Дожидаемся завершения потоков
pthread_join(thread1, NULL);

// Освобождаем ресурсы, связанные с мьютексом
// и условной переменной
pthread_mutex_destroy(&mutex);
pthread_cond_destroy(&cond);
}