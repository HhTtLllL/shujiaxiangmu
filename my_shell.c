#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<dirent.h>
#include<pwd.h>
#include<readline/readline.h>
#include<readline/history.h>


#define normal  0 //一般命令
#define out_redirect  1  //输出重定向
#define in_redirect 2 //输入重定向
#define have_pipe   3  //命令中有管道
#define add_out_redirect 4
#define add_in_redirect 5
#define CLOSE "\001\033[0m\002"                 // 关闭所有属性
char cd_pathnametemp[PATH_MAX] = "/home/tt";    //默认家目录
char cd_pathname[PATH_MAX];


void print_prompt();   //打印提示符  
void get_input(char *);   //得到输入的命令
void explain_input(char *,int *,char a[ ][256]);  //对输入命令进行解析
void do_cmd(int ,char a[ ][256]);  //执行命令
int find_command(char *);    //查找命令中的可执行程序
void my_dir();//cd 到家目录
int main(int argc,char **argv)
{
	 signal(SIGINT, SIG_IGN);
	int i;
	int argcount = 0; //记录 命令的个数
	char arglist[100][256];   //存储命令
	char **arg = NULL;
	char *buf = NULL;

	buf = (char *)malloc(256);
	if(buf == NULL)
	{
		perror("malloc failed\n");
		exit(-1);
	}


	while(1)
	{
		memset(buf,0,sizeof((buf)));
		print_prompt();   //输出命令提示符
		get_input(buf);   //获取输入
		//若输入的命令为 exit 或 logout 则退出本程序
		if(strcmp("exit\n",buf) == 0 || strcmp("logout\n",buf) == 0)  break;
		if(strcmp(buf,"\n") == 0) continue;
		//清空 arglist 
		for(i = 0;i < 100;i++)    arglist[i][0] = '\0';
		argcount = 0;//命令个数 清0


		explain_input(buf,&argcount,arglist);

		do_cmd(argcount,arglist);

	}
	if(buf == NULL)
	{
		free(buf);
		buf = NULL;
	}
	exit(0);

}
void my_chdir()
{
/*	int i,flag = 0,j;
	char name[30];
	char pathname[100];
	char pathnametemp[100];
	int uid;
	struct passwd *data;
	//uid_t uid;
	uid = getuid();  //获取uid 
	data = getpwuid(uid);
	getcwd(pathname,100);
	//处理路径  
	int len = strlen(pathname);
	for(i = 0;i < len;i++)
	{
		if(pathname[i] == '/') flag++;
		if(flag == 3)  break;
	}
	for(j = 0;j < i;j++)
	{
		pathnametemp[j] = pathname[j];
	}

	pathnametemp[j] = '\0';
*/
	//printf( "%s\n",pathnametemp);
	//chdir(pathnametemp);
	chdir("/home/tt");
}

//输出 命令提示符
void print_prompt()
{
	int i,flag = 0,j;
	char name[30];
	char pathname[100];
	char pathnametemp[100];
	int uid;
	struct passwd *data;

	//uid_t uid;
	uid = getuid();
	data = getpwuid(uid);
	printf("\033[43;35m%s@\033[0m",data->pw_name);
	gethostname(name,30);
	printf( "\033[43;35m%s:\033[0m",name);
	getcwd(pathname,100);
	
	if(pathname[1] != 'h' && pathname[2] != 'o')
	{
		printf( "\033[35;43m%s\033[0m",pathname);
		return ;
	}

	//处理路径  
	int len = strlen(pathname);
	for(i = 0;i < len;i++)
	{
		if(pathname[i] == '/') flag++;
		if(flag == 3)  break;
	}
	for(j = i;j < len;j++)
	{
		pathnametemp[j-i] = pathname[j];
	}
	pathnametemp[len-i] = '\0';
	strcpy(pathname,"~");
	strcat(pathname,pathnametemp);
	printf( "\033[35;43m%s\033[0m",pathname);

	//打印用户提示符
	if(0 == uid)  printf( "\033[40;32m#\033[0m");
	else printf( "\033[40;32m$\033[0m");
	
	return ;
}

//获取用户输入  
void get_input(char *buf)
{
	int len = 0;
	int ch;

	char * str = readline(" "CLOSE);
	add_history(str);
	strcpy(buf,str);
	buf[strlen(buf)] = '\n';


	/*free(str);
	ch = getchar();
	while(len < 256 && ch != '\n')
	{
		//char * str = readline("");
		//free(str);
		buf[len++] = ch;
		ch = getchar( );
	}

	if(len == 256)
	{
		printf( "command is too long \n");
		exit(-1);  //若输入的命令太长,则退出程序
	}

	buf[len] = '\n';
	len++;
	buf[len] = '\0';
	*/

}

