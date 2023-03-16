#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include <vector>
using namespace std;
#define err_exit(code, str) { cerr << str << ": " << strerror(code) \
<< endl; exit(EXIT_FAILURE); }
map <int,int> map_res;
int res=0;
typedef void* (*func)(void* arg);
pthread_mutex_t mutex;
struct Param
{
    int* array_ptr;
    int count_elements;
};
void* mapf(void* arg){
    int err;
    err = pthread_mutex_lock(&mutex);
    if(err != 0)
            err_exit(err, "Cannot lock mutex");
    Param* params = (Param*)arg;

    for(int i = 0; i <params->count_elements ; ++i)
    {
        map_res[params->array_ptr[i] ]+=1;
        
    }
    err = pthread_mutex_unlock(&mutex);
    if(err != 0)
        err_exit(err, "Cannot unlock mutex");
    
}

void* reducef(void* arg)
{
    int err;
    err = pthread_mutex_lock(&mutex);
    if(err != 0)
            err_exit(err, "Cannot lock mutex");
    Param* params = (Param*)arg;
     for(int i = 0; i < map_res.size(); ++i)
    {
       res+= map_res[params->array_ptr[i] ]*params->array_ptr[i];
        
    }
        err = pthread_mutex_unlock(&mutex);
    if(err != 0)
        err_exit(err, "Cannot unlock mutex");
}
void mapreduce(Param* array,func mapfunc,func reducefunc,int nthreads)
{

    if(array->count_elements < nthreads)
    {
        nthreads = array->count_elements;
    }

    // Массив аргументов, которые будут переданы в функции потоков
    Param* params = new Param[nthreads];
    // Массив потоков
    pthread_t* threads = new pthread_t[nthreads];

    int count_elements_for_thread = array->count_elements / nthreads;

    int err;
    err = pthread_mutex_init(&mutex, NULL);
    if(err != 0)
        err_exit(err, "Cannot initialize mutex");
    for(int i = 0; i < nthreads; i++)
    {
        params[i].count_elements = count_elements_for_thread;
        params[i].array_ptr = &array->array_ptr[count_elements_for_thread * i];

        if(i == nthreads - 1)
        {
            params[i].count_elements += array->count_elements % nthreads;
        }

        // Создание потока
        err = pthread_create(&threads[i], NULL, mapfunc, (void*)&params[i]);
        if(err != 0)
        {
        cout << "Cannot create a thread: " << strerror(err) << endl;
        exit(-1);
        }

    }  

    // Ждем завершения всех созданных потоков
    for(int i = 0; i < nthreads; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    vector<int> keys;
    for (auto it = map_res.begin(); it != map_res.end(); it++) {
        keys.push_back(it->first);
    }
     for(int i = 0; i < nthreads; i++)
    {
        params[i].count_elements = map_res.size()/nthreads;
        params[i].array_ptr = &keys[map_res.size()*i];

        if(i == nthreads - 1)
        {
            params[i].count_elements += array->count_elements % nthreads;
        }
        // Создание потока
        err = pthread_create(&threads[i], NULL, reducefunc, (void*)&params[i]);
        if(err != 0)
        {
        cout << "Cannot create a thread: " << strerror(err) << endl;
        exit(-1);
        }

    }  

    // Ждем завершения всех созданных потоков
    for(int i = 0; i < nthreads; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    cout << "Сумма всех элементов " <<res<<endl;;
    
    delete[] params;
    delete[] array;
    delete[] threads;
    pthread_mutex_destroy(&mutex);
}

int main()
 {
    int array_size=10;
    int count_threads=5;

    if(array_size <= 0 || count_threads <= 0)
    {
        cout << "неправильные параметры\n";
        exit(-1);
    }

    if(array_size < count_threads)
    {
        count_threads = array_size;
    }

    srand(time(NULL));

    int* array = new int[array_size];

    for(int i = 0; i < array_size; i++)
    {
        array[i] = (rand() % 10);
        cout  << array[i] << '\n';
    }
    Param* p=new Param;
    p->array_ptr=array;
    p->count_elements=array_size;
    mapreduce(p,&mapf,&reducef,count_threads);
    return 0;
}
