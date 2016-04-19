#include "mes.h"
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <cstdlib>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

const int _NameLength = 12;
const int _PwdLength = 6;
const int _IdLength = 6;

static size_t MyId = 0;

int sockfd = socket(AF_INET,SOCK_DGRAM,0);

struct sockaddr_in saddr;
void init(int arg,char **argv)
{
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	//cout<<atoi(argv[1])<<"  "<<argv[2]<<endl;
	saddr.sin_port = htons(atoi(argv[1]));
	saddr.sin_addr.s_addr = inet_addr(argv[2]);
}

enum _ENUM
{
	QUIT=3,
	HELP=0,
	BACK=9
};

enum _LNUM
{
	AddFriend=1,
	AddGroup,
	DelFriend,
	DelGroup,
	SearchId,
	ChatWith,
	SearchName,
	ModifyName
};

class CSend_msg
{
public:
	CSend_msg(struct SendMes mes)
	{
		msg = mes;
		pthread_create(&pth_send,NULL,Sen,(void*)this);
	}
	static void *Sen(void *arg)
	{
		CSend_msg *p = (CSend_msg*)arg;
		p->send_mess();
		return p;
	}
	void send_mess()
	{		
		sendto(sockfd,&msg,sizeof(SendMes),0,(struct sockaddr*)&saddr,sizeof(saddr));
	}
	~CSend_msg()
	{	
		pthread_exit(NULL);
	}
private:
	pthread_t pth_send;
	SendMes msg;
};

Result result[100];

class CRecv_msg
{
public:	
	CRecv_msg()
	{
		pthread_create(&pth_rec,NULL,Rec,(void*)this);
	}
	static void *Rec(void *arg)
	{
		CRecv_msg *p = (CRecv_msg*)arg;
		p->recv_mess();
		return p;
	}

	void recv_mess()
	{	
		cout<<"ready recv...."<<endl;
		int len = sizeof(saddr);
		int n = 0;
		int i = 0;
		while(1)
		{	
			for(;i<100;++i)
			{
				n = recvfrom(sockfd,&result[i],sizeof(result[i]),0,(sockaddr*)&saddr,(socklen_t*)&len);
				if(n > 0)
				{
					cout<<"start recv"<<endl;
					continue;
				}
				else
				{
					break;
				}

				if( i == 99)
				{
					i = 0;
				}
			}
		}
	}
	~CRecv_msg(){}
private:
	pthread_t pth_rec;
};

class CHeart
{
public:
	CHeart()
	{
		pthread_create(&pth,NULL,RUN,(void*)this);
	}
	static void *RUN(void *arg)
	{
		CHeart *p = (CHeart*)arg;
		p->Heart();
		return p;
	}

	void Heart()
	{
		sleep(3);
		//SendMes he(HEART);
		//CSend_msg msga(he);
	}
	~CHeart(){}
private:
	pthread_t pth;
};

class Test
{
	public:
		Test(){}
		~Test(){}
		
	public:
		void run()
		{
			//system("clear");
			_run();
		}
	private:
		void _run();
		void _Login();
		void _LoginSuccess();
		void _Register();
		void _Quit();
		void _Help();

		void _Show(struct Result&);
		void _ShowFriend(struct Result&);
		void _ShowGroup(struct Result&);
		void _ShowPerMes(struct Result&);
		void _ShowGpMes(struct Result&);

		void _AddFriend();
		void _AddGroup();
		void _DelFriend();
		void _DelGroup();
		void _ChatWith();

		void _Analy(struct Result&);
};	

