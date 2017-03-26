#include "threadpool.h"
#include "job.h"

CThread::CThread()
{
    m_ErrCode = 0;
    m_ThreadID = new pthread_t;
    m_Detach = NULL;
    m_CreateSuspended = NULL;
    m_ThreadName = NULL;
    m_ThreadState = THREAD_NONE;
}

CThread::~CThread()
{
    delete m_ThreadID;
}

void * CThread::ThreadFunction(void *mData)
{
    CWorkerThread *worker = (CWorkerThread *)mData;
    //cout << "child thread " << *(worker->GetThreadID()) << " is created" << endl;
    worker->Run();
}

bool CThread::Start(void)
{
    int rt = pthread_create(m_ThreadID,NULL,ThreadFunction,this);
    if(rt != 0){
        delete m_ThreadID;
        throw std::exception();
    }
    
    cout << "child thread " << *m_ThreadID << " is created" << endl;
    return true;
}

bool CThread::Join(void)
{
    int rt = pthread_cancel(*m_ThreadID);
    if(rt != 0){
        throw std::exception();
    }
    cout << "child thread " << *m_ThreadID << " stop" << endl;

    return true;
}

CThreadPool::CThreadPool() 
{ 
    m_MaxNum = 50; 
    m_AvailLow = 5; 
    m_InitNum=m_AvailNum = 10 ;  
    m_AvailHigh = 20; 

    m_BusyList.clear(); 
    m_IdleList.clear(); 
    for(int i=0;i<m_InitNum;i++){ 
        try{
            CWorkerThread* thr = new CWorkerThread(); 
            thr->SetThreadPool(this); 
            AppendToIdleList(thr); 
            thr->Start(); 
        }catch(...){
            //return 1;
        }
    } 
} 

CThreadPool::CThreadPool(int initnum) 
{ 
    assert(initnum>0 && initnum<=30); 
    m_MaxNum   = 30; 
    m_AvailLow = initnum-10>0?initnum-10:3; 
    m_InitNum=m_AvailNum = initnum ;  
    m_AvailHigh = initnum+10; 

    m_BusyList.clear(); 
    m_IdleList.clear(); 
    for(int i=0;i<m_InitNum;i++){ 
        try{
            CWorkerThread* thr = new CWorkerThread(); 
            AppendToIdleList(thr); 
            thr->SetThreadPool(this); 
            thr->Start();        
        }catch(...){

        }
    } 
} 

CThreadPool::~CThreadPool() 
{ 
   TerminateAll(); 
} 

void CThreadPool::TerminateAll() 
{ 
    for(int i=0;i < m_ThreadList.size();i++) { 
        CWorkerThread* thr = m_ThreadList[i]; 
        try{
            thr->Join(); 
        }catch(std::exception &e){
            cout << "delete thread error: " <<  e.what() << endl;;
        }
    } 
} 
 
CWorkerThread* CThreadPool::GetIdleThread(void) 
{ 
    while(m_IdleList.size() ==0 ) 
        m_IdleCond.wait(); 

    m_IdleMutex.lock(); 
    if(m_IdleList.size() > 0 ) 
    { 
        CWorkerThread* thr = (CWorkerThread*)m_IdleList.front(); 
        printf("Get Idle thread %ld\n",*(thr->GetThreadID())); 
        cout << "Get Idle thread " << *(thr->GetThreadID()) << endl;

        m_IdleMutex.unlock(); 
        return thr; 
    } 
    m_IdleMutex.unlock(); 

    return NULL; 
} 

void CThreadPool::AppendToIdleList(CWorkerThread* jobthread) 
{ 
    m_IdleMutex.lock(); 
    m_IdleList.push_back(jobthread); 
    m_ThreadList.push_back(jobthread); 
    m_IdleMutex.unlock(); 
} 

void CThreadPool::MoveToBusyList(CWorkerThread* idlethread) 
{ 
    m_BusyMutex.lock(); 
    m_BusyList.push_back(idlethread); 
    m_AvailNum--; 
    m_BusyMutex.unlock(); 

    m_IdleMutex.lock(); 
    vector<CWorkerThread*>::iterator pos; 
    /*
    for(pos=m_BusyList.begin();pos!=m_BusyList.end();pos++){
            if(*pos == *idlethread)
                m_BusyList.erase(pos);
    }
    */
    pos = find(m_IdleList.begin(),m_IdleList.end(),idlethread); 
    if(pos !=m_IdleList.end()) 
        m_IdleList.erase(pos); 
    m_IdleMutex.unlock(); 
} 

