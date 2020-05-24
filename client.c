
#include "client.h"
	
	
//���������
int main(int argc, const char *argv[])
{

	//������������ж� ��ʽ��client <serv_ip> <serv_port>
	if(argc < 3){
		printf("Usage : %s <serv_ip> <serv_port>\n", argv[0]);	
		exit(-1);
	}

	//�����׽��ֳ�ʼ��
	socket_init(argv);
	
	while(1) {
		//��ʼ������
		printf("*****************************************************\n");
		printf("* 1: register  2: login  3: forget password  4: exit*\n");
		printf("*****************************************************\n");
		printf("please choose : ");

		//�������������������˳���Ϣ�����ر��׽����˳�����
		scanf("%d", &n);
		getchar();
		if(n<1 || n>4){
			printf("invalid num!\n");
			exit(1);
		}
		
		//����nֵ���벻ͬ�ĺ���
		switch(n){
			
		case 1:
			//send R  ע��
			msg.tips[0] = 'R';
			send(socketfd, &msg, sizeof(MSG),0);
			do_register(socketfd, &msg);
			break;
			//send L  ��¼
		case 2:
			msg.tips[0] = 'L';
			send(socketfd, &msg, sizeof(MSG),0);	
			do_login(socketfd, &msg);
			break;
			//send F  �һ�����
		case 3:
			msg.tips[0] = 'F';
			send(socketfd, &msg,sizeof(MSG), 0);
			do_forget_password(socketfd, &msg);
			break;
			//send Q  �˳�
		case 4:		
			msg.tips[0] = 'Q';
			send(socketfd, &msg,sizeof(MSG), 0);
			close(socketfd);
			exit(0);
		default:
			printf("input error!\n");
			break;
		}
	}

	return 0;
}

