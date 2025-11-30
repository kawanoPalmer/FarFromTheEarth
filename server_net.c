/*****************************************************************
鐃春ワ申鐃緒申鐃緒申名	: server_net.c
鐃緒申能		: 鐃緒申鐃緒申鐃出￥申鐃塾ネットワー鐃緒申鐃緒申鐃緒申
*****************************************************************/

#include"server_common.h"
#include"server_func.h"
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include <unistd.h>

/* 鐃緒申鐃初イ鐃緒申鐃緒申箸鐃宿緒申鐃緒申鐃渋わ申鐃� */
typedef struct{
	int		fd;
	char	name[MAX_NAME_SIZE];
}CLIENT;

static CLIENT	gClients[MAX_CLIENTS];	/* 鐃緒申鐃初イ鐃緒申鐃緒申鐃� */
static int	gClientNum;					/* 鐃緒申鐃初イ鐃緒申鐃緒申反鐃� */

static fd_set	gMask;					/* select()鐃術のマワ申鐃緒申 */
static int	gWidth;						/* gMask鐃緒申離鐃緒申鐃緒申奪鐃緒申鐃緒申戮鐃緒申咼奪反鐃� */

static int MultiAccept(int request_soc,int num);
static void Enter(int pos, int fd);
static void SetMask(int maxfd);
static void SendAllName(void);
static int RecvData(int pos,void *data,int dataSize);

/*****************************************************************
鐃舜随申名	: SetUpServer
鐃緒申能	: 鐃緒申鐃初イ鐃緒申鐃緒申箸箸離鐃緒申優鐃緒申鐃緒申鐃緒申鐃緒申鐃塾�鐃緒申鐃緒申
		  鐃醇ー鐃緒申鐃緒申鐃緒申名鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申圓鐃�
鐃緒申鐃緒申	: int		num		  : 鐃緒申鐃初イ鐃緒申鐃緒申反鐃�
鐃緒申鐃緒申	: 鐃緒申鐃粛ワ申鐃緒申鐃緒申鐃祝種申鐃峻わ申鐃緒申鐃緒申-1,鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申0
*****************************************************************/
int SetUpServer(int num)
{
    struct sockaddr_in	server;
    int			request_soc;
    int                 maxfd;
    int			val = 1;
 
    /* 鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申鐃獣ワ申 */
    assert(0<num && num<=MAX_CLIENTS);

    gClientNum = num;
    bzero((char*)&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 鐃緒申鐃緒申鐃獣トわ申鐃緒申鐃緒申鐃緒申鐃� */
    if((request_soc = socket(AF_INET,SOCK_STREAM,0)) < 0){
		fprintf(stderr,"Socket allocation failed\n");
		return -1;
    }
    setsockopt(request_soc,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));

    /* 鐃緒申鐃緒申鐃獣トわ申名鐃緒申鐃緒申弔鐃緒申鐃� */
    if(bind(request_soc,(struct sockaddr*)&server,sizeof(server))==-1){
		fprintf(stderr,"Cannot bind\n");
		close(request_soc);
		return -1;
    }
    fprintf(stderr,"Successfully bind!\n");
    
    /* 鐃緒申鐃初イ鐃緒申鐃緒申箸鐃緒申鐃緒申鐃緒申続鐃竣居申鐃緒申圓鐃� */
    if(listen(request_soc, gClientNum) == -1){
		fprintf(stderr,"Cannot listen\n");
		close(request_soc);
		return -1;
    }
    fprintf(stderr,"Listen OK\n");

    /* 鐃緒申鐃初イ鐃緒申鐃緒申箸鐃緒申鐃渋鰹申鐃緒申鐃� */
    maxfd = MultiAccept(request_soc, gClientNum);
    close(request_soc);
    if(maxfd == -1)return -1;

    /* 鐃緒申鐃緒申鐃初イ鐃緒申鐃緒申箸鐃緒申鐃緒申罅種申鐃緒申鐃縮常申鐃緒申鐃緒申鐃� */
    SendAllName();

    /* select()鐃塾わ申鐃緒申離泪鐃緒申鐃緒申佑鐃緒申鐃緒申蠅刻申鐃� */
    SetMask(maxfd);

    return 0;
}

