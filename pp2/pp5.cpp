#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
using namespace std;
#define err_exit(code, str) { cerr << str << ": " << strerror(code) \
<< endl; exit(EXIT_FAILURE); }
pthread_mutex_t mutex;
bool state = true,state2=true;
// Тип условной переменной
typedef struct CondVar
{
    pthread_mutex_t mutex;
} CondVar;

CondVar cond;
// Инициализация условной переменнной
void cond_init(CondVar *cond)
{
    int err = pthread_mutex_init(&cond->mutex, NULL);
    if (err != 0)
        err_exit(err,"Error: cannot initialize mutex!");
}
// Уничтожение условной переменнной
void cond_destroy(CondVar *cond)
{
    pthread_mutex_destroy(&cond->mutex);
}
// Блокировка до наступления события
void wait(CondVar *cond, pthread_mutex_t *mutex)
{
    // Блокируем условную переменную
    int err = pthread_mutex_lock(&cond->mutex);
    if (err != 0)
        err_exit(err,"Error: cannot lock condition mutex!");
    state=true;
    // Освобождаем условную переменную
    err = pthread_mutex_unlock(&cond->mutex);
    if (err != 0)
        err_exit(err,"Error: cannot unlock condition mutex!");


    // Освобождаем мьютекс до получения сигнала
    err = pthread_mutex_unlock(mutex);
    if (err != 0)
        err_exit(err,"Error: cannot unlock mutex!");


    // Ждём пока не получен сигнал
    while (state);

    // Блокируем мьютекс после получения сигнала
    err = pthread_mutex_lock(mutex);
    if (err != 0)
        err_exit(err,"Error: cannot lock mutex!");
}
// Сигнал для выхода из блокировки одного потока
void signal(CondVar *cond)
{
    // Блокируем условную переменную
    int err = pthread_mutex_lock(&cond->mutex);
    if (err != 0)
        err_exit(err,"Error: cannot lock condition mutex!");
    state= false;
    // Освобождаем условную переменную
    err = pthread_mutex_unlock(&cond->mutex);
    if (err != 0)
        err_exit(err,"Error: cannot unlock condition mutex!");
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
    wait(&cond,&mutex);
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
    signal(&cond);
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
cond_init(&cond);
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
cond_destroy(&cond);
// Освобождаем ресурсы, связанные с мьютексом
pthread_mutex_destroy(&mutex);
}
