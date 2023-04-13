#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include <vector>
#include <chrono>
#include <ctime> 
using namespace std;
#define err_exit(code, str) { cerr << str << ": " << strerror(code) \
<< endl; exit(EXIT_FAILURE); }
map <int,int> map_res;
map <int,int> map_r;
int res=0;
typedef void* (*func)(void* arg);
struct Param
{
    int* array_ptr;
    int count_elements;
};
void* mapf(void* arg){
    Param* params = (Param*)arg;
    for(int i = 0; i <params->count_elements ; ++i)
    {
        map_res[params->array_ptr[i] ]+=1;
        
    }    

}
void* reducef(void* arg)
{
    Param* params = (Param*)arg;
     for(int i = 0; i < params->count_elements; ++i)
    {
       res+= map_r[params->array_ptr[i] ]*params->array_ptr[i];   
    }

}
void mapreduce(Param* array,func mapfunc,func reducefunc,int nthreads)
{
    unsigned int start_time =  clock(); 
    map_res.clear();
    map_r.clear();
    res=0;
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

     if(map_res.size() < nthreads)
    {
        nthreads = map_res.size() ;
    }

     for(int i = 0; i < nthreads; i++)
    {
        params[i].count_elements = map_res.size()/nthreads;
        params[i].array_ptr = &map_res[map_res.size()/nthreads*i];
        if(i == nthreads - 1)
        {
            params[i].count_elements += map_res.size() % nthreads;
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


    //cout << "Сумма всех элементов " <<res<<endl;;
    
    delete[] params;
    delete[] array;
    delete[] threads;
    unsigned int end_time = clock(); // конечное время
    unsigned int time = end_time - start_time;
    cout<<"Потоков "<<nthreads<<" time "<<time<<endl;
}

int main()
 {
    int array_size=10000;
    if(array_size <= 0 )
    {
        cout << "неправильные параметры\n";
        exit(-1);
    }
    srand(time(NULL));

    int* array = new int[array_size];

    for(int i = 0; i < array_size; i++)
    {
        array[i] = (rand() % 19);
        //cout  << array[i] << '\n';
    }
    
    for(int i=1;i<6;++i)
    {
    Param* p=new Param;
    p->array_ptr=array;
    p->count_elements=array_size;
        

    mapreduce(p,&mapf,&reducef,i);

    }
    return 0;
}
