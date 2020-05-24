#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
	int  id;             //Ա���ı��
	char name[20];       
	char passwd[20];
	char tips[50];       //���ڴ�����Ϣ����
	int  type;					 //0λroot�û���1Ϊ��ͨ�û�
	char sex[10];
	int  age;
	char phone[11];
	char addr[50];
}MSG;

//һЩȫ�ֱ���
int socketfd;      								//���������׽���
struct sockaddr_in serveraddr;    //����������Ϣ�ṹ��
typedef struct sockaddr  SA;
MSG msg;                          //������Ϣ�ṹ��
int n;														//��������
//һЩ�����Ķ���
void socket_init();
void do_register(int socketfd, MSG *msg);
int do_login(int socketfd, MSG *msg);
void do_forget_password(int socketfd, MSG *msg);
void do_add_user(int socketfd, MSG * msg);
void do_delete_user(int socketfd, MSG * msg);
void do_update_general_user(int socketfd, MSG * msg);
void do_update_root_user(int socketfd, MSG * msg);

void do_search_general_user(int socketfd, MSG * msg);
void do_search_root_user(int socketfd, MSG * msg);

void do_root_user(int socketfd, MSG *msg);
void do_general_user(int socketfd, MSG *msg);

#endif

