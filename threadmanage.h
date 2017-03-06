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
#include "job.h"
#include "threadpool.h"

using namespace std;

class CThreadManage
{
    private:
        CThreadManage();
        CThreadManage(int num);
        ~CThreadManage();
        //virtual ~CThreadManage();
        //CThreadManage(const CThreadManage &);  
        //CThreadManage & operator = (const CThreadManage &);
    private:
        CThreadPool *m_Pool;
        int m_NumOfThread;
        static CThreadManage *pInstance;
    protected:
    public:
        static CThreadManage *init(int thread_num);
        void SetParallelNum(int num);
        //CThreadManage();
        //CThreadManage(int num);
        //virtual ~CThreadManage();
        void Run(CJob *job, void *jobdata);
        void TerminateAll(void);
        class freeInstance
        {
            public:
                ~freeInstance(){
                    if(CThreadManage::pInstance){
                        delete CThreadManage::pInstance;
                    }
                }
        };
        static freeInstance _instance;
};

#endif
