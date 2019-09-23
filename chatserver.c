#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/types.h>
#include"list.h"
#include<errno.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<mysql/mysql.h>
#include<pthread.h>


#define PORT 45070
#define LISTENQ 20
#define EPOLLLEN 2000
#define BUFSIZE  1024
#define SIZE 30
#define MGSIZE 512
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
        int  flag;
	int  len;
        char sendaccount[SIZE];
	char acceptaccount[SIZE];
	char pathname[SIZE];
        char file[900];
             
}filenode;

typedef struct 
{
	int flag;
	char sendaccount[SIZE];
	char sendname[SIZE];
	char acceptaccount[SIZE];
	char acceptname[SIZE];
	char groupaccount[SIZE];
	char message[MGSIZE];
}historynode;


typedef struct 
{
	int flag;
	int is_pep;
	char sendaccount[SIZE];
	char acceptaccount[SIZE];
	char noc[512];
}noticenode;

typedef struct online_node   //存储上线 的用户 ID,套接字,账号
{
	int fd;
	int id;
	char account[SIZE];
	struct online_node  *next;
	struct online_node  *prev;
}online_node_t,* online_list_t;

typedef struct
{
        int flag;
        int id; 
}downonline;

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

typedef struct 
{
        int  flag;
        int  id;  //申请人的 id
	int  sendfd; //申请人的套接字
	int  group_id;
        char user_account[SIZE];  //申请人的账号
	char administartor_account[SIZE];
	char user_name[SIZE];
        char group_name[SIZE]; 
        char group_account[SIZE];
	int result;
}groupnode;

//online 

typedef struct
{
        int  flag;
        int  id;
	int  result;
        char account[SIZE];
        char name[SIZE];
        char sex[SIZE];
        char data[SIZE];
        char address[SIZE];
        char constellation[SIZE];
        char email[SIZE];
	int line;
}informationnode;  //完善用户信息 结构体

typedef struct     //聊天结构体    
{ 
        int flag;
        char sendaccount[SIZE];   //存放发送方的账号
        char acceptaccount[SIZE];    //存放接受方的账号

        char sendname[SIZE];   //存放发送放 的昵称
        char acceptname[SIZE];  //存放接受者的昵称
        
	int sendid;    //存放发送方用户id
        int acceptid;   //存放接收方 id
	int acceptfd;   //存放接受者的 套接字
	int sendfd;    //存放发送者的套接字
	char group_account[SIZE];   //存放 接受群 的账号 
        char msg[MGSIZE];     //消息的最大长度
}msgnode;


typedef struct 
{
        int flag ;
        char message[MGSIZE];
}mgnode;

online_list_t head;

pthread_mutex_t mutex;
MYSQL mysql;
MYSQL_RES *result;


int judge_is_friend(int friend1,int friend2);     //判断是否为 好友
int Find_idbyaccount(char account[SIZE]);      //通过 账号 找 id
int del_offline_group_add(char account[SIZE],int conn_fd);
int File_transfer_persistence(filenode file, int conn_fd);  //文件传输
int del_send_offline_group_noc(char account[SIZE],int conn_fd);  
int send_offline_group_noc(char account[SIZE],int conn_fd);
int send_offline_group_add(char account[SIZE],int conn_fd);   //  离线群申请
int del_send_offline_group_chat(char account[SIZE],int conn_fd);  //删除群离线消息
int send_offline_group(char account[SIZE],int conn_fd);   //群离线消息
int del_send_offline_chat(char account[SIZE],int conn_fd);   //删除聊天消息
int send_offline_chat(char account[SIZE],int conn_fd);      //发送离线消息
int Find_namebyaccount(char account[SIZE],char name[SIZE]);  //通过账号 找name
int del_send_offline_noc(char account[SIZE],int conn_fd);    //删除离线通知
int del_send_offline_friend(char account[SIZE],int conn_fd);   //删除 离线好友通知
int send_offline_noc(char account[SIZE],int conn_fd);         //检查离线通知
int send_offline_friend(char account[SIZE],int conn_fd);   //发送离线好友通知
int insert_offline(char sendaccount[SIZE],char acceptaccount[SIZE],char buf[1024]);   //加入到 离线消息中
int is_online(char account[SIZE]);    //判断是否在线
int View_chat_group_history(historynode his,int conn_fd); //查看群 聊天记录
int View_chat_friend_history(historynode his,int fd);     //查看好友的聊天记录 
int chat_group_persistence(msgnode msg,int fd);           //群聊天:
int Dissolution_group_persistence(groupnode grp,int fd);   //解散群
int View_group_members(groupnode grp,int fd);    //查看群成员
int View_add_group_persistence(groupnode  grp,int conn_fd);   //查看已加入的 群
int Kicking_people_persistence(groupnode grp);    //踢人
int exit_group_persistence(groupnode grp);  //退群
int Set_administrator_persistence(groupnode grp,int conn_fd);  //只能由群主设置   设置管理员
int join_group_persistence(groupnode grp,int conn_fd);
int is_group(groupnode grp);  //判断群是否存在 ,
int group_add_persistence(groupnode grp,int fd); //处理 群申请
int create_group_persistence(groupnode grp,int conn_fd);  //创建群
int is_block(int sendid,int acceptid);  //判断是否屏蔽了 消息
int block_message_persistence(msgnode msg,int conn_fd);   //屏蔽某人
int Pravite_chat_send_persistence(msgnode msg,int fd);  //消息处理
int Friend_view_persistence(informationnode inf,int conn_fd); //查看在线好友
int Friend_all_view_persistence(informationnode inf,int conn_fd);  //查看所有好友
int friend_del_persistence(friendnode fid,int fd); //删除好友
int friend_add_deal_persistence(friendnode fid); //处理 添加好友函数
int friend_add_send_persistence(friendnode fid,int fd);  //发送添加好友
int offline_persistence(downonline offline);   //下线通知
int View_information_persistence(informationnode inf,int fd); //查看用户信息
int Modity_information_persistence(informationnode inf,int fd);   //修改用户信息
int command_analy_flag(char a[5]);    //用来解析flag
int Account_updatapassword_persistence(loginnode login,int fd); //修改密码
int Account_login_persistence(loginnode login,int fd);    //登录
int Account_resgine_persistence(loginnode log,int fd);  //注册
int Account_foundpassword_persistence(loginnode log ,int fd);   //找回密码

int judge_account(loginnode log,int fd);     //判断账户是否存在
//void printResult(MYSQL *mysqlPrint);  //输出数据库例的数据
static void do_write(int epfd,int fd,char *buf);    
static void do_read(int epfd,int fd,char *buf);
int listenfd_accept(int epfd,int fd);   //连接 客户端
static void handle_events(int epollfd,struct epoll_event *events,int num,int listenfd,char *buf); // 处理事件
void my_err(const char * err_string,int line);     //错误提示
void send_data(int conn_fd,const char *string);   //发送数据