void Test::_Analy(struct Result& res)
{
	int i = 0;
	while(res.count--)
	{
		switch(res.sign)
		{
			case FRI_TABLE:
				cout<<"friend id:"<<res.data[i].table.id<<" name:"<<res.data[i].table.name<<endl;
				break;
			case GROUP_TABLE:
				cout<<"group id:"<<res.data[i].table.id<<"name:"<<res.data[i].table.name<<endl;
				break;
			case MES_PER://0 frimes,1 addfri, 2 argeefri 3 disargeefri
				if(res.data[i].FriMes.type == '0')
				{
					cout<<"your firend "<<res.data[i].FriMes.id<<" send you mes,the mes is:"<<res.data[i].FriMes.mes<<endl;
					break;
				}
				else if(res.data[i].FriMes.type == '1')
				{
					char buff[5] = {0};
					cout<<"the id "<<res.data[i].FriMes.id<<" want to add you"<<res.data[i].FriMes.mes<<endl;
					cout<<"yes/no?"<<endl;
					cin>>buff;
					if(strncmp(buff,"yes",3) == 0)
					{
						SendMes remes(AGREEADDFRI,MyId,res.data[i].FriMes.id,"");
						CSend_msg ms(remes);
					}
					else if(strncmp(buff,"NO",2)==0)
					{
						SendMes remes(REFUSEADD,MyId,res.data[i].FriMes.id,"");
						CSend_msg ms(remes);
					}
					else
					{
						cout<<"input erreo"<<endl;		
						cout<<"the id "<<res.data[i].FriMes.id<<" want to add you"<<res.data[i].FriMes.mes<<endl;
						cout<<"yes/no?"<<endl;
						cin>>buff;
						if(strncmp(buff,"yes",3) == 0)
						{
							SendMes remes(AGREEADDFRI,MyId,res.data[i].FriMes.id,"");
							CSend_msg ms(remes);
						}
						else if(strncmp(buff,"NO",2) ==0)
						{
							SendMes remes(REFUSEADD,MyId,res.data[i].FriMes.id,"");
							CSend_msg ms(remes);
						}
					}
				}
				else if(res.data[i].FriMes.type == '2') 
					cout<<"argee add"<<endl;
				else if(res.data[i].FriMes.type == '3')
					cout<<"disargee add"<<endl;
				break;
			case MES_GRP:
				cout<<"group "<<res.data[i].GrpMes.GrpId<<" have meses"<<endl;
				cout<<"id:"<<res.data[i].GrpMes.FriId<<" say :"<<res.data[i].GrpMes.mes<<endl;
				break;
			default:
				break;
		}		
		++i;
	}
}

void Test::_ShowFriend(struct Result &res)
{
	cout<<"this is Friend List"<<endl;
	_Analy(res);
	res.sign = NON;
    _LoginSuccess();
}

void Test::_ShowGroup(struct Result& res)
{
	cout<<"this is Group"<<endl;
	_Analy(res);
	res.sign = NON;
    _LoginSuccess();
}

void Test::_ShowPerMes(struct Result &res)
{
	cout<<"this is personal mes"<<endl;
	_Analy(res);
	res.sign = NON;
    _LoginSuccess();
}

void Test::_ShowGpMes(struct Result &res)
{
	cout<<"this is Group Mes"<<endl;
	_Analy(res);
	res.sign = NON;
    _LoginSuccess();
}

void Test::_Show(struct Result &res)
{
	cout<<"This is show"<<endl;
	int sign = res.sign;
	if(sign == FRI_TABLE)
	{	
		_ShowFriend(res);
	}
	else if(sign == GROUP_TABLE)
	{
		_ShowGroup(res);
	}
	else if(sign == MES_PER)
	{
		_ShowPerMes(res);
	}
	else if(sign == MES_GRP)
	{
		_ShowGpMes(res);
	}
}

void Test::_AddFriend()
{
	cout<<"Please input Id you want add"<<endl;
	size_t Id = 0;
	cin>>Id;

	SendMes ames(ADDPER,MyId,Id);
	CSend_msg smsg_1(ames);	
	
	_LoginSuccess();
}

void Test::_AddGroup()
{
	cout<<"Please input Group Id you want add"<<endl;
	size_t Id = 0;
	cin>>Id;

//	SendMes ames(ADDGRP,MyId,Id);	
//	CSend_msg smsg_2(ames);

	_LoginSuccess();
}

void Test::_DelFriend()
{
	cout<<"Please input Id you want del"<<endl;
	size_t Id = 0;
	cin>>Id;

//	SendMes ames(DELPER,MyId,Id);	
//	CSend_msg smsg_3(ames);

	_LoginSuccess();
}

void Test::_DelGroup()
{
	cout<<"Please input Group Id you want del"<<endl;
	size_t Id = 0;
	cin>>Id;

//	SendMes ames(DELGRP,MyId,Id);	
//	CSend_msg smsg_4(ames);

	_LoginSuccess();
}

void Test::_ChatWith()
{
	char buff[1024] = {0};
	size_t Id = 0;
	cout<<"Please input Id want Chat:";	
	cin>>Id;
	cout<<"Please input content:(less than 1024 character)";
	cin>>buff;

	SendMes cmsg(PERSONALMES,MyId,Id,buff);
	CSend_msg smsg_5(cmsg);

	_LoginSuccess();
}

