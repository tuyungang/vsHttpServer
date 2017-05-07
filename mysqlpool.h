#ifndef _MYSQL_POOL_
#define _MYSQL_POOL_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exception>
#include <assert.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <mysql.h>
#include "threadlock.h"
#include "mysql_cpp.h"

using namespace std;

typedef enum 
{ 
    CONNECTED=0,
    DISCONNECTED 

}ConnNodeState;

class ConnNode
{
    public:
        ConnNode(){}
        ~ConnNode(){}
    public:
        MYSQL mysql_init;
        MYSQL *mysql_sock;
        MYSQL_RES *_res;
        ConnNodeState state;
        int used; 
};

class CManagePool
{
    public:
        CManagePool(int num);
        ~CManagePool(){}
    private:
        vector<ConnNode*> MysqlConnIdleList;
        vector<ConnNode*> MysqlConnBusyList;
    public:
        int InitMysqlConnPool(int num);
        bool CreateMysqlConn(ConnNode *node,int n);
        ConnNode *_GetMysqlConn();
        void _ReleaseMysqlConn(ConnNode *node);
        void _FreeMysqlConnPool();
        void AppendToIdleList(ConnNode *node);
        void MoveToBusyList(ConnNode *node);
        void MoveToIdleList(ConnNode *node);
    private:
        int m_IdleInitNum;
        int m_CurrentIdleConnNum; 
    public:
        CThreadMutex m_BusyListMutex;
        CThreadMutex m_IdleListMutex;
        CThreadMutex m_GetConnMutex;
};

//class MysqlCPP;

class CMysqlPool
{
    private:
        CMysqlPool(int num);
    public:
        static CMysqlPool* Init(int num);
        class FreeInstance
        {
            public:
                ~FreeInstance(){
                    if(CMysqlPool::pInstance != NULL)
                        delete CMysqlPool::pInstance;
                }
        };
        static FreeInstance _free;
    private:
        static CMysqlPool* pInstance;
        CManagePool *m_pool;
        int m_ConnNum;
        MysqlCPP *m_SQL;
    public:
        ~CMysqlPool(){ 
            delete m_pool; 
            delete m_SQL;
        }
        ConnNode *GetMysqlConn();
        CManagePool *GetManagePool(){ return m_pool; }
        void ReleaseMysqlConn(void *ptr);
        void FreeMysqlConnPool();
        int GetMysqlConnNum(){ return m_ConnNum; }
        void SetSQLsock(MYSQL *mysql);
        void HandleSQL(const char *sql);
};

#endif
