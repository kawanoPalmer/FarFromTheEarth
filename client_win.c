#include "client_func.h"
#include <math.h>
#include "common.h"

static TTF_Font* font;
static SDL_Window *gMainWindow;
static SDL_Renderer *gMainRenderer;
static SDL_Texture *player[4];
static SDL_Texture *enemy;
static SDL_Texture *spaceShip;
static SDL_Texture *BackGround;
static SDL_Texture *ObstaclesTex[OBSTACLE_TYPE_NUM];
static SDL_Texture *GoalOBJTex;
static SDL_Texture *EngineFireTex;
static GameInfo game_info;
static CharaInfo ObstaclesInfo[OBSTACLE_MAXNUM];

static int obstacles_num = 0;
static int obstacles_loaded = 0;
extern int cid;

static int g_shake_x = 0;
static int g_shake_y = 0;

Uint32 lastTick;
Uint32 blinkTick;
SDL_bool blinkRed = SDL_FALSE;

static Uint32 playStartTick = 0;
static Uint32 playEndTick   = 0;
static GameStts prev_stts;

const GameInfo* GetGameInfo(void)
{
    return &game_info;
}

int DistanceToGoal(float x, float y);

int RecvInfo(GameInfo *info){
    game_info.stts = info->stts;
    game_info.oxy_amount = info->oxy_amount;
    game_info.oxy_max = info->oxy_max;
    game_info.fireEffect = info->fireEffect;
    for(int i=0; i<CHARA_NUM; i++){
    game_info.chinf[i] = info->chinf[i];
    if (i==4){
    //fprintf(stderr, "%f, %f\n%d\n", game_info.chinf[i].point.x, game_info.chinf[i].point.y, i);
    }
        fprintf(stderr, "interact=%d\n", info->chinf->act);
    }

    for(int i=0; i<MAX_BULLETS; i++){
        game_info.bullets[i] = info->bullets[i];
    }

    return game_info.stts;
}


void RenderTitle(SDL_Renderer* renderer, int cid)
{
    SDL_Color white = {255, 255, 255, 255};

    // --- 「準備OK!」 ---
    SDL_Surface* msg = TTF_RenderUTF8_Solid(font, "xボタンで準備OK!　あなたのキャラクター: ", white);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, msg);

    SDL_Rect dst = {
        .w = msg->w * 2,
        .h = msg->h * 2,
        .x = MAX_WINDOW_X / 2 - (msg->w * 2) / 2,
        .y = 700
    };


    SDL_RenderCopy(renderer, tex, NULL, &dst);

    dst.w = 26;
    dst.h = 43;
    dst.x = 1230;
    dst.y = 717;

    SDL_Rect src;
    src.w = 26;
    src.h = 43;
    src.x = 0;
    src.y = 0;

    SDL_RenderCopy(renderer, player[cid], &src, &dst); 

    SDL_DestroyTexture(tex);
    SDL_FreeSurface(msg);

    // --- タイトル ---
    msg = TTF_RenderUTF8_Solid(font, "FarFromTheEarth", white);
    tex = SDL_CreateTextureFromSurface(renderer, msg);

    dst.w = msg->w * 3;   // 20倍は大きすぎるので調整
    dst.h = msg->h * 3;
    dst.x = MAX_WINDOW_X / 2 - dst.w / 2;
    dst.y = 100;

    SDL_RenderCopy(renderer, tex, NULL, &dst);

    SDL_DestroyTexture(tex);
    SDL_FreeSurface(msg);
}

