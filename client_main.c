#include"common.h"
#include"client_func.h"

#include "constants.h"

joyconlib_t jc[MAX_CLIENTS];

int main(int argc,char *argv[])
{
    int		num;
    char	name[MAX_CLIENTS][MAX_NAME_SIZE];
    int		endFlag=1;
    char	localHostName[]="localhost";
    char	*serverName;
    int		clientID;

    /* 引き数チェック */
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

    /* サーバーとの接続 */
    if(SetUpClient(serverName,&clientID,&num,name)==-1){
		fprintf(stderr,"setup failed : SetUpClient\n");
		return -1;
	}
    /* ウインドウの初期化 */
	if(InitWindow(clientID,num,name)==-1){
		fprintf(stderr,"setup failed : InitWindows\n");
		return -1;
	}

    /*ジョイコンオープン*/
    for(int i= 0; i<num; i++){
  joycon_err err = joycon_open(&jc[i], JOYCON_R);
  if (JOYCON_ERR_NONE != err) {
      printf("joycon open failed:%d\n", err);
      return -1;
  }
  }
    /* メインイベントループ */
    while(endFlag){
      RenderWindow();
		SendClientCommand(clientID);
		endFlag = SendRecvManager();
    };

    /* 終了処理 */
	DestroyWindow();
	CloseSoc();

    return 0;
}