int main( )
{
	int listenfd;
	struct sockaddr_in servaddr;

	List_Init(head,online_node_t);
	
	pthread_mutex_init(&mutex,NULL);

	//创建一个套接字
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if(listenfd < 0) my_err("socket",__LINE__);
	//设置该套接字使之可以重新绑定端口
	int optval = 1;
	if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(void *) &optval,sizeof(int)) < 0)  my_err("setsockopt",__LINE__);
	
	//初始化服务器端地址结构
	memset(&servaddr,0,sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//将套接字绑定到本地端口
	if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(struct sockaddr_in)) < 0)   my_err("bind",__LINE__);

	//将套接字转化为 监听套接字
	if(listen(listenfd,LISTENQ) < 0)  my_err("listen",__LINE__);

	//创建一个epoll 手柄
	int epfd;
	epfd = epoll_create(EPOLLLEN);
	struct epoll_event events[EPOLLLEN];
	struct epoll_event ev;
	ev.events = EPOLLIN;   //监听事件
	ev.data.fd = listenfd;  //被监听的套接字
	epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
	char buf[BUFSIZE];

	//MYSQL mysql;
        //MYSQL_RES *result;
        mysql_init(&mysql);    
        if(!mysql_real_connect(&mysql,"0.0.0.0","root","073848","chat",0,NULL,0))  my_err("conect mysql",__LINE__);
     
	if(mysql_query(&mysql,"set names utf8"))  printf( "FAlse \n");

	int ret = 0;
	int sendid;    //存放发送方用户id

	while(1)
	{
		//委托内核监测事件
		ret = epoll_wait(epfd,events,EPOLLLEN,-1);
		printf( "ret = %d\n",ret);
		if(ret > 0) handle_events(epfd,events,ret,listenfd,buf);

	}
		
	close(epfd);    //一定要记着关闭
}
static void handle_events(int epfd,struct epoll_event *events,int num,int listenfd,char *buf)
{
	int i,fd;
        online_node_t *curpos;

	for(int i = 0;i < num;i++)
	{
		int fd = events[i].data.fd;

		if((fd == listenfd) && (events[i].events & EPOLLIN))   listenfd_accept(epfd,fd);
		else if(events[i].events & EPOLLIN )
		{
			//printf( "events %d\n",events[i].events);
			if(events[i].events & EPOLLRDHUP)
			{
     	 		  List_ForEach(head,curpos)
    			    {
        		        if(curpos->fd == events[i].data.fd)
               			{
													List_DelNode(curpos);
													printf( "删除成功\n");
               		   		 break;
               			}
        		    }

				printf( "断开连接\n");
				epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
				continue;
			}
			else
			{
				do_read(epfd,fd,buf);
			}
		}
		else if(events[i].events & EPOLLOUT)   do_write(epfd,fd,buf);
	}

}