void RenderResult(SDL_Renderer* renderer)
{
    SDL_Color textColor = {0, 0, 0, 255};

    // テキストサーフェス作成
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, "Game clear!", textColor);
    if (!surface) {
        TTF_CloseFont(font);
        return;
    }

    // テクスチャ化
    SDL_Texture* texture = SDL_CreateTextureFromSurface(gMainRenderer, surface);

    // 中央に配置
    SDL_Rect dst;
    dst.w = surface->w;
    dst.h = surface->h;
    dst.x = (MAX_WINDOW_X - dst.w) / 2;
    dst.y = (MAX_WINDOW_Y - dst.h) / 2;

    SDL_FreeSurface(surface);

    // 描画
    SDL_RenderCopy(gMainRenderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);

    Uint32 playTimeMs = playEndTick - playStartTick; int sec = playTimeMs / 1000; int min = sec / 60; sec %= 60;
    char timeBuf[64]; 
    snprintf(timeBuf, sizeof(timeBuf), "生存時間 %02d:%02d", min, sec);
    SDL_Surface* timeSurface = TTF_RenderUTF8_Blended(font, timeBuf, textColor); 
    SDL_Texture* timeTex = SDL_CreateTextureFromSurface(gMainRenderer, timeSurface); 
    SDL_Rect timeDst; 
    timeDst.w = timeSurface->w; timeDst.h = timeSurface->h;
    timeDst.x = (MAX_WINDOW_X - timeDst.w) / 2; timeDst.y = dst.y + dst.h + 20; 
    SDL_RenderCopy(gMainRenderer, timeTex, NULL, &timeDst);
    SDL_DestroyTexture(timeTex);
    SDL_FreeSurface(timeSurface);

    char killBuf[64];
    snprintf(killBuf, sizeof(killBuf), "撃破数  %d", game_info.killCount);

    SDL_Surface* killSurface = TTF_RenderUTF8_Blended(font, killBuf, textColor);
    SDL_Texture* killTex = SDL_CreateTextureFromSurface(gMainRenderer, killSurface);

    SDL_Rect killDst;
    killDst.w = killSurface->w;
    killDst.h = killSurface->h;
    killDst.x = (MAX_WINDOW_X - killDst.w) / 2;
    killDst.y = timeDst.y + timeDst.h + 10;

    SDL_RenderCopy(gMainRenderer, killTex, NULL, &killDst);

    SDL_FreeSurface(killSurface);
    SDL_DestroyTexture(killTex);

}

void RenderChara(SDL_Renderer* renderer, CharaInfo* ch, SDL_Texture* tex, int cid)
{
    SDL_Rect dst, src;
    dst.x = ch->point.x-ch->w/2;
    dst.y = ch->point.y-ch->h/2;
    dst.w = ch->w;
    dst.h = ch->h;
    src.x = 26 * ch->frameNum;
    src.y = 0;
    src.w = 26;
    src.h = 43;
    if(ch->faceLeft == 1)
        SDL_RenderCopyEx(renderer, tex, &src, &dst, 0, NULL, SDL_FLIP_HORIZONTAL);
    else
        SDL_RenderCopy(renderer, tex, &src, &dst);
}

static inline void WorldToScreen(float obj_x, float obj_y, float ship_x, float ship_y, int *out_x, int *out_y)
{
    int center_x = MAX_WINDOW_X / 2;
    int center_y = MAX_WINDOW_Y / 2;
    *out_x = (int)(obj_x - ship_x) + center_x + g_shake_x;
    *out_y = (int)(obj_y - ship_y) + center_y + g_shake_y;
}


void RenderShip(SDL_Renderer* renderer, SDL_Texture* tex)
{
    SDL_Rect dst;
    dst.w = SPACESHIP_SIZE;
    dst.h = SPACESHIP_SIZE;
    dst.x = MAX_WINDOW_X/2-dst.w/2 + g_shake_x;
    dst.y = MAX_WINDOW_Y/2-dst.h/2 + g_shake_y;

    CharaInfo* ship = &game_info.chinf[ID_SHIP];
    if (ship->damage_timer > 0) {
        // ダメージ中は画像を赤くする (R=255, G=0, B=0)
        SDL_SetTextureColorMod(tex, 255, 0, 0);
    }

    SDL_RenderCopy(renderer, tex, NULL, &dst);

    SDL_SetTextureColorMod(tex, 255, 255, 255);
}

void RenderBar(
    SDL_Renderer* renderer,
    int x, int y,           // 左上座標
    int w, int h,           // バー全体サイズ
    float value, float max, // 現在値 / 最大値
    SDL_Color bg,           // 背景色
    SDL_Color fg            // バー色
)
{
    // 背景
    SDL_Rect back = { x, y, w, h };
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(renderer, &back);

    // 割合計算
    float ratio = value / max;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    // 前景バー
    SDL_Rect front = { x, y, (int)(w * ratio), h };
    SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, fg.a);
    SDL_RenderFillRect(renderer, &front);

    // 枠線（任意）
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &back);
}


