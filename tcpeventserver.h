//TcpEventServer.h
#ifndef TCPEVENTSERVER_H_
#define TCPEVENTSERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>

#include <map>
using std::map;

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

class Conn;
class CThreadManage;
class CJob;
class CMainThread;
class CWorkerThread;

class TcpEventServer
{
    friend class CWorkerThread;
    private:
        int m_ThreadCount;	
        int m_Port;			
        CMainThread *m_MainBase;  
        map<int, event*> m_SignalEvents;
        int idle_thread_count;

    public:
        static CThreadManage *main_manage;  
        static CThreadManage *worker_manage;

    public:
        static const int EXIT_CODE = -1;
        //static int idle_thread_count;

    private:
        void SetupMainThread();
        void SetupWorkerThread(int num);

        static void ThreadProcess(int fd, short which, void *arg);
        static void ListenerEventCb(evconnlistener *listener, evutil_socket_t fd,
            sockaddr *sa, int socklen, void *user_data);
        static void ReadEventCb(struct bufferevent *bev, void *data);
        static void WriteEventCb(struct bufferevent *bev, void *data); 
        static void CloseEventCb(struct bufferevent *bev, short events, void *data);

    protected:

        virtual void ConnectionEvent(Conn *conn) { }
        virtual void ReadEvent(Conn *conn) { }
        virtual void WriteEvent(Conn *conn) { }
        virtual void CloseEvent(Conn *conn, short events) { }
        virtual void ErrorQuit(const char *str);

    public:
        TcpEventServer(int count);
        ~TcpEventServer();

        void SetPort(int port)
        { m_Port = port; }

        bool StartRun();
        void StopRun(timeval *tv);

        bool AddSignalEvent(int sig, void (*ptr)(int, short, void*));
        bool DeleteSignalEvent(int sig);

        event *AddTimerEvent(void(*ptr)(int, short, void*),
            timeval tv, bool once);
        bool DeleteTImerEvent(event *ev);
};

#endif
