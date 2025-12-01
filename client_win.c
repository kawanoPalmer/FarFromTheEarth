#include "client_func.h"
#include <math.h>
#include "common.h"

static SDL_Rect gButtonRect[MAX_CLIENTS+2];
static TTF_Font* font;
static SDL_Window *gMainWindow;
static SDL_Renderer *gMainRenderer;
static SDL_Texture *player[4];
static GameInfo game_info;

void RecvInfo(GameInfo *info){
    for(int i=0; i<MAX_CLIENTS; i++){
    game_info.chinf[i] = info->chinf[i];
    fprintf(stderr, "%f, %f\n%d\n", game_info.chinf[i].point.x, game_info.chinf[i].point.y, i);
    }
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
	font = TTF_OpenFont("DotGothic16/DotGothic16-Regular.ttf", 24);

    /** ?????????(????)????????? **/
    if((gMainWindow = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0)) == NULL) {
		printf("failed to initialize videomode.\n");
		return -1;
	}

	gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_SOFTWARE);

    player[0] = IMG_LoadTexture(gMainRenderer, "materials_win/player1.png");
    player[1] = IMG_LoadTexture(gMainRenderer, "materials_win/player2.png");
    player[2] = IMG_LoadTexture(gMainRenderer, "materials_win/player3.png");
    player[3] = IMG_LoadTexture(gMainRenderer, "materials_win/player4.png");

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


/* ???????
 *  ????????????????????
 */
void RenderWindow(void)
{
    SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
    SDL_RenderClear(gMainRenderer);

    for(int i=0; i<4; i++){
        RenderChara(gMainRenderer, &game_info.chinf[i], player[i], i);
    }
    /* ??????????????
     *  ????????????1?????????
     */
    SDL_RenderPresent(gMainRenderer);
}