#include "TcpEventServer.h"
#include "threadmanage.h"
#include "job.h"
#include "threadpool.h"

CThreadManage* TcpEventServer::main_manage = NULL;
CThreadManage* TcpEventServer::worker_manage = NULL;

TcpEventServer::TcpEventServer(int count)
{
  m_ThreadCount = count;
  m_Port = -1;
  idle_thread_count = 0;
  m_MainBase = new CMainThread();
  m_MainBase->tid = pthread_self();
  m_MainBase->base = event_base_new();

  SetupWorkerThread(10);
  SetupMainThread(); 
}

TcpEventServer::~TcpEventServer()
{
  StopRun(NULL);

  event_base_free(m_MainBase->base);
  for(int i=0; i<m_ThreadCount; i++)
    event_base_free(this->main_manage->GetThreadPool()->m_IdleList[i]->m_WorkerBase);

  delete m_MainBase;
  delete main_manage;
  delete worker_manage;
  //delete [] m_Threads;
}

void TcpEventServer::ErrorQuit(const char *str)
{
  fprintf(stderr, "%s", str);   
  if( errno != 0 )    
    fprintf(stderr, " : %s", strerror(errno));    
  fprintf(stderr, "\n");        
  exit(1);    
}

void TcpEventServer::SetupMainThread()
{
    try{
        main_manage = CThreadManage::init(m_ThreadCount, (void*)this);  
    }catch(CThreadManage::MyException &e){
        cout << e.what() << endl;
        exit(1);
    }
}

void TcpEventServer::SetupWorkerThread(int num)
{
    try{
        worker_manage = CThreadManage::init(0, (void*)this);
    }catch(CThreadManage::MyException &e){
        cout << e.what() << endl;
        exit(1);
    }
}

bool TcpEventServer::StartRun()
{

    evconnlistener *listener;

    if( m_Port != EXIT_CODE )
    {
        sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(m_Port);
        listener = evconnlistener_new_bind(m_MainBase->base, 
        ListenerEventCb, (void*)this,
        LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
        (sockaddr*)&sin, sizeof(sockaddr_in));
        if( NULL == listener )
            ErrorQuit("TCP listen error");
    }

    event_base_dispatch(m_MainBase->base);

    if( m_Port != EXIT_CODE )
    {
        evconnlistener_free(listener);
    }
}

void TcpEventServer::StopRun(timeval *tv)
{
    int contant = EXIT_CODE;
    for(int i=0; i<m_ThreadCount; i++){
        write(this->main_manage->GetThreadPool()->m_IdleList[i]->notifySendFd, &contant, sizeof(int));
    }
    event_base_loopexit(m_MainBase->base, tv);
}

void TcpEventServer::ListenerEventCb(struct evconnlistener *listener, 
                  evutil_socket_t fd,
                  struct sockaddr *sa, 
                  int socklen, 
                  void *user_data)
{
    TcpEventServer *server = (TcpEventServer*)user_data;

    //int num = rand() % server->m_ThreadCount;
    //int idle_thread_count = 0;
    
    int i = server->idle_thread_count;
    int idle_thread_number = server->main_manage->GetThreadPool()->m_IdleList.size();
    /*
    if((idle_thread_number == 0) || (idle_thread_number == 1)){
        server->main_manage->GetThreadPool()->CreateMainIdleThread(1);
    }
    idle_thread_number = server->main_manage->GetThreadPool()->m_IdleList.size();
    */
    do
    {
        if(idle_thread_number != 0){
            break;
        }
        i = (i+1)%idle_thread_number;
    }while( i != server->idle_thread_count );

    if( idle_thread_number == 0 ){

    }
    server->idle_thread_count = (i+1)%idle_thread_number;
    int sendfd = server->main_manage->GetThreadPool()->m_IdleList[i]->notifySendFd;
    write(sendfd, &fd, sizeof(evutil_socket_t));
}