void RenderDistance(SDL_Renderer* renderer, float x, float y)
{
    int distance = DistanceToGoal(x, y);
    char buf1[64], buf2[64];
    snprintf(buf1, sizeof(buf1), "次の惑星まで: %dAU", distance);

    // 白色で描画
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *msg_distance = TTF_RenderUTF8_Solid(font, buf1, white);

    SDL_Texture *textTex = SDL_CreateTextureFromSurface(renderer, msg_distance);

    // 描画位置を上部中央に設定
    SDL_Rect dst;
    dst.w = msg_distance->w*2;
    dst.h = msg_distance->h*2;
    dst.x = 10;
    dst.y = 700;   // 上から少し下げる

    SDL_RenderCopy(renderer, textTex, NULL, &dst);


    // ===== 酸素バー =====
SDL_Color bg = {40, 40, 40, 255};
SDL_Color oxy;

float oxy_ratio = game_info.oxy_amount / game_info.oxy_max;

// 残量で色を変える（ゲーム感UP）
if (oxy_ratio > 0.5f)
    oxy = (SDL_Color){100, 200, 255, 255};   // 青
else if (oxy_ratio > 0.25f)
    oxy = (SDL_Color){255, 200, 0, 255};     // 黄
else
    oxy = (SDL_Color){255, 50, 50, 255};     // 赤

RenderBar(
    gMainRenderer,
    20, 20,          // 左上
    300, 20,         // 幅・高さ
    game_info.oxy_amount,
    game_info.oxy_max,
    bg,
    oxy
);


    SDL_DestroyTexture(textTex);
    SDL_FreeSurface(msg_distance);
}

void RenderOxgeLevel(SDL_Renderer* renderer, TTF_Font* tex, float amount, float max)
{
    fprintf(stderr, "amount=%f max=%f\n", amount, max);
    char buf1[64], buf2[128];
    Uint32 now = SDL_GetTicks();

    snprintf(buf1, sizeof(buf1), "酸素量: %d%%", (int)(amount*100/max));
    snprintf(buf2, sizeof(buf2), "///まもなく酸素が枯渇する! 早急に供給を開始せよ!///");

    if (now - blinkTick >= 300) {
        blinkTick = now;
        blinkRed = !blinkRed;
    }


    SDL_Color white = {255, 255, 255, 255};
    SDL_Color red = {255, 50, 50, 255};

    SDL_Surface *message = TTF_RenderUTF8_Solid(font, buf1, white);
    SDL_Texture *textTex = SDL_CreateTextureFromSurface(renderer, message);
    SDL_Surface *alartmsg;

    if(blinkRed)
        alartmsg = TTF_RenderUTF8_Solid(font, buf2, red);
    else
        alartmsg = TTF_RenderUTF8_Solid(font, buf2, white);

    SDL_Texture *textTex2 = SDL_CreateTextureFromSurface(renderer, alartmsg);

    // 描画位置を上部中央に設定
    SDL_Rect dst;
    dst.w = message->w*2;
    dst.h = message->h*2;
    dst.x = 10;
    dst.y = 40;   // 上から少し下げる
    SDL_RenderCopy(renderer, textTex, NULL, &dst);

    if((int)(amount*100/max) < 30){
        dst.w = alartmsg->w;
        dst.h = alartmsg->h;
        dst.x = MAX_WINDOW_X/2 - alartmsg->w/2;
        dst.y = 120;
        SDL_RenderCopy(renderer, textTex2, NULL, &dst);
    }

    SDL_DestroyTexture(textTex);
    SDL_FreeSurface(message);
}

void ShipHp(SDL_Renderer* renderer, TTF_Font* tex, int x)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "ShipHp: %d", x);

    // 白色で描画
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *message = TTF_RenderUTF8_Solid(font, buf, white);

    SDL_Texture *textTex = SDL_CreateTextureFromSurface(renderer, message);

    // 描画位置を上部中央に設定
    SDL_Rect dst;
    dst.w = message->w*2;
    dst.h = message->h*2;
    dst.x = MAX_WINDOW_X - message->w*2 - 10;
    dst.y = 0;   // 上から少し下げる

// ===== HPバー =====
SDL_Color hp_col;
SDL_Color bg = {40, 40, 40, 255};
float hp_ratio = game_info.chinf[ID_SHIP].hp / 1000.0f;

