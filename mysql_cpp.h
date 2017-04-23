#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <map>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <mysql.h>
#include "threadlock.h"

using namespace std;

typedef struct
{
    char host[50];
    char user[50];
    char password[100];
    char database[20];
    int port;
}MySqlConnInfo;

class MysqlCPP
{
    private:
        MysqlCPP();
        ~MysqlCPP();
    public:
        static MysqlCPP* Instance()
        {
            CThreadMutex _lock;
            if(_instance == NULL){
                _lock.lock();
                if(_instance == NULL){
                    _instance = new MysqlCPP();
                }
                _lock.unlock();
            }
            return _instance;
        }
        class FreeInstance
        {
            public:
                ~FreeInstance()
                {
                    if(MysqlCPP::_instance){
                        delete MysqlCPP::_instance;
                    }
                }

        };
        static FreeInstance _free;
    private:
        static MysqlCPP* _instance;
        MYSQL _mysql;
        MySqlConnInfo m_ConnInfo;
        MYSQL_RES *m_Result;
        map<string,string> m_MapRes;
    public:
        void SetMySqlConnInfo( const char *server, const char *username, const char *password, const char *database,int port);
        //void SetMySqlConnInfo( char *server, char *username, char *password, char *database, int port);
        void Connect_Mysql();
        void Close_Mysql();
        void Mysql_HandleSQL(const char *query);
        void Mysql_SelectSQL(const char *query);
        void Mysql_InsertSQL(const char *query);
        void Mysql_UpdateSQL(const char *query);
        void Mysql_DeleteSQL(const char *query);
        void Mysql_ErrorInfo();
};