//解析buf 中的命令,将结果存入 arglist 中,命令以回车符号'\n'结束
//如 输入命令"ls -l /temp" 则arglist[0],arglist[1],arglist[2],分别为 ls,-l,/temp

void explain_input(char *buf,int *argcount,char arglist[100][256])
{
	char *p = buf;
	char *q = buf;
	int number = 0;

	while(1)
	{
		if(p[0] == '\n')  break;
		if(p[0] == ' ') p++;
		else
		{
			q = p;
			number = 0;  //记录每个命令的长度

			while(q[0] != ' ' && (q[0] != '\n'))
			{
				number++;
				q++;
			}

			strncpy(arglist[*argcount],p,number+1);  //*argcount 为命令的个数
			arglist[*argcount][number] = '\0';
			*argcount = *argcount + 1;
			p = q;
		}
	}
}

//执行arglist 中存放的命令 ,argcount 为待执行命令的参数个数  
void do_cmd(int argcount,char arglist[100][256])
{
	int flag = 0;
	int how = 0;   //用于只是命令中是否含有 < , > , | 
	int background = 0; //表示命令中是否有后台运行表示符
	int status;
	int i;
	int fd;
	char *arg[argcount + 1];
	char *argnext[argcount + 1];
	char *file;  //保存文件名
	pid_t pid;
	int cdflag = 0;

	//将命令取出 
	for(i = 0;i < argcount;i++)   arg[i] = (char *)arglist[i];
	//给ls 加颜色
	if(strcmp(arg[0],"ls") == 0)
	{
		arg[argcount] = "--color=auto";
		arg[argcount + 1] = NULL;
	}
	else 	arg[argcount] = NULL;


	//cd   
	if(strcmp(arg[0],"cd") == 0)
	{
		getcwd(cd_pathname,100);
		if((argcount == 1)  || strcmp(arg[1],"~") == 0)  
		{
			strcpy(cd_pathnametemp,cd_pathname);
			my_chdir();   //更改家目录
		}
		else if(strcmp(arg[1],"-") == 0)
		{
		//	strcpy(cd_pathnametemp,cd_pathname);
			chdir(cd_pathnametemp);
			strcpy(cd_pathnametemp,cd_pathname);
		}
		else
		{
			strcpy(cd_pathnametemp,cd_pathname);
			chdir(arg[1]);    //更改当前工作目录
		}


		return ;
	}

	//查看命令行是否有后台运行符
	for(i = 0;i < argcount; i++)
	{
		if(strncmp(arg[i],"&",1) == 0)
		{
			if(i == argcount -1)
			{
				background = 1;
				arg[argcount - 1] = NULL;
				break;
			}
			else
			{
				printf( "wrong command\n");
				return ;
			}
		}
	}

	//查看命令行 是否 有 重定向 和管道符
	for(i = 0; arg[i] != NULL;i++)
	{
		if(strcmp(arg[i],">") == 0)
		{
			flag++;
			how = out_redirect;
			if(arg[i+1] == NULL) flag++;
		}
		if(strcmp(arg[i],"<") == 0)
		{
			flag++;
			how = in_redirect;
			if(i == 0) flag++;
		}
		if(strcmp(arg[i],"|") == 0)
		{
			flag++;
			how = have_pipe;
			if(arg[i+1] == NULL) flag++;
			if(i == 0)  flag++;
		}
		if(strcmp(arg[i],">>") == 0)
		{
			flag++;
			how = add_out_redirect;
			if(arg[i+1] == NULL)  flag++;
		}
		if(strcmp(arg[i],"<<") == 0)
		{
			flag++;
			how = add_in_redirect;
			if(i == 0) flag++;
		}

	}
	//若 flag == 1, 则有
	//若 flag > 1,则格式错误,不支持
	if(flag > 1)
	{
		printf( "wrong command\n");
		return ;
	}

	//命令中 只含一个输出替换重定向符号
	if(how == out_redirect)
	{
		for(i = 0;arg[i] != NULL;i++)
		{
			if(strcmp(arg[i],">") == 0)
			{
				file = arg[i+1];
				arg[i] = NULL;
			}
		}
	}

	//命令中只含有 一个输入替换重定向	
	if(how == in_redirect)
	{
		for(i = 0;arg[i] != NULL;i++)
		{
			if(strcmp(arg[i],"<") == 0)
			{
				file = arg[i+1];
				arg[i] = NULL;
			}
		}
	}
	//命令中只含有 输出追加重定向
	if(how == add_out_redirect)
	{
		for(i = 0;arg[i] != NULL;i++)
		{
			if(strcmp(arg[i],">>") == 0)
			{
				file = arg[i+1];
				arg[i] = NULL;
				arg[i] = NULL;
			}
		}
	}

	//命令中只含有 输入追加重定向
	if(how == add_in_redirect)
	{
		for(i = 0;arg[i] != NULL;i++)
		{
			if(strcmp(arg[i],"<<") == 0)
			{
				file = arg[i+1];
				arg[i] = NULL;
			}
		}
	}
	//命令中只含有 一管道符号
	//把管道符号后面的部分存入argnext中,管道后面的部分是一个可执行 的shell 命令
	if(how == have_pipe)
	{
		for(i = 0;arg[i] != NULL;i++)
		{
			if(strcmp(arg[i],"|") == 0)
			{
				arg[i] = NULL;
				int j;
				//将| 后面的 命令存入argnext 中
				//j - i -1   就是 从零开始,往后加
				for(j = i+1;arg[j] != NULL;j++)  argnext[j-i-1] = arg[j];
				argnext[j-i-1] = arg[j];
				break;
			}
		}
	}

	if((pid = fork()) < 0)
	{
		printf( "fork error\n");
		return ;
	}

 	    //0  一般命令
	    //1  输出重定向
	    //2  输入重定向
	    //3  命令中有管道
	//pid 为 0 说明是子进程 ,在子进程中执行命令
	switch(how)
	{
		case 0: //一般命令
			if(pid == 0)
			{
				if(!(find_command(arg[0])))
				{
					printf( "%s : command not found\n",arg[0]);
					exit(0);
				}

				execvp(arg[0],arg);
				exit(0);
			}
			break;
		case 1:  //替换输出重定向
			if(pid == 0)
			{
				if(!(find_command(arg[0])))
				{
					printf( "%s : command not found\n",arg[0]);
					exit(0);
				}
				fd = open(file,O_RDWR | O_CREAT | O_TRUNC,0644);  //可读可写 ,若文件不存在就自动建立该文件  ,将文件长度清 0
				dup2(fd,1);  //赋值 文件描述符描述符   将本来 的文件描述符 改为 1 标准输入
				execvp(arg[0],arg);
				exit(0);
			}
			break;
		case 2:  //替换输入重定向
			if(pid == 0)
			{
				if(!(find_command(arg[0])))
				{
					printf( "%s : command not found\n",arg[0]);
					exit(0);
				}
				fd = open(file,O_RDONLY);
				dup2(fd,0);
				execvp(arg[0],arg);
				exit(0);
			}
			break;
		case 3:  //含有管道
			if(pid == 0)
			{
				int pid2;
				int status2;
				int fd2;

				if((pid2 = fork()) < 0)
				{
					printf( "fork2 error\n");
					return ;
				}
				else if(pid2 == 0)
				{
					if(!(find_command(arg[0])))
					{
						printf( "%s : command not found\n",arg[0]);
						exit(0);
					}

					fd2 = open("/tmp/youdonotknowfile",O_WRONLY | O_CREAT | O_TRUNC,0644);
					dup2(fd2,1);
					execvp(arg[0],arg);
					exit(0);
				}
				if(waitpid(pid2,&status2,0) == -1)  printf( "wait for child procrss error\n");

				if(!(find_command(argnext[0])))
				{
					printf( "%s : command not found\n",argnext[0]);
					exit(0);
				}

				fd2 = open("/tmp/youdonotknowfile",O_RDONLY);
				dup2(fd2,0);
				execvp(argnext[0],argnext);

				if(remove( "/tmp/youdonotknowfile"))  printf( "remove error\n");
				exit(0);

			}
			break;
		case 4://追加输出重定向
			if(pid == 0)
			{
				if(!(find_command(arg[0])))
				{
					printf( "%s : command not found\n",arg[0]);
					exit(0);
				}

				fd = open(file,O_RDWR | O_CREAT | O_APPEND);
				dup2(fd,1);
				execvp(arg[0],arg);
				exit(0);
			} break;
		default : break;
	}

	//若命令中有 &,表示后台运行,父进程直接返回,不等待子进程结束
	if(background == 1)
	{
		printf( "[process id %d ]\n",pid);
		return ;
	}

	//父进程等待子进程结束
	if(waitpid(pid,&status,0) == -1)  printf( "wait for child process error\n");

}

//查找命令中的可执行文件
int find_command(char *command)
{
	DIR* dp;
	struct dirent * dirp;
	char * path[] = {"./","/bin","/usr/bin",NULL};

	//使用当前目录下的程序可以运行,如命令"./fork "可以被正确解释和执行 
	if(strncmp(command,"./",2) == 0)   command = command + 2;

	//分别在当前目录 /bin ,/usr/bin 目录查找要执行的程序
	//
	
	int i = 0;
	while(path[i] != NULL)
	{
		if((dp = opendir(path[i])) == NULL)   printf( "can not open /bin\n");

		while((dirp = readdir(dp)) != NULL)
		{
			if(strcmp(dirp->d_name,command) == 0)
			{
				closedir(dp);
				return 1;
			}
		}

		closedir(dp);
		i++;
	}

	return 0;
}


