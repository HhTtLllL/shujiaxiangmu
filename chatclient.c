#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"my_recv.h"
#include<pthread.h>
#include<stdlib.h>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<signal.h>

#define MSGSIZE 512
#define BUFFSIZE 1024
#define MAXSIZE 1024
#define IPADDRESS "192.168.3.206"
#define SERV_PORT 45070
#define FDSIZE 1024
#define SIZE 30
#define MGSZIE 512

typedef struct
{
	int flag;
	int id;
}downonline;

typedef struct 
{
	int  flag;
	int  id;
	char online[SIZE];
	char name[SIZE];
	char account[SIZE];  //account
	char password[SIZE];
	char phonenumber[SIZE];
	char friendname[SIZE];
	char updata_or_foundpassword[SIZE];
	int  result;
}loginnode;

typedef struct 
{
	int flag;
	char sendaccount[SIZE];
	char sendname[SIZE];
	char acceptaccount[SIZE];
	char acceptname[SIZE];
	char groupaccount[SIZE];
	char message[MSGSIZE];
}historynode;

typedef struct
{
	int  flag;
	int  len;
	char sendaccount[SIZE];
	char acceptaccount[SIZE];
	char pathname[SIZE];
	char file[900];
	
}filenode;

typedef struct
{
	int  flag;
	int  id;    //id 为 login 的id
	int  result;
	char account[SIZE];
	char name[SIZE];
	char sex[SIZE];
	char data[SIZE];
	char address[SIZE];
	char constellation[SIZE];
	char email[SIZE];
	int  line;
}informationnode;

typedef struct    //添加好友
{
	int  flag;
	int  result;
	char sendaccount[SIZE];   //
	char acceptaccount[SIZE];
	int  sendid;    //存放发送方用户id
	int  acceptid;   //存放接收方 id
	int  sendfd;     //存放发送方的套接字
	int  acceptfd;   //存放接收方的套接字

}friendnode;


typedef struct     //聊天结构体
{ 
	int  flag;
	char sendaccount[SIZE];   //存放发送方的账号
	char acceptaccount[SIZE];    //存放接受方的账号
	
	char sendname[SIZE];   //存放发送放 的昵称
	char acceptname[SIZE];  //存放接受者的昵称

	int  sendid;    //存放发送方用户id
	int  acceptid;   //存放接收方 id
	int  acceptfd;      //接受者 的套接字
	int  sendfd;      //发送者的套接字
	char group_account[SIZE];   //存放 接受群 的账号
	char msg[MSGSIZE];     //消息的最大长度
}msgnode;
typedef struct
{
        int flag;
	int is_pep;
	char sendaccount[SIZE];
	char acceptaccount[SIZE];
        char noc[512];
}noticenode;


typedef struct 
{
	int  flag;
	int  id;  //申请人的 id
	int  sendfd;   //申请人的套接字
	int  group_id;   //群的id
	char user_account[SIZE];  //申请人的账号
	char administartor_account[SIZE];
	char user_name[SIZE];     //申请人的昵称
	char group_name[SIZE];
	char group_account[SIZE];
	int  result;
}groupnode;


int Flag;    //判断接收到的信息  的flag
char chat[1024];
informationnode inf;  //创建一个存储用户信息的结构体  一旦登录 就 保存了 本用户的 id 和账号
char currentaccount[SIZE];//   表示当前在和谁聊天
//char currentaccount2[SIZE];
char currentgroup[SIZE];    //表示当前在和那个组聊天
pthread_mutex_t mutex;  //创建一把锁
pthread_cond_t cond;    //创建一个信号

int File_transfer_persistence(filenode file);
int File_transfer_UI(int conn_fd);
int Find_group_chat();
int Find_chat();
int write_file_noc(noticenode noc);
int Find_notice();   // 消息通知
int View_chat_group_history(int conn_fd);  //查看群聊天记录
int View_chat_friend_history(int conn_fd);   //查看好友聊天记录
int View_chat_history(int conn_fd);   //查看历史记录
int group_chat_accept(msgnode msg);    //接受群消息
int group_chat(int conn_fd);       //群聊
int Dissolution_group(int conn_fd);
int View_group_members(int conn_fd);     //查看群成员
int View_add_group(int conn_fd);   //查看已加 群
int Kicking_people(int conn_fd);    //踢人
int exit_group(int conn_fd);   //退群
int Setup_administrator(int conn_fd);  //设置管理员
int join_group(int conn_fd);    //加入群
int deal_group(int conn_fd);   //处理入群申请
int write_file_group(groupnode grp); //  将入群申请写入文件中
int create_group(int conn_fd);     // 创建群
int Group_management_UI(int conn_fd);// 群管理
int Private_chat_accept(msgnode msg);//私聊处理
int Private_chat_send(int conn_fd);  //私聊
int Chat_communication_UI(int conn_fd); //聊天
int block_message(int conn_fd);  //屏蔽消息
int Friend_view_UI(int conn_fd);   //查看好友
int Friend_del_UI(int conn_fd);   //删除好友
int Friend_all_view(int conn_fd);   //查看所有好友
int Friend_add_send_UI(int fd);    //添加好友
int offonline(int fd);   //下线通知
int Friend_management_UI(int fd);   //好友管理主界面
int View_information_UI(int fd);    //查看用户信息
int Modity_information_UI(int fd);   //修改用户信息
int major_UI(int fd);    //登录有主界面
//void my_err(const char * err_string,int line);
int command_analy_flag(char a[5]);   //分析命令
int login_connect(int conn_fd);     //登录主界面
int Account_login_UI(int conn_fd);   //登录
int Account_regist_UI(int conn_fd);    //注册
int Account_updatapassword_UI(int conn_fd);  //修改密码
int Account_foundpassword_UI(int conn_fd);   //找回密码

void Find_freind(int fd);   //套接字 描述符
int *main_recv(void *arg);  //新开线程,负责收消息

int write_file_friend(friendnode fid);   //将好友申请写入文件中

