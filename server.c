
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <sqlite3.h>
#include <time.h>
#include <pthread.h>

#define DATABASE "staff.db"
#define N 1024
#define SERV_PORT 8080
#define BACKLOG  5

typedef struct {
	int id; 			//用户id
	char name[20]; 		//姓名
	char passwd[20]; 	//密码
	char tips[50];  	//消息
	int  type; 			//用户类型
	char sex[10]; 		//性别
	int age; 			//年龄
	char phone[11]; 	//电话
	char addr[50]; 		//住址
}MSG;

struct sockaddr_in serveraddr, clientaddr;
typedef struct sockaddr SA;
sqlite3 *db;
socklen_t client_len;
int listenfd;


/**
 *Name 		  : 	socket_init
 *Description : 	初始化网络，为客户端连接做准备, 初始化监听套接字 listenfd
 *Input 	  : 	
 *Output 	  :
 */
void socket_init() 
{
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	int bereuse = 1;

	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &bereuse, sizeof(int)) < 0)
	{
		perror("setsockopt failed!\n");
		exit(1);
	}
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERV_PORT);
	//绑定服务器端口
	if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0) {
		perror("bind");
		exit(1);
	}
	//服务器开启监听，等待客服段连接
	listen(listenfd, BACKLOG);
	//设置地址端口快速复用
	printf ("serv is OK!listening......\n");
	return;
}

/**
 *Name 		  : 	create_database
 *Description : 	创建数据库
 *Input 	  :
 *Output 	  :
 */
int create_database() 
{
	char **dbResult;
	char **result;
	int nrow, ncolumn;
	char *errmsg;
	char sql[1024];
	printf("create_database!\n");

	//打开数据库，如果数据库不存在则创建
	if (sqlite3_open(DATABASE, &db) != SQLITE_OK) {
		printf("%s\n", sqlite3_errmsg(db));
		exit(1);
	} else {
		printf("sqlite3_open DATABASE success!\n");
	}

	sprintf(sql, "create table user_login(id integer primary key,name vchar(20),password vchar(20), type integer, tips vchar(50));"); // 为数据库创建表 user_login 
	printf("create_database!\n");
	sqlite3_exec(db, sql, NULL, NULL, &errmsg);    // 执行sql 语句 

	printf("create_database!\n");
	sprintf(sql, "create table user_info(id integer primary key,name vchar(20),password vchar(20), sex vchar(10), age integer, phone vchar(11), addr vchar(50));"); // 为数据库创建表 user_info
	sqlite3_exec(db, sql, NULL, NULL, &errmsg); // 执行sql 语句 


	// 确保 user_login 表中有 root 用户  密码 root
	// ..
	sprintf(sql, "select * from user_login where name = 'root';");
	if (sqlite3_get_table(db, sql, &result, &nrow, &ncolumn, &errmsg) != SQLITE_OK){
		printf("%s\n",errmsg);
		return -1;
	}
	if (nrow == 0){
		sprintf(sql, "insert into user_login values(null, 'root', 'root', 0, 'root');");
		sqlite3_exec(db, sql, NULL, NULL, &errmsg); 
		printf("root user user_login success!\n");

		sprintf(sql, "insert into user_info values(null, 'root', null, null, null, null, null);");
		sqlite3_exec(db, sql, NULL, NULL, &errmsg); 
		printf("root user user_info success!\n");
	}

	return 1;
}

void do_quit(MSG *userMsg, int clientfd) 
{
	free(userMsg);
	close(clientfd);
	printf("user quit\n");

	return;
}


/**
 *Name 		  : 	do_register
 *Description : 	注册用户信息，将用户登录信息添加到user_login表中，将
 用户信息添加到user_info表中
 *Input 	  :     用户信息（MSG）
 *Output 	  :
 */

void do_register(MSG *userMsg, int clientfd) 
{
	char sql[256] = {};
	char *errmsg;
	recv(clientfd, userMsg, sizeof(MSG), 0);

	sprintf(sql, "insert into user_login values (null, '%s', '%s', %d, '%s')", userMsg->name, userMsg->passwd, userMsg->type, userMsg->tips);
	if(sqlite3_exec(db,sql,NULL,NULL, &errmsg) != SQLITE_OK) {
		printf("%s\n", errmsg);
	}

	sprintf(sql, "insert into user_info values (null, '%s', '%s','%s', %d,'%s', '%s')", userMsg->name, userMsg->passwd, userMsg->sex, userMsg->age, userMsg->phone, userMsg->addr);
	if(sqlite3_exec(db,sql,NULL,NULL, &errmsg) != SQLITE_OK) {
		printf("%s\n", errmsg);
	}else {
		strcpy(userMsg->addr, "register ok!");
	}

	send(clientfd, userMsg, sizeof(MSG), 0);
	return;
}


void do_add(MSG *userMsg, int clientfd)
{
	do_register(userMsg, clientfd);
}

