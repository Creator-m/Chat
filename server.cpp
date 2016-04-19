#include"mes.h"
#include"threadpool.h"
#include"Mysql.h"
#include<time.h>
#include<pthread.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<assert.h>
#include<arpa/inet.h>
#include<vector>
#include<stdlib.h>
#include<algorithm>
#include<string.h>
#include<string>
#include<stdio.h>
#include<sys/select.h>
using namespace std;

const int MAX_CLIENT = 1024;                       

class locker;
class User
{
	public:
		User(size_t id,struct sockaddr_in &addr):_UserId(id),maddr(addr){}
		User(const User &src)
		{
			_UserId = src._UserId;
			maddr = src.maddr;
		}
		User& operator=(const User &src)
		{
			if(this == &src)
			{
				return *this;
			}
			_UserId = src._UserId;
			maddr = src.maddr;
			return *this;
		}
		bool operator==(const User &src)
		{
			return _UserId == src._UserId;
		}
		struct sockaddr_in GetAddr()
		{
			return maddr;
		}
	private:
		size_t  _UserId;
		struct sockaddr_in maddr;
};

class Client
{
public:
	 Client(){}
	~Client(){}
	void ClientInit(const int sockfd,const SendMes mes,struct sockaddr_in caddr)
	{
		this->sockfd = sockfd;
		this->mes = mes;
		this->caddr = caddr;
	}
	void  process()
	{
		switch(mes.sign)
		{
			case LOGIN:
			  	cout<<"login------------------------------------"<<endl;	
				login();      
				break;
			case REGISTER:   
				cout<<"2come in process------------------------------------"<<endl;
				Register();     
				break;
			case PERSONALMES:  
			case GROUPCHAT:  
 				ChatHandle();   
				break;      
			case ADDPER:
				AddId();
			    break;
			case AGREEADDFRI:
				agree();
				break;
			case REFUSEADD:
				refuse();
				break;
			default:break;
		}
	}

	void agree()
	{

		size_t AddId = mes.GetChatMesFriId();
		size_t MyId= mes.GetChatMesMyId();
		ostringstream out;
		out<<MyId;
		string UserId=out.str();
		out<<AddId;
		string RelatedId=out.str();
		string str1 = "insert into FriTable values(";
		str1+=UserId;
		str1+=",";
		str1+=RelatedId;
		str1+=")";
		string str2 = "inset into FriTable values(";
		str2+=RelatedId;
		str2+=",";
		str2+=UserId;
		str2+=")";
		mysql->QueryData(str1);
		mysql->QueryData(str2);
	}

