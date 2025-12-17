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
static GameInfo game_info;

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
    dst.x = ch->point.x;
    dst.y = ch->point.y;
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

void RenderDistance(SDL_Renderer* renderer, TTF_Font* tex, float x, float y)
{
    int distance = DistanceToGoal(x, y);
    char buf[64];
    snprintf(buf, sizeof(buf), "次の惑星まで: %dAU", distance);

    // 白色で描画
    SDL_Color white = {0, 0, 0, 255};
    SDL_Surface *message = TTF_RenderUTF8_Solid(font, buf, white);

    SDL_Texture *textTex = SDL_CreateTextureFromSurface(renderer, message);

    // 描画位置を上部中央に設定
    SDL_Rect dst;
    dst.w = message->w*2;
    dst.h = message->h*2;
    dst.x = 10;
    dst.y = 5;   // 上から少し下げる

    SDL_RenderCopy(renderer, textTex, NULL, &dst);

    SDL_DestroyTexture(textTex);
    SDL_FreeSurface(message);
}

void RenderBackGround(SDL_Renderer* renderer, SDL_Texture* tex)
{
    SDL_Rect src, dst;
    src.w = 45;
    src.h = 24;
    src.x = 0;
    src.y = 0;
    dst.w = MAX_WINDOW_X;
    dst.h = MAX_WINDOW_Y;
    dst.x = 0;
    dst.y = 0;

    SDL_RenderCopy(renderer, tex, &src, &dst);
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

	gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_SOFTWARE);
    game_info.stts = GS_Playing;

    player[0] = IMG_LoadTexture(gMainRenderer, "materials_win/player1.png");
    player[1] = IMG_LoadTexture(gMainRenderer, "materials_win/player2.png");
    player[2] = IMG_LoadTexture(gMainRenderer, "materials_win/player3.png");
    player[3] = IMG_LoadTexture(gMainRenderer, "materials_win/player4.png");
    spaceShip = IMG_LoadTexture(gMainRenderer, "materials_win/spaceship_proto2.png");
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

    float ship_x = game_info.chinf[4].point.x;
    float ship_y = game_info.chinf[4].point.y;
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
            RenderRelativeChara(gMainRenderer, &game_info.chinf[i], enemy, ship_x, ship_y);
            }
        }
    }
    /* ??????????????
     *  ????????????1?????????
     */
    RenderDistance(gMainRenderer, font, ship_x, ship_y);
    SDL_RenderPresent(gMainRenderer);
}

int DistanceToGoal(float x, float y)
{ 
    float d_x = x-GOAL_POSITION_X;
    float d_y = y-GOAL_POSITION_Y;
    float dist = sqrtf(d_x * d_x + d_y * d_y);
    return (int)dist;
}