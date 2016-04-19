#include<list>
#include<cstdio>
#include<pthread.h>
#include<iostream>
#include "locker.h"
using namespace std;

class locker;
template<typename T>
class threadpool
{
public:
	threadpool(int threadNumber = 10,int maxRequest=1024);
	~threadpool();
	bool append(T*request);             //往请求队列中添加任务
private:
	static void *worker(void *arg);
	void run();
private:
	int mthreadNumber;               //线程的最大数量
	int mmaxRequest;                 //请求队列中的最大请求数
	pthread_t* mthreads;             //描述线程池的数组,大小为mthreadNumber
	std::list<T*> mrequestQueue;     //请求队列    
	locker mqueueLocker;             //保护请求队列的互斥锁
	sem mprosess;                    //是否有任务需要处理
	bool mstop;                      //是否结束线程
};
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
threadpool<T>::threadpool(int threadNumber,int maxRequest)
:mthreadNumber(threadNumber)
,mmaxRequest(maxRequest)
,mstop(false)
,mthreads(NULL)
{
	if( (mthreadNumber <= 0) || (mmaxRequest <= 0) )
	{
		throw"";
	}
	mthreads = new pthread_t[mthreadNumber];
	if (!mthreads)
	{
		throw"";
	}
	//创建threadNumber个线程,并都设置成脱离线程
	for(int i = 0; i < threadNumber; ++i)
	{
		if( pthread_create(mthreads+i,NULL,worker,this) != 0 )
		{
			delete [] mthreads;
			throw"";
		}
		if (pthread_detach(mthreads[i]) != 0)
		{
			delete [] mthreads;
			throw"";
		}
	}
}

template<typename T>
threadpool<T>::~threadpool()
{
	delete [] mthreads;
	mstop = true;
}

template<typename T>
bool threadpool<T>::append(T* request)
{
	//操作请求队列时要加锁,因为他被所有线程共享
	mqueueLocker.lock();                      
	if (mrequestQueue.size() > mmaxRequest)
	{
		mqueueLocker.unlock();
		return false;
	}
	mrequestQueue.push_back(request);
	mqueueLocker.unlock();
	mprosess.post();
	return true;
}



template<typename T>
void* threadpool<T>::worker(void *arg)
{
	threadpool *pool = (threadpool  *)arg;
	pool->run();
	return pool;
}



template<typename T>
void threadpool<T>::run()
{
	while(!mstop)
	{
		mprosess.wait();                //等待信号量
		mqueueLocker.lock();
		if ( mrequestQueue.empty())
		{
			mqueueLocker.unlock();
			continue;
		}
		T*request = mrequestQueue.front();
		mrequestQueue.pop_front();
		mqueueLocker.unlock();
		if (!request)
		{
			continue;
		}
		request->process();
	}
}