void TcpEventServer::ThreadProcess(int fd, short which, void *arg)
{
    CWorkerThread *work = (CWorkerThread*)arg;

    cout<<"choose man thread id:"<< pthread_self() << endl;
    int pipefd = work->notifyReceiveFd;
    evutil_socket_t confd;
    read(pipefd, &confd, sizeof(evutil_socket_t));

    if( EXIT_CODE == confd )
    {
        event_base_loopbreak(work->m_WorkerBase);
        return;
    }

    //evutil_make_socket_nonblocking(fd);
    struct bufferevent *bev;
    bev = bufferevent_socket_new(work->m_WorkerBase, confd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev)
    {
        fprintf(stderr, "Error constructing bufferevent!");
        event_base_loopbreak(work->m_WorkerBase);
        return;
    }

    Conn *conn = work->connectQueue.InsertConn(confd, work, bev);

    bufferevent_setcb(bev, ReadEventCb, WriteEventCb, CloseEventCb, conn);
    bufferevent_enable(bev, EV_WRITE);
    bufferevent_enable(bev, EV_READ);
    conn->m_ReadBuf = bufferevent_get_input(bev);
    conn->m_WriteBuf = bufferevent_get_output(bev);
    //evbuffer_enable_locking(conn->m_WriteBuf, NULL);

}

void TcpEventServer::ReadEventCb(struct bufferevent *bev, void *data)
{
    Conn *conn = (Conn*)data;
    size_t m_ReadBuf_Len = evbuffer_get_length(conn->m_ReadBuf);
    conn->GetReadBuffer(conn->m_read_buf, evbuffer_get_length(conn->m_ReadBuf));
    cout <<"1 main thread id:"<<pthread_self()<<"recv m_read_buf:"<<conn->m_read_buf<<endl;
    /*
    evbuffer_lock(conn->m_WriteBuf);
    int n = evbuffer_reserve_space(conn->m_WriteBuf,2048,conn->m_iv,2);
    conn->m_iv[0].iov_base = temp;
    conn->m_iv[0].iov_len = strlen(temp);
    conn->m_iv[1].iov_base = temp;
    conn->m_iv[1].iov_len = strlen(temp);
    evbuffer_commit_space(conn->m_WriteBuf,conn->m_iv,2);
    evbuffer_unlock(conn->m_WriteBuf);
    evbuffer_write(conn->m_WriteBuf,conn->m_fd);
    */

    if(m_ReadBuf_Len != 0)
        worker_manage->Run(conn,NULL);
} 

void TcpEventServer::WriteEventCb(struct bufferevent *bev, void *data)
{

}

void TcpEventServer::CloseEventCb(struct bufferevent *bev, short events, void *data)
{
    Conn *conn = (Conn*)data;
    conn->m_ReadBuf = bufferevent_get_input(bev);
    int finished = 0;
    if(events & BEV_EVENT_EOF){
        printf("Connection closed.\n");
        size_t len = evbuffer_get_length(conn->m_ReadBuf);
        finished = 1;
    }else if(events & BEV_EVENT_ERROR){
        printf("Got an error:%s\n",evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        finished = 1;
    }else if(events & BEV_EVENT_TIMEOUT){
        finished = 3;
        printf("Got an timeout error\n");

    }
    if(finished == 1){
        //free(conn);
        bufferevent_free(bev);
        //conn->m_Thread->connectQueue.DeleteConn(conn);
        conn->m_Thread->GetThreadPool()->CheckIfInBusyList(conn->m_Thread,(void*)conn);
    }
}

bool TcpEventServer::AddSignalEvent(int sig, void (*ptr)(int, short, void*))
{
    event *ev = evsignal_new(m_MainBase->base, sig, ptr, (void*)this);
    if ( !ev || 
        event_add(ev, NULL) < 0 )
    {
        event_del(ev);
        return false;
    }

    DeleteSignalEvent(sig);
    m_SignalEvents[sig] = ev;

    return true;
}

bool TcpEventServer::DeleteSignalEvent(int sig)
{
    map<int, event*>::iterator iter = m_SignalEvents.find(sig);
    if( iter == m_SignalEvents.end() )
        return false;

    event_del(iter->second);
    m_SignalEvents.erase(iter);
    return true;
}

event *TcpEventServer::AddTimerEvent(void (*ptr)(int, short, void *), 
                  timeval tv, bool once)
{
    int flag = 0;
    if( !once )
        flag = EV_PERSIST;

    event *ev = new event;
    event_assign(ev, m_MainBase->base, -1, flag, ptr, (void*)this);
    if( event_add(ev, &tv) < 0 )
    {
        event_del(ev);
        return NULL;
    }
    return ev;
}

bool TcpEventServer::DeleteTImerEvent(event *ev)
{
    int res = event_del(ev);
    return (0 == res);
}
