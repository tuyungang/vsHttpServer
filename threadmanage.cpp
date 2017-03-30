#include "threadmanage.h"
#include "threadlock.h"
#include "job.h"
#include "threadpool.h"

CThreadManage* CThreadManage::pInstance = NULL;
int CThreadManage::m_numObjects = 0;

CThreadManage::CThreadManage(){ 
    ++m_numObjects;
    m_NumOfThread = 10; 
    m_WorkerPool = new CThreadPool(); 
   // m_WorkerPool = new CThreadPool(m_NumOfThread); 
} 

/*
CThreadManage::CThreadManage(int num){ 
    ++m_numObjects;
    m_NumOfThread = num; 
    m_Pool = new CThreadPool(m_NumOfThread); 
} 
*/

CThreadManage::CThreadManage(int num, void *arg)
{
    ++m_numObjects;
    m_NumOfThread = num;
    m_Pool = new CThreadPool(m_NumOfThread,arg);
}

CThreadManage::~CThreadManage(){ 
     if((NULL != m_Pool) &&(NULL != m_WorkerPool)){ 
        delete m_Pool; 
        delete m_WorkerPool;
     }
     --m_numObjects; 
 } 

CThreadManage* CThreadManage::init(int thread_num, void *arg)
{
    CThreadMutex m_lock;
    if(m_numObjects < MAX_OBJECT){
        m_lock.lock();
        if(m_numObjects < MAX_OBJECT){
            if(thread_num == 0){
                pInstance = new CThreadManage();
            }else {
                pInstance = new CThreadManage(thread_num, arg);
            }
        }
        m_lock.unlock();
    }else{
        throw MyException();
    }
    return pInstance;
}

void CThreadManage::SetParallelNum(int num)
{ 
    m_NumOfThread = num; 
} 

void CThreadManage::Run(CJob* job,void* jobdata)
{ 
    m_WorkerPool->Run(job,jobdata); 
} 

void CThreadManage::TerminateAll(void)
{ 
    m_Pool->TerminateAll(); 
}

