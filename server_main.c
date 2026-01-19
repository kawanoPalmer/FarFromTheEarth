/*****************************************************************
�ե�����̾	: server_main.c
��ǽ		: �����С��Υᥤ��롼����
*****************************************************************/

#include<SDL2/SDL.h>
#include"server_func.h"

static Uint32 SignalHandler(Uint32 interval, void *param);
extern void AfterPlayingRoop(void);
static Uint32 CallAfter4Sec(Uint32 interval, void *param);

int	endFlag;

int main(int argc,char *argv[])
{
	int	num;
	endFlag = 1;

	/* �����������å� */
	if(argc != 2 && argc != 3){
		fprintf(stderr,"Usage: number of clients\n");
		exit(-1);
	}
	if((num = atoi(argv[1])) < 0 ||  num > MAX_CLIENTS){
		fprintf(stderr,"clients limit = %d \n",MAX_CLIENTS);
		exit(-1);
	}
	
	/* SDL�ν���� */
	if(SDL_Init(SDL_INIT_TIMER) < 0) {
		printf("failed to initialize SDL.\n");
		exit(-1);
	}

	/* ���饤����ȤȤ���³ */
	if(SetUpServer(num) == -1){
		fprintf(stderr,"Cannot setup server\n");
		exit(-1);
	}
	
	InitGameInfo();
	while(endFlag){
		endFlag = SendRecvManager();
	}
	endFlag = 1;
	/* �����߽����Υ��å� */
	SDL_AddTimer(16,SignalHandler,NULL);
	
	/* �ᥤ�󥤥٥�ȥ롼�� */
	while(endFlag == GS_Playing){
		endFlag = SendRecvManager();
	};

	AfterPlayingRoop();
	SDL_AddTimer(4000, CallAfter4Sec, NULL);

	while(endFlag == GS_Result){
		continue;
	}


	return 0;
}

/*****************************************************************
�ؿ�̾  : SignalHandler
��ǽ    : �������Ѵؿ� 
����    : Uint32	interval	: �����ޡ�
		  void		*param		: �����߽����ΰ���
����    : �����ޡ��μ��δֳ�
*****************************************************************/
static Uint32 SignalHandler(Uint32 interval, void *param)
{
	//SendDiamondCommand();
	UpdateEnemy();
	UpdateSdhipOperate();
	UpdateBullets();
	UpdateOxygen();
	BroadcastGameInfo();
	return interval;
}

static Uint32 CallAfter4Sec(Uint32 interval, void *param)
{
	endFlag = GS_End;
	Ending();
    return 0;   // 0 を返すと「一度だけ実行」で終了
}
