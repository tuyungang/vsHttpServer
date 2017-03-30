#include "http_conn.h"
#include "threadpool.h"

Conn::Conn(int fd) : CJob(),m_fd(fd)
{
  m_Prev = NULL;
  m_Next = NULL;
}

Conn::~Conn()
{

}

ConnQueue::ConnQueue()
{
  m_head = new Conn(0);
  m_tail = new Conn(0);
  m_head->m_Prev = m_tail->m_Next = NULL;
  m_head->m_Next = m_tail;
  m_tail->m_Prev = m_head;
  m_CurrentConnNum = 0;
}

ConnQueue::~ConnQueue()
{
  Conn *tcur, *tnext;
  tcur = m_head;
  while( tcur != NULL )
  {
    tnext = tcur->m_Next;
    delete tcur;
    tcur = tnext;
  }
}

Conn *ConnQueue::InsertConn(int fd, CWorkerThread *t, struct bufferevent *bev)
{
    Conn *c;
    if(m_CurrentConnNum < MAX_OPEN_FD){
        c = new Conn(fd);
        c->m_Thread = t;
        c->m_Bev = bev;
        Conn *next = m_head->m_Next;

        c->m_Prev = m_head;
        c->m_Next = m_head->m_Next;
        m_head->m_Next = c;
        next->m_Prev = c;
        m_CurrentConnNum = m_CurrentConnNum + 1;
        cout << "socket fd "<<fd<<"count queue:"<<m_CurrentConnNum<<endl;
    }else{
        return NULL;
    }
    if(MAX_OPEN_FD < m_CurrentConnNum + 1){
        t->GetThreadPool()->MoveToBusyList(t);
    }
    if((c->m_Thread->GetThreadPool()->m_IdleList.size() == 1) && (c->m_Thread->GetThreadPool()->m_IdleList[0]->connectQueue.m_CurrentConnNum == MAX_OPEN_FD-1))
        c->m_Thread->GetThreadPool()->CreateMainIdleThread(1);

    return c;
}

void ConnQueue::DeleteConn(Conn *c)
{
    cout << "ConnQueue::DeleteConn" << endl;
    c->m_Prev->m_Next = c->m_Next;
    c->m_Next->m_Prev = c->m_Prev;
    m_CurrentConnNum = m_CurrentConnNum - 1;
    delete c;
}

/*
void ConnQueue::PrintQueue()
{
  Conn *cur = m_head->m_Next;
  while( cur->m_Next != NULL )
  {
    printf("%d ", cur->m_fd);
    cur = cur->m_Next;
  }
  printf("\n");
}
*/

/*
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file from this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";
//const char* doc_root = "/var/www/html";
//const char* doc_root = "/mnt/hgfs/linux_study/index.html";
const char* doc_root = "/mnt/hgfs/linux_study";

int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}
*/

/*
void http_conn::Run(void* jobData)
{
    this->process();
}
*/
