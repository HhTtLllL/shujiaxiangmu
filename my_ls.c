#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/stat.h>  
#include<unistd.h>
#include<sys/types.h>
#include<linux/limits.h>
#include<dirent.h>
#include<grp.h>
#include<pwd.h>
#include<errno.h>

#define PARAM_NONE 0     //无参数
#define PARAM_A 1       //-a 显示所有文件
#define PARAM_L 2       // -l 一行只显示一个文件的详细信息
#define PARAM_R 4      //-R 递归显示目录
#define MAXROWLEN 80    //一行显示的最多字符数

#define BLUE                 "\e[0;34m"
#define L_BLUE               "\e[1;34m"
void display_dir(int flag_param,char *path);
int g_leave_len = MAXROWLEN;       //一行剩余长度,用于输出对齐
int g_maxlen;        //存放某目录下最长文件名的长度
//错误处理函数  ,打印出错误所在行的行数和错误信息
int flag_param = PARAM_NONE;

void my_err(const char * err_string,int line)
{
	fprintf(stderr,"line:%d ",line);
	perror(err_string);
}
// 选项 l 获取文件属性并打印
void display_attribute(struct stat buf,char *name)
{

	char buf_time[32];
	struct passwd *psd;  //从该结构体中获取文件所有者的用户名
	struct group *grp;    //从该结构体中获取文件所有者所属组的组名
	int flag = 0; //用来 判断是否是 链接文件

	//获取并打印文件类型
	if(S_ISLNK(buf.st_mode))printf( "l");  //判断是否为链接文件 
	else if(S_ISREG(buf.st_mode))  printf( "-");  //判断是否为一般文件
	else if(S_ISDIR(buf.st_mode))  printf("d");  //判断是否为目录文件
	else if(S_ISCHR(buf.st_mode))  printf( "c"); //判断是否为字符设备文件
	else if(S_ISBLK(buf.st_mode))  printf( "b");  //判断是否为快设备文件
	else if(S_ISFIFO(buf.st_mode)) printf( "f");  //判断是否为先进先出 FIFO
	else if(S_ISSOCK(buf.st_mode)) printf( "s");   //判断是否为socket 
	
	//获取并打印文件所有者的权限
	
	if(buf.st_mode & S_IRUSR) printf( "r");
	else printf( "-");
	if(buf.st_mode & S_IWUSR) printf( "w");
	else printf( "-");
	if(buf.st_mode & S_IXUSR) printf( "x");
	else printf( "-");

	//获取并打印与文件所有者同组的用户对该文件的操作权限
	
	if(buf.st_mode & S_IRGRP) printf( "r");
	else printf( "-");
	if(buf.st_mode & S_IWGRP) printf( "w");
	else printf( "-");
	if(buf.st_mode & S_IXGRP) printf( "x");
	else printf( "-");

	//获取并打印其他用户对该文件的操作权限
	if(buf.st_mode & S_IROTH)  printf( "r");
	else printf( "-");
	if(buf.st_mode & S_IWOTH)  printf( "w");
	else printf( "-");
	if(buf.st_mode & S_IXOTH) printf( "x");
	else printf( "-");
	printf( "    ");

	//根据uid 与gid 获取文件所有者的用户名和用户组
	
	psd = getpwuid(buf.st_uid);  //存储用户名
	grp = getgrgid(buf.st_gid);   //存储用户组
	printf( "%4ld ",buf.st_nlink); //打印文件的链接数
	printf( "%-8s",psd->pw_name);
	printf( "%-8s",grp->gr_name);

	printf( "%6ld",buf.st_size);   //打印文件的大小
	strcpy(buf_time,ctime(&buf.st_mtime));
	buf_time[strlen(buf_time) - 1] = '\0';   //去掉换行符
	printf( " %s",buf_time);     //打印文件的时间信息
	
	return ;
}
//在没有使用 -l 选项时 打印一个文件名,打印时上下行对齐
void display_single(char *name,int flag)
{
	int i,len;

	//如果本行不足以打印
	if(g_leave_len < g_maxlen)
	{
		printf( "\n");
		g_leave_len = MAXROWLEN;
	}

	len = strlen(name);
	len = g_maxlen - len;
	printf( " %-s",name);
				   /*    if(flag == 1)
				       {
					       len = printf("\033[34m %-s\033[0m\n", name);
				       }
				       else    printf( " %-s\n",name);
				       */
	len = g_maxlen - len;
	for(i = 0;i < len;i++)  printf( " ");

	printf( " ");
	//下面的 2 指示空2格
	
	g_leave_len -= (g_maxlen + 2);
}


/*
 * 根据命令行参数和完整路径名显示目标文件
 * 参数 flag : 命令行参数
 * 参数pathname : 包含了文件名的路径名
 * */
