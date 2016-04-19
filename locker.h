#ifndef _LOCKER
#define _LOCKER
#include<pthread.h>
#include<semaphore.h>

//封装信号量的类
class sem
{
public:
	sem()
	{
		if( sem_init(&msem,0,0) != 0 )
		{
			//throw std::exception();
		}
	}
	~sem(){ sem_destroy(&msem); }
	//等待信号量
	bool wait(){ return sem_wait(&msem) == 0; }
	//增加信号量
	bool post(){ return sem_post(&msem) == 0; }
private:
	sem_t msem;
};

/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//封装锁的类
class locker
{
public:
	//创建并初始化互斥锁
	locker()
	{
		if(pthread_mutex_init(&mmutex,NULL) != 0)
		{
			//throw std::exception();
		}
	}
	//销毁互斥锁
	~locker()
	{
		pthread_mutex_destroy(&mmutex);
	}
	//获取互斥锁
	bool lock()
	{
		return pthread_mutex_lock(&mmutex) == 0;
	}
	//释放互斥锁
	bool unlock()
	{
		return pthread_mutex_unlock(&mmutex) == 0;
	}
private:
	pthread_mutex_t mmutex;
};
#endif
