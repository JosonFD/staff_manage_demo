
#include "client.h"

//网络套接字初始化  全局套接字 socketfd
void socket_init(const char *argv[])
{
	socketfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&serveraddr, sizeof(serveraddr));

	//网络信息结构体填充
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port = htons(atoi(argv[2]));

	connect(socketfd,(SA *)&serveraddr, sizeof(serveraddr)); 
}

/**
 * Name         :  do_register 
 * Description  :  用户进行注册，填充信息后发送给服务系接收，阻塞等待
 接收一些提示信息。
 * Input ：   
 *			           socketfd： 通信套接字
 *			           msg： 接受到的信息结构体指针
 * output       :
 * Author       :  陈阿磊
 * Other        :  返回的提示信息中 msg->tips[0]为'#'，表示失败
 */

void do_register(int socketfd, MSG *msg){

	msg->type = 1;//由于注册的只能是普通用户，所以这里直接把类型
	//定义为 '1'（普通用户）		
	//输入用户信息			
	printf("input user name:");
	scanf("%s", msg->name);
	getchar();
	printf("input password:");
	scanf("%s", msg->passwd);
	getchar();
	printf("input tips:");
	scanf("%s", msg->tips);
	getchar();
	printf("input sex:");
	scanf("%s", msg->sex);
	getchar();
	printf("input age:");
	scanf("%d", &(msg->age));
	getchar();
	printf("input phonenum:");
	scanf("%s", msg->phone);
	getchar();
	printf("input addr:");
	scanf("%s", msg->addr);
	getchar();

	msg->type = 1;

	send(socketfd, msg, sizeof(MSG),0);//发送消息结构体
	recv(socketfd, msg, sizeof(MSG),0);//阻塞等待接收一些提示信息，

	printf("%s", msg->addr);
	//无论成功与否，打印查询结果。
	return;
}


/**
 * Name         :  do_login 
 * Description  :  登录功能，通过用户名和密码进行登录，
 *								 登录成功的同时会根据不同的用户类型接入不同的入口。
 * Input    
 *			           socketfd： 通信套接字
 *			           msg： 接受到的信息结构体指针
 * output       :
 * Author       :  马建业
 * Other        :  返回的提示信息中 msg->tips[0]为'#'，表示失败

*/
int do_login(int socketfd, MSG *msg){

	//输入用户信息
	printf("input name:");
	scanf("%s", msg->name);
	getchar();
	printf("input passwd:");
	scanf("%s", msg->passwd);
	getchar();
	
	send(socketfd,msg,sizeof(MSG),0);
	//如果登录成功，此时应该会传回用户的 id 号，以及 用户类型 type 
	recv(socketfd,msg,sizeof(MSG),0);

	//无论成功与否，打印查询结果。
	printf("tips = %s\n", msg->tips);
	printf("type = %d\n", msg->type);
	printf("id = %d\n", msg->id);
	if (msg->type == 1) {
		do_general_user(socketfd, msg);  //登录为普通用户

	}
	else {
		do_root_user(socketfd, msg);  	 //登录为root用户
	}

	return 1;
}


/**
 * Name         :  do_forget_password 
 * Description  :  找回密码功能，忘记密码时通过姓名和注册时填写的备注信息
 *								 来找回密码
 * Input    
 *			           socketfd： 通信套接字
 *			           msg： 接受到的信息结构体指针
 * output       :
 * Author       :  黄文康
 * Other        :  返回的提示信息中 msg->tips[0]为'#'，表示失败，否则正确
 *								 打印出密码。

*/

void do_forget_password(int socketfd, MSG *msg){
	printf("input name:");
	scanf("%s", msg->name);
	getchar();
	printf("input tips:");
	scanf("%s", msg->tips);
	getchar();

	send(socketfd,msg,sizeof(MSG),0);
	recv(socketfd,msg,sizeof(MSG),0);
	//无论成功与否都会打印提示信息，成功找到后同时会打印密码，实现密码找回功能
	printf("%s\n", msg->tips);
	printf("password : %s\n",msg->passwd);
	return;

}


/**以下实现 增删改查 等功能 */


//添加用户：超级用户执行的添加用户功能实际上与注册永和的功能相同，所以这里直接调用
void do_add_user(int socketfd, MSG * msg){
	do_register(socketfd,msg);
	return;
}

//删除用户：删除用户是通过输入 id 号进行删除
void do_delete_user(int socketfd, MSG * msg){

	printf("input user id:");
	scanf("%d", &(msg->id));
	getchar();
	send(socketfd,msg,sizeof(MSG),0);
	recv(socketfd,msg,sizeof(MSG),0);

	printf("%s\n", msg->addr);
	return;
}

