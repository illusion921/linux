#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>




int main()
{
	int a;
	printf("--------------------------------------\n请输入对应的数字\n--------------------------------------\n1:创建两个子进程并输出信息\n--------------------------------------\n2:进程间的信号通讯\n--------------------------------------\n3:多进程管道的读写操作\n--------------------------------------\n");
	printf("您的选择：");
	scanf("%d",&a);
	if (a==1)
	{
		process();
	}
	else if (a==2)
	{
		test_signal();
	}
	else
	{
		test_pipe();
	}
}

/*创建两个子进程并输出信息*/
process() /*进程的创建和控制*/
{
	int pid1,pid2;
	pid1=fork();  /*创建一个子进程*/
	if (pid1==0)  /*判断子进程是否创建成功*/
	{
		printf("This is the first child process!\n");/*子进程1在屏幕上输出信息*/
		exit(1);
	}
	else if (pid1<0)
	{
		printf("ERROR:first child process call fail!\n");
		exit(2);
	}
	else /*pid > 0 父进程的入口*/
	{
		pid2=fork(); /*再创建一个子进程*/
		if (pid2==0)
		{
			printf ("This is the second child process!\n");/*子进程2在屏幕上输出信息*/
			exit(3);
		}
		else if (pid2<0)
		{
			printf ("ERROR:second child process call fail!\n");
			exit(4);
		}
		else
		{
			printf ("This is the parent process!\n");/*父进程在屏幕上输出信息*/
			exit(5);
		}
	}
}


/*进程间的信号通信*/
int flag;
test_signal()
{
	int pid1,pid2;
	if ((pid1=fork())==0)/*创建子进程1*/
	{
		flag = 0;/*设置标志位*/
		void top();/*引用函数top*/
		signal(16,top);/*捕获父进程发来的信号*/
		printf("Child1 is running!\n");
		while (flag!=1);/*等待父进程的信号，使标志位变为1，之前一直做死循环，使子进程一直在运行*/
		printf("The child1 process was killed!\n");/*输出信息*/

	}
	else/*父进程入口*/
	{
		if ((pid2=fork())==0)/*创建子进程2*/
		{
			flag = 0;/*设置标志位*/
			void top();/*引用函数top*/
			signal(17,top);/*捕获父进程发来的信号*/
			printf("Child2 is running!\n");
			while (flag!=1);/*等待父进程的信号，使标志位变为1，之前一直做死循环，使子进程一直在运行*/
			printf("The child2 process was killed!\n");/*输出信息*/
		}
		else
		{
			void catchalarm(int signo);
			flag=0;/*设置标志位*/
			alarm(5);/*设置闹钟*/
			signal(SIGALRM,catchalarm);/*捕获闹钟到时的报警信号*/
			while (flag!=1);/*等待捕获到闹钟到时的信号使标志位置1，跳出循环*/
			printf("The parent has sent message to child!\n");
			sleep(1);
			printf("Child process has recived message!\n");
			kill(pid1,16);/*给子进程1发送信号*/
			kill(pid2,17);/*给子进程1发送信号*/
			wait(NULL);/*等待子进程1结束*/
			wait(NULL);/*等待子进程2结束*/
			sleep(1);
			printf("The parent process was killed!\n");/*输出信息*/
		}
	}
}

void  catchalarm(int signo)
{
	flag=1;
}

void top()
{
	flag=1;
}


/*进程的管道通信*/

test_pipe()
{
	int f_des[2];
	int pid1,pid2;
	char buff[1024];
	static char message[2][1024]={"Child1 wrote a message!","Child2 wrote a message!"};	/*创建消息数组*/
	int i=5;
	if (pipe(f_des)==-1)/*创建管道*/
	{
		perror("pipe");
		exit(1);
	}
	if ((pid1=fork())==0)/*创建子进程1*/
	{
		for (i;i--;i>0)/*循环5次写*/
		{
			close(f_des[0]);/*关闭管道的读*/
			lockf(f_des[1],1,1024);/*将管道锁定，其他进程不能使用*/
			if (write(f_des[1],message[0],1024) !=-1)/*写操作*/
			{
				printf("Child1 sent a message:%s\n",message[0]);
				printf("------------------------------------------------\n");
				fflush(stdout);
			}
			else
			{
				perror("write");
				exit(2);
			}
			lockf(f_des[1],0,1024);/*解锁*/
			sleep(1);/*使另外的子进程在循环的次数也能写*/
		}
	}
	else
	{
		if ((pid2=fork())==0)/*创建子进程2*/
		{
			for (i;i--;i>0)/*循环5次写*/
			{
				close(f_des[0]);/*关闭管道的读*/
				lockf(f_des[1],1,1024);/*将管道锁定，其他进程不能使用*/
				if (write(f_des[1],message[1],1024) !=-1)/*写操作*/
				{
					printf("Child2 sent a message:%s\n",message[1]);
					printf("------------------------------------------------\n");
					fflush(stdout);
				}
				else
				{
					perror("write");
					exit(3);
				}
				lockf(f_des[1],0,1024);	
				sleep(1);/*使另外的子进程在循环的次数也能写*/
			}
		}
		else if(pid2==-1)
		{
			perror("fork");
		}
		else/*父进程*/
		{
			for (i;i--;i>0)/*循环5次读*/
			{
				close(f_des[1]);/*关闭管道的写*/
				if (read(f_des[0],buff,1024) !=-1)/*读操作*/
				{
					printf ("The parent received a message:%s\n",buff);
					printf("------------------------------------------------\n");
					sleep(1);
					fflush(stdout);
				}
				else
				{
					perror("read");
					exit(4);
				}
			}
		}
	}
	exit(0);
}




















