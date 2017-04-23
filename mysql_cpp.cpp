#include "mysql_cpp.h"

MysqlCPP* MysqlCPP::_instance = NULL;


MysqlCPP::MysqlCPP()
{
    if(mysql_init(&_mysql) == NULL){
        throw std::exception();
    }
    //mysql_options();

}

MysqlCPP::~MysqlCPP()
{

}

void MysqlCPP::Connect_Mysql()
{
    //if(!mysql_real_connect(&_mysql,"localhost","root","123456","test",0,NULL,CLIENT_FOUND_ROWS)){
    if(!mysql_real_connect(&_mysql,m_ConnInfo.host,m_ConnInfo.user,m_ConnInfo.password,m_ConnInfo.database,m_ConnInfo.port,NULL,CLIENT_FOUND_ROWS)){
        fprintf(stderr, "Failed to connect to database: Error: %s\n", mysql_error(&_mysql));
        throw std::exception();
    }
    cout << "connect mysql server sucessfully" << endl;

}

void MysqlCPP::Close_Mysql()
{
    mysql_close(&_mysql);

}

void MysqlCPP::Mysql_HandleSQL(const char *query)
{
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    unsigned int num_fields;
    unsigned int num_rows;
    //int ret = mysql_real_query(&_mysql,query,strlen(query)); 
    int ret = mysql_query(&_mysql,query); 
    if(ret != 0){
        if(!mysql_ping(&_mysql)){
            if(!mysql_query(&_mysql,query)){
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
        m_Result = mysql_store_result(&_mysql);
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
            if(mysql_field_count(&_mysql) == 0){
                cout << "not select sql" << endl;
                num_rows = mysql_affected_rows(&_mysql);
            }else{
                fprintf(stderr, "Error: %s\n",mysql_error(&_mysql));
            }
        }
    }while(!mysql_next_result(&_mysql));

}

void MysqlCPP::Mysql_SelectSQL(const char *query)
{
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    unsigned int num_fields;
    unsigned int num_rows;
    //int ret = mysql_real_query(&_mysql,query,strlen(query)); 
    int ret = mysql_query(&_mysql,query); 
    if(ret != 0){
        if(!mysql_ping(&_mysql)){
            if(!mysql_query(&_mysql,query)){
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
        m_Result = mysql_store_result(&_mysql);
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
            if(mysql_field_count(&_mysql) == 0){
                cout << "not select sql" << endl;
                num_rows = mysql_affected_rows(&_mysql);
            }else{
                fprintf(stderr, "Error: %s\n",mysql_error(&_mysql));
            }
        }
    }while(!mysql_next_result(&_mysql));

}

void MysqlCPP::Mysql_InsertSQL(const char *query)
{
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    unsigned int num_fields;
    unsigned int num_rows;
    //int ret = mysql_real_query(&_mysql,query,strlen(query)); 
    int ret = mysql_query(&_mysql,query); 
    if(ret != 0){
        if(!mysql_ping(&_mysql)){
            if(!mysql_query(&_mysql,query)){
            }else
                throw std::exception();
        }else{
            throw std::exception();
        }
    }

    do{
        m_Result = mysql_store_result(&_mysql);
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
            if(mysql_field_count(&_mysql) == 0){
                cout << "insert sql" << endl;
                num_rows = mysql_affected_rows(&_mysql);
            }else{
                fprintf(stderr, "Error: %s\n",mysql_error(&_mysql));
            }
        }
    }while(!mysql_next_result(&_mysql));

}

void MysqlCPP::Mysql_UpdateSQL(const char *query)
{
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    unsigned int num_fields;
    unsigned int num_rows;
    //int ret = mysql_real_query(&_mysql,query,strlen(query)); 
    int ret = mysql_query(&_mysql,query); 
    if(ret != 0){
        if(!mysql_ping(&_mysql)){
            if(!mysql_query(&_mysql,query)){
            }else
                throw std::exception();
        }else{
            throw std::exception();
        }
    }

    do{
        m_Result = mysql_store_result(&_mysql);
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
            if(mysql_field_count(&_mysql) == 0){
                cout << "update sql" << endl;
                num_rows = mysql_affected_rows(&_mysql);
            }else{
                fprintf(stderr, "Error: %s\n",mysql_error(&_mysql));
            }
        }
    }while(!mysql_next_result(&_mysql));

}

void MysqlCPP::Mysql_DeleteSQL(const char *query)
{
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    unsigned int num_fields;
    unsigned int num_rows;
    //int ret = mysql_real_query(&_mysql,query,strlen(query)); 
    int ret = mysql_query(&_mysql,query); 
    if(ret != 0){
        if(!mysql_ping(&_mysql)){
            if(!mysql_query(&_mysql,query)){
            }else
                throw std::exception();
        }else{
            throw std::exception();
        }
    }

    do{
        m_Result = mysql_store_result(&_mysql);
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
            if(mysql_field_count(&_mysql) == 0){
                cout << "delete sql" << endl;
                num_rows = mysql_affected_rows(&_mysql);
            }else{
                fprintf(stderr, "Error: %s\n",mysql_error(&_mysql));
            }
        }
    }while(!mysql_next_result(&_mysql));

}

void MysqlCPP::SetMySqlConnInfo( const char *server, const char *username, const char *password, const char *database,int port)
{
    strcpy(m_ConnInfo.host, server);
    strcpy(m_ConnInfo.user, username);
    strcpy(m_ConnInfo.password, password);
    strcpy(m_ConnInfo.database, database);
    m_ConnInfo.port = port;
}

void MysqlCPP::Mysql_ErrorInfo()
{

}
