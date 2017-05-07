#include "mysqlpool.h"
//#include "mysql_cpp.h"

CMysqlPool* CMysqlPool::pInstance = NULL;

CMysqlPool::CMysqlPool(int num)
{
    m_ConnNum = num;
    m_pool = new CManagePool(m_ConnNum);

}

CMysqlPool* CMysqlPool::Init(int num)
{
    CThreadMutex mutex;
    if(pInstance == NULL){
        mutex.lock();
        if(pInstance == NULL){
            pInstance = new CMysqlPool(num);
        }
        mutex.unlock();
    }
    return pInstance;
}

ConnNode* CMysqlPool::GetMysqlConn()
{
    ConnNode *node = m_pool->_GetMysqlConn();
    if(!node)
        return NULL;
    return node;
}

void CMysqlPool::ReleaseMysqlConn(void *ptr)
{
    ConnNode *_conn = (ConnNode*)ptr;
    m_pool->_ReleaseMysqlConn(_conn);
}

void CMysqlPool::FreeMysqlConnPool()
{
    m_pool->_FreeMysqlConnPool();
}

void CMysqlPool::SetSQLsock(MYSQL *mysql)
{
    m_SQL = new MysqlCPP(mysql);

}

void CMysqlPool::HandleSQL(const char *sql)
{
    m_SQL->Mysql_HandleSQL(sql);
}

CManagePool::CManagePool(int num)
{
    m_CurrentIdleConnNum = 0;
    m_IdleInitNum = num;
    assert(InitMysqlConnPool(m_IdleInitNum) == 0);
}

int CManagePool::InitMysqlConnPool(int num)
{
    assert(num > 0);
    for(int i=0; i<num; i++){
        ConnNode *_node = new ConnNode();
        if(NULL == mysql_init(&_node->mysql_init))
            return -1;
        char value = 1;
        mysql_options(&_node->mysql_init,MYSQL_OPT_RECONNECT,&value);
        if(CreateMysqlConn(_node,i))
            continue;
        else
            return -1;
    }
    return 0;
}

bool CManagePool::CreateMysqlConn(ConnNode *node,int n)
{
    assert(node != NULL);
    if((node->mysql_sock = mysql_real_connect(&(node->mysql_init),"localhost","root","123456","test",0,NULL,0)) == NULL){
        fprintf(stderr,"Connect Error: %s\n",mysql_error(&(node->mysql_init)));
        return false;
    }else{
        cout << "connect mysql sucessfully" << endl;
        node->used = n;
        node->state = CONNECTED;
        AppendToIdleList(node);
        return true;
    }
}

void CManagePool::AppendToIdleList(ConnNode *node)
{
    m_IdleListMutex.lock();
    MysqlConnIdleList.push_back(node);
    m_CurrentIdleConnNum++;
    m_IdleListMutex.unlock();
}

void CManagePool::MoveToBusyList(ConnNode *node)
{
    m_BusyListMutex.lock();
    MysqlConnBusyList.push_back(node);
    m_BusyListMutex.unlock();

    m_IdleListMutex.lock();
    vector<ConnNode*>::iterator pos;
    pos = find(MysqlConnIdleList.begin(),MysqlConnIdleList.end(),node);
    if(pos != MysqlConnIdleList.end())
        MysqlConnIdleList.erase(pos);
    m_CurrentIdleConnNum--;
    m_IdleListMutex.unlock();
}

void CManagePool::MoveToIdleList(ConnNode *node)
{
    m_IdleListMutex.lock();
    MysqlConnBusyList.push_back(node);
    m_CurrentIdleConnNum++;
    m_IdleListMutex.unlock();

    m_BusyListMutex.lock();
    vector<ConnNode*>::iterator pos;
    pos = find(MysqlConnBusyList.begin(),MysqlConnBusyList.end(),node);
    if(pos != MysqlConnBusyList.end())
        MysqlConnBusyList.erase(pos);
    m_BusyListMutex.unlock();
}

ConnNode* CManagePool::_GetMysqlConn()
{
    m_GetConnMutex.lock();
    ConnNode *conn = MysqlConnIdleList.front();
    if(conn == NULL)
        return NULL;
    if(!mysql_ping(conn->mysql_sock)){
        conn->used = 1;
        MoveToBusyList(conn);
    }else{
        conn->used = 0;
        conn->state = DISCONNECTED;
        return NULL;
    }
    m_GetConnMutex.unlock();
    return conn;
}

void CManagePool::_ReleaseMysqlConn(ConnNode *node)
{
    node->used = 0;
    MoveToIdleList(node);
}

void CManagePool::_FreeMysqlConnPool()
{
    m_IdleListMutex.lock();
    vector<ConnNode*>::iterator pos = MysqlConnIdleList.begin();
    while(pos != MysqlConnIdleList.end()){
        mysql_close(&((*pos++)->mysql_init));
        //pos++;
    }
    MysqlConnIdleList.clear();
    m_CurrentIdleConnNum = 0;
    m_IdleInitNum = 0;
    m_IdleListMutex.unlock();
}

