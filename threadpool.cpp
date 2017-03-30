#include "threadpool.h"
#include "job.h"
#include "TcpEventServer.h"
#include "http_conn.h"

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

void* CThread::ThreadFunction(void *mData)
{
    CWorkerThread *worker = (CWorkerThread *)mData;
    event_base_dispatch(worker->m_WorkerBase);
}

void* CThread::WorkerThreadFunction(void *mData)
{
    CWorkerThread *worker = (CWorkerThread *)mData;
    worker->Run();

}

bool CThread::StartWorkerThread(void)
{
    int rt = pthread_create(m_ThreadID, NULL, WorkerThreadFunction, this);
    if(rt != 0){
        delete m_ThreadID;
        throw std::exception();
    }
    
    return true;

}

bool CThread::Start(void)
{
    int rt = pthread_create(m_ThreadID,NULL,ThreadFunction,this);
    if(rt != 0){
        delete m_ThreadID;
        throw std::exception();
    }
    
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
    m_MaxNum = 20; 
    m_AvailLow = 5; 
    m_InitNum=m_AvailNum = 10 ;  
    m_AvailHigh = 15; 

    m_BusyList.clear(); 
    m_IdleList.clear(); 
    for(int i=0;i<m_InitNum;i++){ 
        try{
            CWorkerThread* thr = new CWorkerThread(0,0); 
            thr->SetWorkerThreadPool(this); 
            AppendToIdleList(thr); 
            thr->StartWorkerThread();
        }catch(...){
            //return 1;
        }
    } 
} 

/*
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
*/

CThreadPool::CThreadPool(int initnum,void *arg) 
{
    m_MainServer = (TcpEventServer *)arg;

    assert(initnum>0 && initnum<=30); 
    m_MaxNum   = 30; 
    m_AvailLow = initnum-10>0?initnum-10:3; 
    m_InitNum=m_AvailNum = initnum ;  
    m_AvailHigh = initnum+10; 

    m_BusyList.clear(); 
    m_IdleList.clear(); 
    for(int i=0;i<m_InitNum;i++){ 
        try{
            CWorkerThread* thr = new CWorkerThread((void*)this); 
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
        cout << "Get worker Idle thread " << *(thr->GetThreadID()) << endl;

        m_IdleMutex.unlock(); 
        return thr; 
    } 
    m_IdleMutex.unlock(); 

    return NULL; 
} 

void CThreadPool::CheckIfInBusyList(CWorkerThread* busythread,void *connthread)
{
    Conn *conn = (Conn*)connthread;
    m_BusyMutex.lock(); 
    vector<CWorkerThread*>::iterator pos; 
    pos = find(m_BusyList.begin(),m_BusyList.end(),busythread); 
    if(pos!=m_BusyList.end()){ 
        m_BusyMutex.unlock(); 
        MoveToIdleList(busythread);
    }
    m_BusyMutex.unlock(); 
    busythread->connectQueue.DeleteConn(conn);
        //m_BusyList.erase(pos); 
    //m_BusyMutex.unlock(); 
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

void CThreadPool::CreateMainIdleThread(int num)
{
    for(int i=0;i<num;i++){ 
        try{
            CWorkerThread* thr = new CWorkerThread((void*)this); 
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

void CThreadPool::CreateIdleThread(int num) 
{ 
    for(int i=0;i<num;i++){ 
        try{
            CWorkerThread* thr = new CWorkerThread(0,0); 
            thr->SetWorkerThreadPool(this); 
            AppendToIdleList(thr); 
            m_VarMutex.lock(); 
            m_AvailNum++; 
            m_VarMutex.unlock(); 
            thr->StartWorkerThread();
        }catch(...){

        }
    } 
} 

void CThreadPool::DeleteIdleThread(int num) 
{ 
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
        //idlethr->SetThreadPool(this); 
        idlethr->SetWorkerThreadPool(this); 
        job->SetWorkThread(idlethr); 
        cout << "Job is set to worker thread" << *(idlethr->GetThreadID()) <<endl;
        idlethr->SetJob(job,jobdata); 
        idlethr->m_WorkMutex.unlock(); 
    } 
}

CWorkerThread::CWorkerThread(void *arg):CThread(),connectQueue() 
{ 
    m_Job = NULL; 
    m_JobData = NULL; 
    m_ThreadPool = (CThreadPool*)arg; 
    m_WorkerThreadPool = NULL;
    m_IsEnd = false; 
    m_WorkerBase = event_base_new();
    if(NULL == m_WorkerBase){
        m_ThreadPool->m_MainServer->ErrorQuit("event base new error");
    }
    int fds[2];
    if(pipe(fds))
        m_ThreadPool->m_MainServer->ErrorQuit("create pipe error");
    notifyReceiveFd = fds[0];
    notifySendFd = fds[1];
    event_set(&m_WorkerEvent,notifyReceiveFd,
        EV_READ | EV_PERSIST, m_ThreadPool->m_MainServer->ThreadProcess, (void*)this);
    event_base_set(m_WorkerBase,&m_WorkerEvent);
    if ( event_add(&m_WorkerEvent, 0) == -1 )
        m_ThreadPool->m_MainServer->ErrorQuit("Can't monitor libevent notify pipe\n");

} 

CWorkerThread::CWorkerThread(int type,int data):CThread(),connectQueue()
{
    m_Job = NULL; 
    m_JobData = NULL; 
    m_ThreadPool = NULL; 
    m_WorkerThreadPool = NULL;
    m_IsEnd = false; 
}

CWorkerThread::~CWorkerThread() 
{ 
    if(NULL != m_Job) 
        delete m_Job; 
    if(m_ThreadPool != NULL) 
        delete m_ThreadPool; 
    if(m_WorkerThreadPool != NULL)
        delete m_WorkerThreadPool;
} 

void CWorkerThread::Run() 
{ 
    //SetThreadState(THREAD_BLOCKED); 
    for(;;) 
    { 
        while(m_Job == NULL){ 
            SetThreadState(THREAD_BLOCKED); 
            m_JobCond.wait(); 
        }
        SetThreadState(THREAD_RUNNING);
        m_WorkMutex.lock(); 
        m_Job->Run(m_JobData); 
        m_Job->SetWorkThread(NULL); 
        m_Job = NULL; 
        m_JobData = NULL;
        //m_WorkMutex.lock(); 
        m_WorkerThreadPool->MoveToIdleList(this);
        if(m_WorkerThreadPool->m_IdleList.size() > m_WorkerThreadPool->GetAvailHighNum()) { 
            m_WorkerThreadPool->DeleteIdleThread(m_WorkerThreadPool->m_IdleList.size()-m_WorkerThreadPool->GetInitNum()); 
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
void CWorkerThread::SetThreadPool(CThreadPool *thrpool) 
{ 
    m_VarMutex.lock(); 
    m_ThreadPool = thrpool; 
    m_VarMutex.unlock(); 
}

void CWorkerThread::SetWorkerThreadPool(CThreadPool *wthrpool)
{
    m_VarMutex.lock();
    m_WorkerThreadPool = wthrpool;
    m_VarMutex.unlock();
}