/**
 *Name 		  : 	do_delete
 *Description :     根据用户的id删除用户记录 
 *Input 	  : 	用户id(msg->id)
 *Output 	  :
 */
void do_delete(MSG *userMsg, int clientfd) 
{
	printf(">>>>>>%s>>>>>>>>%d>>>>>>", __func__, __LINE__);
	char sql[256];
	char *errmsg;
	char **result;
	int nrow, ncloum;

	recv(clientfd, userMsg, sizeof(MSG), 0);
	sprintf(sql, "select * from user_login where id='%d'", userMsg->id);
	if(sqlite3_get_table(db, sql, &result, &nrow, &ncloum, &errmsg) != SQLITE_OK){
		perror("exec");
	}else{
		printf("nrow = %d\n", nrow);
		if (nrow == 1){
			sprintf(sql, "delete from user_login where id='%d'", userMsg->id);
			sqlite3_exec(db, sql, NULL, NULL, &errmsg);
			sprintf(sql, "delete from user_info where id='%d'", userMsg->id);
			sqlite3_exec(db, sql, NULL, NULL, &errmsg);
			strcpy(userMsg->addr, "deleted!");
			send(clientfd, userMsg, sizeof(MSG), 0);
		}
		if (nrow == 0){
			strcpy(userMsg->addr, "user do not exist!");
			send(clientfd, userMsg, sizeof(MSG), 0);
		}
	}


	return;
}

/**
 *Name 		  : 	do_select 
 *Description : 	根据用户id查询某个用户的信息
 *Input 	  : 	用户id
 *Output 	  :
 */

int callback1(void *arg, int c_num, char ** c_value, char ** c_name)
{
	int clientfd = *(int *)arg;
	MSG userMsg;
	printf("----%s-----%d-------\n", __func__, __LINE__);

	userMsg.id =atoi(c_value[0]);
	strcpy(userMsg.name, c_value[1]);
	strcpy(userMsg.sex, c_value[3]);
	userMsg.age = atoi(c_value[4]);
	strcpy(userMsg.phone, c_value[5]);
	strcpy(userMsg.addr, c_value[6]);
	printf("name = %s\n", userMsg.name);


	send(clientfd, &userMsg, sizeof(MSG), 0);

	printf("----%s-----%d-------\n", __func__, __LINE__);

	return 1;
}


void do_select(MSG *userMsg, int clientfd) 
{
	char sql[256];
	char *errmsg;


	recv(clientfd, userMsg, sizeof(MSG), 0);
	printf("----%s-----%d-------\n", __func__, __LINE__);
	sprintf(sql, "select * from user_info where id = %d",userMsg->id);
	sqlite3_exec (db, sql, callback1, &clientfd, &errmsg);


	return;
}

/**
 *Name 		  : 	do_updata 
 *Description : 	管理员根据用户id修改用户信息
 *Input 	  : 	用户id
 *Output 	  :
 */
void do_updata(MSG *userMsg, int clientfd) 
{
	char sql[256];
	char *errmsg;
	recv(clientfd, userMsg, sizeof(MSG), 0); 
	switch(userMsg->type){
	case 0:
		sprintf(sql, "update user_info set phone='%s',sex='%s' where id=%d;",userMsg->phone, userMsg->sex, userMsg->id);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK){
			perror("sqlite3_exec");
			strcpy(userMsg->addr, "msg update failed");
			send(clientfd, userMsg, sizeof(MSG), 0);
			exit(1); 
		} else {
			strcpy(userMsg->addr, "msg update success");
			send(clientfd, userMsg, sizeof(MSG), 0);
		} 
		
		return;
	case 1:
		sprintf(sql, "update user_login set password='%s' where id='%d';", userMsg->passwd, userMsg->id);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK){
			perror("sqlite3_exec");
			strcpy(userMsg->addr, "passwd fixed failed");
			send(clientfd, userMsg, sizeof(MSG), 0);
			exit(1); 
		} else {
			strcpy(userMsg->addr, "passwd fixed success");
			send(clientfd, userMsg, sizeof(MSG), 0);
		} 
		return;
	default:
		break;
	}
}

void do_passwd(MSG *userMsg, int clientfd) 
{

	return;
}



void do_login_success(MSG *userMsg, int clientfd)
{
	while(1) {
		if (recv(clientfd, userMsg, sizeof(*userMsg), 0) < 0) {
			printf("fail to receive!\n");		
		}

		switch(userMsg->tips[0]) {
		case 'A':
			do_add(userMsg, clientfd);
			break;
		case 'D':
			printf("do_delete func\n");
			do_delete(userMsg, clientfd);
			break;
		case 'U':
			do_updata(userMsg, clientfd);
			return;
			break;
		case 'S':
			do_select(userMsg, clientfd);
			break;
		case 'P':
			do_passwd(userMsg, clientfd);
			return;
			break;
		case 'Q':
			return;
			break;
		default:
			break;
		}
	}

	return;
}



/**
 *Name 		  : 	do_login
 *Description : 	判断用户输入的帐号与密码是否正确
 *Input 	  : 	用户id,密码
 *Output 	  :
 */
