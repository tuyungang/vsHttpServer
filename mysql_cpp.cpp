#include "mysql_cpp.h"

/*
MysqlCPP::MysqlCPP()
{
    if(mysql_init(mysql) == NULL){
        throw std::exception();
    }
}
*/
MysqlCPP::MysqlCPP(MYSQL *mSQL)
{
    mysql = mSQL;
}

/*
void MysqlCPP::Connect_Mysql()
{
    if(!mysql_real_connect(mysql,m_ConnInfo.host,m_ConnInfo.user,m_ConnInfo.password,m_ConnInfo.database,m_ConnInfo.port,NULL,CLIENT_FOUND_ROWS)){
        fprintf(stderr, "Failed to connect to database: Error: %s\n", mysql_error(mysql));
        throw std::exception();
    }
    cout << "connect mysql server sucessfully" << endl;
}

void MysqlCPP::Close_Mysql()
{
    mysql_close(mysql);
}
*/

void MysqlCPP::Mysql_HandleSQL(const char *query)
{
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    unsigned int num_fields;
    unsigned int num_rows;
    //int ret = mysql_real_query(mysql,query,strlen(query)); 
    int ret = mysql_query(mysql,query); 
    if(ret != 0){
        if(!mysql_ping(mysql)){
            if(!mysql_query(mysql,query)){
            }else
                throw std::exception();
        }else{
            /*
            try{
                Connect_Mysql();
            }catch(std::exception &e){
                cout << e.what() << endl;
            }
            throw std::exception();
            */
        }
    }

    do{
        m_Result = mysql_store_result(mysql);
        if(m_Result){
            num_fields = mysql_num_fields(m_Result);
            field = mysql_fetch_fields(m_Result);
            while((row = mysql_fetch_row(m_Result))){
                unsigned long *lengths = mysql_fetch_lengths(m_Result);
                for(int i=0; i< num_fields; i++){
                    m_MapRes.insert(map<string,string>::value_type(field[i].name,((row[i]==NULL) ? "NULL":row[i])));
                }
            }
            mysql_free_result(m_Result);

        }else{
            if(mysql_field_count(mysql) == 0){
                cout << "not select sql" << endl;
                num_rows = mysql_affected_rows(mysql);
            }else{
                fprintf(stderr, "Error: %s\n",mysql_error(mysql));
            }
        }
    }while(!mysql_next_result(mysql));

}