if (hp_ratio > 0.5f)
    hp_col = (SDL_Color){0, 255, 0, 255};
else if (hp_ratio > 0.2f)
    hp_col = (SDL_Color){255, 180, 0, 255};
else
    hp_col = (SDL_Color){255, 0, 0, 255};

RenderBar(
    gMainRenderer,
    MAX_WINDOW_X - 320, 20, // 右上
    300, 20,
    game_info.chinf[ID_SHIP].hp,
    1000.0f,
    bg,
    hp_col
);

    SDL_RenderCopy(renderer, textTex, NULL, &dst);

    SDL_DestroyTexture(textTex);
    SDL_FreeSurface(message);
}


void RenderBackGround(SDL_Renderer* renderer, SDL_Texture* tex, int x, int y)
{

    SDL_Rect src,dst; 
    
    src.w = MAX_WINDOW_X;
    src.h = MAX_WINDOW_Y;
    src.x = x;
    src.y = y;
    dst.w = MAX_WINDOW_X;
    dst.h = MAX_WINDOW_Y;
    dst.x = 0 + g_shake_x;
    dst.y = 0 + g_shake_y;

    SDL_RenderCopy(renderer, tex, &src, &dst);
}

void RenderObstacles(SDL_Renderer* renderer, float ship_x, float ship_y)
{
    int center_x = MAX_WINDOW_X / 2;
    int center_y = MAX_WINDOW_Y / 2;

    for (int i = 0; i < obstacles_num; ++i) {
        int sx, sy;
        WorldToScreen(ObstaclesInfo[i].point.x, ObstaclesInfo[i].point.y, ship_x, ship_y, &sx, &sy);

        SDL_Rect dst;
        dst.x = sx - ObstaclesInfo[i].r;
        dst.y = sy - ObstaclesInfo[i].r;
        dst.w = ObstaclesInfo[i].r * 2;
        dst.h = ObstaclesInfo[i].r * 2;

        // 画面外なら描画をスキップして高速化
        if (dst.x + dst.w < 0 || dst.x > MAX_WINDOW_X || dst.y + dst.h < 0 || dst.y > MAX_WINDOW_Y) continue;

        int type_id = ObstaclesInfo[i].type;

        if (type_id < 0 || type_id >= OBSTACLE_TYPE_NUM) {
            type_id = 0;
        }

        if (ObstaclesTex[type_id]) {
        SDL_RenderCopy(renderer, ObstaclesTex[type_id], NULL, &dst);
        }
    }
    // --- ゴールの描画 ---
    if (GoalOBJTex) {
        int gx, gy;
        WorldToScreen(GOAL_POSITION_X, GOAL_POSITION_Y,
                      ship_x, ship_y, &gx, &gy);

        SDL_Rect goalDst;
        goalDst.x = gx - GOAL_POSITION_R;
        goalDst.y = gy - GOAL_POSITION_R;
        goalDst.w = GOAL_POSITION_R * 2;
        goalDst.h = GOAL_POSITION_R * 2;

        // 画面外ならスキップ
        if (!(goalDst.x + goalDst.w < 0 || goalDst.x > MAX_WINDOW_X ||
              goalDst.y + goalDst.h < 0 || goalDst.y > MAX_WINDOW_Y))
        {
            SDL_RenderCopy(renderer, GoalOBJTex, NULL, &goalDst);
        }
    }

}

void RenderBullets(SDL_Renderer* renderer, float ship_x, float ship_y)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 黄色

    for(int i=0; i<MAX_BULLETS; i++){
        if(game_info.bullets[i].active == 0) continue;

        // 世界座標 -> スクリーン座標変換
        int sx, sy;
        WorldToScreen(game_info.bullets[i].point.x, game_info.bullets[i].point.y, ship_x, ship_y, &sx, &sy);

        SDL_Rect r;
        r.w = BULLET_R * 2;
        r.h = BULLET_R * 2;
        r.x = sx - BULLET_R;
        r.y = sy - BULLET_R;

        SDL_RenderFillRect(renderer, &r);
    }
}

