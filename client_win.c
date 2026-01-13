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
static SDL_Texture *ObstaclesTex;
static SDL_Texture *GoalOBJTex;
static GameInfo game_info;
static CharaInfo ObstaclesInfo[OBSTACLE_MAXNUM];

static int obstacles_num = 0;
static int obstacles_loaded = 0;
int DistanceToGoal(float x, float y);

int RecvInfo(GameInfo *info){
    game_info.stts = info->stts;
    game_info.oxy_amount = info->oxy_amount;
    game_info.oxy_max = info->oxy_max;
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


void RenderTitle(SDL_Renderer* renderer)
{
    SDL_Color white = {255, 255, 255, 255};

    // --- 「準備OK!」 ---
    SDL_Surface* msg = TTF_RenderUTF8_Solid(font, "xボタンで準備OK!", white);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, msg);

    SDL_Rect dst = {
        .w = msg->w * 2,
        .h = msg->h * 2,
        .x = MAX_WINDOW_X / 2 - (msg->w * 2) / 2,
        .y = 700
    };

    SDL_RenderCopy(renderer, tex, NULL, &dst);

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

}

void RenderChara(SDL_Renderer* renderer, CharaInfo* ch, SDL_Texture* tex, int cid)
{
    SDL_Rect dst;
    dst.x = ch->point.x-ch->w/2;
    dst.y = ch->point.y-ch->h/2;
    dst.w = ch->w;
    dst.h = ch->h;

    SDL_RenderCopy(renderer, tex, NULL, &dst);
}

static inline void WorldToScreen(float obj_x, float obj_y, float ship_x, float ship_y, int *out_x, int *out_y)
{
    int center_x = MAX_WINDOW_X / 2;
    int center_y = MAX_WINDOW_Y / 2;
    *out_x = (int)(obj_x - ship_x) + center_x;
    *out_y = (int)(obj_y - ship_y) + center_y;
}


void RenderShip(SDL_Renderer* renderer, SDL_Texture* tex)
{
    SDL_Rect dst;
    dst.w = SPACESHIP_SIZE;
    dst.h = SPACESHIP_SIZE;
    dst.x = MAX_WINDOW_X/2-dst.w/2;
    dst.y = MAX_WINDOW_Y/2-dst.h/2;

    SDL_RenderCopy(renderer, tex, NULL, &dst);
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

    SDL_DestroyTexture(textTex);
    SDL_FreeSurface(msg_distance);
}

void RenderOxgeLevel(SDL_Renderer* renderer, TTF_Font* tex, float amount, float max)
{
    //fprintf(stderr, "amount=%f max=%f\n", amount, max);
    char buf[64];
    //snprintf(buf, sizeof(buf), "酸素量: %d%%", (int)(amount*100/max));

    // 白色で描画
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *message = TTF_RenderUTF8_Solid(font, buf, white);

    SDL_Texture *textTex = SDL_CreateTextureFromSurface(renderer, message);

    // 描画位置を上部中央に設定
    SDL_Rect dst;
    dst.w = message->w*2;
    dst.h = message->h*2;
    dst.x = 10;
    dst.y = 40;   // 上から少し下げる

    SDL_RenderCopy(renderer, textTex, NULL, &dst);

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
    dst.x = 0;
    dst.y = 0;

    SDL_RenderCopy(renderer, tex, &src, &dst);
}

void RenderObstacles(SDL_Renderer* renderer, SDL_Texture* tex, float ship_x, float ship_y)
{
    if (!tex) return;
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

        SDL_RenderCopy(renderer, tex, NULL, &dst);
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
    BackGround = IMG_LoadTexture(gMainRenderer, "materials_win/spacebackground (1).png"); 
    ObstaclesTex = IMG_LoadTexture(gMainRenderer, "materials_win/obstacle.png");
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
            int x, y, r;
            if (sscanf(linebuf, "%d %d %d", &x, &y, &r) == 3) {
                ObstaclesInfo[obstacles_num].point.x = x;
                ObstaclesInfo[obstacles_num].point.y = y;
                ObstaclesInfo[obstacles_num].r       = r;
                obstacles_num++;
                obstacles_loaded++;
            }
        }
    }
    fclose(fp);

    BackGround = IMG_LoadTexture(gMainRenderer, "materials_win/spacebackground.png"); 
    enemy = IMG_LoadTexture(gMainRenderer, "materials_win/enemy_sample.png");
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
    switch(game_info.stts){
        case GS_Title:
            RenderBackGround(gMainRenderer, BackGround, 0,0);
            RenderTitle(gMainRenderer);
            SDL_RenderPresent(gMainRenderer);
            break;

        case GS_Playing:{
            float ship_world_x = game_info.chinf[4].point.x;
            float ship_world_y = game_info.chinf[4].point.y;

            RenderBackGround(gMainRenderer, BackGround, (int)(ship_world_x/40), (int)(ship_world_y/40));
            RenderObstacles(gMainRenderer, ObstaclesTex, ship_world_x, ship_world_y);

            /*Uint8 r = (int)fabs(ship_x) % 255;
            Uint8 g = (int)fabs(ship_y) % 255;
            SDL_SetRenderDrawColor(gMainRenderer, r, g, 50, 255);
            SDL_RenderClear(gMainRenderer);*/
            RenderShip(gMainRenderer, spaceShip);

            /*for(int i=0; i<4; i++){
                RenderChara(gMainRenderer, &game_info.chinf[i], player[i], i);
            }*/

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