/*****************************************************************
鐃舜随申名	: SendRecvManager
鐃緒申能	: 鐃緒申鐃緒申鐃出￥申鐃緒申鐃緒申鐃緒申鐃緒申鐃銃わ申鐃緒申鐃叔￥申鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申鐃�
鐃緒申鐃緒申	: 鐃淑わ申
鐃緒申鐃緒申	: 鐃竣ワ申鐃緒申鐃緒申狃�了鐃緒申鐃殉ワ申匹鐃緒申鐃緒申鐃緒申討鐃緒申鐃緒申鐃�0鐃緒申鐃瞬わ申鐃緒申
		  鐃緒申鐃緒申奮鐃緒申鐃�1鐃緒申鐃瞬わ申
*****************************************************************/
int SendRecvManager(void)
{
    unsigned char	buf[1024];
    //今送られているデータのサイズ
    fd_set	readOK;
    int		i;
    int		endFlag = 1;

    readOK = gMask;
    /* 鐃緒申鐃初イ鐃緒申鐃緒申箸鐃緒申鐃叔￥申鐃緒申鐃緒申鐃熟わ申鐃銃わ申鐃暑か調鐃駿わ申 */
    if(select(gWidth,&readOK,NULL,NULL,NULL) < 0){
        /* 鐃緒申鐃初ー鐃緒申鐃緒申鐃緒申鐃獣わ申 */
        return endFlag;
    }
    
    for(i=0;i<gClientNum;i++){
		if(FD_ISSET(gClients[i].fd,&readOK)){
	    	if(recv(gClients[i].fd, buf, sizeof(ClientCommand), 0) > 0){
        	ProcessClientData(buf, sizeof(ClientCommand));
		    }
    }
    }
    BroadcastGameInfo();
    return endFlag;
}

/*****************************************************************
鐃舜随申名	: RecvIntData
鐃緒申能	: 鐃緒申鐃初イ鐃緒申鐃緒申箸鐃緒申鐃�int鐃緒申鐃塾デ￥申鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申
鐃緒申鐃緒申	: int		pos	        : 鐃緒申鐃初イ鐃緒申鐃緒申鐃緒申峭鐃�
		  int		*intData	: 鐃緒申鐃緒申鐃緒申鐃緒申鐃叔￥申鐃緒申
鐃緒申鐃緒申	: 鐃緒申鐃緒申鐃緒申辰鐃緒申丱鐃緒申反鐃�
*****************************************************************/
int RecvIntData(int pos,int *intData)
{
    int n,tmp;
    
    /* 鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申鐃獣ワ申 */
    assert(0<=pos && pos<gClientNum);
    assert(intData!=NULL);

    n = RecvData(pos,&tmp,sizeof(int));
    (*intData) = ntohl(tmp);
    
    return n;
}

/*****************************************************************
鐃舜随申名	: SendData
鐃緒申能	: 鐃緒申鐃初イ鐃緒申鐃緒申箸縫如鐃緒申鐃緒申鐃緒申鐃緒申鐃�
鐃緒申鐃緒申	: int	   pos		: 鐃緒申鐃初イ鐃緒申鐃緒申鐃緒申峭鐃�
							  ALL_CLIENTS鐃緒申鐃緒申鐃所さ鐃曙た鐃緒申鐃祝わ申鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申
		  void	   *data	: 鐃緒申鐃緒申如鐃緒申鐃�
		  int	   dataSize	: 鐃緒申鐃緒申如鐃緒申鐃緒申離鐃緒申鐃緒申鐃�
鐃緒申鐃緒申	: 鐃淑わ申
*****************************************************************/
void SendData(int pos,void *data,int dataSize)
{
    int	i;
   
    /* 鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申鐃獣ワ申 */
    assert(0<=pos && pos<gClientNum || pos==ALL_CLIENTS);
    assert(data!=NULL);
    assert(0<dataSize);

    if(pos == ALL_CLIENTS){
    	/* 鐃緒申鐃緒申鐃初イ鐃緒申鐃緒申箸縫如鐃緒申鐃緒申鐃緒申鐃緒申鐃� */
		for(i=0;i<gClientNum;i++){
			write(gClients[i].fd,data,dataSize);
		}
    }
    else{
		write(gClients[pos].fd,data,dataSize);
    }
}

/*****************************************************************
鐃舜随申名	: Ending
鐃緒申能	: 鐃緒申鐃緒申鐃初イ鐃緒申鐃緒申箸箸離鐃緒申優鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申任鐃緒申鐃�
鐃緒申鐃緒申	: 鐃淑わ申
鐃緒申鐃緒申	: 鐃淑わ申
*****************************************************************/
void Ending(void)
{
    int	i;

    printf("... Connection closed\n");
    for(i=0;i<gClientNum;i++)close(gClients[i].fd);
}