	void refuse()
	{
		size_t MyId = mes.GetChatMesMyId();
		size_t AddId = mes.GetChatMesFriId();
		ostringstream out;
		out<<MyId;
		string UserId=out.str();
		out<<AddId;
		string CId=out.str();
		struct sockaddr_in CliAddr;
		memset(&CliAddr,0,sizeof(CliAddr));
		ostringstream outstr;
		outstr<<AddId;
		string str=outstr.str();
		str+=" refuse to add you!";
	    User tmp(AddId,CliAddr);
		list<User>::iterator it = find(UserTable.begin(),UserTable.end(),tmp);
	    if(it != UserTable.end())
		{
			CliAddr = (*it).GetAddr();
			Result res;
			res.count = 1;
			res.sign = MES_PER;
			strcpy(res.data[0].FriMes.mes,str.c_str());
			res.data[0].FriMes.type='3';
			sendto(sockfd,&res,sizeof(res),0,(struct sockaddr *)&CliAddr,sizeof(CliAddr));
		}
		else
		{
			string str1="insert into FriMes values(";
			str1+=UserId;
			str1+=",";
			str1+=CId;
			str1+="\'";
			str1+=str;
			str1+="\',3)";
			mysql->QueryData(str1);
		}
	}
	bool login()
	{
		char buf[8]={0};
		size_t id = mes.GetLoginId();
		ostringstream outstr;
		outstr<<id;
		string idstr = outstr.str();
		string str = "select id from user where id=";
		str+=idstr;
		str+=" and  passwd= \'";
		str+=mes.GetLoginPwd();
		str+="\';";
		cout<<"str:"<<str<<endl;
		vector< vector<string> > result;
		mysql->QueryData(str);
		if(mysql->DealResult())
		{
			cout<<"login success---------------------"<<endl;
			vector<string> query;
			str.clear();
			str = "select id,name from user,FriTable where FriId=user.id and MainId=";
			str+=idstr;
			query.push_back(str);
			str.clear();
			str = "select id,name from User_Grp,GroupTable where GrpId=id and UserId=";
			str+=idstr;
			query.push_back(str);
			str.clear();
			str = "select SrcId,mes,type from FriMes where DesId=";
			str+=idstr;
			query.push_back(str);
			str.clear();
			str="select SrcGrpId,SrcUserId,mes from GrpMes where DesId= ";
			str+=idstr;
			query.push_back(str);
			str.clear();
			vector<string>::iterator it = query.begin();
			RESULTTYPE type[] = {FRI_TABLE,GROUP_TABLE,MES_PER,MES_GRP};
			int i = 0;
			while(it != query.end())
			{
				mysql->QueryData(*it);
				if(mysql->DealResult())
				{
					result = mysql->GetResult();
					Result mes;
					trans(mes,result,type[i]);
					cout<<"IP:"<<inet_ntoa(caddr.sin_addr)<<"port:"<<ntohl(caddr.sin_port)<<endl;
					sendto(sockfd,&mes,sizeof(mes),0,(struct sockaddr*)&caddr,sizeof(caddr));
					result.clear();
				}
				++i;
				++it;
			}
			User val(id,caddr);
			lk.lock();
			UserTable.push_back(val);
			lk.unlock();
			string str1="delete from FriMes where DesId=";
			str1+=idstr;
			string str2="delete from GrpMes where DesId=";
			str2+=idstr;
			mysql->QueryData(str1);
			mysql->QueryData(str2);
			return true;
		}
		else
		{
			Result mes;
			mes.count=1;
			mes.sign=ERROR;
			strcpy(mes.data[0].responseMes.buf,"your id or passwd error, login failed!");
		    sendto(sockfd,&mes,sizeof(mes),0,(struct sockaddr*)&caddr,sizeof(caddr));
			return false;
		}
	}
	bool Register()
	{
	    string str="insert into user(name,passwd) values(\'";
	 	str+=mes.GetRegName();
	 	str+="\',\'";
	 	str+=mes.GetRegPwd();
	 	str+="\');";
		cout<<"str:"<<str<<endl;
		vector< vector<string> > res;
		mysql->QueryData(str);
		mysql->QueryData("select max(id) from user;");
		if(mysql->DealResult())
		{
				cout<<"come in------------------------------"<<endl;
				res=mysql->GetResult();
				Result mes;
				mes.count=1;
				mes.sign=REG_ID;
				istringstream  instr(((res.front()).front()));
				instr>>mes.data[0].Id.New_Id;
				sendto(sockfd,&mes,sizeof(mes),0,(struct sockaddr*)&caddr,sizeof(caddr));	
				cout<<"---------------------------"<<endl;
				return true;
		}
		else
		{
		    Result mes;
			mes.count = 1;
			mes.sign = ERROR;
			strcpy(mes.data[0].responseMes.buf,"register failed,please register again!");
			sendto(sockfd,&mes,sizeof(mes),0,(struct sockaddr*)&caddr,sizeof(caddr));
			return false;
		}
	}
void ChatHandle()
{
		size_t MyId = mes.GetChatMesMyId();
		size_t FriId = mes.GetChatMesFriId();
		string Mes = mes.GetChatMes();
		ostringstream outstr;
		outstr<<FriId;
		string desid=outstr.str();
		outstr<<MyId;
		string srcid=outstr.str();
		struct sockaddr_in CliAddr;
		memset(&CliAddr,0,sizeof(CliAddr));
	switch(mes.sign)
	{
		case PERSONALMES:
		if(1)
		{

	    	 User tmp(FriId,CliAddr);
			 list<User>::iterator it = find(UserTable.begin(),UserTable.end(),tmp);
			if(it != UserTable.end())
			{
				CliAddr = (*it).GetAddr();
				Result res1;
				res1.count=1;
				res1.sign=MES_PER;
				res1.data[0].FriMes.id=MyId;
				strcpy(res1.data[0].FriMes.mes,Mes.c_str());
				res1.data[0].FriMes.type=0;
				sendto(sockfd,&res1,sizeof(res1),0,(struct sockaddr*)&CliAddr,sizeof(CliAddr));
			}
			else
			{
			    string str = "insert into FriMes values(";
				str+=srcid;
				str+=",";
				str+=desid;
				str+=",\'";
				str+=Mes;
				str+="\',";
				str+="0)";
				mysql->QueryData(str);
			}
		}
		break;
		case GROUPCHAT:
			size_t IdArray[128]={0};
			int n = 0;
			string str = "select UserId from User_Grp where GrpId=";
			str+=desid;
			mysql->QueryData(str);
			if(mysql->DealResult())
			{
				vector< vector<string> > StrTab = mysql->GetResult();
				vector< vector<string> >::iterator it = StrTab.begin();
				for(;it != StrTab.end(); ++it)
				{
					istringstream instr((*it).front());
					instr>>IdArray[n];
					++n;
				}
				for(int i = 0;i<n;++i)
				{
					User u(IdArray[i],CliAddr);
					list<User>::iterator itr = find(UserTable.begin(),UserTable.end(),u);
					if(itr == UserTable.end())
					{
						
			             string str = "insert into GrpMes  values(";
						 outstr<<IdArray[i];
						 string FId = outstr.str();
				         str+=FId;
			        	 str+=",";
				         str+=desid;
						 str+=",";
						 str+=srcid;
						 str+=",\'";
						 str+=Mes;
				         str+="\')";
			        	mysql->QueryData(str);
					}
					else
					{
						CliAddr = (*itr).GetAddr();
				        Result res2;
						res2.count = 1;
						res2.sign=MES_GRP;
						res2.data[0].GrpMes.GrpId=FriId;
						res2.data[0].GrpMes.FriId=MyId;
						strcpy(res2.data[0].GrpMes.mes,Mes.c_str());
				        sendto(sockfd,&res2,sizeof(res2),0,(struct sockaddr*)&CliAddr,sizeof(CliAddr));
					}
				}
			}
			break;
    	}
  }
	void AddId()
	{
		size_t MyId = mes.GetMyId();
		size_t AddId = mes.GetRelatedId();
		ostringstream out;
		out<<AddId;
		string RelatedId=out.str();
	    string str = "select id from user where id=";
		str+=RelatedId;
		mysql->QueryData(str);
	    if(mysql->DealResult())
		{
			 struct sockaddr_in CliAddr;
			 memset(&CliAddr,0,sizeof(CliAddr));
	    	 User tmp(AddId,CliAddr);
			 list<User>::iterator it = find(UserTable.begin(),UserTable.end(),tmp);
			if(it != UserTable.end())
			{
				Result r1;
				r1.count=1;
				r1.sign=ADDFRIREQ;
				r1.data[0].FriMes.id=MyId;
				strcpy(r1.data[0].FriMes.mes,"");
				r1.data[0].FriMes.type='1';
				sendto(sockfd,&r1,sizeof(r1),0,(struct sockaddr*)&caddr,sizeof(caddr));
			}
		}
		else
		{
			Result r2;
			r2.count=1;
			r2.sign=ERROR;
			strcpy(r2.data[0].responseMes.buf,"The id is not exist!");
			sendto(sockfd,&r2,sizeof(r2),0,(struct sockaddr*)&caddr,sizeof(caddr));
		} 
	}
private:
	int sockfd; //服务器的sockfd套接字描述符
	SendMes mes;
	struct sockaddr_in caddr;
	static list<User>  UserTable;         
	static Mysql *mysql;
	static locker lk;
};