/*
void MysqlCPP::Mysql_SelectSQL(const char *query)
{
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    unsigned int num_fields;
    unsigned int num_rows;
    //int ret = mysql_real_query(mysql,query,strlen(query)); 
    int ret = mysql_query(mysql,query); 
    if(ret != 0){
        if(!mysql_ping(mysql)){
            if(!mysql_query(mysql,query)){
            }else
                throw std::exception();
        }else{
            try{
                Connect_Mysql();
            }catch(std::exception &e){
                cout << e.what() << endl;
            }
            throw std::exception();
        }
    }

    do{
        m_Result = mysql_store_result(mysql);
        if(m_Result){
            num_fields = mysql_num_fields(m_Result);
            field = mysql_fetch_fields(m_Result);
            while((row = mysql_fetch_row(m_Result))){
                unsigned long *lengths = mysql_fetch_lengths(m_Result);
                for(int i=0; i< num_fields; i++){
                    m_MapRes.insert(map<string,string>::value_type(field[i].name,((row[i]==NULL) ? "NULL":row[i])));
                    cout << field[i].name << ": " << row[i] << endl;
                }
            }
            mysql_free_result(m_Result);


        }else{
            if(mysql_field_count(mysql) == 0){
                cout << "not select sql" << endl;
                num_rows = mysql_affected_rows(mysql);
            }else{
                fprintf(stderr, "Error: %s\n",mysql_error(mysql));
            }
        }
    }while(!mysql_next_result(mysql));

}

void MysqlCPP::Mysql_InsertSQL(const char *query)
{
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    unsigned int num_fields;
    unsigned int num_rows;
    //int ret = mysql_real_query(mysql,query,strlen(query)); 
    int ret = mysql_query(mysql,query); 
    if(ret != 0){
        if(!mysql_ping(mysql)){
            if(!mysql_query(mysql,query)){
            }else
                throw std::exception();
        }else{
            throw std::exception();
        }
    }

    do{
        m_Result = mysql_store_result(mysql);
        if(m_Result){
            num_fields = mysql_num_fields(m_Result);
            field = mysql_fetch_fields(m_Result);
            while((row = mysql_fetch_row(m_Result))){
                unsigned long *lengths = mysql_fetch_lengths(m_Result);
                for(int i=0; i< num_fields; i++){
                    m_MapRes.insert(map<string,string>::value_type(field[i].name,((row[i]==NULL) ? "NULL":row[i])));
                }
            }
            mysql_free_result(m_Result);


        }else{
            if(mysql_field_count(mysql) == 0){
                cout << "insert sql" << endl;
                num_rows = mysql_affected_rows(mysql);
            }else{
                fprintf(stderr, "Error: %s\n",mysql_error(mysql));
            }
        }
    }while(!mysql_next_result(mysql));

}

void MysqlCPP::Mysql_UpdateSQL(const char *query)
{
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    unsigned int num_fields;
    unsigned int num_rows;
    //int ret = mysql_real_query(mysql,query,strlen(query)); 
    int ret = mysql_query(mysql,query); 
    if(ret != 0){
        if(!mysql_ping(mysql)){
            if(!mysql_query(mysql,query)){
            }else
                throw std::exception();
        }else{
            throw std::exception();
        }
    }

    do{
        m_Result = mysql_store_result(mysql);
        if(m_Result){
            num_fields = mysql_num_fields(m_Result);
            field = mysql_fetch_fields(m_Result);
            while((row = mysql_fetch_row(m_Result))){
                unsigned long *lengths = mysql_fetch_lengths(m_Result);
                for(int i=0; i< num_fields; i++){
                    m_MapRes.insert(map<string,string>::value_type(field[i].name,((row[i]==NULL) ? "NULL":row[i])));
                }
            }
            mysql_free_result(m_Result);


        }else{
            if(mysql_field_count(mysql) == 0){
                cout << "update sql" << endl;
                num_rows = mysql_affected_rows(mysql);
            }else{
                fprintf(stderr, "Error: %s\n",mysql_error(mysql));
            }
        }
    }while(!mysql_next_result(mysql));

}

void MysqlCPP::Mysql_DeleteSQL(const char *query)
{
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    unsigned int num_fields;
    unsigned int num_rows;
    //int ret = mysql_real_query(mysql,query,strlen(query)); 
    int ret = mysql_query(mysql,query); 
    if(ret != 0){
        if(!mysql_ping(mysql)){
            if(!mysql_query(mysql,query)){
            }else
                throw std::exception();
        }else{
            throw std::exception();
        }
    }

    do{
        m_Result = mysql_store_result(mysql);
        if(m_Result){
            num_fields = mysql_num_fields(m_Result);
            field = mysql_fetch_fields(m_Result);
            while((row = mysql_fetch_row(m_Result))){
                unsigned long *lengths = mysql_fetch_lengths(m_Result);
                for(int i=0; i< num_fields; i++){
                    m_MapRes.insert(map<string,string>::value_type(field[i].name,((row[i]==NULL) ? "NULL":row[i])));
                }
            }
            mysql_free_result(m_Result);


        }else{
            if(mysql_field_count(mysql) == 0){
                cout << "delete sql" << endl;
                num_rows = mysql_affected_rows(mysql);
            }else{
                fprintf(stderr, "Error: %s\n",mysql_error(mysql));
            }
        }
    }while(!mysql_next_result(mysql));

}
*/

/*
void MysqlCPP::SetMySqlConnInfo( const char *server, const char *username, const char *password, const char *database,int port)
{
    strcpy(m_ConnInfo.host, server);
    strcpy(m_ConnInfo.user, username);
    strcpy(m_ConnInfo.password, password);
    strcpy(m_ConnInfo.database, database);
    m_ConnInfo.port = port;
}
*/

void MysqlCPP::Mysql_ErrorInfo()
{

}
