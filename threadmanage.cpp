#include "threadmanage.h"
#include "threadlock.h"

CThreadManage* CThreadManage::pInstance = NULL;

CThreadManage::CThreadManage(){ 
    m_NumOfThread = 10; 
    m_Pool = new CThreadPool(m_NumOfThread); 
} 

CThreadManage::CThreadManage(int num){ 
    m_NumOfThread = num; 
    m_Pool = new CThreadPool(m_NumOfThread); 
} 

CThreadManage::~CThreadManage(){ 
     if(NULL != m_Pool) 
     delete m_Pool; 
 } 

CThreadManage* CThreadManage::init(int thread_num)
{
    CThreadMutex m_lock;
    if(pInstance == NULL){
        m_lock.lock();
        if(pInstance == NULL){
            if(thread_num == 0){
                pInstance = new CThreadManage();
            }else {
                pInstance = new CThreadManage(thread_num);
            }
        }
        m_lock.unlock();
    }
    return pInstance;
}

void CThreadManage::SetParallelNum(int num)
{ 
    m_NumOfThread = num; 
} 

void CThreadManage::Run(CJob* job,void* jobdata)
{ 
    m_Pool->Run(job,jobdata); 
} 

void CThreadManage::TerminateAll(void)
{ 
    m_Pool->TerminateAll(); 
}

