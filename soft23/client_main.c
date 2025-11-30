/*****************************************************************
�ե�����̾	: client_main.c
��ǽ		: ���饤����ȤΥᥤ��롼����
*****************************************************************/

#include"common.h"
#include"client_func.h"

int main(int argc,char *argv[])
{
    int		num;
    char	name[MAX_CLIENTS][MAX_NAME_SIZE];
    int		endFlag=1;
    char	localHostName[]="localhost";
    char	*serverName;
    int		clientID;

    /* �����������å� */
    if(argc == 1){
    	serverName = localHostName;
    }
    else if(argc == 2){
    	serverName = argv[1];
    }
    else{
		fprintf(stderr, "Usage: %s, Cannot find a Server Name.\n", argv[0]);
		return -1;
    }

    /* �����С��Ȥ���³ */
    if(SetUpClient(serverName,&clientID,&num,name)==-1){
		fprintf(stderr,"setup failed : SetUpClient\n");
		return -1;
	}
    /* ������ɥ��ν���� */
	if(InitWindow(clientID,num,name)==-1){
		fprintf(stderr,"setup failed : InitWindows\n");
		return -1;
	}

    /* �ᥤ�󥤥٥�ȥ롼�� */
    while(endFlag){
		WindowEvent(num);
		endFlag = SendRecvManager();
    };

    /* ��λ���� */
	DestroyWindow();
	CloseSoc();

    return 0;
}