void CThreadPool::MoveToIdleList(CWorkerThread* busythread) 
{ 
    m_IdleMutex.lock(); 
    m_IdleList.push_back(busythread); 
    m_AvailNum++; 
    m_IdleMutex.unlock(); 

    m_BusyMutex.lock(); 
    vector<CWorkerThread*>::iterator pos; 
    pos = find(m_BusyList.begin(),m_BusyList.end(),busythread); 
    if(pos!=m_BusyList.end()) 
        m_BusyList.erase(pos); 
    m_BusyMutex.unlock(); 

    m_IdleCond.signal(); 
    m_MaxNumCond.signal(); 
} 

void CThreadPool::CreateIdleThread(int num) 
{ 
    for(int i=0;i<num;i++){ 
        try{
            CWorkerThread* thr = new CWorkerThread(); 
            thr->SetThreadPool(this); 
            AppendToIdleList(thr); 
            m_VarMutex.lock(); 
            m_AvailNum++; 
            m_VarMutex.unlock(); 
            thr->Start();        
        }catch(...){

        }
    } 
} 

void CThreadPool::DeleteIdleThread(int num) 
{ 
    printf("Enter into CThreadPool::DeleteIdleThreadn\n"); 
    m_IdleMutex.lock(); 
    printf("Delete Num is %d\n",num); 
    for(int i=0;i<num;i++){ 
        CWorkerThread* thr; 
        if(m_IdleList.size() > 0 ){ 
            thr = (CWorkerThread*)m_IdleList.front(); 
            printf("Get Idle thread %ld\n",*(thr->GetThreadID())); 
        } 

        vector<CWorkerThread*>::iterator pos; 
        pos = find(m_IdleList.begin(),m_IdleList.end(),thr); 
        if(pos!=m_IdleList.end()) 
            m_IdleList.erase(pos); 
        pos = find(m_ThreadList.begin(),m_ThreadList.end(),thr);
        if(pos != m_ThreadList.end()){
            m_ThreadList.erase(pos);
        }
        //thr->Join();
        m_AvailNum--; 
        printf("The idle thread available num:%d \n",m_AvailNum); 
        printf("The idlelist num:%d \n",m_IdleList.size()); 
    } 
    m_IdleMutex.unlock(); 
} 

void CThreadPool::Run(CJob* job,void* jobdata) 
{ 
    assert(job != NULL); 

    if(GetBusyNum() == m_MaxNum) 
        m_MaxNumCond.wait(); 

    if(m_IdleList.size()<m_AvailLow) 
    { 
        if(GetAllNum()+m_InitNum-m_IdleList.size() < m_MaxNum ) 
        CreateIdleThread(m_InitNum-m_IdleList.size()); 
    else 
        CreateIdleThread(m_MaxNum-GetAllNum()); 
    } 

    CWorkerThread* idlethr = GetIdleThread(); 
    if(idlethr !=NULL) 
    { 
        idlethr->m_WorkMutex.lock(); 
        MoveToBusyList(idlethr); 
        idlethr->SetThreadPool(this); 
        job->SetWorkThread(idlethr); 
        printf("Job is set to thread %ld \n",*(idlethr->GetThreadID())); 
        idlethr->SetJob(job,jobdata); 
        idlethr->m_WorkMutex.unlock(); 
    } 
}

CWorkerThread::CWorkerThread():CThread() 
{ 
    m_Job = NULL; 
    m_JobData = NULL; 
    m_ThreadPool = NULL; 
    m_IsEnd = false; 
} 
CWorkerThread::~CWorkerThread() 
{ 
    if(NULL != m_Job) 
        delete m_Job; 
    if(m_ThreadPool != NULL) 
        delete m_ThreadPool; 
} 

void CWorkerThread::Run() 
{ 
    SetThreadState(THREAD_RUNNING); 
    for(;;) 
    { 
        while(m_Job == NULL) 
            m_JobCond.wait(); 

        m_WorkMutex.lock(); 
        m_Job->Run(m_JobData); 
        m_Job->SetWorkThread(NULL); 
        m_Job = NULL; 
        m_JobData = NULL;
        m_ThreadPool->MoveToIdleList(this); 
        if(m_ThreadPool->m_IdleList.size() > m_ThreadPool->GetAvailHighNum()) { 
            m_ThreadPool->DeleteIdleThread(m_ThreadPool->m_IdleList.size()-m_ThreadPool->GetInitNum()); 
        } 
        m_WorkMutex.unlock(); 
    } 
} 

void CWorkerThread::SetJob(CJob* job,void* jobdata) 
{ 
    m_VarMutex.lock(); 
    m_Job = job; 
    m_JobData = jobdata; 
    job->SetWorkThread(this); 
    m_VarMutex.unlock(); 
    m_JobCond.signal(); 
} 
void CWorkerThread::SetThreadPool(CThreadPool* thrpool) 
{ 
    m_VarMutex.lock(); 
    m_ThreadPool = thrpool; 
    m_VarMutex.unlock(); 
}
