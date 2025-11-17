#include "client_func_win.h"
#include <math.h>
#include "common.h"

static SDL_Rect gButtonRect[MAX_CLIENTS+2];
static TTF_Font* font;
static SDL_Window *gMainWindow;
static SDL_Renderer *gMainRenderer;

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
int InitWindow(int num, char name[][MAX_NAME_SIZE])
{
    SDL_Texture *texture;
    SDL_Surface *player[4];

    assert(0<num && num<=MAX_CLIENTS);

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("failed to initialize SDL.\n");
		return -1;
	}

	TTF_Init();
	font = TTF_OpenFont("DotGothic16/DotGothic16-Regular.ttf", 24);

    /** ?????????(????)????????? **/
    if((gMainWindow = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 300, 0)) == NULL) {
		printf("failed to initialize videomode.\n");
		return -1;
	}

	gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_SOFTWARE);

    player[0] = IMG_LoadTexture(gMainRenderer, "player1.png");
    player[1]  = IMG_LoadTexture(gMainRenderer, "player2.png");
    player[2] = IMG_LoadTexture(gMainRenderer, "player3.png");
    player[3] = IMG_LoadTexture(gMainRenderer, "player4.png");
    return 0;
}

/* ?????????? */
void DestroyWindow(void)
{
    SDL_Quit();
}

/* ???????
 *  ????????????????????
 */
void RenderWindow(void)
{

    for(int i=0; i<4; i++){
        RenderChara();
    }
    /* ??????????????
     *  ????????????1?????????
     */
    SDL_RenderPresent(gMainRenderer);
}