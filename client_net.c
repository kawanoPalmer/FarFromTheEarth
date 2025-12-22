/*****************************************************************
�ե�����̾	: client_net.c
��ǽ		: ���饤����ȤΥͥåȥ������
*****************************************************************/

#include"common.h"
#include"client_func.h"
#include<sys/socket.h>
#include<netdb.h>
#include <unistd.h>

#define	BUF_SIZE	100

static int	gSocket;	/* �����å� */
static fd_set	gMask;	/* select()�ѤΥޥ��� */
static int	gWidth;		/* gMask��ΤΥ����å����٤��ӥåȿ� */

static void GetAllName(int *clientID,int *num,char clientNames[][MAX_NAME_SIZE]);
static void SetMask(void);
int RecvData(void *data,int dataSize);

/*****************************************************************
�ؿ�̾	: SetUpClient
��ǽ	: �����С��ȤΥ��ͥ���������Ω����
		  �桼������̾������������Ԥ�
����	: char	*hostName		: �ۥ���
		  int	*num			: �����饤����ȿ�
		  char	clientNames[][]		: �����饤����ȤΥ桼����̾
����	: ���ͥ������˼��Ԥ�����-1,����������0
*****************************************************************/
int SetUpClient(char *hostName,int *clientID,int *num,char clientNames[][MAX_NAME_SIZE])
{
    struct hostent	*servHost;
    struct sockaddr_in	server;
    int			len;
    char		str[BUF_SIZE];

    /* �ۥ���̾����ۥ��Ⱦ�������� */
    if((servHost = gethostbyname(hostName))==NULL){
		fprintf(stderr,"Unknown host\n");
		return -1;
    }

    bzero((char*)&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    bcopy(servHost->h_addr,(char*)&server.sin_addr,servHost->h_length);

    /* �����åȤ�������� */
    if((gSocket = socket(AF_INET,SOCK_STREAM,0)) < 0){
		fprintf(stderr,"socket allocation failed\n");
		return -1;
    }

    /* �����С�����³���� */
    if(connect(gSocket,(struct sockaddr*)&server,sizeof(server)) == -1){
		fprintf(stderr,"cannot connect\n");
		close(gSocket);
		return -1;
    }
    fprintf(stderr,"connected\n");

    /* ̾�����ɤ߹��ߥ����С������� */
    do{
		printf("Enter Your Name\n");
		fgets(str,BUF_SIZE,stdin);
		len = strlen(str)-1;
		str[len]='\0';
    }while(len>MAX_NAME_SIZE-1 || len==0);
    SendData(str,MAX_NAME_SIZE);

    printf("Please Wait\n");

    /* �����饤����ȤΥ桼����̾������ */
    GetAllName(clientID,num,clientNames);

    /* select()�Τ���Υޥ����ͤ����ꤹ�� */
    SetMask();
    
    return 0;
}

/*****************************************************************
�ؿ�̾	: SendRecvManager
��ǽ	: �����С����������Ƥ����ǡ������������
����	: �ʤ�
����	: �ץ�����ཪλ���ޥ�ɤ������Ƥ�����0���֤���
		  ����ʳ���1���֤�
*****************************************************************/
int SendRecvManager(void)
{
    fd_set	readOK;
    char	player_position[MAX_CLIENTS];
    int		i;
    int		endFlag = 1;
    struct timeval	timeout;
    GameInfo Info; 

    /* select()���Ԥ����֤����ꤹ�� */
    timeout.tv_sec = 0;
    timeout.tv_usec = 20;

    readOK = gMask;
    /* �����С�����ǡ������Ϥ��Ƥ��뤫Ĵ�٤� */
    select(gWidth,&readOK,NULL,NULL,&timeout);
    if(FD_ISSET(gSocket,&readOK)){
		/* �����С�����ǡ������Ϥ��Ƥ��� */
    	/* ���ޥ�ɤ��ɤ߹��� */
		RecvData(&Info, sizeof(GameInfo));
    endFlag = RecvInfo(&Info);
    	/* ���ޥ�ɤ��Ф��������Ԥ� */
		//endFlag = ExecuteCommand(command);
    }//END_COMMAND��ä���EndFlag��Ω�ä�server_main.c�Υ��٥�ȥ롼�פ���λ���롣
    return endFlag;
}

/*****************************************************************
�ؿ�̾	: RecvIntData
��ǽ	: �����С�����int���Υǡ�����������
����	: int		*intData	: ���������ǡ���
����	: ������ä��Х��ȿ�
*****************************************************************/
int RecvIntData(int *intData)
{
    int n,tmp;
    
    /* �����������å� */
    assert(intData!=NULL);

    n = RecvData(&tmp,sizeof(int));
    (*intData) = ntohl(tmp);
    
    return n;
}

/*****************************************************************
�ؿ�̾	: SendData
��ǽ	: �����С��˥ǡ���������
����	: void		*data		: ����ǡ���
		  int		dataSize	: ����ǡ����Υ�����
����	: �ʤ�
*****************************************************************/
void SendData(void *data,int dataSize)
{
    /* �����������å� */
    assert(data != NULL);
    assert(0 < dataSize);

    write(gSocket,data,dataSize);
}

/*****************************************************************
�ؿ�̾	: CloseSoc
��ǽ	: �����С��ȤΥ��ͥ����������Ǥ���
����	: �ʤ�
����	: �ʤ�
*****************************************************************/
void CloseSoc(void)
{
    printf("...Connection closed\n");
    close(gSocket);
}

/*****
static
*****/
/*****************************************************************
�ؿ�̾	: GetAllName
��ǽ	: �����С����������饤����ȤΥ桼����̾���������
����	: int		*num			: ���饤����ȿ�
		  char		clientNames[][]	: �����饤����ȤΥ桼����̾
����	: �ʤ�
*****************************************************************/
static void GetAllName(int *clientID,int *num,char clientNames[][MAX_NAME_SIZE])
{
    int	i;

    /* ���饤������ֹ���ɤ߹��� */
    RecvIntData(clientID);
    /* ���饤����ȿ����ɤ߹��� */
    RecvIntData(num);

    /* �����饤����ȤΥ桼����̾���ɤ߹��� */
    for(i=0;i<(*num);i++){
		RecvData(clientNames[i],MAX_NAME_SIZE);
    }
#ifndef NDEBUG
    printf("#####\n");
    printf("client number = %d\n",(*num));
    for(i=0;i<(*num);i++){
		printf("%d:%s\n",i,clientNames[i]);
    }
#endif
}

/*****************************************************************
�ؿ�̾	: SetMask
��ǽ	: select()�Τ���Υޥ����ͤ����ꤹ��
����	: �ʤ�
����	: �ʤ�
*****************************************************************/
static void SetMask(void)
{
    int	i;

    FD_ZERO(&gMask);
    FD_SET(gSocket,&gMask);

    gWidth = gSocket+1;
}

/*****************************************************************
�ؿ�̾	: RecvData
��ǽ	: �����С�����ǡ�����������
����	: void		*data		: ���������ǡ���
		  int		dataSize	: ��������ǡ����Υ�����
����	: ������ä��Х��ȿ�
*****************************************************************/
int RecvData(void *data,int dataSize)
{
    /* �����������å� */
    assert(data != NULL);
    assert(0 < dataSize);
    char *buf = (char *)data;
    int received = 0;
    int n;
    //return read(gSocket,data,dataSize);
    while (received < dataSize) {
        n = read(gSocket, buf + received, dataSize - received);
        received += n;
    }

    return received;
}