int callback (void *arg, int c_num, char **c_value, char ** c_name)
{
	int listenfd = *(int *)arg;
	MSG userMsg;

	printf("callback func!\n");
	int test = atoi(c_value[3]);
	printf("test = %d\n", test);
	userMsg.type = test;
	printf("type=%d\n", userMsg.type);
	strcpy(userMsg.tips, "login success!");
	printf("tips=%s\n", userMsg.tips);
	int myid = atoi(c_value[0]);

	userMsg.id = myid;
	printf("tips=%s,type=%d,id=%d\n", userMsg.tips, userMsg.type, userMsg.id);
	send(listenfd, &userMsg, sizeof(MSG), 0);
	printf("send succ!\n");

	return 1;
}

void do_login(MSG *userMsg, int clientfd) 
{
	char sql[256];
	char *errmsg;
	int nrow;
	int ncloum;
	char **result;

	recv(clientfd, userMsg, sizeof(MSG), 0);
	sprintf(sql, "select * from user_login where name='%s' and password='%s'", userMsg->name, userMsg->passwd);
	if (sqlite3_get_table(db, sql, &result, &nrow, &ncloum, &errmsg) != SQLITE_OK){
		printf("%s", errmsg);
		exit(1);
	}else {
		printf("nrow=%d\n",nrow);
		if(nrow == 1) {
			//sprintf (sql, "select * from user_login where name = '%s';", userMsg->name);
			sqlite3_exec (db, sql, callback, &clientfd, &errmsg);
		}else if (nrow == 0) {
			strcpy (userMsg->addr, "usr do not exist or wrong passwd");
			if(send(clientfd, userMsg,sizeof(MSG), 0) < 0) {
				perror ("send");
				exit(1);
			}		
		}
	}

	do_login_success(userMsg, clientfd);

	return;
}

/**
 *Name 		  : 	do_findPassword 
 *Description : 	在用户忘记密码时提供密保找回密码功能
 *Input 	  : 	用户id,密保问题
 *Output 	  :
 */

int callback2 (void *arg, int c_num, char **c_value, char ** c_name)
{
	int listenfd = *(int *)arg;
	MSG userMsg;

	printf("callback func!\n");
	strcpy(userMsg.tips, "password get!");
	strcpy(userMsg.passwd, c_value[2]);

	send(listenfd, &userMsg, sizeof(MSG), 0);
	return 1;
}
void do_findPassword(MSG *userMsg, int clientfd) 
{
	char sql[256];
	char **result;
	int nrow, ncloum;
	char *errmsg;


	recv(clientfd, userMsg, sizeof(MSG), 0);

	sprintf(sql, "select * from user_login where name='%s' and tips='%s'", userMsg->name, userMsg->tips);
	if (sqlite3_get_table(db, sql, &result, &nrow, &ncloum, &errmsg) != SQLITE_OK){
		printf("%s", errmsg);
		exit(1);
	}else {
		if(nrow == 1) {
			sprintf (sql, "select * from user_login where name = '%s';", userMsg->name);
			sqlite3_exec (db, sql, callback2, &clientfd, &errmsg);
		}
		if (nrow == 0) {
			strcpy (userMsg->tips, "usr do not exist or wrong tips");
			if(send(clientfd, userMsg,sizeof(MSG), 0) < 0) {
				perror ("send");
				exit(1);
			}		
		}
	}

	return;
}

void * do_client(void *arg) 
{
	int length;
	int clientfd = *(int *)arg;
	MSG *userMsg = (MSG *)malloc(sizeof(MSG));

	while(1) {
		if ((length = recv(clientfd, userMsg, sizeof(*userMsg), 0)) < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
		}

		printf("%c\n", userMsg->tips[0]);
		switch(userMsg->tips[0]) {
		case 'R':
			do_register(userMsg, clientfd);
			break;
		case 'F':
			do_findPassword(userMsg, clientfd);
			break;
		case 'L':
			do_login(userMsg, clientfd);
			break;
		case 'Q':
			do_quit(userMsg, clientfd);
			return NULL;
			break;
		default:
			printf("Server Error\n");
			break;
		}
	}

	return NULL;
}

int main(int argc, const char *argv[])
{
	pthread_t tid; 		//线程id
#if 0
	if(argc < 3) {
		fprintf(stderr,"Usage :%s  <valid>  <valid>\n",argv[0]);
		exit(EXIT_FAILURE);
	}
#endif
	create_database(); //创建数据库
	socket_init();
	client_len = sizeof(clientaddr);

	while(1) {
		int clientfd;

		//连接客服端
		if ((clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &client_len)) < 0) {
			perror("accept");
			return -1;
		}

		// 为每个 链接进来的 客户端创建一个 线程 
		if (pthread_create(&tid, NULL, do_client, (void *)&clientfd) < 0){ 
			perror("pthread_create failed!\n");
			exit(-1);
		}

	}


	close(listenfd);

	return 0;
}