void RenderEngineFire(SDL_Renderer* renderer, SDL_Texture* tex)
{
    SDL_Rect dst;
    dst.w = SPACESHIP_SIZE;
    dst.h = SPACESHIP_SIZE;
    dst.x = MAX_WINDOW_X/2-dst.w/2;
    dst.y = MAX_WINDOW_Y/2-dst.h/2;
    if(game_info.fireEffect != 4)
    {
        SDL_Rect src;
        src.w = 512;
        src.h = 512;
        src.x = 512 * game_info.fireEffect;
        src.y = 0;
        SDL_RenderCopy(renderer, tex, &src, &dst);
    }
    game_info.fireEffect = 4;
}

/* ???????????????????
 *
 * ??
 *   ????: 0
 *   ???  : ??
 */
int InitWindow(int clientID, int num, char name[][MAX_NAME_SIZE])
{
    assert(0<num && num<=MAX_CLIENTS);

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("failed to initialize SDL.\n");
		return -1;
	}

	TTF_Init();
	font = TTF_OpenFont("DotGothic16-Regular.ttf", 24);

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError());
    }

    /** ?????????(????)????????? **/
    if((gMainWindow = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAX_WINDOW_X, MAX_WINDOW_Y, 0)) == NULL) {
		printf("failed to initialize videomode.\n");
		return -1;
	}

	gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_ACCELERATED);
    game_info.stts = GS_Title;

    player[0] = IMG_LoadTexture(gMainRenderer, "materials_win/player1.png");
    player[1] = IMG_LoadTexture(gMainRenderer, "materials_win/player2.png");
    player[2] = IMG_LoadTexture(gMainRenderer, "materials_win/player3.png");
    player[3] = IMG_LoadTexture(gMainRenderer, "materials_win/player4.png");
    spaceShip = IMG_LoadTexture(gMainRenderer, "materials_win/spaceship.png");
    BackGround = IMG_LoadTexture(gMainRenderer, "materials_win/spacebackground1.png"); 
    ObstaclesTex[0] = IMG_LoadTexture(gMainRenderer, "materials_win/obstacle0.png");
    ObstaclesTex[1] = IMG_LoadTexture(gMainRenderer, "materials_win/obstacle1.png");
    ObstaclesTex[2] = IMG_LoadTexture(gMainRenderer, "materials_win/obstacle2.png");
    EngineFireTex = IMG_LoadTexture(gMainRenderer, "materials_win/fire.png");
    GoalOBJTex = IMG_LoadTexture(gMainRenderer, "materials_win/goal.png");

    /** マップ情報読込 **/
    FILE* fp = fopen("materials_win/obstacles.txt", "r");
    if (fp == NULL) {
        return fprintf(stderr, "failed to open map data file.\n");
    }

    /* 前ステージで読み込んだ行数分スキップ */
    for (int i = 0; i < obstacles_loaded; i++) {
        char dummy[256];
        if (!fgets(dummy, sizeof(dummy), fp)) {
            // ファイル終端に到達した場合は break
            break;
        }
    }

    char linebuf[256];
    while (fgets(linebuf, sizeof(linebuf), fp)) {
        if (linebuf[0] == '#') continue;
        if (linebuf[0] == '*'){
            
            break;
        }
        if (obstacles_num < OBSTACLE_MAXNUM) {
            int x, y, r, type;
            int items = sscanf(linebuf, "%d %d %d %d", &x, &y, &r, &type);

            if (items >= 3){
                ObstaclesInfo[obstacles_num].point.x = x;
                ObstaclesInfo[obstacles_num].point.y = y;
                ObstaclesInfo[obstacles_num].r       = r;
            }
            if (items == 4) {
                ObstaclesInfo[obstacles_num].type = type;
            } else {
                ObstaclesInfo[obstacles_num].type = 0;
            }
                obstacles_num++;
                obstacles_loaded++;
            
        }
    }
    fclose(fp);

    BackGround = IMG_LoadTexture(gMainRenderer, "materials_win/spacebackground (1).png"); 
    enemy = IMG_LoadTexture(gMainRenderer, "materials_win/enemy.png");
    game_info.fireEffect = 4;

    prev_stts = GS_Title;
    
    return 0;
}

