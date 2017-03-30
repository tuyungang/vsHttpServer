#ifndef _THREAD_MANAGE
#define _THREAD_MANAGE


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
#include <exception>

using namespace std;

class CThreadPool;
class CJob;

class CThreadManage
{
    public:
        enum {MAX_OBJECT = 2000};
    private:
        CThreadManage();
        //CThreadManage(int num);
        CThreadManage(int num,void *arg); 
        //~CThreadManage();
        //virtual ~CThreadManage();
        //CThreadManage(const CThreadManage &);  
        //CThreadManage & operator = (const CThreadManage &);
    private:
        CThreadPool *m_Pool;
        CThreadPool *m_WorkerPool;
        int m_NumOfThread;
        static CThreadManage *pInstance;
        static int m_numObjects;
    protected:
    public:
        ~CThreadManage(); 
        class MyException : public exception{
            public:
                MyException():exception(){}
                const char *what()const throw(){return "ERROR! over two objects is not allowed!";}
        };
        static CThreadManage *init(int thread_num, void *arg);
        void SetParallelNum(int num);
        //CThreadManage();
        //CThreadManage(int num);
        //virtual ~CThreadManage();
        void Run(CJob *job, void *jobdata);
        CThreadPool *GetThreadPool(void){return m_Pool;}
        CThreadPool *GetWorkerThreadPool(void){return m_WorkerPool;}
        void TerminateAll(void);
        class FreeInstance
        {
            public:
                ~FreeInstance(){
                    if(CThreadManage::pInstance){
                        delete CThreadManage::pInstance;
                    }
                }
        };
        static FreeInstance _instance;
};

/*
class CMainThreadManage : public CThreadManage
{
    friend class CThreadManage;
    private:
        CMainThreadManage(){}
    public:
    
};

class CWorkerThreadManage : public CThreadManage
{
    friend class CThreadManage;
    private:
        CWorkerThreadManage(){}
    public:

};
*/

#endif
