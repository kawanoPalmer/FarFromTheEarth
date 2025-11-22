/*****************************************************************
????????	: client_win.c
???		: ?????????¶¥è√Í????????????????????
*****************************************************************/

#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<SDL2/SDL2_gfxPrimitives.h>
#include"common.h"
#include"client_func.h"

static SDL_Window *gMainWindow;
static SDL_Renderer *gMainRenderer;
static SDL_Rect gButtonRect[MAX_CLIENTS+2];

static int CheckButtonNO(int x,int y,int num);

/*****************************************************************
????	: InitWindows
???	: ???????????????????????
????	: int	clientID		: ????????????
		  int	num				: ????????????
????	: ????????????????0????????????-1
*****************************************************************/
int InitWindows(int clientID,int num,char name[][MAX_NAME_SIZE])
{
	int i;
	SDL_Texture *texture;
	SDL_Surface *image;
	SDL_Rect src_rect;
	SDL_Rect dest_rect;
	char clientButton[4][6]={"0.jpg","1.jpg","2.jpg","3.jpg"};
	char endButton[]="END.jpg";
	char allButton[]="ALL.jpg";
	char *s,title[10];

    /* ???????????è´©? */
    assert(0<num && num<=MAX_CLIENTS);
	
	/* SDL?¶Õ???? */
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("failed to initialize SDL.\n");
		return -1;
	}
	
	/* ????¶¥???????????????? */
	if((gMainWindow = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 300, 0)) == NULL) {
		printf("failed to initialize videomode.\n");
		return -1;
	}

	gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_SOFTWARE);//SDL_RENDERER_ACCELERATED |SDL_RENDERER_PRESENTVSYNC);//0);

	/* ?????????¶¥???????è´©? */
	sprintf(title,"%d",clientID);
	SDL_SetWindowTitle(gMainWindow, title);
	
	/* ?????????? */
	SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
  	SDL_RenderClear(gMainRenderer);

	/* ?????¶ ??? */
	for(i=0;i<num+2;i++){
		gButtonRect[i].x = 20+80*i;
		gButtonRect[i].y=10;
		gButtonRect[i].w=70;
		gButtonRect[i].h=20;
      
		if(i==num){
			s=allButton;
		}
		else if(i==num+1){
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
????	: DestroyWindow
???	: SDL??¶À????
????	: ???
????	: ???
*****************************************************************/
void DestroyWindow(void)
{
	SDL_Quit();
}

/*****************************************************************
????	: WindowEvent
???	: ????????????ß∂??????????????
????	: int		num		: ????????????
????	: ???
*****************************************************************/
void WindowEvent(int num)
{
	SDL_Event event;
	SDL_KeyboardEvent *key;
	int buttonNO;

    /* ???????????è´©? */
    assert(0<num && num<=MAX_CLIENTS);

	if(SDL_PollEvent(&event)){

		switch(event.type){
			case SDL_QUIT:
				SendEndCommand();
				break;
			case SDL_KEYDOWN:
				key = (SDL_KeyboardEvent*)&event;
				Sendinfo(1);
				break;
#ifndef NDEBUG
					printf("#####\n");
					printf("WindowEvent()\n");
					printf("Button %d is pressed\n",buttonNO);
#endif
					
		}
	}
}

/*****************************************************************
????	: DrawRectangle
???	: ?????????????????????
????	: int		x			: ???¶ ???? x ???
		  int		y			: ???¶ ???? y ???
		  int		width		: ???¶¬???
		  int		height		: ???¶…?
????	: ???
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
