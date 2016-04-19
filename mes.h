#include<string>
#include<iostream>
#include<sstream>
#include<string.h>
#include<stdlib.h>
#include <vector>
#include<algorithm>
using namespace std;
const int SIZE = 256;
const int MESSIZE = 128;
//登录用户--->好友  登录用户--->群组
//登录    注册    私人聊天消息    群消息    添加好友
typedef enum{LOGIN=1,REGISTER,PERSONALMES,GROUPCHAT,ADDPER,AGREEADDFRI,REFUSEADD}  MESTYPE;
struct SendMes
{
public:
	SendMes(){}
	SendMes(size_t id,const char *passwd)
	{
		sign = LOGIN;
		data.lmes._LoginId = id;
		strcpy(data.lmes._LoginPwd,passwd);
	}
	SendMes(const char* name,const char* passwd)
	{
		sign = REGISTER;
		strcpy(data.reg._name,name);
		strcpy(data.reg._RegPwd,passwd);
	}
	SendMes(MESTYPE type ,size_t My_Id,size_t Fri_id,const char *mes)
	{
		sign = type;
		data.chatmes.MyId = My_Id;
		data.chatmes.FriId = Fri_id;
		strcpy(data.chatmes.mes,mes);
	}
	SendMes(MESTYPE type,size_t My_id,size_t Related_id)
	{
		sign = type;
		data.add.MyId = My_id;
		data.add.Relatedid = Related_id;
	}
	size_t GetLoginId()const     {return data.lmes._LoginId;  }
	string  GetLoginPwd()const   {return data.lmes._LoginPwd; }
	string GetRegName()const     {return data.reg._name;      }
	string GetRegPwd()const      {return data.reg._RegPwd;    }
	size_t GetChatMesFriId()const{return data.chatmes.FriId;  }
	size_t GetChatMesMyId()const {return data.chatmes.MyId;   }
	string GetChatMes()const     {return data.chatmes.mes;    }
	size_t GetMyId()const        {return data.add.MyId;       }
	size_t GetRelatedId()const   {return data.add.Relatedid;  }
	~SendMes(){}
public:
	MESTYPE sign;
	union
	{
		struct LMES
		{
			size_t _LoginId;
			char   _LoginPwd[SIZE];
		}lmes;
		struct REG
		{
			char _name[SIZE];
			char _RegPwd[SIZE];
		}reg;
		struct ChatMes
		{
			size_t MyId;
			size_t FriId;
			char mes[SIZE];
		}chatmes;
		struct Add 
		{
			size_t MyId;
			size_t Relatedid;
		}add;
	}data;
};


typedef enum{FRI_TABLE=1,GROUP_TABLE,MES_PER,MES_GRP,REG_ID,ERROR,ADDFRIREQ,NON=80} RESULTTYPE;
struct Result
{
	static const int DATACOUNT=128;
	RESULTTYPE sign;
	int count;
	union
	{
		struct 
		{
			size_t id;
			char name[SIZE];
		}table;
		struct 
		{
			size_t id;
			char mes[SIZE];
			char type;
		}FriMes;
		struct 
		{
			size_t GrpId;
			size_t FriId;
			char mes[SIZE];
		}GrpMes;
		struct 
		{
			char buf[SIZE];
		}responseMes;
		struct 
		{
			size_t New_Id;
		}Id;
	}data[DATACOUNT];
};
void trans(Result &res,vector<vector<string> > &vec,RESULTTYPE type)
{
	vector< vector<string> >::iterator it = vec.begin();
	vector<string>::iterator  itr;
	int i = 0;
	switch(type)
	{
	case FRI_TABLE:
	case GROUP_TABLE:
		for(;it != vec.end();++it)
		{
			itr=(*it).begin();
			istringstream in1(*itr);
			in1>>res.data[i].table.id;
			strcpy(res.data[i].table.name,(*(itr+1)).c_str());
			++i;
		}
		break;
	case MES_PER:
		for(;it != vec.end();++it)
		{
			itr = (*it).begin();
			istringstream in2(*itr);
			in2>>res.data[i].FriMes.id;
			strcpy(res.data[i].FriMes.mes,(*(itr+1)).c_str());
			istringstream in5(*(itr+2));
			in5>>res.data[i].FriMes.type;
			++i;
		}
	case MES_GRP:
		for(;it != vec.end();++it)
		{
			itr = (*it).begin();
			istringstream in3(*itr);
			in3>>res.data[i].GrpMes.GrpId;
			istringstream in4(*(itr+1));
			in4>>res.data[i].GrpMes.FriId;
			strcpy(res.data[i].GrpMes.mes,(*(itr+2)).c_str());
			++i;
		}
		break;
	default:break;
	}
	res.count = i;
	res.sign = type;
}