/*****
static
*****/
/*****************************************************************
鐃舜随申名	: MultiAccept
鐃緒申能	: 鐃緒申続鐃竣居申里鐃緒申辰鐃緒申鐃緒申薀わ申鐃緒申鐃夙とのワ申鐃粛ワ申鐃緒申鐃緒申鐃緒申鐃緒申立鐃緒申鐃緒申
鐃緒申鐃緒申	: int		request_soc	: 鐃緒申鐃緒申鐃獣ワ申
		  int		num     	: 鐃緒申鐃初イ鐃緒申鐃緒申反鐃�
鐃緒申鐃緒申	: 鐃緒申鐃緒申鐃獣トデワ申鐃緒申鐃緒申鐃緒申廛鐃�
*****************************************************************/
static int MultiAccept(int request_soc,int num)
{
    int	i,j;
    int	fd;
    
    for(i=0;i<num;i++){
		if((fd = accept(request_soc,NULL,NULL)) == -1){
			fprintf(stderr,"Accept error\n");
			for(j=i-1;j>=0;j--)close(gClients[j].fd);
			return -1;
		}
		Enter(i,fd);
    }
    return fd;
}

/*****************************************************************
鐃舜随申名	: Enter
鐃緒申能	: 鐃緒申鐃初イ鐃緒申鐃緒申箸離罅種申鐃緒申鐃縮常申鐃緒申鐃緒申鐃緒申鐃緒申
鐃緒申鐃緒申	: int		pos		: 鐃緒申鐃初イ鐃緒申鐃緒申鐃緒申峭鐃�
		  int		fd		: 鐃緒申鐃緒申鐃獣トデワ申鐃緒申鐃緒申鐃緒申廛鐃�
鐃緒申鐃緒申	: 鐃淑わ申
*****************************************************************/
static void Enter(int pos, int fd)
{
	/* 鐃緒申鐃初イ鐃緒申鐃緒申箸離罅種申鐃緒申鐃縮常申鐃緒申鐃緒申鐃緒申鐃緒申 */
	read(fd,gClients[pos].name,MAX_NAME_SIZE);
#ifndef NDEBUG
	printf("%s is accepted\n",gClients[pos].name);
#endif
	gClients[pos].fd = fd;
}

/*****************************************************************
鐃舜随申名	: SetMask
鐃緒申能	: int		maxfd	: 鐃緒申鐃緒申鐃獣トデワ申鐃緒申鐃緒申鐃緒申廛鐃緒申虜鐃緒申鐃緒申鐃�
鐃緒申鐃緒申	: 鐃淑わ申
鐃緒申鐃緒申	: 鐃淑わ申
*****************************************************************/
static void SetMask(int maxfd)
{
    int	i;

    gWidth = maxfd+1;

    FD_ZERO(&gMask);    
    for(i=0;i<gClientNum;i++)FD_SET(gClients[i].fd,&gMask);
}

/*****************************************************************
鐃舜随申名	: SendAllName
鐃緒申能	: 鐃緒申鐃緒申鐃初イ鐃緒申鐃緒申箸鐃緒申鐃緒申罅種申鐃緒申鐃縮常申鐃緒申鐃緒申鐃�
鐃緒申鐃緒申	: 鐃淑わ申
鐃緒申鐃緒申	: 鐃淑わ申
*****************************************************************/
static void SendAllName(void)
{
  int	i,j,tmp1,tmp2;

    tmp2 = htonl(gClientNum);
    for(i=0;i<gClientNum;i++){
		tmp1 = htonl(i);
		SendData(i,&tmp1,sizeof(int));
		SendData(i,&tmp2,sizeof(int));
		for(j=0;j<gClientNum;j++){
			SendData(i,gClients[j].name,MAX_NAME_SIZE);
		}
	}
}

/*****************************************************************
鐃舜随申名	: RecvData
鐃緒申能	: 鐃緒申鐃初イ鐃緒申鐃緒申箸鐃緒申鐃叔￥申鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申
鐃緒申鐃緒申	: int		pos	        : 鐃緒申鐃初イ鐃緒申鐃緒申鐃緒申峭鐃�
		  void		*data		: 鐃緒申鐃緒申鐃緒申鐃緒申鐃叔￥申鐃緒申
		  int		dataSize	: 鐃緒申鐃緒申鐃緒申鐃緒申如鐃緒申鐃緒申離鐃緒申鐃緒申鐃�
鐃緒申鐃緒申	: 鐃緒申鐃緒申鐃緒申辰鐃緒申丱鐃緒申反鐃�
*****************************************************************/
static int RecvData(int pos,void *data,int dataSize)
{
    int n;
    
    /* 鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申鐃獣ワ申 */
    assert(0<=pos && pos<gClientNum);
    assert(data!=NULL);
    assert(0<dataSize);

    n = read(gClients[pos].fd,data,dataSize);
    
    return n;
}
