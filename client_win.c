/*****************************************************************
鐃春ワ申鐃緒申鐃緒申名	: client_win.c
鐃緒申能		: 鐃緒申鐃初イ鐃緒申鐃緒申箸離罅種申鐃緒申鐃緒申鐃緒申鵐拭鐃緒申侫鐃緒申鐃緒申鐃緒申鐃緒申鐃?
*****************************************************************/

#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include"common.h"
#include"client_func.h"

static SDL_Window *gMainWindow;
static SDL_Renderer *gMainRenderer;
static SDL_Rect gButtonRect[MAX_CLIENTS+2];
static TTF_Font* font;

static int CheckButtonNO(int x,int y,int num);
static int n=0;
/*****************************************************************
鐃舜随申名	: InitWindows
鐃緒申能	: 鐃潤イ鐃藷ウワ申鐃緒申疋鐃緒申鐃宿緒申鐃緒申鐃緒申鐃緒申鐃緒申圓鐃?
鐃緒申鐃緒申	: int	clientID		: 鐃緒申鐃初イ鐃緒申鐃緒申鐃緒申峭鐃?
		  int	num				: 鐃緒申鐃緒申鐃初イ鐃緒申鐃緒申反鐃?
鐃緒申鐃緒申	: 鐃緒申鐃緒申鐃緒申鐃緒申鐃叔わ申鐃緒申鐃夙わ申0鐃緒申鐃緒申鐃峻わ申鐃緒申鐃夙わ申-1
*****************************************************************/
int InitWindows(int clientID,int num,char name[][MAX_NAME_SIZE])
{
	int i;                                                                                       
	SDL_Texture *texture;
	SDL_Surface *image;
	SDL_Rect src_rect;
	SDL_Rect dest_rect;
	char clientButton[4][12]={"rock.jpg","paper.jpg","scissor.jpg","3.jpg"};
	char endButton[]="END.jpg";
	char allButton[]="ALL.jpg";

	char *s,title[10];

    /* 鐃緒申鐃緒申鐃緒申鐃緒申鐃緒申鐃獣ワ申 */
    assert(0<num && num<=MAX_CLIENTS);
	
	/* SDL鐃塾緒申鐃緒申鐃? */
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("failed to initialize SDL.\n");
		return -1;
	}

	TTF_Init();
	font = TTF_OpenFont("DotGothic16/DotGothic16-Regular.ttf", 24);
	
	/* 鐃潤イ鐃緒申離鐃緒申鐃緒申鐃宿ワ申鐃緒申鐃緒申鐃緒申鐃緒申鐃? */
	if((gMainWindow = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 300, 0)) == NULL) {
		printf("failed to initialize videomode.\n");
		return -1;
	}

	gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_SOFTWARE);//SDL_RENDERER_ACCELERATED |SDL_RENDERER_PRESENTVSYNC);//0);

	/* 鐃緒申鐃緒申鐃緒申疋鐃緒申離鐃緒申鐃緒申肇鐃薯セッワ申 */
	sprintf(title,"%d",clientID);
	SDL_SetWindowTitle(gMainWindow, title);
	
	/* 鐃舜景わ申鐃緒申砲鐃緒申鐃? */
	SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
  	SDL_RenderClear(gMainRenderer);

	/* 鐃旬ワ申鐃緒申虜鐃緒申鐃? */
	for(i=0;i<4;i++){
		gButtonRect[i].x = 20+91*i;
		gButtonRect[i].y=10;
		gButtonRect[i].w=91;
		gButtonRect[i].h=62;
      
		if(i==3){
			s=endButton;
		}
		else{
			s=clientButton[i];
		}
		image = IMG_Load(s);
		texture = SDL_CreateTextureFromSurface(gMainRenderer, image);
		src_rect = (SDL_Rect){0, 0, image->w, image->h};
		SDL_RenderCopy(gMainRenderer, texture, &src_rect, (&gButtonRect[i]));
		SDL_FreeSurface(image);
	}
	SDL_RenderPresent(gMainRenderer);
	
	return 0;
}