void Test::_LoginSuccess()
{
	//CHeart pth_1;
	
//	static int tt = 0;
//	if(tt<5)
//	{
		int k = 0;
	//	++tt;
		for(;k<10;k++)
		{
			if(result[k].count>0)
			_Show(result[k]);
		}
//	}
	cout<<"1.addFriend"<<endl;
	cout<<"2.addGroup"<<endl;
	cout<<"2.delFriend"<<endl;
	cout<<"3.SearchID"<<endl;
	cout<<"4.SearchName"<<endl;
	cout<<"5.ModifyName"<<endl;
	cout<<"6.chat with"<<endl;
	cout<<"9.QuitLogin"<<endl;
	

	int rev =0;
	cin>>rev;
	
	switch(rev)
	{
		case AddFriend:
			_AddFriend();
			break;
		case AddGroup:
			_AddGroup();
			break;
		case DelFriend:
			_DelFriend();
			break;
		case DelGroup:
			_DelGroup();
			break;
		case SearchId:
		break;
		case SearchName:
			break;
		case ModifyName:
			break;
		case ChatWith:
			_ChatWith();
			break;	
		case BACK:
			run();
			break;
        defalut:
			break;
	}
}

void Test::_Login()
{	
	size_t Id;
	char *Pwd = new char[_PwdLength];
	cout<<"1.UserID:";
	cin>>Id;
	cout<<endl;
	cout<<"2.PassWD:";
	cin>>Pwd;

	SendMes lmsg(Id,Pwd);
	MyId = Id;
	
	CSend_msg msgg(lmsg);
	cout<<"Please wait....."<<endl;
	sleep(3);
	int k = 0;
	for(;k<100;++k)
	{
	//	if(result[k].sign == LOGINOK)
	if(1)
		{
		//	result[k].sign = NON;
			cout<<"Login success"<<endl;
			_LoginSuccess();
		}
		else if(result[k].sign == ERROR)
		{
			result[k].sign = NON;
			cout<<"Login Failure!"<<endl;
			cout<<"User OR Passwd Error!"<<endl;
			run();	
		}
	}
}

void Test::_Register()
{	
	char *Name = new char[_NameLength];
	char *Pwd = new char[_PwdLength];
	cout<<"1.UserName:";;
	cin>>Name;	
	if(strlen(Name) > _NameLength)
	{
		cout<<"Name too lang"<<endl;
		_Register();
	}
	cout<<endl;
	cout<<"2.PassWD:";
	cin>>Pwd;
	if(strlen(Pwd) > _PwdLength)
	{
		cout<<"Pwd too lang"<<endl;
		_Register();
	}
	
	SendMes msg(Name,Pwd);
	CSend_msg msg_6(msg);
	cout<<"Please wait....."<<endl;
	sleep(10);
		
	int k=0;
	for(;k<100;++k)
	{	
		if(result[k].sign == REG_ID)
		{
			result[k].sign=NON;
			cout<<"this is your Id"<<result[k].data[k].Id.New_Id<<endl;
			run();
		}
		else
		{
			cout<<"register failure"<<endl;
			run();
		}
	}

}

void Test::_Quit()
{
	system("clear");
	cout<<"hope you use again"<<endl;
	cout<<"Bye Bye"<<endl;
	exit(0);
}

void Test::_Help()
{
	system("clear");
	int fd = open("./Help",O_RDONLY);
	assert(fd != -1);

	char buff[1024] = {0};
	int n=0;
	while((n=read(fd,buff,1023))>0)
	{
		cout<<buff<<endl;
	}

	char tmp =0;
	cin>>tmp;
	system("clear");
	run();
}

void Test::_run()
{
	cout<<"This is Test!"<<endl;
	cout<<"1.Login"<<endl;
	cout<<"2.Register"<<endl;
	cout<<"3.Quit"<<endl;
	cout<<"0.Help"<<endl;
	
	int rev = 0;
	cin>>rev;
	switch(rev)
	{
		case LOGIN:
			system("clear");
			cout<<"LOGIN"<<endl;
			_Login();
			break;
		case REGISTER:
			system("clear");
			cout<<"REGISTER"<<endl;
			_Register();
			break;
		case QUIT:
			_Quit();
			break;
		case HELP:
			_Help();
			break;
		default:
			break;
	}
}

int main(int argc,char **argv)
{
	init(argc,argv);
	CRecv_msg mmm;

	Test t1;
	t1.run();

	close(sockfd);
	return 0;
}
