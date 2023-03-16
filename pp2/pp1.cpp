#include <cstdlib>
#include <iostream>
#include <cstring>
#include <pthread.h>
#include <chrono>
using namespace std;
#define err_exit(code, str) { cerr << str << ": " << strerror(code) \
    << endl; exit(EXIT_FAILURE); }
const int TASKS_COUNT = 10;
int task_list[TASKS_COUNT]; // Массив заданий
int current_task = 0; // Указатель на текущее задание
pthread_mutex_t mutex; // Мьютекс
pthread_spinlock_t spinlock;//spinlock
void do_task(int task_no)
{
    //cout<< "id_thread  "<<pthread_self()<<"   tsk_no   "<<task_no<<endl;
    int m=1;
    for(int i=0;i<100000;++i)
    {
        m*=1;
        m*=1;
    }
        
}
void *thread_job(void *arg)
{
    
    int task_no;
    int err;
    // Перебираем в цикле доступные задания
    while(true) {
        // Захватываем мьютекс для исключительного доступа
        // к указателю текущего задания (переменная
        // current_task)
        err = pthread_mutex_lock(&mutex);
        if(err != 0)
            err_exit(err, "Cannot lock mutex");
        // Запоминаем номер текущего задания, которое будем исполнять
        task_no = current_task;
        // Сдвигаем указатель текущего задания на следующее
        current_task++;
        // Освобождаем мьютекс
        err = pthread_mutex_unlock(&mutex);
        if(err != 0)
            err_exit(err, "Cannot unlock mutex");
        // Если запомненный номер задания не превышает
        // количества заданий, вызываем функцию, которая
        // выполнит задание.
        // В противном случае завершаем работу потока
        
        if(task_no < TASKS_COUNT){
            do_task(task_no);
            
        }

        else
            return NULL;
        
    }
}
void *thread_job_s(void *arg)
{

    int task_no;
    int err;
    // Перебираем в цикле доступные задания
    while(true) {
        // Захватываем мьютекс для исключительного доступа
        // к указателю текущего задания (переменная
        // current_task)
        err = pthread_spin_lock(&spinlock);
        if(err != 0)
            err_exit(err, "Cannot lock spin");
        // Запоминаем номер текущего задания, которое будем исполнять
        task_no = current_task;
        // Сдвигаем указатель текущего задания на следующее
        current_task++;
        // Освобождаем мьютекс
        err = pthread_spin_unlock(&spinlock);
        if(err != 0)
            err_exit(err, "Cannot unlock spin");
        // Если запомненный номер задания не превышает
        // количества заданий, вызываем функцию, которая
        // выполнит задание.
        // В противном случае завершаем работу потока
        
        if(task_no < TASKS_COUNT){
            do_task(task_no);
            
        }

        else
            return NULL;
        
    }
}
int main()
{
    int N=21,list[2*N];
    pthread_t thread[N],thread2[N]; // Идентификаторы потоков
    int err; // Код ошибки
    // Инициализируем массив заданий случайными числами
    for(int i=0; i<TASKS_COUNT; ++i)
        task_list[i] = rand() % TASKS_COUNT;
    //Инициализируем мьютекс
    err = pthread_mutex_init(&mutex, NULL);
    if(err != 0)
        err_exit(err, "Cannot initialize mutex");
    err = pthread_spin_init(&spinlock, 0);
    if(err != 0)
        err_exit(err, "Cannot initialize spinlock");
    cout<<"mutex"<<endl;
    // Создаём потоки
    for(int j=2;j<N;++j){
        auto begin= chrono::steady_clock::now();
        for( int i=0;i<j;++i){
            err = pthread_create(&thread[i], NULL, thread_job, NULL);
            if(err != 0)
                err_exit(err, "Cannot create thread 1");
        }
        auto end= chrono::steady_clock::now();
        auto time = chrono::duration_cast<std::chrono::microseconds>(end - begin);
        list[j]=time.count();
        for( int i=0;i<j;++i)
            pthread_join(thread[i], NULL);
    }
    cout<<"spinlock"<<endl;
    for(int j=2;j<N;++j){
        auto begin= chrono::steady_clock::now();
        for( int i=0;i<j;++i){
            err = pthread_create(&thread2[i], NULL, thread_job_s, NULL);
            if(err != 0)
                err_exit(err, "Cannot create thread 1");
        }
        auto end= chrono::steady_clock::now();
        auto time2 = chrono::duration_cast<std::chrono::microseconds>(end - begin);
        list[N+j]=time2.count();
        
        for( int i=0;i<j;++i)
           pthread_join(thread2[i], NULL);
    }
    for(int j=2;j<N;++j)
        cout<<"Количество потоков "<<j<<" time mutex "<<list[j]<<" time spinlock "<<list[N+j]<<endl;
    // Освобождаем ресурсы, связанные с мьютексом
    pthread_mutex_destroy(&mutex);
    pthread_spin_destroy(&spinlock);
}