int main(int argc,char **argv)
{
	
	signal(SIGINT, SIG_IGN);
	int i;
	pthread_t tid;
	int ret,ret2;
	int conn_fd;
	int serv_port;
	struct sockaddr_in serv_addr;
	char recv_buf1[BUFFSIZE];

	conn_fd = socket(AF_INET,SOCK_STREAM,0);
	int pthread_conn_fd = conn_fd;
	
	memset(&serv_addr,0,sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET,IPADDRESS,&serv_addr.sin_addr);
	int conre; 
	int optval;
	optval = 1;
	if(setsockopt(conn_fd,SOL_SOCKET,SO_REUSEADDR,(void *) &optval,sizeof(int)) < 0)  my_err("setsockopt",__LINE__);

	if(conre = connect(conn_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) < 0)   my_err("connect",__LINE__);
	if(conre == 0)	printf( " 连接服务器成功...\n");
		else 
		{
			printf("连接服务器失败...\n");
			return 0;
		}
	if(pthread_create(&tid,NULL,(void *)main_recv,(void *)pthread_conn_fd) != 0)
	{
		perror("recv pthread\n");
		exit(1);
	}

		int re;	

		login_connect(conn_fd);

	close(conn_fd);

	return 0;

}
int login_connect(int conn_fd)
{
	int command;
	int success = 0;    //判断是否成功进行操作  1为成功,2 为失败
	do
	{
		system("clear");
		printf( "                                  欢迎使用chat\n");
		printf( "                                  [1]  登录\n");
		printf( "                                  [2]  注册\n");
		printf( "                                  [3]  修改密码\n");
		printf( "                                  [4]  找回密码\n");
		printf( "                                  [5]  退出\n");

		printf( "                                  请输入你的选项 :");
		scanf( "%d",&command);
		getchar();

		switch(command)
		{
			case 1:
				success = Account_login_UI(conn_fd);
				if(success == 0)    continue;
				major_UI(conn_fd);
				break;
			case 2:
				success = Account_regist_UI(conn_fd);break;
			case 3:
				success = Account_updatapassword_UI(conn_fd);break;
			case 4:
				success = Account_foundpassword_UI(conn_fd);break;
			case 5:
				offonline(conn_fd);
		//		exit(1);
				break;
			default :
				printf( "选项错误\n");

		}
	}while(command != 5);
}
int Account_foundpassword_UI(int conn_fd)
{

	loginnode log;
	int re = 0;
	log.flag = 4;
	char buf[BUFFSIZE];
	int result;
	printf( "                                 ****** 找回密码 *******\n");
	printf( "                                      请输入账号:");
	gets(log.account);
	printf( "                                    请输入预留手机号:");
	gets(log.phonenumber);
	printf( "                                  请输入最好的朋友的名字:");
	gets(log.friendname);
	
	memset(buf,0,1024);    //初始化
	memcpy(buf,&log,sizeof(loginnode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");	
	
	//printf( "found re = %d\n",re);

	pthread_mutex_lock(&mutex);   //加锁  ,对全局变量进行操作
        pthread_cond_wait(&cond,&mutex);
	
	result = Flag;

        pthread_mutex_unlock(&mutex);

	
	return result;
}
int Account_updatapassword_UI(int conn_fd)
{
	int result;
	loginnode log;
	log.flag = 3;
	char buf[BUFFSIZE];
	char temppassword[SIZE];
	int re = 0;
	printf( "                                 ******** 修改密码 ***********\n");
	printf( "                                         请输入账号:");
	gets(log.account);
	printf( "                                         请输入旧密码");
	gets(log.password);
	printf( "                                         请输入新密码");
	gets(log.updata_or_foundpassword);
	while(1)
	{
		printf( "                                 请重新输入新密码:");
		gets(temppassword);
		if(strcmp(temppassword,log.updata_or_foundpassword) == 0)	break;
	}

	memset(buf,0,1024);    //初始化
	memcpy(buf,&log,sizeof(loginnode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");	

	pthread_mutex_lock(&mutex);   //加锁  ,对全局变量进行操作
        pthread_cond_wait(&cond,&mutex);
	
	result = Flag;

        pthread_mutex_unlock(&mutex);
	//printf( "updata re = %d\n",re);

	return result;
}
int Account_login_UI(int conn_fd)
{
	int result;
	int re = 0;
	char buf[BUFFSIZE];
	loginnode log;
	printf("                                 请输入账号: ");
	gets(log.account);
	strcpy(inf.account,log.account);
	printf("                                 请输入密码: ");
	gets(log.password);

	log.flag = 1;
	memset(buf,0,1024);    //初始化
	memcpy(buf,&log,sizeof(loginnode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");	
	
	//printf( "加锁\n");
	pthread_mutex_lock(&mutex);   //加锁  ,对全局变量进行操作
        pthread_cond_wait(&cond,&mutex);
	result = Flag;

        pthread_mutex_unlock(&mutex);
	
	strcpy(inf.account,log.account);

	//printf( "login re = %d\n",re);

	return result;
}

int Account_regist_UI(int conn_fd)
{
	int re = 0;
	loginnode log;
	int result;
	char temp[SIZE];
	char buf[BUFFSIZE];
	printf("                                 请输入昵称:");
	gets(log.name);
	printf( "                                 请输入账号:");
	gets(log.account);
	printf( "                                 请输入密码:");
	gets(log.password);
	while(1)
	{
		printf("                                请重新输入密码 :");
		gets(temp);
		if(strcmp(temp,log.password) == 0)  break;
		printf( "                                 密码不正确\n");
	}
	printf( "                                 密保问题: 手机号 :");
	gets(log.phonenumber);
	printf( "                                 密保问题: 友好朋友的名字 :");
	gets(log.friendname);
	log.flag = 2;
	memset(buf,0,1024);    //初始化
	memcpy(buf,&log,sizeof(loginnode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");	
	
	pthread_mutex_lock(&mutex);   //加锁  ,对全局变量进行操作
        pthread_cond_wait(&cond,&mutex);
	
	result = Flag;

        pthread_mutex_unlock(&mutex);
	//printf( "send_re = %d\n",re);
	printf( "发送\n");
	
	return result;
}

int *main_recv(void *arg)
{
	filenode file;
	noticenode noc;
	historynode his;
	msgnode msg;
	loginnode log;
	friendnode fid;
	groupnode grp;
	char recv_buf[BUFFSIZE];
	int  fd = (int)arg;   //转化 fd
	char *p = recv_buf;   //用来接受信息
	int ret;  
	int ret1;
	int lack;
	int judge = 0;

	pthread_mutex_init(&mutex,NULL);
	pthread_cond_init(&cond,NULL);

	char anly[5];   //解析命令
	while(1)
	{

		if((ret = recv(fd,p,1024,MSG_WAITALL)) < 0)  my_err("recv",__LINE__);
		while(1)
		{
			if(ret != 1024)
			{
				lack = 1024 - ret;
				for(int i = 0;i < ret;i++)  recv_buf[i] = *p++;
				if((ret1 = recv(fd,p,1024,MSG_WAITALL)) < 0)  my_err("recv",__LINE__);
				ret += ret1;
			}
			else break;
		}
		//printf( "输出\n");
//		printf( "main_ret = %d\n",ret);
		strncpy(anly,recv_buf,sizeof(int));
	//	printf( "解析之前flag = %d\n",log.flag);

		judge = command_analy_flag(anly);

		//printf( "%s",recv_buf);
		//printf( "\n");
		switch(judge)
		{
			case 0:
				memcpy(&noc,recv_buf,sizeof(noticenode));break;
			case 1:
			case 2:
			case 3:
			case 4:
				memset(&log,0,sizeof(loginnode));
        	 		memcpy(&log,recv_buf,sizeof(loginnode));    //将结构体的内容转为字符串
				break;
			case 5:
			case 6:
			case 9:   //查看好友信息
			case 10:
				//memset(&log,0,sizeof(loginnode));
        	 		memcpy(&inf,recv_buf,sizeof(informationnode));    //将结构体的内容转为字符串
				break;
			case 7:
			case 8:
				memcpy(&fid,recv_buf,sizeof(friendnode));
				break;
			case 11:
			case 15:
				memcpy(&msg,recv_buf,sizeof(msgnode));
				break;
			case 12:
			case 13:
			case 14:
				memcpy(&grp,recv_buf,sizeof(groupnode));
				break;
			case 16:
			case 17:
				memcpy(&his,recv_buf,sizeof(historynode));break;
			case 26:
				memcpy(&file,recv_buf,sizeof(filenode));break;
		}
		//printf( "judge = %d\n",judge);
		//printf( "result = %d\n",log.result);
		pthread_mutex_lock(&mutex);
		switch(judge)
		{
			case 0:
				write_file_noc(noc);
				break;
			case 1:
				if(log.result == 1)
				{
					printf( "                                 登录成功\n");
					printf( "                                 登录id  为 %d\n",log.id);
					inf.id = log.id;
					Flag = 1;
				}
				else
				{
					printf( "                                 登录失败\n");
					Flag = 0;
				}
				break;
			case 2:
				if(log.result == 1)
				{
					printf( "                                 注册成功\n");
					Flag = 1;
				}
				else
				{
					printf( "                                 注册失败\n");
					printf( "                                 账号已经被使用\n");
					Flag = 0;
				}
				break;
			case 3:
				if(log.result == 1)
				{
					Flag = 1;
					printf( "                                 修改成功\n");
					printf( "                                 新密码为 %s:\n",log.updata_or_foundpassword);
				}
				else
				{
					Flag = 0;
					printf( "                                 修改失败\n");
				}break;
			case 4:
				if(log.result == 1)
				{
					printf( "                                 找回成功\n");
					printf( "                                 密码为:%s\n",log.updata_or_foundpassword);
					Flag = 1;
				}
				else
				{
					Flag = 0;
					printf( "                                 找回失败\n");
				}
				break;
			case 5:
				if(inf.result == 1)
				{
					printf( "                                 用户信息修改成功\n");
					Flag = 1;
				}
				else
				{
					printf( "                                 用户信息修改失败\n");
					Flag = 0;
				}
				break;
			case 6:
				if(inf.result == 1)
				{
					printf( "                                 账号:%s\n",inf.account);
					printf( "                                 昵称:%s\n",inf.name);
					printf( "                                 性别:%s\n",inf.sex);
					printf( "                                 生日:%s\n",inf.data);
					printf( "                                 地址:%s\n",inf.address);
					printf( "                                 星座:%s\n",inf.constellation);
					printf( "                                 邮箱:%s\n",inf.email);
				}
				break;
			case 7:
		//		printf("好友申请通知\n");
				write_file_friend(fid);
				//friend_add_accept_UI(fid,fd);
				break;
			case 8:
		//		friend_add_accept_UI(fid,fd);
				break;
			case 9:
				printf("%s     %s     ",inf.account,inf.name);
				if(inf.line)   printf( "                                 是\n");
				else           printf( "                                 否\n");
				break;
			case 10:
				printf("                                 %s     %s     \n",inf.account,inf.name);
				break;
			case 11:
				Private_chat_accept(msg);
				break;
			case 12:
				write_file_group(grp);
				break;
			case 13:
				printf( "                                 user_account = %s    user_name = %s  \n",grp.group_account,grp.group_name);
				break;
			case 14:
				printf( "                                 user_account = %s    user_name = %s  \n",grp.user_account,grp.user_name);break;
			case 15:
				group_chat_accept(msg); break;
			case 16:
				printf( "                                 name = %s,msg = %s\n",his.acceptname,his.message);break;
			case 17:
				printf( "                                 name = %s,msg = %s\n",his.sendname,his.message);break;
			case 26:
				File_transfer_persistence(file);
				break;

		}

                pthread_mutex_unlock(&mutex);
                pthread_cond_signal(&cond);


				
	}
	
}

int command_analy_flag(char a[5])    //用来解析flag
{
	int flag;
	memcpy(&flag,a,sizeof(int));
	return flag;
}

int major_UI(int conn_fd)
{
	int command;
	do
	{
		system("clear");
		printf( "                                 [1]  好友管理\n");
		printf( "                                 [2]  聊天通信\n");
		printf( "                                 [3]  群管理\n");
		printf( "                                 [4]  传送文件\n");
		printf( "                                 [5]  修改信息\n");
		printf( "                                 [6]  查看用户信息\n");
		printf( "                                 [7]  查看好友普通消息通知\n");
		printf( "                                 [8]  查看系统消息通知\n");
		printf( "                                 [9]  接受文件\n");
		printf( "                                 [10]  退出\n");
	
		printf( "                                 请输入选项 :");
		scanf( "                                 %d",&command);
		getchar();
	
		switch(command)
		{
	
			case 1:
				Friend_management_UI(conn_fd);
				break;
			case 2:
				Chat_communication_UI(conn_fd);
				break;
			case 3:
				Group_management_UI(conn_fd);
				break;
			case 4:
				File_transfer_UI(conn_fd);
				break;
			case 5:
				Modity_information_UI(conn_fd);
				break;
			case 6:
				View_information_UI(conn_fd);

				break;
			case 7:
				Find_freind(conn_fd);
				getchar();
				break;
			case 8:
				Find_notice();
				getchar();
				break;
			case 9:
				//File_transfer_persistence(file,conn_fd)
				break;
			default :
				printf( "                                 选项错误\n");
		}	
		printf( "                                 command = %d\n",command);
	}while(command != 10);
}
int Modity_information_UI(int fd)
{
		system("clear");
 	 int re;
	 int result;
	 inf.flag = 5;
	 char buf[1024];
	 // inf 已知 用户的 主键 id,和账号
	 printf("                                 请输入昵称:");
	 gets(inf.name);
	 printf("                                 请输入性别:");
	 gets(inf.sex);
	 printf("                                 请输入出生日期:");
	 gets(inf.data);
	 printf("                                 请输入地址:");
	 gets(inf.address);
	 printf( "                                 请输入星座:");
	 gets(inf.constellation);
	 printf( "                                 请输入邮箱:");
	 gets(inf.email);
	 memset(buf,0,1024);    //初始化
	 memcpy(buf,&inf,sizeof(informationnode));    //将结构体的内容转为字符串
	
	 if((re = (send(fd,buf,1024,0))) < 0)  printf( "                                 错误\n");

	 pthread_mutex_lock(&mutex);   //加锁  ,对全局变量进行操作
	 pthread_cond_wait(&cond,&mutex);
	 result = Flag;
	 
	 pthread_mutex_unlock(&mutex);


}
int View_information_UI(int fd)
{
         int re;
         int result;
         inf.flag = 6;
         char buf[1024];
	 //已知inf用户的主键 ID,和账号;

	 
	 memset(buf,0,1024);    //初始化                                                   
         memcpy(buf,&inf,sizeof(informationnode));    //将结构体的内容转为字符串
         
	 if((re = (send(fd,buf,1024,0))) < 0)  printf( "                                 错误\n");
	
         pthread_mutex_lock(&mutex);   //加锁  ,对全局变量进行操作
         pthread_cond_wait(&cond,&mutex);
         result = Flag;
         
         pthread_mutex_unlock(&mutex);
	
	 getchar();
}
int Friend_management_UI(int conn_fd)
{
	int command;
	do
	{
		system("clear");
		printf( "                                 [1]  添加好友\n");
		printf( "                                 [2]  删除好友\n");
		printf( "                                 [3]  查看所有好友\n");
		printf( "                                 [4]  查看所有在线好友\n");
		printf( "                                 [5]  退出\n");
		printf( "                                 请输入选项:");

		scanf( "%d",&command);
		getchar();

		switch(command)
		{
			case 1:
				Friend_add_send_UI(conn_fd);
				break;
			case 2:
				Friend_del_UI(conn_fd);
				break;
			case 3:
				Friend_all_view(conn_fd);   //查看所有好友
				usleep(1000);
				getchar();
				break;
			case 4:
				Friend_view_UI(conn_fd);  //查看所有在线好友
				usleep(1000);
				getchar();
				break;
			default :
				printf( "                                 选项错误\n");
		}
	}while(command != 5);

	return 0;

}

int Friend_add_send_UI(int conn_fd)
{
	int re;
	char buf[1024];

	friendnode fid;
	fid.flag = 7;
	//printf( "                                 conn_fd = %d\n",conn_fd);
	printf( "                                 请输入你想要加为好友的 账号:");
	gets(fid.acceptaccount);
	strcpy(fid.sendaccount,inf.account);
	fid.sendid = inf.id;   //发送方的id
	fid.sendfd = conn_fd;       //发送方的套接字
	memset(buf,0,1024);    //初始化                                                   
        memcpy(buf,&fid,sizeof(friendnode));    //将结构体的内容转为字符串
        if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "                                 错误\n");

}



int offonline(int fd)
{
	int re;
	char buf[1024];
	downonline online;
	online.id = inf.id;
	online.flag = -1;
	memset(buf,0,1024);    //初始化                                                   
        memcpy(buf,&online,sizeof(downonline));    //将结构体的内容转为字符串
        if((re = (send(fd,buf,1024,0))) < 0)  printf( "                                 错误\n");
//	printf( "                                 re = %d\n",re);
	printf( "                                 退出\n");
}



int write_file_friend(friendnode fid)
{
	int fd;
	        //操作文件不正确 会 输出乱码
	if(!(fd = open("friend.txt",O_CREAT | O_WRONLY,0777)))
	{
		printf( "                                 文件打开失败\n");
	}
	//写数据
	
	if(write(fd,&fid,sizeof(friendnode)) != sizeof(friendnode))
	{
		printf( "                                 写入失败\n");
	}

	printf("有好友申请通知\n");
	close(fd);


}

void Find_freind(int conn_fd)
{
	int num = 0;
	char ch;
	int re;
	char buf[1024];
	friendnode fid;
	int fd;
	if((fd = open("friend.txt",O_CREAT | O_APPEND,S_IRUSR | S_IWUSR)) == -1)
	{
		printf( "                                 文件打开失败\n");
	}

	int sum = read(fd,&fid,sizeof(friendnode));
//	if(sum == 0)  printf( "没有通知\n");
//	else
	
	while(sum != 0)
	{
		printf( "                                 是否还想继续阅读 好友请求\n");
		printf("                                 Y / N:");
		scanf( "%c",&ch);
		getchar();
		if(ch == 'N') break;

		char ch;
		printf( "                                 账号 为 %s 接受 %s 的好友请求\n",fid.acceptaccount,fid.sendaccount);
		printf( "                                 是否接受?\n");
		printf( "                                 接受 Y /拒绝 N\n");
		scanf("%c",&ch);
		getchar();
		if(ch == 'Y' || ch == 'y')   fid.result = 1;
		else  fid.result = 0;
		fid.flag = 8;
		fid.acceptid = inf.id;
		memset(buf,0,1024);    //初始化                                                   
	        memcpy(buf,&fid,sizeof(friendnode));    //将结构体的内容转为字符串
	        if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
		num++;     //记录已读 的几条消息  一会删除的 前 num 条消息
		//	fid.result = 1;   //将本地的 reslut 改为1 代表已经 读 过这条消息,

		sum = read(fd,&fid,sizeof(friendnode));
	}

	if(sum == 0)   printf( "                                 无通知\n");
	close(fd);
	
	//删除 已读 数据
	if(rename("friend.txt","friend.txt_temp") < 0)   printf( "776 错误\n");

	FILE *fpsour,*fptarg;

	fpsour = fopen("friend.txt_temp","rb");
	if(NULL == fpsour)   printf( "                                 不能打开文件 781\n");

	fptarg = fopen("friend.txt","wb");
	if(NULL == fptarg)    printf( "                                 不能打开文件  784\n");
	
	friendnode temp;

	while(!feof(fpsour))
	{
		if(fread(&temp,sizeof(friendnode),1,fpsour))
		{
			if(num)  
			{
				num--;
				continue;
			}
			else
			{
				fwrite(&temp,sizeof(friendnode),1,fptarg);
			}
		}
	}

	fclose(fptarg);
	fclose(fpsour);
	
	remove("friend.txt_temp");
	//printf( "                                 judgeee = %d\n",fid.flag);
	//printf( "                                 restt = %d\n",fid.result);
		
}
int Friend_del_UI(int conn_fd)
{
	int re;
	char buf[1024];

	friendnode fid;
	printf( "                                 请输入要删除好友的 账号:");
	gets(fid.acceptaccount);

	strcpy(fid.sendaccount,inf.account);   // 将本用户的 账号保存
	fid.sendid = inf.id;

	fid.flag = 9;
	memset(buf,0,1024);    //初始化 
	memcpy(buf,&fid,sizeof(friendnode));    //将结构体的内容转为字符串
       
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
	
	printf( "                                 您已成功删除 %s 好友",fid.acceptaccount);
	return 0;
}

int Friend_all_view( int conn_fd)
{
	int re;
	char buf[1024];

	printf( "                                 ******** 好友列表 *********\n");
	printf( "                                 账号      昵称        是否在线\n");
	//fid.flag = 10;
	inf.flag = 10;
	memset(buf,0,1024);    //初始化 
	memcpy(buf,&inf,sizeof(informationnode));    //将结构体的内容转为字符串

	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");


}


int Friend_view_UI(int conn_fd)
{
	int re;
	char buf[1024];

	printf( "                                 ******** 好友列表 *********\n");
	printf( "                                 账号      昵称     \n");

	inf.flag = 11;
	memset(buf,0,1024);    //初始化 
	memcpy(buf,&inf,sizeof(informationnode));    //将结构体的内容转为字符串

	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
	
}
int  Chat_communication_UI(int conn_fd)
{

	int command;

	do
	{

		system("clear");
		printf("                                 ***** 聊天通信 *****\n");
		printf("                                 [1]  私聊\n");
		printf("                                 [2]  群聊\n");
		printf("                                 [3]  查看聊天记录\n");
		printf("                                 [4]  离线消息\n");
		printf("                                 [5]  屏蔽某人消息\n");
		printf("                                 [6]  查看好友聊天消息\n");
		printf("                                 [7]  查看群聊聊天消息\n");
		printf("                                 [8]  查看群通知\n");
		printf("                                 [9]  退出\n");
		printf( "                                 请输入选项:");
		scanf( "%d",&command);
		getchar();
		switch(command)
		{
			case 1:

			//	Friend_view_UI(conn_fd);
				usleep(1000);
				Private_chat_send(conn_fd);
				break;
			case 2:
				group_chat(conn_fd);
				break;
			case 3:
				View_chat_history(conn_fd);
				break;
			case 4:
				//offline_message( );
				//break;
			case 5:
				block_message(conn_fd);
				break;
			case 6:
				Find_chat();
				break;
			case 7:Find_group_chat();
			getchar();
			       break;
			case 8:
			       break;
			default:
				printf( "选项错误\n");
		}

	}while(command != 9);
}
int Private_chat_send(int conn_fd)
{
	char buf[1024];
	int re;

	msgnode msg;
	printf("                                 请输入你想要聊天的账号\n");
	gets(msg.acceptaccount);

	strcpy(currentaccount,msg.acceptaccount);   //标记当前在和谁聊天
	//printf("current = %s\n",currentaccount);
	msg.sendid = inf.id;
	msg.sendfd = conn_fd;
	strcpy(msg.sendaccount,inf.account);
	msg.flag = 12;
	

	printf( "                                 输入' Q '退出聊天.\n");
	do
	{
		printf( "                                  请输入聊天信息: ");

		gets(msg.msg);

		memset(buf,0,1024);    //初始化 
		memcpy(buf,&msg,sizeof(msgnode));    //将结构体的内容转为字符串
		if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
	}while(strcmp(msg.msg,"Q") != 0);
	
	strcpy(currentaccount," ");
	
	return 0;
}
int Private_chat_accept(msgnode msg)  
{
	//判断是否 在和当前 这个人聊天  通过账号判断
	//  是 将消息打印到屏幕
	if(strcmp(msg.sendaccount,currentaccount) == 0)
	{
		printf("                                 name = %s 发送:%s\n",msg.sendname,msg.msg);
	}
	else   //否则 保存到消息盒子中
	{
		int fd;
		        //操作文件不正确 会 输出乱码

		if(!(fd = open("chat.txt",O_CREAT | O_WRONLY | O_APPEND,0777)))
		{
			printf( "                                 文件打开失败\n");
		}
		//写数据

		int re = 1;
		if((re = write(fd,&msg,sizeof(msgnode))) != sizeof(msgnode))
		{
			printf( "写入失败\n");
		}
		printf( "                                 好友消息通知\n");
		//printf( "                                 re = %d\n",re);
		close(fd);
		
	}
	
}

int block_message(int conn_fd)
{
	int re;
	char buf[1024];
	msgnode msg;

	msg.flag = 13;
	printf( "                                 请输入你想要屏蔽的账号:");
	gets(msg.acceptaccount);
	msg.sendid = inf.id;

	memset(buf,0,1024);    //初始化 
	memcpy(buf,&msg,sizeof(msgnode));    //将结构体的内容转为字符串		
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");


}

int Group_management_UI(int conn_fd)
{
	int command;

	do
	{
		system("clear");
		printf( "                                 [1]   创建群\n");
		printf( "                                 [2]   加群\n");   //群人数 +1
		printf( "                                 [3]   退群\n");   //群人数 -1
		printf( "                                 [4]   查看已加群\n");
		printf( "                                 [5]   查看群成员\n");
		printf( "                                 [6]   解散群\n");  
		printf( "                                 [7]   设置管理员\n");
		printf( "                                 [8]   踢人\n");   // 群人数  -1 只有群主,管理员 
		printf( "                                 [9]   查看群申请\n");
		printf( "                                 [10]  退出\n");
		printf( "                                 请输入选项:\n");
		scanf( "%d",&command);
		getchar();
		switch(command)
		{
			case 1:
				create_group(conn_fd);
				break;
			case 2:
				join_group(conn_fd);
				break;
			case 3:
				exit_group(conn_fd);
				break;
			case 4:
				View_add_group(conn_fd);
				getchar();
				break;
			case 5:
				View_group_members(conn_fd);
				getchar();
				break;
			case 6:
				Dissolution_group(conn_fd);
				break;
			case 7:
				Setup_administrator(conn_fd);
				break;
			case 8:
				Kicking_people(conn_fd);
				break;
			case 9:
				deal_group(conn_fd);
				break;
			default:
				printf( "                                 选项错误\n");
				break;
		}
	}while(command != 10);
}

int create_group(int conn_fd)
{
	char buf[1024];
	int re;

	groupnode grp;
	grp.flag = 14;
	printf( "                                 请输入群账号:");
	gets(grp.group_account);
	printf( "                                 请输入群昵称:");
	gets(grp.group_name);

//	printf( "account = %s\n",grp.group_account);
//	printf( "name = %s\n",grp.group_name);
	grp.id = inf.id;
	strcpy(grp.user_account,inf.account);
	
	memset(buf,0,1024);    //初始化 
	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串		
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");

}

int join_group(int conn_fd)
{
	char buf[1024];
	int re;
	groupnode grp;
	grp.flag = 15;
	printf( "                                 请输入想要加入群的账号\n");
	gets(grp.group_account);
	strcpy(grp.user_account,inf.account);
	grp.id = inf.id;

	memset(buf,0,1024);    //初始化 
	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串		
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
}
int deal_group(int conn_fd)
{
	int num = 0;
	char ch;
	int re;
	char buf[1024];
	groupnode grp;
	int fd;
	if((fd = open("group.txt",O_CREAT | O_APPEND,S_IRUSR | S_IWUSR)) == -1)
	{
		printf( "                                 文件打开失败\n");
	}

	int sum = read(fd,&grp,sizeof(groupnode));
	
	while(sum != 0)
	{
		printf( "                                 是否还想继续阅读 申请入群请求\n");
		printf("Y / N:");
		scanf( "%c",&ch);
		getchar();
		if(ch == 'N') break;

		char ch;
		printf( "                                 账号 为 %s 申请入群\n",grp.user_account);
		printf( "                                 是否接受?\n");
		printf( "                                 接受 Y /拒绝 N\n");
		scanf("%c",&ch);
		getchar();
		if(ch == 'Y')   grp.result = 1;
		else  grp.result = 0;
		grp.flag = 16;
		memset(buf,0,1024);    //初始化
	        memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串
	        if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
		num++;     //记录已读 的几条消息  一会删除的 前 num 条消息


		sum = read(fd,&grp,sizeof(groupnode));
	}

	if(sum == 0)   printf( "无通知\n");
	close(fd);
	
	//删除 已读 数据
	if(rename("group.txt","group.txt_temp") < 0)   printf( "776 错误\n");

	FILE *fpsour,*fptarg;

	fpsour = fopen("group.txt_temp","rb");
	if(NULL == fpsour)   printf( "不能打开文件 781\n");

	fptarg = fopen("group.txt","wb");
	if(NULL == fptarg)    printf( "不能打开文件  784\n");
	
	groupnode temp;

	while(!feof(fpsour))
	{
		if(fread(&temp,sizeof(groupnode),1,fpsour))
		{
			if(num)  
			{
				num--;
				continue;
			}
			else
			{
				fwrite(&temp,sizeof(groupnode),1,fptarg);
			}
		}
	}

	fclose(fptarg);
	fclose(fpsour);
	
	remove("group.txt_temp");

}



int write_file_group(groupnode grp)
{
	int fd;
	        //操作文件不正确 会 输出乱码
	if(!(fd = open("group.txt",O_CREAT | O_APPEND | O_WRONLY,0777)))
	{
		printf( "文件打开失败\n");
	}
	
	//写数据
	//
	if(write(fd,&grp,sizeof(groupnode)) != sizeof(groupnode))
	{
		printf( "写入失败\n");
	}
	printf("有群申请通知!\n");

	close(fd);
}

int Setup_administrator(int conn_fd)  //设置管理员
{
	char buf[1024];
	int re;
	groupnode grp;

	grp.flag = 17;
	printf( "                                 请输入你想要设置管理员的账号:");
	gets(grp.user_account);    //保存将要申请的账号
	printf( "                                 请输入群账号:");
	gets(grp.group_account);
	grp.id = inf.id;

	memset(buf,0,1024);    //初始化
	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");

}
int exit_group(int conn_fd)
{
	char buf[1024];
	int re;

	groupnode grp;
	printf( "                                 请输入你退出群的账号:");
	gets(grp.group_account);
	grp.id = inf.id;
	grp.flag = 18;
	strcpy(grp.user_account,inf.account);
	memset(buf,0,1024);    //初始化
	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");


}
int Kicking_people(int conn_fd)
{
	int re;
	char buf[1024];
	groupnode grp;

	printf("                                 请输入你想要在那个群里踢人(账号) :");
	gets(grp.group_account);
	printf( "                                 请输入你想套踢人的账号:");
	gets(grp.user_account);
	grp.id = inf.id;
	strcpy(grp.administartor_account,inf.account);

	grp.flag = 19;
	memset(buf,0,1024);    //初始化
	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");

}
int View_add_group(int conn_fd)
{
	char buf[1024];
	int re;
	groupnode grp;

	grp.id = inf.id;
	strcpy(grp.user_account,inf.account);
	grp.flag = 20;
	memset(buf,0,1024);    //初始化
	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");

}
int View_group_members(int conn_fd)
{
	char buf[1024];
	int re;
	groupnode grp;
	printf( "                                 请输入想要查看群的账号:");
	gets(grp.group_account);
	grp.id = inf.id;
	strcpy(grp.user_account,inf.account);
	grp.flag = 21;
	memset(buf,0,1024);    //初始化
	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");

}
int Dissolution_group(int conn_fd)
{
	char buf[1024];
	int re;

	groupnode grp;
	printf( "                                 请输入你想要解散的群的账号\n");
	gets(grp.group_account);
	grp.id = inf.id;
	strcpy(grp.user_account,inf.account);

	grp.flag = 22;
	memset(buf,0,1024);    //初始化
	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");

	return 0;
}
int group_chat(int conn_fd)
{
	msgnode msg;
	char buf[1024];
	int re;
	
	printf( "                                 请输入你想要进入的群聊账号\n");
	gets(msg.group_account);
	strcpy(currentgroup,msg.group_account);

	printf( "                                 输入 qwe 退出群聊!\n");
	msg.sendid = inf.id;
	msg.sendfd = conn_fd;
	strcpy(msg.sendaccount,inf.account);
	msg.flag = 23;
	do
	{
		printf( "                                 请输入消息:");
		gets(msg.msg);
		
		memset(buf,0,1024);
		memcpy(buf,&msg,sizeof(msgnode));
		if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
	}while(strcmp("qwe",msg.msg) != 0);
	
	strcpy(currentgroup," ");


	return 0;
}
int group_chat_accept(msgnode msg)
{
		//printf( "                                 group = %s\n",msg.group_account);
		//printf( "                                 curr = %s\n",currentgroup);
	//printf("account = %s\ncurrent")
	if(strcmp(msg.group_account,currentgroup) == 0)
	{
		//printf( "                                 group = %s\n",msg.group_account);
		//printf( "                                 curr = %s\n",currentgroup);

		printf( "                                 name = %s  发送 :%s\n",msg.sendname,msg.msg);
	}
	else
	{
		int fd;
		        //操作文件不正确 会 输出乱码
		if(!(fd = open("group_chat.txt",O_CREAT | O_APPEND | O_WRONLY,0777)))
		{
			printf( "文件打开失败\n");
		}
	
		//写数据
		//
		if(write(fd,&msg,sizeof(msgnode)) != sizeof(msgnode))
		{
			printf( "                                 写入失败\n");
		}

		printf("有一条群聊通知\n");
	
		close(fd);
	
	}

}


int View_chat_history(int conn_fd)
{
	int command;

	printf("                                 [1] 查看好友聊天历史记录\n");
	printf("                                 [2] 查看群聊天历史记录\n");
	printf("                                 [3] 退出\n");
	scanf( "%d",&command);
	getchar();
	switch(command)
	{
		case 1:
			View_chat_friend_history(conn_fd);
			getchar();
			break;
		case 2:
			View_chat_group_history(conn_fd);
			getchar();
			break;
		default:
			printf( "                                 选项错误");
	}
}

int View_chat_friend_history(int conn_fd)
{
		char buf[1024];
		int re;

		historynode  his;
		printf("                                 请输入你想要查看的好友账号:");
		gets(his.acceptaccount);
		his.flag = 24;
		strcpy(his.sendaccount,inf.account);
	
		memset(buf,0,1024);
		memcpy(buf,&his,sizeof(historynode));
		if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
}

int View_chat_group_history(int conn_fd)
{
	char buf[1024];
	int re;

	historynode his;
	printf("                                 请输入你想要查看的群聊账号:");
	gets(his.groupaccount);
	his.flag = 25;

	strcpy(his.sendaccount,inf.account);

	memset(buf,0,1024);
	memcpy(buf,&his,sizeof(historynode));
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
}


int write_file_noc(noticenode noc)
{
	int fd;
	        //操作文件不正确 会 输出乱码
		//
	if(!(fd = open("noc.txt",O_CREAT | O_APPEND | O_WRONLY,0777)))
	{
		printf( "                                 文件打开失败\n");
	}
	
	//写数据
	//
	if(write(fd,&noc,sizeof(noticenode)) != sizeof(noticenode))
	{
		printf( "                                 写入失败\n");
	}
	printf("有一条系统通知\n");
	close(fd);

}

int Find_notice()   // 消息通知
{

	int num = 0;
	char ch;
	char buf[1024];

	noticenode noc;
	int fd;
	if((fd = open("noc.txt",O_CREAT | O_APPEND,S_IRUSR | S_IWUSR)) == -1)
	{
		printf( "                                 文件打开失败\n");
	}

	int sum = read(fd,&noc,sizeof(noticenode));
	
	while(sum != 0)
	{
		printf( "                                 是否还想继续阅读 系统通知\n");
		printf("Y / N:");
		scanf( "%c",&ch);
		getchar();
		if(ch == 'N') break;
		if(noc.is_pep == 1)
		{
			printf( "%s  ",noc.acceptaccount);
		}
		printf("%s\n",noc.noc);

		num++;
		sum = read(fd,&noc,sizeof(noticenode));
	}

	if(sum == 0)   printf( "无通知\n");
	close(fd);
	
	//删除 已读 数据
	if(rename("noc.txt","noc.txt_temp") < 0)   printf( "776 错误\n");

	FILE *fpsour,*fptarg;

	fpsour = fopen("noc.txt_temp","rb");
	if(NULL == fpsour)   printf( "不能打开文件 1626\n");

	fptarg = fopen("noc.txt","wb");
	if(NULL == fptarg)    printf( "不能打开文件  1629\n");
	
	noticenode  temp;

	while(!feof(fpsour))
	{
		if(fread(&temp,sizeof(noticenode),1,fpsour))
		{
			if(num)  
			{
				num--;
				continue;
			}
			else
			{
				fwrite(&temp,sizeof(noticenode),1,fptarg);
			}
		}
	}

	fclose(fptarg);
	fclose(fpsour);
	
	remove("noc.txt_temp");

}

int Find_chat()
{
	int num = 0;
	char ch;
	char buf[1024];

	msgnode msg;

	int fd;
	if((fd = open("chat.txt",O_CREAT | O_APPEND,S_IRUSR | S_IWUSR)) == -1)
	{
		printf( "                                 文件打开失败\n");
	}
	
	int sum = read(fd,&msg,sizeof(msgnode));
//	printf( "sum = %d\n",sum);
	while(sum != 0)
	{
		printf( "                                 是否还想继续阅读 好友消息\n");
		printf("Y / N:");
		scanf( "%c",&ch);
		getchar();
		if(ch == 'N') break;
		
		printf("                                 name = %s 发送:%s\n",msg.sendname,msg.msg);
		//printf( "                                 num = %d\n",num);
		num++;
		sum = read(fd,&msg,sizeof(msgnode));
	//	printf( "                                 sum = %d\n",sum);
	}

	if(sum == 0)   printf( "                                 无通知\n");
	close(fd);
	
	//删除 已读 数据
	if(rename("chat.txt","chat.txt_temp") < 0)   printf( "776 错误\n");

	FILE *fpsour,*fptarg;

	fpsour = fopen("chat.txt_temp","rb");
	if(NULL == fpsour)   printf( "                                 不能打开文件 781\n");

	fptarg = fopen("chat.txt","wb");
	if(NULL == fptarg)    printf( "                                 不能打开文件  784\n");
	
	msgnode  temp;

	while(!feof(fpsour))
	{
		if(fread(&temp,sizeof(msgnode),1,fpsour))
		{
			if(num)  
			{
				num--;
				continue;
			}
			else
			{
				fwrite(&temp,sizeof(msgnode),1,fptarg);
			}
		}
	}

	fclose(fptarg);
	fclose(fpsour);
	
	remove("chat.txt_temp");
	getchar();

}
int Find_group_chat()
{
	int num = 0;
	char ch;
	char buf[1024];

	msgnode msg;

	int fd;
	if((fd = open("group_chat.txt",O_CREAT | O_APPEND,S_IRUSR | S_IWUSR)) == -1)
	{
		printf( "                                 文件打开失败\n");
	}
	
	int sum = read(fd,&msg,sizeof(msgnode));
	printf( "sum = %d\n",sum);
	while(sum != 0)
	{
		printf( "                                 是否还想继续阅读 群消息\n");
		printf("                                 Y / N:");
		scanf( "%c",&ch);
		getchar();
		if(ch == 'N') break;
		
		printf("                                 name = %s 发送:%s\n",msg.sendname,msg.msg);
		//printf( "                                 num = %d\n",num);
		num++;
		sum = read(fd,&msg,sizeof(msgnode));
		printf( "sum = %d\n",sum);
	}

	if(sum == 0)   printf( "无通知\n");
	close(fd);
	
	//删除 已读 数据
	if(rename("group_chat.txt","group_chat.txt_temp") < 0)   printf( "776 错误\n");

	FILE *fpsour,*fptarg;

	fpsour = fopen("group_chat.txt_temp","rb");
	if(NULL == fpsour)   printf( "不能打开文件 781\n");

	fptarg = fopen("group_chat.txt","wb");
	if(NULL == fptarg)    printf( "不能打开文件  784\n");
	
	msgnode  temp;

	while(!feof(fpsour))
	{
		if(fread(&temp,sizeof(msgnode),1,fpsour))
		{
			if(num)  
			{
				num--;
				continue;
			}
			else
			{
				fwrite(&temp,sizeof(msgnode),1,fptarg);
			}
		}
	}

	fclose(fptarg);
	fclose(fpsour);
	
	remove("group_chat.txt_temp");

	getchar();
}


int File_transfer_UI(int conn_fd)
{
	filenode file;
	int sum = 0;
	FILE *fp;
	char buf[1024];
	int re;
	char pathname[100];

	printf( "                                 请输入对方的账号:");
	gets(file.acceptaccount);

	printf( "                                 请输入文件路径:");
	gets(file.pathname);
	printf( "%s",file.pathname);
	int fd;
	fd = open(file.pathname,O_RDONLY);

	//printf( "fd = %d\n",fd);
	file.flag = 26;
	

	sum = read(fd,file.file,sizeof(file.file));
	file.len = sum;
printf("                                     正在发送\n");
	//printf( "sum = %d\n",sum);
	while(sum != 0)
	{        

		memset(buf,0,1024);
		memcpy(buf,&file,sizeof(filenode));
		if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
	
	
		printf( "发送中............\n",sum);
		sum = read(fd,file.file,sizeof(file.file));
		file.len = sum;
		if(sum < 0)   break;
	}

}
int File_transfer_persistence(filenode file)
{
        int fd;

        fd = open(file.pathname,O_CREAT | O_WRONLY | O_APPEND,0777);
        perror(file.pathname);

		printf("接受文件中.........\n");
    //    printf( "                                 fd = %d\n",fd);
        int sum = 0;
      //  printf( "                                 len = %d\n",file.len);
        sum = write(fd,file.file,file.len);
        //printf( "                                 sum = %d\n",sum);

        close(fd);

        return 0;
}

