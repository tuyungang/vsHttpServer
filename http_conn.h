#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "job.h"
#include "TcpEventServer.h"

class CWorkerThread;

class Conn : public CJob
{
    friend class ConnQueue;
    friend class TcpEventServer;

    private:
        const int m_fd;				
        evbuffer *m_ReadBuf;		
        evbuffer *m_WriteBuf;		
        struct evbuffer_iovec m_iv[2];

        Conn *m_Prev;				
        Conn *m_Next;				
        CWorkerThread *m_Thread;
        //CWorkerThread *m_WorkerThread;
        
        struct bufferevent *m_Bev;

        char m_Read_Buf[2048];

        Conn(int fd=0);
        ~Conn();

    public:
        CWorkerThread *GetThread() { return m_Thread; }
struct evbuffer_iovec *GetConnIovec(){return m_iv;}
        struct evbuffer *GetConnWriteEvbuffer(){return m_WriteBuf;}
        struct bufferevent *GetConnBufferevent(){ return m_Bev; }
        void Run(void* jobData){ this->process(); }
        int GetFd() { return m_fd; }

        int GetReadBufferLen()
        { return evbuffer_get_length(m_ReadBuf); }

        int GetReadBuffer(char *buffer, int len)
        { 
            m_read_idx+= len;
            return evbuffer_remove(m_ReadBuf, buffer, len); 
        }

        int CopyReadBuffer(char *buffer, int len)
        { return evbuffer_copyout(m_ReadBuf, buffer, len); }

        int GetWriteBufferLen()
        { return evbuffer_get_length(m_WriteBuf); }

        int AddToWriteBuffer(char *buffer, int len)
        { return evbuffer_add(m_WriteBuf, buffer, len); }

        void MoveBufferData()
        { evbuffer_add_buffer(m_WriteBuf, m_ReadBuf); }

};

class ConnQueue
{
    public:
        enum {MAX_OPEN_FD = 2};
        int m_CurrentConnNum;
    private:
        Conn *m_head;
        Conn *m_tail;
    public:
        ConnQueue();
        ~ConnQueue();
        //Conn *InsertConn(int fd, LibeventThread *t);
        Conn *InsertConn(int fd, CWorkerThread *t, struct bufferevent *bev);
        void DeleteConn(Conn *c);
        //void PrintQueue();
};


/*
class http_conn : public CJob
{
public:
    http_conn(){}
    ~http_conn(){}
    void Run(void* jobData);  //add by tu

};
*/

#endif
