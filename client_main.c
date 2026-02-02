#include"common.h"
#include"client_func.h"

#include "constants.h"
#include <SDL2/SDL_mixer.h>
joyconlib_t jc;
SDL_Event e;
extern Mix_Music* bgm_interstellar;


int main(int argc,char *argv[])
{
    int     num;
    char    name[MAX_CLIENTS][MAX_NAME_SIZE];
    int     endFlag = GS_Title;
    char    localHostName[]="localhost";
    char    *serverName;
    int     clientID;

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

    /* サウンドの初期化 */
    if(InitSound() != 0){
        fprintf(stderr,"setup failed : InitSound\n");
        return -1;
    }

    /* ジョイコンオープン */
    joycon_err err = joycon_open(&jc, JOYCON_R);
    if (JOYCON_ERR_NONE != err) {
        printf("joycon open failed:%d\n", err);
        return -1;
    }
    
    /* メインイベントループ */
    while(endFlag != GS_End){
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                endFlag = GS_End;
            }
        }
        Sound_Update();
        RenderWindow();
        SendClientCommand(clientID);
        endFlag = SendRecvManager();
    };

    /* ★ BGM 停止 */
    Mix_HaltMusic();

    /* 終了処理 */
    joycon_close(&jc);
    DestroyWindow();
    Sound_Quit();
    CloseSoc();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}