//root用户修改信息：通过 id号查找用户，找到之后可以更改电话号码，地址信息								
void do_update_root_user(int socketfd, MSG * msg)
{

	printf("input update id:");
	scanf("%d", &msg->id);
	getchar();
	printf("input new phone:");
	scanf("%s", msg->phone);
	getchar();
	printf("input new sex:");
	scanf("%s", msg->sex);
	getchar();
	send(socketfd,msg,sizeof(MSG),0);
	recv(socketfd,msg,sizeof(MSG),0);
	printf("%s\n", msg->addr);
	return;
}


//普通用户修改信息：如果是普通用户则可以更改自己的密码
void do_update_general_user(int socketfd, MSG * msg){

	printf("pls input new password:");
	scanf("%s",msg->passwd);
	getchar();
	send(socketfd,msg,sizeof(MSG),0);
	recv(socketfd,msg,sizeof(MSG),0);
	printf("%s\n",msg->addr);

	return;
}

//root用户查询信息：通过 id号查看不同的用户用户
void do_search_root_user(int socketfd, MSG * msg){

#if 1
	printf("input search id:");
	scanf("%d", &(msg->id));
	getchar();
	send(socketfd,msg,sizeof(MSG),0);
	recv(socketfd,msg,sizeof(MSG),0);
	printf("id : %d\n",msg->id);	
	printf("name : %s\n", msg->name);
	printf("sex : %s\n", msg->sex);
	printf("age : %d\n", msg->age);
	printf("phone : %s\n", msg->phone);
	printf("address : %s\n", msg->addr);

#endif
	return;
}

//普通用户查询信息：普通用户只能查看自己的信息
void do_search_general_user(int socketfd, MSG * msg){

	send(socketfd,msg,sizeof(MSG),0);
	printf("----%s-----%d-------\n", __func__, __LINE__);
	recv(socketfd,msg,sizeof(MSG),0);
	printf("----%s-----%d-------\n", __func__, __LINE__);

	if(msg->tips[0] == '#'){
		printf("%s\n",msg->tips+1);
	}
	//
	//打印查询结果：
	else{
		printf("id : %d\n",msg->id);	
		printf("name : %s\n", msg->name);
		printf("sex : %s\n", msg->sex);
		printf("age : %d\n", msg->age);
		printf("phone : %s\n", msg->phone);
		printf("address : %s\n", msg->addr);
	}
	return;
}

//root用户：在登录时type为0，进入root用户界面
void do_root_user(int socketfd, MSG *msg)
{
	int n;

	while(1){
		printf("*****************************************************\n");
		printf("* 1: add  2: delete  3: update   4: search  5: exit *\n");
		printf("*****************************************************\n");
		printf("please choose : ");

		scanf("%d", &n);
		getchar();
		switch(n){
		case 1:
			msg->tips[0] = 'A';
			send(socketfd,msg,sizeof(MSG),0);
			do_add_user(socketfd,msg);   	 //添加用户
			break;

		case 2:				
			msg->tips[0] = 'D';
			printf("n=%d",n);
			send(socketfd,msg,sizeof(MSG),0);
			do_delete_user(socketfd,msg);  //删除用户
			break;

		case 3:			
			msg->tips[0] = 'U';
			send(socketfd,msg,sizeof(MSG),0);
			do_update_root_user(socketfd,msg); //root用户修改信息
			break;

		case 4:				
			msg->tips[0] = 'S';
			send(socketfd,msg,sizeof(MSG),0);
			do_search_root_user(socketfd,msg); //root用户查询信息
			break;

		case 5:			
			msg->tips[0] = 'Q';                //退出
			send(socketfd,msg,sizeof(MSG),0);
			return;
		default:
			printf("input error!\n");
			break;
		}
	}
}


//普通用户：在登录时type为1，进入普通用户界面
void do_general_user(int socketfd, MSG *msg){
	int n;

	while(1){

		printf("********************************\n");
		printf("* 1: update  2: search  3: exit*\n");
		printf("********************************\n");
		printf("please choose : ");

		scanf("%d", &n);
		getchar();

		switch(n){
		case 1:
			msg->tips[0] = 'U';
			send(socketfd,msg,sizeof(MSG),0);
			do_update_general_user(socketfd,msg);  //普通用户修改信息
			return;
			break;

		case 2:
			msg->tips[0] = 'S';
			send(socketfd,msg,sizeof(MSG),0);
			do_search_general_user(socketfd,msg);  //普通用户查询信息
			break;

		case 3:
			msg->tips[0] = 'Q';
			send(socketfd,msg,sizeof(MSG),0);      //退出
			return;
		default:
			printf("input error!\n");
			break;
		}
	}
}