void display(int flag,char *pathname)
{
	int i,j;
	struct stat buf;
	struct stat lbuf;
	char name[NAME_MAX + 1];
	char temp[PATH_MAX];
	int lflag = 0;  //用来判断是否为链接
	char lname[PATH_MAX];

	//从路径中解析出文件名
	
	for(i = 0,j = 0;i < strlen(pathname);i++)
	{
		if(pathname[i] == '/')
		{
			j = 0;
			continue;
		}
		name[j++] = pathname[i];
	}
	name[j] = '\0';

	//用lstat 而不是 stat 以方便解析链接文件
	if(lstat(pathname,&buf) == -1)
	{
		my_err("stat",__LINE__);
	}

	if(S_ISLNK(buf.st_mode))   lflag = 1;

	if(lflag == 1)
	{
		readlink(pathname,lname,PATH_MAX-1);
	}
	int flag_color= 0;
	if(S_ISDIR(buf.st_mode))  flag_color = 1;;  //判断是否为目录文件

	switch(flag)
	{
		case PARAM_NONE : //没有选项
			if(name[0] != '.') display_single(name,flag_color); 
			break;
		case PARAM_A : display_single(name,flag_color);  break;
		case PARAM_L : if(name[0] != '.')
			       {
				       display_attribute(buf,name);
				       if(flag_color == 1)
				       {
					       printf("\033[34m %s\033[0m", name);
				       }
				       else    printf( " %-s",name);
				
				       if(lflag == 1)
					{
				      		 printf(" -> ");
						 printf( "%s",lname);
					}

				       printf( "\n");

			       }
				break;
		case PARAM_L + PARAM_A : display_attribute(buf,name);
				       if(flag_color == 1)
				       {
					       printf("\033[34m %s\033[0m\n", name);
				       }
				       else    printf( " %-s\n",name);
					 break;
		case PARAM_R : if(name[0] != '.')
			       {
					if(name[0] != '.') display_single(name,flag_color); 
			       }
			       break;
		case PARAM_R + PARAM_A:if(name[0] != '.')
			       {
					if(name[0] != '.') display_single(name,flag_color); 
			       }
			       break;
		case PARAM_R + PARAM_A + PARAM_L : display_attribute(buf,name);
						       if(flag_color == 1)
						       {
						      	 printf("\033[34m %s\033[0m\n", name);
						       }
						       else    printf( " %-s\n",name);
						   break;
		case PARAM_R + PARAM_L : if(name[0] != '.')
				       {
					       display_attribute(buf,name);
					       if(flag_color == 1)
					       {
					     	  printf("\033[34m %s\033[0m\n", name);
					       }
					       else    printf( " %-s\n",name);
				      
					       if(lflag == 1)
						{
				      			 printf(" -> ");
							 printf( "%s",pathname);
						}

					       printf( "\n");
				       }
					break;
		default : break;
	}
}
void display_dir(int flag_param,char *path)
{
	char temp[PATH_MAX];
	strcpy(temp,path);
	DIR *dir;       //目录流
	struct dirent   *ptr;   //目录信息结构体
	int count  = 0;    //该目录下的文件总数

	
	char name[256];
	struct stat buf;    //存储文件信息的结构体
	char new_path [256];
	//获取该目录下文件总数和最长的文件名  
	dir = opendir(path);
	printf( "\npath %s\n",path);
	if(dir == NULL)  my_err("opendir",__LINE__);
	else {
	while((ptr = readdir(dir)) != NULL)
	{
		if(g_maxlen < strlen(ptr->d_name))   //获取最长文件名
		{
			g_maxlen = strlen(ptr->d_name);
		}

		 count++;    //获取总数
	}}
	closedir(dir);
	char **filenames = (char **)malloc(sizeof(char *) * count);
	for(int i = 0;i < count;i++)
	{
		filenames[i] = (char *)malloc(sizeof(char) * PATH_MAX + 1);
	}


	int i,j,len = strlen(path);
	//获取 该 目录下的所有文件名
	
	dir = opendir(path);   //获取目录流
	for(i = 0;i < count;i++)
	{
		ptr = readdir(dir);
		
		if(ptr == NULL)   my_err("dir_ptr",__LINE__);
		strcpy(filenames[i],path);  
                filenames[i][len] = '\0';
                strcpy(filenames[i],ptr->d_name);  
                filenames[i][len+strlen(ptr->d_name)] = '\0';
	}

	//将文件 名 和 路径进行拼接 
	for(int i = 0;i < count;i++)
	{
		int len = strlen(path);
		temp[len+1] = '\0';
		strcat(temp,filenames[i]);
		strcpy(filenames[i],temp);
		strcpy(temp,path);

	}


	//使用冒泡法进行排序,排序后文件名按 字母顺序存储于 filenames 

	for(i = 0;i < count - 1;i++)
	{
		for(j = 0;j < count-i-1;j++)
		{
			if(strcmp(filenames[j],filenames[j+1]) > 0)  // j > j+1 
			{
				strcpy(temp,filenames[j+1]);
				temp[strlen(filenames[j+1])]  = '\0';
				strcpy(filenames[j+1],filenames[j]);
				filenames[j+1][strlen(filenames[j])] = '\0';
				strcpy(filenames[j],temp);
				filenames[j][strlen(temp)] = '\0';
			}
		}
	}
	
	for(i = 0;i < count;i++) 
	{
	//	if(strcmp(filenames[i],"") == 0) continue;
		display(flag_param,filenames[i]);
	}
	int k;
	if(flag_param & PARAM_R)
	{
		for(i = 0;i < count;i++)
		{
			if(lstat(filenames[i],&buf) == -1)
			{
				my_err("stat",__LINE__);
			}
			
			if(S_ISDIR(buf.st_mode))
			{
				        for(j = 0,k = 0;j < strlen(filenames[i]);j++)
					{   
				                if(filenames[i][j] == '/')
              				        {   
        		        		        k = 0;
              	 	 			        continue;
       					        }   
     					        name[k++] = filenames[i][j];
    	  				}   
		     		   name[k] = '\0';

				

				if(strcmp(name,".") == 0 || strcmp(name,"..") == 0)  continue;
				int len = strlen(filenames[i]);
				if(flag_param & PARAM_A)
				{
					if(filenames[i][strlen(filenames[i]) - 1] != '/')  
					{
						filenames[i][len] = '/';
						filenames[i][len+1] = '\0';
					}
					display_dir(flag_param,filenames[i]);
					free(filenames[i]);
				}
				{
					if(name[0] != '.')
					{
						if(filenames[i][strlen(filenames[i]) - 1] != '/')  
						{
							filenames[i][len] = '/';
							filenames[i][len+1] = '\0';
						}
						display_dir(flag_param,filenames[i]);
						free(filenames[i]);
					}
				}
			}

		}
	}

	closedir(dir);

	//如果命令中没有 -l 选项,打印一个换行符
	//当 flag 中 没有 l 选项时 
	if((flag_param & PARAM_L) == 0)   printf( "\n");


}
int main(int argc,char ** argv)
{
	int i;
	int j; // 记录 参数的个数
	int k;
	int num;    //记录 '-'  的个数
	char path[PATH_MAX + 1];
	char param[32];   //保存命令行参数 ,目标文件名 和 目录名不在此列
//	int flag_param = PARAM_NONE;   //参数种类,即是否有 -l ,-a ,-R 选项 
	struct stat  buf;

	//命令行解析   
	j = 0;
	num = 0;

	for(i = 1;i < argc;i++)
	{
		if(argv[i][0] == '-')
		{
			for(k = 1;k < strlen(argv[i]);k++,j++)
			{
				param[j] = argv[i][k];   //获取 - 后的参数
			}
			num++;     //计算 - 的个数
		}
	}

	//暂时仅 支持 a / l / R
	
	for(i = 0;i < j;i++)
	{
		if(param[i] == 'a')
		{
			flag_param |= PARAM_A;
			continue;
		}
		else if (param[i] == 'l')
		{
			flag_param |= PARAM_L;
			continue;
		}
		else if( param[i] == 'R')
		{
			flag_param |= PARAM_R;
			continue;
		}
		else
		{
			printf( "my_ls : invalid option -%c\n",param[i]);
			exit(1);
		}
	}
	param[j] = '\0';





	//如果没有输入文件名或 目录,就显示当前目录  
	if((num + 1) == argc)
	{
		strcpy(path,"./");
		path[2] = '\0';   //路径为当前目录

		display_dir(flag_param,path);
		return 0;
	}

	i = 1;
	do
	{
		//如果不是目标文件或目录,解析下一个命令行参数
		if(argv[i][0] == '-')
		{
			i++;
			continue;
		}
		else 
		{
			strcpy(path,argv[i]);

			//如果目标文件或目录不存在,报错并退出程序
			if(stat(path,&buf) == -1)  my_err("stat",__LINE__);

			if(S_ISDIR(buf.st_mode)) //如果argv[i] 是一个目录
			{
				//如果目录的最后一个 不是'/' ,就加上一个'/'
				if(path[strlen(argv[i]) - 1] != '/')
				{
					path[strlen(argv[i])] = '/';
					path[strlen(argv[i]) + 1] = '\0';
				}
				else	path[strlen(argv[i])] = '\0';

				display_dir(flag_param,path);
				i++;
			}
			else
			{
				//如果argv[i] 是一个文件
				display(flag_param,path);
				i++;
			}
		}
	}while(i < argc);

	return 0;
}