/*****************************************************************
鐃舜随申名	: DestroyWindow
鐃緒申能	: SDL鐃緒申了鐃緒申鐃緒申
鐃緒申鐃緒申	: 鐃淑わ申
鐃緒申鐃緒申	: 鐃淑わ申
*****************************************************************/
void DestroyWindow(void)
{
	SDL_Quit();
}

/*****************************************************************
鐃舜随申名	: WindowEvent
鐃緒申能	: 鐃潤イ鐃藷ウワ申鐃緒申疋鐃緒申鐃緒申个鐃緒申襯わ申戰鐃夙緒申鐃緒申鐃緒申圓鐃?
鐃緒申鐃緒申	: int		num		: 鐃緒申鐃緒申鐃初イ鐃緒申鐃緒申反鐃?
鐃緒申鐃緒申	: 鐃淑わ申
*****************************************************************/
void WindowEvent(int num)
{
    SDL_Event event;
    SDL_MouseButtonEvent *mouse;
    int buttonNO;

    assert(0 < num && num <= MAX_CLIENTS);

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            SendEndCommand();
            break;
        case SDL_MOUSEBUTTONUP:
            mouse = (SDL_MouseButtonEvent *)&event;
            if (mouse->button == SDL_BUTTON_LEFT) {
                fprintf(stderr, "MOUSE: x=%d y=%d\n", mouse->x, mouse->y); fflush(stderr);
                buttonNO = CheckButtonNO(mouse->x, mouse->y, num);
                fprintf(stderr, "DEBUG: Button %d is pressed\n", buttonNO); fflush(stderr);
                if (buttonNO < 0) break;
                switch (buttonNO) {
                case 0: SendCommand(ROCK_COMMAND); break;
                case 1: SendCommand(PAPER_COMMAND); break;
                case 2: SendCommand(SCISSORS_COMMAND); break;
                case 3: SendCommand(END_COMMAND); break;
                default: break;
                }
            }
            break;
        default:
            break;
        }
    }
}


/*****************************************************************
鐃舜随申名	: DrawRectangle
鐃緒申能	: 鐃潤イ鐃藷ウワ申鐃緒申疋鐃緒申忙由僂鐃宿緒申鐃緒申鐃緒申鐃?
鐃緒申鐃緒申	: int		x			: 鐃粛角の削申鐃緒申鐃? x 鐃緒申標
		  int		y			: 鐃粛角の削申鐃緒申鐃? y 鐃緒申標
		  int		width		: 鐃粛角の駕申鐃緒申
		  int		height		: 鐃粛角の高さ
鐃緒申鐃緒申	: 鐃淑わ申
*****************************************************************/
void DrawRectangle(int x,int y,int width,int height)
{
#ifndef NDEBUG
    printf("#####\n");
    printf("DrawRectangle()\n");
    printf("x=%d,y=%d,width=%d,height=%d\n",x,y,width,height);
#endif


	rectangleColor(gMainRenderer,x,y,x+width,y+height,0xff0000ff);
	SDL_RenderPresent(gMainRenderer);

}

/*****************************************************************
鐃舜随申名	: DrawCircle
鐃緒申能	: 鐃潤イ鐃藷ウワ申鐃緒申疋鐃緒申鳳澆鐃宿緒申鐃緒申鐃緒申鐃?
鐃緒申鐃緒申	: int		x		: 鐃淳わ申 x 鐃緒申標
		  int		y		: 鐃淳わ申 y 鐃緒申標
		  int		r		: 鐃淳わ申半鐃緒申
鐃緒申鐃緒申	: 鐃淑わ申
*****************************************************************/
void DrawCircle(int x,int y,int r)
{
#ifndef NDEBUG
	printf("#####\n");
    printf("DrawCircle()\n");
    printf("x=%d,y=%d,tyokkei=%d\n",x,y,r);
#endif

     circleColor(gMainRenderer,x,y,r,0xff0000ff);
	SDL_RenderPresent(gMainRenderer);
}

