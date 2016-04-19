/*************************************************************************
	> File Name: MySQL.cpp
	> Author: Jone
	> Mail: mjf2019@126.com 
	> Created Time: 2015年11月06日 星期五 22时06分50秒
 ************************************************************************/
#include<mysql.h>
#include<iostream>
#include<string>
#include<stdio.h>
#include<vector>
using namespace std;

class Mysql
{
private:
    Mysql(string host="localhost",int port=3306,string name="root",string passwd="cherish",string table="chat")
    {
        mysql =new MYSQL;
		result = NULL;
        mysql_init(mysql);
        if(!mysql_real_connect(mysql,host.c_str(),name.c_str(),passwd.c_str(),table.c_str(),port,NULL,0))
        {
            cout<<"ral_connect fail"<<mysql_error(mysql)<<endl;
        }
        else
        {
            cout<<"connect with server succeed"<<endl;
        }
    
    }
public:
	static Mysql* GetInstance()
	{
		if(_Instance == NULL)
		{
			_Instance = new Mysql();
		}
		return _Instance;
	}
    bool QueryData(string str)
    {
        bool tag=true;
        if(mysql_real_query(mysql,str.c_str(),str.size()))
        {
            tag=false;
            cout<<"query fail"<<mysql_error(mysql)<<endl;
        }
        return tag;
    }

    bool DealResult()
    {
        int num=0;
        bool tag=true; 
        result=mysql_store_result(mysql);
        if(NULL==result)
        {
            int n=mysql_affected_rows(mysql);
            if(n>0)
            {
                cout<<" there are "<<n<<" row influenced"<<endl;
            }
            else
            {      
				cout<<"get result fail"<<mysql_error(mysql)<<endl;
				tag = false;
            }
        }
        else
        {
            num=mysql_num_fields(result);
			int m = mysql_num_rows(result);
			res.clear();
			if( m > 0 )
			{
				while(row=mysql_fetch_row(result))
				{
					vector<string> vec;
				     for(int i=0;i<num;++i)
			         {
						vec.push_back(row[i]);
					 }
					 res.push_back(vec);
					 vec.clear();
				}

            }
			else
			{
				tag = false;
				cout<<"return result failed"<<endl;
			}
		}
        return tag;
    }
    ~Mysql()
    {
        if(result!=NULL)
        {
            mysql_free_result(result);
        }
        mysql_close(mysql);
    }
	vector< vector<string> > GetResult()
	{
		return res;
	}
	void Show()
	{
		vector< vector<string> >::iterator it = res.begin();
		for(;it != res.end(); ++it)
		{
			vector<string>::iterator itr = (*it).begin();
			for(;itr != (*it).end();++itr)
			{
				printf("%s  ",(*itr).c_str());
			}
			printf("\n");
		}
	}
private:
	vector< vector<string> > res;
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_FIELD *fd;
    MYSQL_ROW  row;
	static Mysql* _Instance;
};
Mysql* Mysql::_Instance = NULL;


