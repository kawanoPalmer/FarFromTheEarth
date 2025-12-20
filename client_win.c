#include "client_func.h"
#include <math.h>
#include "common.h"

static TTF_Font* font;
static SDL_Window *gMainWindow;
static SDL_Renderer *gMainRenderer;
static SDL_Texture *player[4];
static SDL_Texture *spaceShip;
static SDL_Texture *BackGround;
static SDL_Texture *ObstaclesTex;
static GameInfo game_info;
static CharaInfo ObstaclesInfo[OBSTACLE_MAXNUM];

static int obstacles_num = 0;
static int obstacles_loaded = 0;
int DistanceToGoal(float x, float y);

void RecvInfo(GameInfo *info){
    for(int i=0; i<CHARA_NUM; i++){
    game_info.chinf[i] = info->chinf[i];
    if (i==4){
    fprintf(stderr, "%f, %f\n%d\n", game_info.chinf[i].point.x, game_info.chinf[i].point.y, i);
    }
    }
}

int RecvStts(GameInfo *info)
{
    int endFlag = 1;
    if(info->stts == GS_End)
        endFlag = 0;

    return endFlag;
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

void RenderObstacles(SDL_Renderer* renderer, SDL_Texture* tex, int ship_x, int ship_y)
{
    int center_x = MAX_WINDOW_X/2;
    int center_y = MAX_WINDOW_Y/2;
    for (int i=0; i<obstacles_num; i++){
        SDL_Rect dst;
        dst.x = (ObstaclesInfo[i].point.x - ship_x) + center_x - ObstaclesInfo[i].r;
        dst.y = (ObstaclesInfo[i].point.y - ship_y) + center_y - ObstaclesInfo[i].r;
        dst.w = ObstaclesInfo[i].r * 2;
        dst.h = ObstaclesInfo[i].r * 2;
        SDL_RenderCopy(renderer, tex, NULL, &dst);
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
    game_info.stts = GS_Playing;

    player[0] = IMG_LoadTexture(gMainRenderer, "materials_win/player1.png");
    player[1] = IMG_LoadTexture(gMainRenderer, "materials_win/player2.png");
    player[2] = IMG_LoadTexture(gMainRenderer, "materials_win/player3.png");
    player[3] = IMG_LoadTexture(gMainRenderer, "materials_win/player4.png");
    spaceShip = IMG_LoadTexture(gMainRenderer, "materials_win/spaceship_proto2.png");
    BackGround = IMG_LoadTexture(gMainRenderer, "materials_win/spacebackground (1).png"); 
    ObstaclesTex = IMG_LoadTexture(gMainRenderer, "materials_win/obstacle.png");

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
    SDL_Rect dst;
    
    // 画面中央（船の中心）の座標
    int center_x = MAX_WINDOW_X / 2;
    int center_y = MAX_WINDOW_Y / 2;

    // 計算式: (オブジェクトの座標 - 船の座標) + 画面オフセット
    // 船が右(x+)に進むと、(obj - ship) は小さくなるので、オブジェクトは左に流れる
    dst.x = (int)(ch->point.x - ship_x) + center_x - ch->w / 2;
    dst.y = (int)(ch->point.y - ship_y) + center_y - ch->h / 2;
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

    float ship_x = game_info.chinf[4].point.x + MAX_WINDOW_X/2-250;
    float ship_y = game_info.chinf[4].point.y + MAX_WINDOW_Y/2-250;

    RenderBackGround(gMainRenderer, BackGround, (int)(ship_x/10), (int)(ship_y/10));
    RenderObstacles(gMainRenderer, ObstaclesTex, ship_x, ship_y);

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
            // 敵・ゴール (5~6) 
            // 船の動きに合わせて動かす（相対描画）
            // ※ここでは仮に player[0] の画像を敵として使ってる、
            //   本来は敵やゴール用のテクスチャを用意して
            RenderRelativeChara(gMainRenderer, &game_info.chinf[i], player[0], ship_x, ship_y);
        }
    }
    /* ??????????????
     *  ????????????1?????????
     */
    RenderDistance(gMainRenderer, ship_x, ship_y);
    SDL_RenderPresent(gMainRenderer);
}

int DistanceToGoal(float x, float y)
{ 
    float d_x = x-GOAL_POSITION_X;
    float d_y = y-GOAL_POSITION_Y;
    float dist = sqrtf(d_x * d_x + d_y * d_y);
    return (int)dist;
}