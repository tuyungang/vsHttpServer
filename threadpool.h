#ifndef _THREAD_POOL
#define _THREAD_POOL

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <cassert>
#include <sys/epoll.h>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include "threadlock.h"

using namespace std;

typedef enum {
    THREAD_NONE = 0,
    THREAD_READY = 1,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_TERMINATED
}ThreadState;

class CThread
{
    private:
        int m_ErrCode;
        CSemaphore m_ThreadSemaphore;
        //unsigned long m_ThreadID;
        pthread_t *m_ThreadID;
        bool m_Detach;
        bool m_CreateSuspended;
        char *m_ThreadName;
        ThreadState m_ThreadState;
    protected:
        void SetErrcode(int errcode){m_ErrCode = errcode;}
        static void *ThreadFunction(void*);
    public:
        CThread();
        CThread(bool createsuspended, bool detach);
        virtual ~CThread();
        virtual void Run(void)=0;
        void SetThreadState(ThreadState state){m_ThreadState = state;}
        bool Terminate(void);
        bool Start(void);
        //void Exit(void);
        //bool Wakeup(void);
        ThreadState GetThreadState(void){return m_ThreadState;}
        //int GetLastError(void){return m_ErrCode;}
        void SetThreadName(char *thrname){strcpy(m_ThreadName, thrname);}
        char *GetThreadName(void){return m_ThreadName;}
        //int GetThreadID(void){return m_ThreadID;}
        pthread_t* GetThreadID(void){return m_ThreadID;}
        //bool SetPriorit(int priority);
        //int GetPriority(void);
        //int GetConcurrency(void);
        //void SetConcurrency(int num);
        bool Detach(void);
        //bool Join(void){pthread_join(*m_ThreadID,NULL);}
        bool Join(void);
        //bool Yield(void);
        //int Self(void);
};

class CJob;
class CThreadPool;

class CWorkerThread:public CThread 
{ 
    private: 
        CThreadPool*  m_ThreadPool; 
        CJob*    m_Job; 
        void*    m_JobData; 

        CThreadMutex m_VarMutex; 
        bool      m_IsEnd; 
    protected: 
    public: 
        CCondition   m_JobCond; 
        CThreadMutex m_WorkMutex; 
    public:
        CWorkerThread(); 
        virtual ~CWorkerThread(); 
    public:
        void Run(); 
        void SetJob(CJob* job,void* jobdata); 
        CJob *GetJob(void){return m_Job;} 
        void SetThreadPool(CThreadPool* thrpool); 
        CThreadPool* GetThreadPool(void){return m_ThreadPool;} 
};


class CThreadPool 
{ 
    friend class CWorkerThread; 
    private: 
        unsigned int m_MaxNum;   
        unsigned int m_AvailLow;  
        unsigned int m_AvailHigh;    
        unsigned int m_AvailNum;  
        unsigned int m_InitNum;  
    protected: 
        CWorkerThread* GetIdleThread(void); 

        void    AppendToIdleList(CWorkerThread* jobthread); 
        void    MoveToBusyList(CWorkerThread* idlethread); 
        void    MoveToIdleList(CWorkerThread* busythread); 

        void    DeleteIdleThread(int num); 
        void    CreateIdleThread(int num); 
    public: 
        CThreadMutex m_BusyMutex;     
        CThreadMutex m_IdleMutex;     
        CThreadMutex m_JobMutex;  
        CThreadMutex m_VarMutex; 

        CCondition       m_BusyCond;  
        CCondition       m_IdleCond;  
        CCondition       m_IdleJobCond;   
        CCondition       m_MaxNumCond; 

        vector<CWorkerThread*>   m_ThreadList; 
        vector<CWorkerThread*>   m_BusyList;      
        vector<CWorkerThread*>   m_IdleList;  
        vector<CJob*> m_JobList;

        CThreadPool(); 
        CThreadPool(int initnum); 
        virtual ~CThreadPool(); 

        void    SetMaxNum(int maxnum){m_MaxNum = maxnum;} 
        int     GetMaxNum(void){return m_MaxNum;} 
        void    SetAvailLowNum(int minnum){m_AvailLow = minnum;} 
        int     GetAvailLowNum(void){return m_AvailLow;} 
        void    SetAvailHighNum(int highnum){m_AvailHigh = highnum;} 
        int     GetAvailHighNum(void){return m_AvailHigh;} 
        int     GetActualAvailNum(void){return m_AvailNum;} 
        int     GetAllNum(void){return m_ThreadList.size();} 
        int     GetBusyNum(void){return m_BusyList.size();} 
        void    SetInitNum(int initnum){m_InitNum = initnum;} 
        int     GetInitNum(void){return m_InitNum;} 

        void    TerminateAll(void); 
        void    Run(CJob* job,void* jobdata); 
};

#endif
