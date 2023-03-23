#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include <vector>
#include <chrono>
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
    int err;
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
    vector<int> keys;
    for (auto it = map_res.begin(); it != map_res.end(); it++) {
        
        if(map_r.count(it->first)==0)
        {
            keys.push_back(it->first);
            map_r[it->first]=it->second;
        }
        else
        {
            map_r[it->first]+=it->second;
        }
    }

     if(map_r.size() < nthreads)
    {
        nthreads = map_r.size() ;
    }
     for(int i = 0; i < nthreads; i++)
    {
        params[i].count_elements = map_r.size()/nthreads;
        params[i].array_ptr = &keys[map_r.size()/nthreads*i];

        if(i == nthreads - 1)
        {
            params[i].count_elements += map_r.size() % nthreads;
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
}

int main()
 {
    int array_size=10000000;
    if(array_size <= 0 )
    {
        cout << "неправильные параметры\n";
        exit(-1);
    }
    srand(time(NULL));

    int* array = new int[array_size];

    for(int i = 0; i < array_size; i++)
    {
        array[i] = (rand() % 10);
        //cout  << array[i] << '\n';
    }
    
    for(int i=1;i<11;++i)
    {
    Param* p=new Param;
    p->array_ptr=array;
    p->count_elements=array_size;
        
    auto begin= chrono::steady_clock::now();
    mapreduce(p,&mapf,&reducef,i);
    auto end= chrono::steady_clock::now();
    auto time = chrono::duration_cast<std::chrono::microseconds>(end - begin);
    cout<<"Потоков "<<i<<" time "<<time.count()<<endl;
    }
    return 0;
}