int listenfd_accept(int epfd,int fd)
{
	int clifd;
	struct sockaddr_in cliaddr; //用来保存发起连接请求的 主机的地址和端口
	socklen_t cliaddrlen;
	struct epoll_event ev;
	
	cliaddrlen = sizeof(struct sockaddr);   //要初始化

	clifd = accept(fd,(struct sockaddr *)&cliaddr,&cliaddrlen);
	if(clifd < 0) my_err("accept",__LINE__);

	printf( "accept a new client : %s : %d \n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
	
//	send_data(clifd,"welcome login my tcp server1\n");
	printf( "clifd = %d\n",clifd);
	ev.data.fd = clifd;
	ev.events = EPOLLIN;
	ev.events |= EPOLLRDHUP;
	//向 fd 进行 add 操作 
	epoll_ctl(epfd,EPOLL_CTL_ADD,clifd,&ev);
	
	return 0;
}

static void do_read(int epfd,int fd,char *buf)
{

	filenode file;
	historynode his;
	msgnode msg;
	friendnode fid;
	downonline offline;
	groupnode grp;
	loginnode log;
	
	char anly[5];
	int lack;    //计算还剩 多少数据 需要接收
	char *p = buf;
	int ret;
	int ret1;  // 计算 第二次 接受到的数据
	int i;           //sizeof(buf)   出错   因为 buf 是个指针,指针一次只有 8 个字节
	int judge;
	informationnode inf;

	memset(buf,0,1024);
	memset(&log,0,sizeof(loginnode));

	if((ret = recv(fd,p,1024,MSG_WAITALL)) < 0)
	{
		my_err("recv",__LINE__);
		close(fd);
	}
	while(1)
	{   
		if(ret != 1024)
		{   
			lack = 1024 - ret;
			for(int i = 0;i < ret;i++)  buf[i] = *p++;
			if((ret1 = recv(fd,p,1024,MSG_WAITALL)) < 0)  my_err("recv",__LINE__);
			ret += ret1;
		}   
		else break;
	}   
//	printf( "retqwe = %d\n",ret);
	if(ret == 0)
	{
		printf("client exit\n");
		epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
	
	}
	buf[ret] = '\0';  //将数据 结束标志 '\n' 替换成 字符串结束标志
//	puts(buf);
	printf( "接受\n");

	strncpy(anly,buf,sizeof(int));
	judge = command_analy_flag(anly);
	switch(judge)
	{
		case 1:
		case 2:
		case 3:
		case 4:
			memcpy(&log,buf,sizeof(loginnode));
			break;
		case 5:
		case 6:
		case 10:
		case 11:
			memcpy(&inf,buf,sizeof(informationnode));
			break;
		case -1:
			memcpy(&offline,buf,sizeof(downonline));
			break;
		case 7:
		case 8:
		case 9:
			memcpy(&fid,buf,sizeof(friendnode));
			break;
		case 12:
		case 13:
		case 23:
			memcpy(&msg,buf,sizeof(msgnode));
			break;
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
			memcpy(&grp,buf,sizeof(groupnode));
		case 24:
		case 25:
			memcpy(&his,buf,sizeof(historynode));
			break;
		case 26:
			memcpy( &file,buf,sizeof(filenode));break;

	}
	printf( " judgeee = %d\n",judge);   
	switch(judge)
	{
		case -1:offline_persistence(offline);break;
		case 1: Account_login_persistence(log,fd);break;
		case 2: Account_resgine_persistence(log,fd);break;
		case 3: Account_updatapassword_persistence(log,fd);break;
		case 4: Account_foundpassword_persistence(log,fd);break;
		case 5: Modity_information_persistence(inf,fd);break;
		case 6: View_information_persistence(inf,fd);break;
		case 7:friend_add_send_persistence(fid,fd);break;
		case 8:friend_add_deal_persistence(fid);break;
		case 9:friend_del_persistence(fid,fd);break;
		case 10:Friend_all_view_persistence(inf,fd);break;
		case 11:Friend_view_persistence(inf,fd);break;
		case 12:Pravite_chat_send_persistence(msg,fd);break;
		case 13:block_message_persistence(msg,fd);break;
		case 14:create_group_persistence(grp,fd);break;
		case 15:join_group_persistence(grp,fd);break;
		case 16:group_add_persistence(grp,fd);break;
		case 17:Set_administrator_persistence(grp,fd);break;
		case 18:exit_group_persistence(grp);break;
		case 19:Kicking_people_persistence(grp);break;
		case 20:View_add_group_persistence(grp,fd);break;
		case 21:View_group_members(grp,fd);break;
		case 22:Dissolution_group_persistence(grp,fd);break;
		case 23:chat_group_persistence(msg,fd);break;
		case 24:View_chat_friend_history(his,fd);break;
		case 25:View_chat_group_history(his,fd);break;
		case 26:File_transfer_persistence(file,fd);break;
	}

	memset(buf,0,sizeof(buf));
}

int Account_resgine_persistence(loginnode log,int fd)
{
	int re;
	char buf[1024];

	strcpy(log.online,"0");
	int flag = 1;
	printf( "注册\n");
	char data[1000];
	sprintf(data,"insert into login values('%d','%s','%s','%s','%s','%s','%s')",NULL,log.account,log.password,log.name,log.phonenumber,log.friendname,log.online);
	
	printf("log.account = %s\n",log.account);
	if(mysql_query(&mysql,data))
	{
		printf( "false\n");
		flag = 0;
	}
	log.result = flag;
	printf( "log.result = %d\n",log.result);

        memset(buf,0,1024);    //初始化
        memcpy(buf,&log,sizeof(loginnode));    //将结构体的内容转为字符串
        if((re = (send(fd,buf,1024,0))) < 0)  printf( "错误\n"); 	

	printf( "log1\n");
	return flag;
}
int Account_login_persistence(loginnode log,int fd)    //登录
{
	char buf[1024];
	char data[1024];

	printf( "尝试登录\n");
	//登录时更新状态
	char temp[1000];
	sprintf(temp,"update login set online = 0 where id = '%d'",log.id);
	mysql_query(&mysql,temp);  //执行成功返回false  ,失败返回true
	//检查离线消息

	int re;
        int flag = 0;
        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	strcpy(log.online,"1");

	sprintf(data,"update login set online = '%s' where account = '%s'",log.online,log.account);
	mysql_query(&mysql,data);  //执行成功返回false  ,失败返回true
        
	mysql_query(&mysql,"select account,password,id from login ");
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	//if(result == NULL)   printf( "111\n");
	while(row = mysql_fetch_row(result))
	{
		if(strcmp(log.account,row[0]) == 0 && strcmp(log.password,row[1]) == 0)  //在数据库里查找是否 有账号和密码
		{
			flag = 1;
			log.id = atoi(row[2]);
			break;
		}
	}


	if(flag == 1)
	{
		online_node_t *temp = (online_node_t *)malloc(sizeof(online_node_t));
		temp->fd = fd;
		strcpy(temp->account,log.account);
		temp->id = log.id;
		List_AddHead(head,temp);
	}
	log.result = flag;


  memset(buf,0,1024);    //初始化
  memcpy(buf,&log,sizeof(loginnode));    //将结构体的内容转为字符串
	
  if((re = (send(fd,buf,1024,0))) < 0)  printf( "错误\n"); 	

	send_offline_friend(log.account,fd);
	del_send_offline_friend(log.account,fd);
	send_offline_noc(log.account,fd);
	del_send_offline_noc(log.account,fd);
	send_offline_chat(log.account,fd);
	del_send_offline_chat(log.account,fd);
	send_offline_group(log.account,fd);
	del_send_offline_group_chat(log.account,fd);
	send_offline_group_add(log.account,fd);
 del_offline_group_add( log.account,fd);
	send_offline_group_noc(log.account,fd);
	del_send_offline_group_noc(log.account,fd); 


	return flag;
}
int Account_updatapassword_persistence(loginnode log,int fd)
{        
	char data[1000];
	int flag = 0;
	int re;
	char buf[1024];
        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
        mysql_query(&mysql,"select account,password from login ");
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	if(NULL == result)   printf( "111\n");
	while(row = mysql_fetch_row(result))
	{
		printf( "%s\t",row[0]);
		if(strcmp(log.account,row[0]) == 0 && strcmp(log.password,row[1]) == 0)  //如果账号和密码匹配
		{
			sprintf(data,"update login set password = '%s' where account = '%s'",log.updata_or_foundpassword,log.account);
			mysql_query(&mysql,data);  //执行成功返回false  ,失败返回true
			flag = 1;
		}
	}

	log.result = flag;
        memset(buf,0,1024);    //初始化
        memcpy(buf,&log,sizeof(loginnode));    //将结构体的内容转为字符串
        if((re = (send(fd,buf,1024,0))) < 0)  printf( "错误\n"); 	

	printf( "updata flag = %d\n",flag);
	return flag;

}
int Account_foundpassword_persistence(loginnode log,int fd)
{
	int re = 0;
        int flag = 0;
        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	char buf[1024];
        mysql_query(&mysql,"select account,password,phonenumber,friendname from login ");
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	while(row = mysql_fetch_row(result))
	{
		printf( "%s\t",row[0]);
		if(strcmp(log.account,row[0]) == 0)  //在数据库里查找是否 有账号和密码
		{
			if(strcmp(log.friendname,row[3]) == 0 ||strcmp(log.phonenumber,row[2]) == 0)
			{
				flag = 1;
				strcpy(log.updata_or_foundpassword,row[1]);
				break;
			}
		}
	}
	log.result = flag;
        memset(buf,0,1024);    //初始化
        memcpy(buf,&log,sizeof(loginnode));    //将结构体的内容转为字符串
        if((re = (send(fd,buf,1024,0))) < 0)  printf( "错误\n"); 	

	printf( "re = %d\n",re);
	printf( "password = %s\n",log.updata_or_foundpassword);
	return flag;
}
static void do_write(int epfd,int fd,char *buf)
{

}//default charset = utf8; 
void my_err(const char * err_string,int line)
{
	fprintf(stderr,"line : %d ",line);
	perror(err_string);
}

/*
int judge_account_byID(loginnode log)
{
	int flag = 0;
	MYSQL_FIELD * field;
	MYSQL_ROW row;
	MYSQL_RES *result = NULL;
	mysql_query(&mysql,"select ID from login ");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	//while(field = mysql_fetch_field(result))
	{
//		if(strcmp("ID",field->name) == 0)
		{
			while(row = mysql_fetch_row(result))
			{
				printf( "%s\t",row[0]);
				if(strcmp(log.number,row[0]) == 0)
				{
					flag = 1;     //已经存在这个 id
				}
			}
		}
	}

	return flag;

}
*/
void send_data(int conn_fd,const char *string)
{
	int ret;
	if((ret = (send(conn_fd,string,1024,0))) < 0)   my_err("send",__LINE__);
//	printf( "re3t = %d\n",ret);
//	printf( "string =default charset = utf8;  %d\n",strlen( string));
}

int command_analy_flag(char a[5])    //用来解析flag
{
        int flag;
        memcpy(&flag,a,sizeof(int));
        return flag;
}
int Modity_information_persistence(informationnode inf,int fd)
{
	// inf.loginid 为 login 中的Id
	//先 根据 ID 匹配将 login 中的name 修改
	//在 将infor  中的数据进行修改
	// 首先 判断 inf 表中 是否存在这个数据,若 不存在这直接插入 ,若存在则 进行更新
	
	printf( "id = %d\n",inf.id);

	loginnode log;
	char buf[1024];
	char temp[1000];
	int re = 0;
        int flag = 0;
        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	//memcpy(chstid,&inf.id,sizeof(int));

	//将 login 中的 name 更新
	sprintf(temp,"update login set name = '%s' where id = %d",inf.name,inf.id);
	mysql_query(&mysql,temp);  //执行成功返回false  ,失败返回true
        
	mysql_query(&mysql,"select loginID from information ");
	
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	if(result == NULL)   //如果这个表一个人也没有
	{
	//	printf( "111\n");
		sprintf(temp,"insert into information values('%d','%s','%s','%s','%s','%s','%s','%d')",NULL,inf.name,inf.sex,inf.data,inf.address,inf.constellation,inf.email,inf.id);
		mysql_query(&mysql,temp);  //执行成功返回false  ,失败返回true
	}
	else    //若 flag = 1,则 更新,否者 插入
	{
		while(row = mysql_fetch_row(result))
		{
			printf( "%s\t",row[0]);
			if(inf.id == atoi(row[0]))  //在 inf 数据库里查找是否 有这个人
			{
				flag = 1;
				break;
			}
		}
		printf( "infor flag = %d\n",flag);
		if(flag == 1)
		{
			sprintf(temp,"update information set name = '%s',sex = '%s',data = '%s',address = '%s',constellation = '%s',email = '%s' where loginID = %d",inf.name,inf.sex,inf.data,inf.address,inf.constellation,inf.email,inf.id);
			mysql_query(&mysql,temp);  //执行成功返回false  ,失败返回true
		}
		else
		{
			sprintf(temp,"insert into information values('%d','%s','%s','%s','%s','%s','%s','%d')",NULL,inf.name,inf.sex,inf.data,inf.address,inf.constellation,inf.email,inf.id);
			mysql_query(&mysql,temp);  //执行成功返回false  ,失败返回true
		}
	}
	
	inf.flag = 5;
	inf.result = 1;
        memset(buf,0,1024);    //初始化
        memcpy(buf,&inf,sizeof(informationnode));    //将结构体的内容转为字符串
        if((re = (send(fd,buf,1024,0))) < 0)  printf( "错误\n"); 	
	
	//更新的反返回值 只能是成功
}

int View_information_persistence(informationnode inf,int fd)
{
	char buf[1024];
	char temp[1000];
	int re = 0;
        int flag = 0;
        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	mysql_query(&mysql,"select loginID,name,sex,data,address,constellation,email from information ");
	
	//if(result == NULL) printf( "information 中 要找的信息为空\n");
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	while(row = mysql_fetch_row(result))
	{
		if(inf.id == atoi(row[0]))
		{
			strcpy(inf.name,row[1]);
			strcpy(inf.sex,row[2]);
			strcpy(inf.data,row[3]);
			strcpy(inf.address,row[4]);
			strcpy(inf.constellation,row[5]);
			strcpy(inf.email,row[6]);
		}
	}

	inf.result = 1;
        memset(buf,0,1024);    //初始化
        memcpy(buf,&inf,sizeof(informationnode));    //将结构体的内容转为字符串
        if((re = (send(fd,buf,1024,0))) < 0)  printf( "错误\n"); 	

}
int offline_persistence(downonline offline)  //下线时 ,更新 状态
{
	char temp[1000];
//	printf( "id = %d\n",offline.id);
	sprintf(temp,"update login set online = 0 where id = '%d'",offline.id);
	mysql_query(&mysql,temp);  //执行成功返回false  ,失败返回true
}

int friend_add_send_persistence(friendnode fid,int fd)
{

	char buf[1024];
	char data[1024];

	int re = 0;
	int flag = 0;
	int line = 0;

	
	//先 根据账号 在数据库 里面找到 接受者 的id; 和是否在线
	
	fid.sendfd = fd;
        online_node_t *curpos;

        List_ForEach(head,curpos)
        {
                if(strcmp(curpos->account,fid.acceptaccount) == 0)
                {
                        flag = 1;
                        fid.acceptfd = curpos->fd;
                        fid.acceptid = curpos->id;
                        break;
                }
        }
	
        fid.flag = 7;
	if(flag == 1)
	{
		memset(buf,0,1024);    //初始化
       	        memcpy(buf,&fid,sizeof(friendnode));    //将结构体的内容转为字符串
       		if((re = (send(fid.acceptfd,buf,1024,0))) < 0)  printf( "错误\n"); 	
	}
	else
	{

		sprintf(data,"insert into offline_friend (sendaccount,acceptaccount)values('%s','%s')",fid.sendaccount,fid.acceptaccount);
	//	printf( "data = %s\n",data);
	//	printf( "fid.acceptaccount = %s\n",fid.acceptaccount);
		if(mysql_query(&mysql,data))    printf( "false\n");  //执行成功返回false  ,失败返回true
	}

        /*online_node_t *curpos;

        List_ForEach(head,curpos)
        {
                if(strcmp(curpos->account,fid.acceptaccount) == 0)
                {
                        flag = 1;
                        fid.acceptfd = curpos->fd;
                        fid.acceptid = curpos->id;
                        break;
                }
        }
//	else  (不在线)   保存在本地
	{

	}


*/

}
int friend_add_deal_persistence(friendnode fid)
{

	int flag = 0;
	char temp[1000];
	char buf [1024];
	int re;
	noticenode noc;
        
	online_node_t *curpos;
	
	printf( "fid send = %s\n",fid.sendaccount);
        List_ForEach(head,curpos)
        {
                if(strcmp(curpos->account,fid.sendaccount) == 0)
                {
                        flag = 1;
                        fid.sendfd = curpos->fd;
                        fid.sendid = curpos->id;
                        break;
                }
        }
	//  处理好友
	if(flag == 1)    
	{
		printf( "在线\n");
		if(fid.result == 1)
		{
			sprintf(temp,"insert into friend values('%d','%d')",fid.sendid,fid.acceptid);
			mysql_query(&mysql,temp);  //执行成功返回false  ,失败返回true
			sprintf(buf,"%s 对方同意了您的好友请求!\n",fid.acceptaccount);
	
			strcpy(noc.noc,buf);
			noc.flag = 0;
		
			memset(buf,0,1024);    //初始化
       		        memcpy(buf,&noc,sizeof(noticenode));    //将结构体的内容转为字符串
	//       		printf( "aaaaaaaaaaaaaaa\n");
			if((re = (send(fid.sendfd,buf,1024,0))) < 0)  printf( "错误\n"); 	
	
			
		}
		else 
		{
			sprintf(buf,"%s 对方拒绝了您的好友请求!\n",fid.acceptaccount);
	
			strcpy(noc.noc,buf);
			noc.flag = 0;
		
			memset(buf,0,1024);    //初始化
	       	        memcpy(buf,&noc,sizeof(noticenode));    //将结构体的内容转为字符串
	/*		printf( "sendfd = %d",fid.sendfd);
			printf( "bbbbbbbbbbbbbbbbbbbbbbbb\n");*/
			if((re = (send(fid.sendfd,buf,1024,0))) < 0)  printf( "错误\n"); 	
		}
	}
	else
	{
		printf( "离线\n");
		if(fid.result == 1)
		{
			sprintf(temp,"insert into friend values('%d','%d')",fid.sendid,fid.acceptid);
			if(mysql_query(&mysql,temp))     printf( "false\n");  //执行成功返回false  ,失败返回true
			char tmp[] = "对方同意了你的请求";
			sprintf(buf,"insert into offline_noc (sendaccount,acceptaccount,noc) values ('%s','%s','%s')",fid.sendaccount,fid.acceptaccount,tmp);
			if(mysql_query(&mysql,buf))     printf( "false\n");  //执行成功返回false  ,失败返回true
		}
		else
		{
			char tmp[] = "对方拒绝了你的请求";
			sprintf(buf,"insert into offline_noc (sendaccount,acceptaccount,noc) values ('%s','%s','%s')",fid.sendaccount,fid.acceptaccount,tmp);
			if(mysql_query(&mysql,buf))     printf( "false\n");  //执行成功返回false  ,失败返回true
		}
	}


}
int friend_del_persistence(friendnode fid,int fd)
{	
	char buf[1024];
	char temp[11024];
	char data[1024];

	int re = 0;
        int flag = 0;
        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	int id2;
	mysql_query(&mysql,"select account,id from login ");
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	while(row = mysql_fetch_row(result))
	{
		if(strcmp(row[0],fid.acceptaccount) == 0)
		{
			id2 = atoi(row[1]);
			flag = 1;
		}
	}
	printf( "flag = %d\n",flag);
	printf( "id2 = %d   id1 = %d\n",id2,fid.sendid);

	if(flag == 0)  return 0;
	//mysql_query(&mysql,"select *from friend");
	
	//if(result == NULL) printf( "information 中 要找的信息为空\n");
       // result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	sprintf(temp,"delete from friend where (friend1 = '%d' && friend2 = '%d' || friend1 = '%d' && friend2 = '%d')",fid.sendid,id2,id2,fid.sendid);
	if(mysql_query(&mysql,temp))
	{
		printf( "false\n");
	}

	return 0;
}
int Friend_all_view_persistence(informationnode inf,int conn_fd)
{
	char temp[1024];
	int re = 0;
	char data[1024];
	char buf[1024];
	
        MYSQL_FIELD * field;
        MYSQL_ROW row;
	MYSQL_ROW row1;
        MYSQL_RES *result_id = NULL;
        MYSQL_RES *result_account = NULL;
	mysql_query(&mysql,"select friend1,friend2 from friend");
        result_id = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	if(result_id == NULL)  
	{
			printf("结果集 为空, 985\n");
			return 0;
	}

	while(row = mysql_fetch_row(result_id))
	{
		if(row == NULL)   return 0;
		memset(data,0,sizeof(data));
		if(atoi(row[0]) == inf.id)
		{
			inf.flag = 9;
			sprintf(data,"select account,name,online from login where id = %d",atoi(row[1]));
			mysql_query(&mysql,data);
			result_account = mysql_store_result(&mysql);
			row1 = mysql_fetch_row(result_account);
			printf("asdsdasdsada\n");
			strcpy(inf.account,row1[0]);
			strcpy(inf.name,row1[1]);
			inf.line = atoi(row1[2]);
       			
			memset(buf,0,1024);    //初始化
      	 		memcpy(buf,&inf,sizeof(informationnode));    //将结构体的内容转为字符串
			if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n"); 					
		}
		else if(atoi(row[1]) == inf.id)
		{
			inf.flag = 9;
			sprintf(data,"select account,name,online from login where id = %d",atoi(row[0]));
			mysql_query(&mysql,data);
			result_account = mysql_store_result(&mysql);

			row1 = mysql_fetch_row(result_account);
		
			strcpy(inf.account,row1[0]);
			strcpy(inf.name,row1[1]);
			inf.line = atoi(row1[2]);
       			
			memset(buf,0,1024);    //初始化
      	 		memcpy(buf,&inf,sizeof(informationnode));    //将结构体的内容转为字符串
			if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n"); 					
		}
	}
}
int Friend_view_persistence(informationnode inf,int conn_fd)
{
	char temp[1024];
	int re = 0;
	char data[1024];
	char buf[1024];
	
        MYSQL_FIELD * field;
        MYSQL_ROW row;
	MYSQL_ROW row1;
        MYSQL_RES *result_id = NULL;
        MYSQL_RES *result_account = NULL;
	mysql_query(&mysql,"select friend1,friend2 from friend");
        result_id = mysql_store_result(&mysql);//将查询的全部结果读取到客户端


	while(row = mysql_fetch_row(result_id))
	{
		memset(data,0,sizeof(data));

		if(atoi(row[0]) == inf.id)
		{
			inf.flag = 10;
			sprintf(data,"select account,name,online from login where id = %d",atoi(row[1]));
			mysql_query(&mysql,data);
			result_account = mysql_store_result(&mysql);
			row1 = mysql_fetch_row(result_account);
			
			strcpy(inf.account,row1[0]);
			strcpy(inf.name,row1[1]);
			inf.line = atoi(row1[2]);
       			if(inf.line)
			{
				memset(buf,0,1024);    //初始化
      	 			memcpy(buf,&inf,sizeof(informationnode));    //将结构体的内容转为字符串
				if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n"); 					
				printf( "re = %d\n",re);
			}

		}
		else if(atoi(row[1]) == inf.id)
		{
			inf.flag = 10;
			sprintf(data,"select account,name,online from login where id = %d",atoi(row[0]));
			mysql_query(&mysql,data);
			result_account = mysql_store_result(&mysql);

			row1 = mysql_fetch_row(result_account);
		
			strcpy(inf.account,row1[0]);
			strcpy(inf.name,row1[1]);
			inf.line = atoi(row1[2]);
       			if(inf.line)	
			{
			
				memset(buf,0,1024);    //初始化
      	 			memcpy(buf,&inf,sizeof(informationnode));    //将结构体的内容转为字符串
				if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n"); 					
			}
		}
	}
}
int Pravite_chat_send_persistence(msgnode msg,int conn_fd)
{
	char buf[1024];
	char data[1024];
	
	int  re;
	int flag = 0;
        online_node_t *curpos;
		int friend1 = 0,friend2 = 0;

		friend1 = Find_idbyaccount(msg.sendaccount);
		friend2 = Find_idbyaccount(msg.acceptaccount);
	printf("id1 = %d \n  id2 = %d  \n",friend1,friend2);


int judge = judge_is_friend(friend1,friend2);
if(judge == 0)  return 0;
        List_ForEach(head,curpos)
        {
                if(strcmp(curpos->account,msg.acceptaccount) == 0)
                {
                        flag = 1;
												printf("在线\n");
                        msg.acceptfd = curpos->fd;
                        msg.acceptid = curpos->id;
                        break;
                }
        }
	if(is_block(msg.sendid,msg.acceptid))  return 0;

//	printf( "flag = = = %d\n",flag);
//	printf( "账号 %s\n",msg.acceptaccount);

	if(flag == 0)
	{


		sprintf(data,"insert into offline_chat values(%d,'%s','%s','%s')",NULL,msg.sendaccount,msg.acceptaccount,msg.msg);
		
		printf( "离离线\n");
		mysql_query(&mysql,data);
		//发送对方不在线, 给发送者	
	}
	else
	{
//		printf( "qweqweqweqwewqeqe\n");
		// 在数据库 中 通过id 找到 name
    		MYSQL_FIELD * field;
    	   	MYSQL_ROW row;
      	 	MYSQL_RES *result = NULL;
		sprintf(data,"select name from login where id = %d",msg.sendid);
		mysql_query(&mysql,data);
		result = mysql_store_result(&mysql);
		row = mysql_fetch_row(result);
		strcpy(msg.sendname,row[0]);
		memset(data,0,sizeof(data));

		sprintf(data,"select name from login where id = %d",msg.acceptid);
printf("  send = %s     ,,,,,accept = %s\n",msg.sendaccount,msg.acceptaccount);
		mysql_query(&mysql,data);
		result = NULL;
		result = mysql_store_result(&mysql);
		row = mysql_fetch_row(result);
		strcpy(msg.acceptname,row[0]);

		// 在数据库通过 id 找到昵称

		//将消息保存下来
		memset(data,0,sizeof(data));

		printf("asdasdasdas\n");
		sprintf(data,"insert into friend_history values('%d','%s','%s','%s','%s','%s')",NULL,msg.sendaccount,msg.sendname,msg.acceptaccount,msg.acceptname,msg.msg);
		mysql_query(&mysql,data);
		//将消息发送给接受者
		msg.flag = 11;
		memset(buf,0,1024);    //初始化
      	 	memcpy(buf,&msg,sizeof(msgnode));    //将结构体的内容转为字符串
		if((re = (send(msg.acceptfd,buf,1024,0))) < 0)  printf( "错误\n");
	}

	return 0;

}

int block_message_persistence(msgnode msg,int conn_fd)
{
	char data[1024];

    	MYSQL_FIELD * field;
       	MYSQL_ROW row;
     	MYSQL_RES *result = NULL;
	sprintf(data,"select id from login where account = '%s'",msg.acceptaccount);
	if(mysql_query(&mysql,data))     printf( "false\n");
	result = mysql_store_result(&mysql);
	
/*	//问题
 *	if(result == NULL)
	{
		printf( "无此账号\n");
		return 0;
	}
	if(row == NULL)   printf( "无此账号\n");

	*/
	row = mysql_fetch_row(result);
	int id2 = atoi(row[0]);
//	printf( "id2 = %d\n",id2);
	memset(data,0,sizeof(data));

	sprintf(data,"insert into shield values('%d','%d')",msg.sendid,id2);
	mysql_query(&mysql,data);  //执行成功返回false  ,失败返回true
	
	return 0;
}

int is_block(int sendid,int acceptid)
{


        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	mysql_query(&mysql,"select user1,user2 from shield ");
        result = mysql_store_result(&mysql);    
	int flag = 0;
	while(row = mysql_fetch_row(result))
	{
		if(acceptid == atoi(row[0]) && sendid == atoi(row[1]))  flag = 1;
	}

	return flag;

}
int create_group_persistence(groupnode grp,int conn_fd)
{
	char data[1024];
	int group_id;
//	printf( "name = %s\n",grp.group_name);
//	printf( "account = %s\n",grp.group_account);

	sprintf(data,"insert into group_information values('%d','%s','%s',%d,%d)",NULL,grp.group_name,grp.group_account,1,grp.id);
	if(mysql_query(&mysql,data))     //;  //执行成功返回false  ,失败返回true
	{
		printf( "false\n");
	}
	//在 群信息中 通过 群账号 找到群 id,
        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	MYSQL_RES *result2 = NULL;

	memset(data,0,sizeof(data));

	sprintf( data,"select group_id from group_information where  group_account = '%s'",grp.group_account);
	mysql_query(&mysql,data);
        result = mysql_store_result(&mysql);    
	row = mysql_fetch_row(result);
	group_id = atoi(row[0]);
	//在 用户信息中 通过  用户 id 找到用户名字;
	memset(data,0,sizeof(data));
	
	sprintf(data,"select name from login where id = %d",grp.id);
	mysql_query(&mysql,data);
        result = mysql_store_result(&mysql);    
	row = mysql_fetch_row(result);
	strcpy(grp.user_name,row[0]);
	memset(data,0,sizeof(data));

	sprintf(data,"insert into group_members values(%d,'%s','%s','%s','%s',1,0)",group_id,grp.group_account,grp.group_name,grp.user_name,grp.user_account);
	mysql_query(&mysql,data);


	return 0;
}
int join_group_persistence(groupnode grp,int conn_fd)
{
	//先检查是否 有这个群
	if(!is_group(grp)) return 0;
	char data[1024];
	char buf[1024];
	int flag = 0;
	int re;
	char account[SIZE];   //用来保存群主的账号
	int fd; //用来保存群主的 套接字
	int id;  //用来保存群主的 id 主键
	//获取 申请人 的昵称

	grp.sendfd = conn_fd;
	
	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select name from login where id = '%d'",grp.id);
	mysql_query(&mysql,data);
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	row = mysql_fetch_row(result);

	strcpy(grp.user_name,row[0]);
	//在群 的 账号的信息中  找到 群主的id,和群的id ,还有群的 昵称
	result = NULL;
	memset(row,0,sizeof(MYSQL_ROW));
	memset(data,0,sizeof(data));
	sprintf(data,"select id,group_id,group_name from group_information  where group_account = '%s'",grp.group_account);
	mysql_query(&mysql,data);
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	row = mysql_fetch_row(result);
	id = atoi(row[0]);

	grp.group_id = atoi(row[1]);
	strcpy(grp.group_name,row[2]);
	//通过群主的 id  找到 群主的账号
	result = NULL;
	memset(row,0,sizeof(MYSQL_ROW));
	memset(data,0,sizeof(data));
	sprintf(data,"select account from login  where id = '%d'",id);
	mysql_query(&mysql,data);
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	row = mysql_fetch_row(result);
	
	strcpy(account,row[0]);

	//通过群主的账号 找到他的套接字
	online_node_t *curpos;

        List_ForEach(head,curpos)
        {
                if(strcmp(curpos->account,account) == 0)
                {
                        flag = 1;
												printf("群主在线\n");
                        fd = curpos->fd;
                        break;
                }
        }
	//将消息发送 给群主
	if(flag == 1)
	{
		grp.flag = 12;
		printf("发送给群主");
		memset(buf,0,1024);    //初始化
	      	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串
		if((re = (send(fd,buf,1024,0))) < 0)  printf( "错误\n");
	}
	else    //保存在数据库
	{
		memset(data,0,sizeof(data));
		sprintf(data,"insert into offline_group_add values(%d,'%s','%s','%s','%s')",NULL,grp.group_account,grp.user_account,grp.user_name,account);
		if(mysql_query(&mysql,data))   printf( "false 1299 \n"); 	

	}
	return 0;
}

int is_group(groupnode grp)
{
	char data[1024];
        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	printf( "群的账号 :%s\n",grp.group_account);
	sprintf(data,"select id from group_information where group_account = '%s'",grp.group_account);
	mysql_query(&mysql,data);
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	row = mysql_fetch_row(result);
	
	printf( "是否存在:");
	if(row == NULL)
	{
		printf( "NO\n");
		return 0;
	}
	else
	{
		printf( "YES\n");
		return 1;
	}
}
int group_add_persistence(groupnode grp,int fd)
{
	int flag = 0;

	noticenode noc;
	noc.flag = 0;

	char data[1024];
        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL; 
	int re;
	// 如果在线  就获取 send 的套接字
	
  	online_node_t *curpos;

	List_ForEach(head,curpos)
        {
		if(strcmp(curpos->account,grp.user_account) == 0)
		{
			flag = 1;
			grp.sendfd = curpos->fd;
			grp.id = curpos->id;
		break;
		}
        }


	if(flag == 0)
	{
		memset(data,0,sizeof(data));
		char noc[100];

		if(grp.result == 1)	strcpy(noc,"对方已接受您入群");
		else	strcpy(noc,"对方已拒绝您入群");

		printf( "account = %s\n",grp.group_account);
		sprintf(data,"insert into offline_group_noc values(%d,'%s','%s','%s')",NULL,grp.group_account,grp.user_account,noc);
		if(mysql_query(&mysql,data))
		{
			fprintf(stderr,"%s\n",mysql_error(&mysql));
		};
		return 0;
	}
	//
	//
	//
	//
	//
	if(grp.result == 0)
	{
		sprintf(data," %s 对方已拒绝您入群 ",grp.group_name);

                strcpy(noc.noc,data);
                noc.flag = 0;

                memset(data,0,1024);    //初始化
                memcpy(data,&noc,sizeof(noticenode));    //将结构体的内容转为字符串
                if((re = (send(grp.sendfd,data,1024,0))) < 0)  printf( "123123错误\n");

		return 0;
	}

	//	printf( "result = %d\n",grp.result);
	sprintf(data,"insert into group_members values(%d,'%s','%s','%s','%s',0,0)",grp.group_id,grp.group_account,grp.group_name,grp.user_name,grp.user_account);
	if(mysql_query(&mysql,data)) printf( "false\n");     //;  //执行成功返回false  ,失败返回true
	
	memset(data,0,sizeof(data));
	
	sprintf(data,"select group_members from group_information where group_account = '%s'",grp.group_account);
	mysql_query(&mysql,data);
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	row = mysql_fetch_row(result);
	if(NULL == result)   printf( "asdadasd\n");
	memset(data,0,sizeof(data));
	sprintf(data,"update group_information set group_members = %d where group_account = '%s'",atoi(row[0])+1,grp.group_account);
	mysql_query(&mysql,data);

	sprintf(data," %s 对方同意您入群 ",grp.group_name);

        strcpy(noc.noc,data);
        noc.flag = 0;

        memset(data,0,1024);    //初始化
        memcpy(data,&noc,sizeof(noticenode));    //将结构体的内容转为字符串
        if((re = (send(grp.sendfd,data,1024,0))) < 0)  printf( "错误\n");

	return 0;
}
int Set_administrator_persistence(groupnode grp,int conn_fd)   //只能由群主设置
{
	//判断这个群是否存在,并且判断这个人是否为群主
	
	char data[1024];

        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	MYSQL_RES *result1 = NULL;
	sprintf(data,"select id from group_information where group_account = '%s'",grp.group_account);
	mysql_query(&mysql,data);
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	row = mysql_fetch_row(result);
	
	if(result != NULL)
	{
		if(grp.id == atoi(row[0]))   //如果这个人是群主的话
		{
			printf( "account = %s\n",grp.user_account);
			sprintf(data,"update group_members set administartor = 1 where user_account = '%s'",grp.user_account);
			mysql_query(&mysql,data);
		}
	}


}
int exit_group_persistence(groupnode grp)
{
	char data[1024];
        MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
int fd;
	sprintf(data,"delete from group_members where (group_account = '%s' && user_account = '%s') ",grp.group_account,grp.user_account);
	mysql_query(&mysql,data);
// 群主退出 解散群,
memset(data,0,sizeof(data));
grp.id = Find_idbyaccount(grp.user_account);

Dissolution_group_persistence(grp,fd);

//普通成员退群 通知群主
	//更新群人数

	memset(data,0,sizeof(data));
	
	sprintf(data,"select group_members from group_information where group_account = '%s'",grp.group_account);
	mysql_query(&mysql,data);
        result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
if(result == NULL)   return 0;
	row = mysql_fetch_row(result);

if(row == NULL)    return  0;
	memset(data,0,sizeof(data));
	sprintf(data,"update group_information set group_members = %d where group_account = '%s'",atoi(row[0])-1,grp.group_account);
	mysql_query(&mysql,data);

}


int Kicking_people_persistence(groupnode grp)
{
	char data[1024];
	//判断是否为群主 或 管理员 
        int flag = 0;

	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select lord,administartor from group_members where (group_account = '%s' && user_account = '%s')",grp.group_account,grp.administartor_account);
	mysql_query(&mysql,data);
        
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	row = mysql_fetch_row(result);
	
	if(atoi(row[0]) || atoi(row[1]))   flag = 1;

	if(flag == 1)    //若为群主或 管理员  
	{
	/*	sprintf(data,"delete from group_members where (group_account = '%s' && user_account = '%s') ",grp.group_account,grp.user_account);
		mysql_query(&mysql,data);
		memset(data,0,sizeof(data));
		
		sprintf(data,"select group_members from group_information where group_account = '%s'",grp.group_account);
		mysql_query(&mysql,data);
       		
		result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
		row = mysql_fetch_row(result);
	
		memset(data,0,sizeof(data));
		sprintf(data,"update group_information set group_members = %d where group_account = '%s'",atoi(row[0])-1,grp.group_account);
		mysql_query(&mysql,data);
	*/
		exit_group_persistence(grp);
	}
}
int View_add_group_persistence(groupnode  grp,int conn_fd)
{

	char data[1024];
	char buf[1024];
	int re;

	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select group_account,group_name from group_members where user_account = '%s'",grp.user_account);

	mysql_query(&mysql,data);
        
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	while(row = mysql_fetch_row(result))
	{
		strcpy(grp.group_account,row[0]);
		strcpy(grp.group_name,row[1]);
	printf("aaaaaaa\n");
		grp.flag = 13;
		memset(buf,0,1024);    //初始化
      	 	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串
		if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
	}

	return 0;
}
int View_group_members(groupnode grp,int conn_fd)
{
	char data[1024];
	char buf[1024];
	int re;
	//首先判断是否为群成员
	
	
	
	//
	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select user_name,user_account from group_members where group_account = '%s'",grp.group_account);

	mysql_query(&mysql,data);
        
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	while(row = mysql_fetch_row(result))
	{
		strcpy(grp.user_name,row[0]);
		strcpy(grp.user_account,row[1]);
		//printf("name = %s,account = %s\n",row[0],row[1]);
		grp.flag = 14;
		memset(buf,0,1024);    //初始化
      	 	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串
		if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
	}

}
int Dissolution_group_persistence(groupnode grp,int conn_fd)
{
	char data[1024];
	int flag = 0;    //表示这个人是不是群主
	int group_id;
	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	//判断这个 人是不是群主
	sprintf(data,"select id from group_information where group_account = '%s'",grp.group_account);
	mysql_query(&mysql,data);
	
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	row = mysql_fetch_row(result);
	if(grp.id == atoi(row[0]))   flag = 1;

	memset(data,0,sizeof(data));

	if(flag == 1)
	{
		//先通过群账号 获得 群 id 并删除这个  数据
		sprintf(data,"select group_id from group_information where group_account = '%s'",grp.group_account);
		mysql_query(&mysql,data);
	        
		result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
		if(result == NULL)   return 0;
		row = mysql_fetch_row(result);

		if(row == NULL)   return 0;
		group_id = atoi(row[0]);
	
		//删除这条数据
	
		memset(data,0,sizeof(data));
		sprintf(data,"delete from group_information where group_id = %d ",atoi(row[0]));
		mysql_query(&mysql,data);
	
		//在通过 id 在member 删除所有成员
		
		memset(data,0,sizeof(data));
		sprintf(data,"delete from group_members where group_id = %d ",atoi(row[0]));
		mysql_query(&mysql,data);
	
	}

	return 0;
}
int chat_group_persistence(msgnode msg,int fd)
{
	char buf[1024];
	char data[1024];
	int flag = 0;
	int re;
	//通过 用户 sendid 找到 对应的 昵称

	//遍历 群 里所有的成员, 然后 判断 是否 在线 若 不在线 存入离线,若存在  直接发送

	//通过 群id  找到所有群成员
	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
        MYSQL_RES *result1 = NULL;
	//获取 发送者 的名字
	sprintf(data,"select name from login where id = %d",msg.sendid);
	mysql_query(&mysql,data);
	result1 = mysql_store_result(&mysql);//将查询的全部结果读取到客户端
	row = mysql_fetch_row(result1);
	strcpy(msg.sendname,row[0]);

	memset(data,0,sizeof(data));
	sprintf(data,"insert into group_history values(%d,'%s','%s','%s','%s')",NULL,msg.group_account,msg.sendaccount,msg.sendname,msg.msg);
	mysql_query(&mysql,data);
	//  
	memset(data,0,sizeof(data));
	sprintf(data,"select user_account from group_members where group_account = '%s'",msg.group_account);
	mysql_query(&mysql,data);
	
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	while(row = mysql_fetch_row(result))
	{
		strcpy(msg.acceptaccount,row[0]);
  	      online_node_t *curpos;

	      List_ForEach(head,curpos)
	      {
			if(strcmp(curpos->account,row[0]) == 0)
			{
				flag = 1;
				msg.acceptfd = curpos->fd;
				msg.acceptid = curpos->id;
			break;
			}
	      }

	      if(flag == 1)
	      {
		    //  printf( "11111\n");
		        msg.flag = 15;
			memset(buf,0,1024);    //初始化
	      	 	memcpy(buf,&msg,sizeof(msgnode));    //将结构体的内容转为字符串
			if((re = (send(msg.acceptfd,buf,1024,0))) < 0)  printf( "错误\n");
	      }
	      else
	      {
		 //     printf( "2222\n");
		        memset(data,0,sizeof(data));
			sprintf(data,"insert into offline_group values(%d,'%s','%s','%s','%s','%s')",NULL,msg.group_account,msg.acceptaccount,msg.sendaccount,msg.sendname,msg.msg);
			if(mysql_query(&mysql,data))   printf( "false\n");
	      }


	}
        

}
int View_chat_friend_history(historynode his,int conn_fd)
{
 	char data[1024];
	char buf[1024];
	int re;
	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;

	printf( "account = %s\nsendaccount = %s\n",his.acceptaccount,his.sendaccount);
	sprintf(data,"select sendname,acceptname,msg from friend_history where (acceptaccount = '%s'  && sendaccount = '%s') || (acceptaccount = '%s'  && sendaccount = '%s' )",his.acceptaccount,his.sendaccount,his.sendaccount,his.acceptaccount );
	
	if(mysql_query(&mysql,data))  printf( "false\n");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	if(result == NULL) 
	 {
	 				printf( " 1710   行   结果集为空\n");
					 printf("无内容\n");
	 				return 0;
	 }
	while(row = mysql_fetch_row(result))
	{
		if(row == NULL) 	break;
		
			strcpy(his.sendname,row[0]);
			strcpy(his.acceptname,row[1]);
			strcpy(his.message,row[2]);
		        his.flag = 16;
			memset(buf,0,1024);    //初始化
	      	 	memcpy(buf,&his,sizeof(historynode));    //将结构体的内容转为字符串
			if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
	}


}
int View_chat_group_history(historynode his,int conn_fd)
{
	char data[1024];
	char buf[1024];
	int re;
	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select sendname,sendaccount,message  from group_history where groupaccount = '%s'",his.groupaccount);
	
	if(mysql_query(&mysql,data))  printf( "false\n");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	if(result == NULL)   printf( " 1478     结果集为空\n");
	while(row = mysql_fetch_row(result))
	{
			strcpy(his.sendname,row[0]);
			strcpy(his.sendaccount,row[1]);
			strcpy(his.message,row[2]);
		        his.flag = 17;
			memset(buf,0,1024);    //初始化
	      	 	memcpy(buf,&his,sizeof(historynode));    //将结构体的内容转为字符串
			if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
	}
	
}
int is_online(char account[SIZE])    //判断是否在线
{

	int flag = 0;

        online_node_t *curpos;

        List_ForEach(head,curpos)
        {
                if(strcmp(curpos->account,account) == 0)
                {
                        flag = 1;
                        break;
                }
        }


	return flag;
}

int insert_offline(char sendaccount[SIZE],char acceptaccount[SIZE],char buf[1024])   //加入到 离线消息中
{
	char data[1024];
	sprintf(data,"insert into offline values('%s','%s','%s')",sendaccount,acceptaccount,buf);

	if(mysql_query(&mysql,data))  printf( "false\n");

}

int send_offline_friend(char account[SIZE],int conn_fd)
{
	char data[1024];
	char buf[1024];

	int re;
	friendnode fid;
	fid.flag = 7;

	MYSQL_FIELD * field;
  MYSQL_ROW row;
	MYSQL_RES *result = NULL;
	sprintf(data,"select sendaccount  from offline_friend where acceptaccount = '%s'",account);
	
	printf( "account = %s\n",account);
	if(mysql_query(&mysql,data))  printf( "false\n");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	while(row = mysql_fetch_row(result))
	{
		if(NULL == row)   break;
		strcpy(fid.sendaccount,row[0]);
		strcpy(fid.acceptaccount,account);
		printf( "fid.sendaccount = %s \n ",fid.sendaccount);
		memset(buf,0,sizeof(buf));
		memcpy(buf,&fid,sizeof(friendnode));
	
		if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");	
	}

}

int send_offline_noc(char account[SIZE],int conn_fd)
{
	char data[1024];
	char buf[1024];

	int re;
	noticenode noc;
	noc.is_pep = 1;
	noc.flag = 0;

	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select acceptaccount,noc  from offline_noc where sendaccount = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	if(result == NULL)
	{
		printf( " 1728     结果集为空\n");
		return 0;
	}
	while(row = mysql_fetch_row(result))
	{
		
		if(NULL == row)   break;
		strcpy(noc.noc,row[1]);
		strcpy(noc.sendaccount,account);
		strcpy(noc.acceptaccount,row[0]);
		memset(buf,0,sizeof(buf));
		memcpy(buf,&noc,sizeof(noc));
	
		if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");	
	}




}
int del_send_offline_friend(char account[SIZE],int conn_fd)
{
	char data[1024];

	sprintf(data,"delete from offline_friend where acceptaccount = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");
}
int del_send_offline_noc(char account[SIZE],int conn_fd)
{
	char data[1024];

	sprintf(data,"delete from offline_noc where sendaccount = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");
}

int del_send_offline_chat(char account[SIZE],int conn_fd)
{
	char data[1024];

	sprintf(data,"delete from offline_chat where acceptaccount = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");

}
int del_send_offline_group_chat(char account[SIZE],int conn_fd)
{
	char data[1024];

	sprintf(data,"delete from offline_group where acceptaccount = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");

	return 0;
}
int del_send_offline_group_noc(char account[SIZE],int conn_fd)
{
	char data[1024];
	
	sprintf(data,"delete from offline_group_noc where sendaccount = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");
	printf("asdsadasdas\n");
	return 0;
}
int del_offline_group_add(char account[SIZE],int conn_fd)
{
		char data[1024];
		sprintf(data,"delete from offline_group_add where account = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");

		return 0;

}

int send_offline_chat(char account[SIZE],int conn_fd)
{
	char data[1024];
	char buf[1024];
	char temp_name[SIZE];
	int re;
	msgnode msg;
	
	msg.flag = 11;


	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select msg,sendaccount  from offline_chat where acceptaccount = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	if(result == NULL)   //若没有这个账号接受的消息 退出
	{
		printf( " 1910     结果集为空\n");
		return 0;
	}
	Find_namebyaccount(account,msg.acceptname);
	while(row = mysql_fetch_row(result))
	{
		if(NULL == row)   break;
		Find_namebyaccount(row[1],msg.sendname);
		strcpy(msg.msg,row[0]);

		memset(data,0,sizeof(data));
		sprintf(data,"insert into friend_history values('%d','%s','%s','%s','%s','%s')",NULL,msg.sendaccount,msg.sendname,msg.acceptaccount,msg.acceptname,msg.msg);
		mysql_query(&mysql,data);
		//将消息发送给接受者
		msg.flag = 11;
		memset(buf,0,1024);    //初始化
      	 	memcpy(buf,&msg,sizeof(msgnode));    //将结构体的内容转为字符串
		if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
	}
}

int send_offline_group(char account[SIZE],int conn_fd)
{
	char data[1024];
	char buf[1024];
	char temp_name[SIZE];
	int re;
	msgnode msg;
	
	msg.flag = 15;


	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select msg,sendaccount,sendname,group_account from offline_group where acceptaccount = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	if(result == NULL)   //若没有这个账号接受的消息 退出
	{
		printf( " 1796     结果集为空\n");
		return 0;
	}
	
	while(row = mysql_fetch_row(result))
	{
		if(NULL == row)   break;
	//	Find_namebyaccount(row[1],msg.sendname);
		strcpy(msg.msg,row[0]);
		strcpy(msg.sendaccount,row[1]);
		strcpy(msg.sendname,row[2]);
		strcpy(msg.group_account,row[3]);
		memset(data,0,sizeof(data));
		sprintf(data,"insert into group_history values('%d','%s','%s','%s','%s')",NULL,msg.group_account,msg.sendaccount,msg.sendname,msg.msg);
		mysql_query(&mysql,data);
		//将消息发送给接受者
		msg.flag = 15;
		memset(buf,0,1024);    //初始化
      	 	memcpy(buf,&msg,sizeof(msgnode));    //将结构体的内容转为字符串
		if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");
	}


}

int send_offline_group_add(char account[SIZE],int conn_fd)
{

	char data[1024];
	char buf[1024];
	char temp_name[SIZE];
	int re;
	
	groupnode grp;
	grp.flag = 12;

	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select sendaccount,sendname,group_account from offline_group_add where account = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	if(result == NULL)   //若没有这个账号接受的消息 退出
	{
		printf( " 1796     结果集为空\n");
		return 0;
	}
	row = mysql_fetch_row(result);
	if(row == NULL)   return 0;
	strcpy(grp.user_account,row[0]);
	strcpy(grp.user_name,row[1]);
	strcpy(grp.group_account,row[2]);

	memset(buf,0,1024);    //初始化
      	memcpy(buf,&grp,sizeof(groupnode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");

}

int send_offline_group_noc(char account[SIZE],int conn_fd)
{

	char data[1024];
	char buf[1024];
	char temp_name[SIZE];
	int re;
	
	noticenode noc;
	noc.flag = 0;

	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select group_account,msg from offline_group_noc where sendaccount = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	if(result == NULL)   //若没有这个账号接受的消息 退出
	{
		printf( " 1796     结果集为空\n");
		return 0;
	}
	row = mysql_fetch_row(result);
	if(row == NULL)   return 0;
	strcpy(noc.acceptaccount,row[0]);
	printf("account = %s\n",noc.acceptaccount);
	strcpy(noc.noc,row[1]);
	noc.is_pep = 1;
	memset(buf,0,1024);    //初始化
      	memcpy(buf,&noc,sizeof(noticenode));    //将结构体的内容转为字符串
	if((re = (send(conn_fd,buf,1024,0))) < 0)  printf( "错误\n");

}
int Find_namebyaccount(char account[SIZE],char name[SIZE])
{
	char data[1024];
	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select name  from login where account = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	if(result == NULL)   //若没有这个账号接受的消息 退出
	{
		printf( " 1796     结果集为空\n");
		return 0;
	}
	row = mysql_fetch_row(result);

		if(NULL == row)   return 0;
	strcpy(name,row[0]);

	return 0;
}

int File_transfer_persistence(filenode file, int conn_fd)
{

	int fd;
	int flag = 0;
	int re;
	char buf[1024];

        online_node_t *curpos;

        List_ForEach(head,curpos)
        {
                if(strcmp(curpos->account,file.acceptaccount) == 0)
                {
                        flag = 1;
                        fd = curpos->fd;
                        //fid.acceptid = curpos->id;
                        break;
                }
        }
	
	printf( "fd = %d\n",fd);
	memset(buf,0,1024);    //初始化
      	memcpy(buf,&file,sizeof(filenode));    //将结构体的内容转为字符串
	if((re = (send(fd,buf,1024,0))) < 0)  printf( "错误\n");
	perror(buf);
	printf( "reeeeee   =%d\n",re);
	return 0;
}


int judge_is_friend(int friend1,int friend2)
{
	char data[1024];
	MYSQL_FIELD * field;
  MYSQL_ROW row;
  MYSQL_RES *result = NULL;

	sprintf(data,"select *  from friend where ((friend1 = %d && friend2 = %d)  || (friend1 = %d && friend2 = %d ))",friend1,friend2,friend2,friend1);
	printf("%d %d \n",friend1,friend2);
	if(mysql_query(&mysql,data))  printf( "false\n");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	if(result == NULL)   //若没有这个账号接受的消息 退出
	{
		printf( " 2139     结果集为空\n");
		return 0;
	}

	row = mysql_fetch_row(result);

		if(NULL == row)   return 0;


return 1;
}







int Find_idbyaccount(char account[SIZE])
{
	int id;
	char data[1024];
	MYSQL_FIELD * field;
        MYSQL_ROW row;
        MYSQL_RES *result = NULL;
	sprintf(data,"select id  from login where account = '%s'",account);
	if(mysql_query(&mysql,data))  printf( "false\n");
	result = mysql_store_result(&mysql);//将查询的全部结果读取到客户端

	if(result == NULL)   //若没有这个账号接受的消息 退出
	{
		printf( " 2194     结果集为空\n");
		return 0;
	}
	row = mysql_fetch_row(result);

		if(NULL == row)   return 0;
	id = atoi(row[0]);
	printf("id = %s\n",row[0]);
	printf("id = %d\n",id);

	return id;
}