list<User>  Client::UserTable;
Mysql*     Client::mysql = Mysql::GetInstance();
locker Client::lk;

int main(int argc,char **argv)
{
	if(argc < 3)
	{
		exit(0);
	}
	const char *ip =argv[1];
	int port = atoi(argv[2]);
	threadpool<Client> *pool = new threadpool<Client>;
	Client *cli = new Client[MAX_CLIENT];
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	int sockHd = socket(AF_INET,SOCK_DGRAM,0);
	assert(sockfd != -1);
	assert(sockHd != -1);
	struct sockaddr_in saddr,caddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(ip);
	int res = bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	assert(res != -1);
	saddr.sin_port=htons(port+1);
	res = bind(sockHd,(struct sockaddr*)&saddr,sizeof(saddr));
	assert(res != -1);
	fd_set read_fds;
	FD_ZERO(&read_fds);
	int ret = 0;
	int count = 0;
	SendMes mes;
	while(1)
	{
		FD_SET(sockfd,&read_fds);
		FD_SET(sockHd,&read_fds);
		ret = select(sockfd+1,&read_fds,NULL,NULL,NULL);
		if(ret < 0)
		{
			cout<<"selection failure !"<<endl;
			break;
		}
		else if(ret == 0)
		{
			continue;	
		}
		else
		{
			if(FD_ISSET(sockfd,&read_fds))
			{
				socklen_t  len = sizeof(caddr);
				ret = recvfrom(sockfd,&mes,sizeof(mes),0,(struct sockaddr*)&caddr,&len);
				if(ret == -1)
				{
					continue;
				}
				if(count>=MAX_CLIENT)
				{
					count = 0;
				}
				cli[count].ClientInit(sockfd,mes,caddr);
				pool->append(cli+count);
				++count;
			}
			else if(FD_ISSET(sockHd,&read_fds))
			{
				continue;
			}
		}

	}
	close(sockfd);
	delete []cli;
	delete pool;
	return 0;
}