/* ?????????? */
void DestroyWindow(void)
{
    for(int i = 0; i < 4; i++) {
        if(player[i]) SDL_DestroyTexture(player[i]);
    }
    if(font) TTF_CloseFont(font);
    if(gMainRenderer) SDL_DestroyRenderer(gMainRenderer);
    if(gMainWindow) SDL_DestroyWindow(gMainWindow);

    TTF_Quit();
    SDL_Quit();
}

void RenderRelativeChara(SDL_Renderer* renderer, CharaInfo* ch, SDL_Texture* tex, float ship_x, float ship_y)
{
    if (!tex || !ch) return;
    int sx, sy;
    WorldToScreen(ch->point.x, ch->point.y, ship_x, ship_y, &sx, &sy);

    SDL_Rect dst;
    dst.x = sx - ch->w / 2;
    dst.y = sy - ch->h / 2;
    dst.w = ch->w;
    dst.h = ch->h;

    SDL_RenderCopy(renderer, tex, NULL, &dst);
}



/* ???????
 *  ????????????????????
 */
void RenderWindow(void)
{
    SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
    SDL_RenderClear(gMainRenderer);
    // 状態遷移チェック
    if (prev_stts == GS_Title && game_info.stts == GS_Playing) {
        playStartTick = SDL_GetTicks();
    }
    if (prev_stts == GS_Playing && game_info.stts == GS_Result) {
        playEndTick = SDL_GetTicks();
    }
    prev_stts = game_info.stts;

    switch(game_info.stts){
        case GS_Title:
            RenderBackGround(gMainRenderer, BackGround, 0,0);
            RenderTitle(gMainRenderer, cid);
            SDL_RenderPresent(gMainRenderer);
            break;

        case GS_Playing:{
            float ship_world_x = game_info.chinf[4].point.x;
            float ship_world_y = game_info.chinf[4].point.y;

            CharaInfo* ship = &game_info.chinf[4];

            // ダメージを受けた直後（タイマーが残っている）なら揺らす
            // 例: timerが60から始まり、40になるまでの20フレーム間揺らす
            if (ship->damage_timer > 40) { 
                int power = 10; // 揺れ幅
                g_shake_x = (rand() % (power * 2)) - power; // -10 ～ +10
                g_shake_y = (rand() % (power * 2)) - power;
            } else {
                // 揺れ終了
                g_shake_x = 0;
                g_shake_y = 0;
            }

            RenderBackGround(gMainRenderer, BackGround, (int)(ship_world_x/40), (int)(ship_world_y/40));
            RenderObstacles(gMainRenderer, ship_world_x, ship_world_y);

            RenderShip(gMainRenderer, spaceShip);

            for(int i=0; i<CHARA_NUM; i++){
                
                if(game_info.chinf[i].type == 0 && i >= 4) continue; 

                if (i < 4) {
                    // プレイヤー (0~3) 
                    // 船の上に乗っているので、これまで通り画面座標で描画
                    RenderChara(gMainRenderer, &game_info.chinf[i], player[i], i);
                }
                else if (i == 4) {
                    // 宇宙船自身 (すでにRenderShipで描画しているのでスキップ)
                    continue;
                }
                else {
                    if (game_info.chinf[i].type == CT_Enemy) {
                        if (game_info.chinf[i].stts != CS_Alive) continue;
                    RenderRelativeChara(gMainRenderer, &game_info.chinf[i], enemy, ship_world_x, ship_world_y);
                    }
                }
            }
            /* ??????????????
            *  ????????????1?????????
            */
            RenderDistance(gMainRenderer, ship_world_x, ship_world_y);
            RenderOxgeLevel(gMainRenderer, font, game_info.oxy_amount, game_info.oxy_max);
            ShipHp(gMainRenderer, font, game_info.chinf[ID_SHIP].hp);
            RenderBullets(gMainRenderer, ship_world_x, ship_world_y);
            RenderEngineFire(gMainRenderer, EngineFireTex);

            SDL_RenderPresent(gMainRenderer);
            break;
        }
        case GS_Result :
            RenderResult(gMainRenderer);
            SDL_RenderPresent(gMainRenderer);
            break;
        default:
            break;

    }
}

int DistanceToGoal(float x, float y)
{ 
    float d_x = x-GOAL_POSITION_X;
    float d_y = y-GOAL_POSITION_Y;
    float dist = sqrtf(d_x * d_x + d_y * d_y);
    return (int)dist;
}