/*****************************************************************
鐃舜随申名	: DrawDiamond
鐃緒申能	: 鐃潤イ鐃藷ウワ申鐃緒申疋鐃緒申鐃宿?鐃緒申鐃緒申表鐃緒申鐃緒申鐃緒申
鐃緒申鐃緒申	: int		x		: 鐃緒申鐃緒申鐃? x 鐃緒申標
		  int		y		: 鐃緒申鐃緒申鐃? y 鐃緒申標
		  int		height		: 鐃盾さ
鐃緒申鐃緒申	: 鐃淑わ申
*****************************************************************/
void DrawDiamond(int x,int y,int height)
{
	Sint16	vx[5],vy[5];
	int	i;

#ifndef NDEBUG
    printf("#####\n");
    printf("DrawDiamond()\n");
    printf("x=%d,y=%d,height=%d\n",x,y,height);
#endif

    for(i=0;i<4;i++){
        vx[i] = x + height*((1-i)%2)/2;
        vy[i] = y + height*((2-i)%2);
    }
    vx[4]=vx[0];
    vy[4]=vy[0];
	
	polygonColor(gMainRenderer, vx, vy, 5 , 0xff0000ff);
	SDL_RenderPresent(gMainRenderer);

}


void ShowResult(int result)
{
    if (!gMainRenderer) return;

    const char *msg;
    switch (result) {
    case DRAW_RESULT: msg = "引き分け"; break;
    case WIN1_RESULT: msg = "Client0の勝ち"; break;
    case WIN2_RESULT: msg = "Client1の勝ち"; break;
    default: msg = "結果不明"; break;
    }

    if (!font) {
        fprintf(stderr, "ShowResult: font is NULL -> %s\n", msg); fflush(stderr);
        return;
    }

    SDL_Color color = {0, 0, 0, 255};
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, msg, color);
    if (!surf) {
        fprintf(stderr, "TTF_RenderUTF8_Blended failed: %s\n", TTF_GetError()); fflush(stderr);
        return;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(gMainRenderer, surf);
    if (!tex) {
        fprintf(stderr, "SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError()); fflush(stderr);
        SDL_FreeSurface(surf);
        return;
    }
    SDL_FreeSurface(surf);

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    int tw, th;
    SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
    int winW = 0, winH = 0;
    SDL_GetWindowSize(gMainWindow, &winW, &winH);
    SDL_Rect dst = { (winW - tw) / 2, (winH - th) / 2, tw, th };

    static SDL_Rect prev = {0,0,0,0};
    static int has_prev = 0;

    if (has_prev) {
        SDL_SetRenderDrawBlendMode(gMainRenderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255); 
        SDL_RenderFillRect(gMainRenderer, &prev);
    }


    SDL_RenderCopy(gMainRenderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_RenderPresent(gMainRenderer);

    const int pad = 4;
    prev.x = dst.x - pad;
    prev.y = dst.y - pad;
    prev.w = dst.w + pad * 2;
    prev.h = dst.h + pad * 2;
    has_prev = 1;
}

/*****
static
*****/
/*****************************************************************
鐃舜随申名	: CheckButtonNO
鐃緒申能	: 鐃緒申鐃緒申奪鐃緒申鐃緒申譴随申椒鐃緒申鐃緒申鐃瞬刻申鐃緒申屬鐃?
鐃緒申鐃緒申	: int	   x		: 鐃殉ワ申鐃緒申鐃塾駕申鐃緒申鐃曙た x 鐃緒申標
		  int	   y		: 鐃殉ワ申鐃緒申鐃塾駕申鐃緒申鐃曙た y 鐃緒申標
		  char	   num		: 鐃緒申鐃緒申鐃初イ鐃緒申鐃緒申反鐃?
鐃緒申鐃緒申	: 鐃緒申鐃緒申鐃曙た鐃旬ワ申鐃緒申鐃緒申峭鐃緒申鐃瞬わ申
		  鐃旬ワ申鐃藷が駕申鐃緒申鐃緒申討鐃緒申覆鐃緒申鐃緒申鐃?-1鐃緒申鐃瞬わ申
*****************************************************************/
static int CheckButtonNO(int x,int y,int num)
{
	int i;

 	for(i=0;i<4;i++){
		if(gButtonRect[i].x < x &&
			gButtonRect[i].y < y &&
      		gButtonRect[i].x + gButtonRect[i].w > x &&
			gButtonRect[i].y + gButtonRect[i].h > y){
			return i;
		}
	}
 	return -1;
}
