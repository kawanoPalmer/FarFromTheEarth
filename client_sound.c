#include <SDL2/SDL_mixer.h>
#include "client_func.h"
#include "common.h"

//static GameInfo game_info;
static Mix_Chunk* se_shot = NULL;
//static int prev_bullet[MAX_BULLETS] = {0};
static Mix_Chunk* se_alarm = NULL;
static Mix_Chunk* se_damage = NULL;

int InitSound(void)
{
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    se_shot = Mix_LoadWAV("materials_sound/se_shot.wav");
    se_alarm = Mix_LoadWAV("materials_sound/se_alarm.mp3");
    se_damage = Mix_LoadWAV("materials_sound/se_damage.mp3");
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

    static int alarm_timer = 0;
    static int alarm_channel = 1;

    float max = game_info->oxy_max;
    float current = game_info->oxy_amount;
    if (max <= 0) max = 100.0f;
    float ratio = current / max;
    if (ratio < 0.3f && current > 0.0f) {

    if (alarm_timer <= 0) {
            if (se_alarm) {
                Mix_HaltChannel(alarm_channel);
                Mix_PlayChannel(alarm_channel, se_alarm, 0);
            }
            
            // 次に鳴らすまでの間隔を設定 (60フレーム = 1秒)
            // 酸素が減るほど間隔を短くする演出
            if (ratio < 0.1f) {
                alarm_timer = 30; // 0.5秒間隔 (超ピンチ！)
            } else {
                alarm_timer = 60; // 1.0秒間隔 (ピンチ)
            }
        }
        
        // 毎フレームタイマーを減らす
        alarm_timer--;
    } else {
        Mix_HaltChannel(alarm_channel);
        alarm_timer = 0;
    }

    static int prev_damage_timer = 0;
    int current_timer = game_info->chinf[ID_SHIP].damage_timer;

    if (current_timer > prev_damage_timer) {
        if (se_damage) {
            // 被弾音は重要なので、重ならないように強制再生しても良いですが、
            // 空いているチャンネルで鳴らす(-1)のが無難です。
            Mix_PlayChannel(-1, se_damage, 0);
        }
    }
    prev_damage_timer = current_timer;
}

void Sound_Quit(void)
{
    Mix_FreeChunk(se_shot);

    if (se_alarm) Mix_FreeChunk(se_alarm);
    if (se_damage) Mix_FreeChunk(se_damage);
    Mix_CloseAudio();
}
