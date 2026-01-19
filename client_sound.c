#include <SDL2/SDL_mixer.h>
#include "client_func.h"
#include "common.h"

//static GameInfo game_info;
static Mix_Chunk* se_shot = NULL;
//static int prev_bullet[MAX_BULLETS] = {0};

int InitSound(void)
{
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    se_shot = Mix_LoadWAV("materials_sound/se_shot.wav");
    return 0;
}

void Sound_Update(void)
{
    const GameInfo* game_info = GetGameInfo();
    static int prev_bullet[MAX_BULLETS] = {0};

    for(int i=0;i<MAX_BULLETS;i++){
        if(prev_bullet[i]==0 && game_info->bullets[i].active==1){
            fprintf(stderr,"soundupdate");
            Mix_PlayChannel(-1, se_shot, 0);
        }
        prev_bullet[i]=game_info->bullets[i].active;
    }
}

void Sound_Quit(void)
{
    Mix_FreeChunk(se_shot);
    Mix_CloseAudio();
}
