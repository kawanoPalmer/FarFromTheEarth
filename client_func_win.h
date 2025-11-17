#include<SDL2/SDL_image.h>
#include<SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#define MAX_NAME_SIZE 10



extern int InitWindows(int clientID,int num,char name[][MAX_NAME_SIZE]);
extern void DestroyWindow(void);
extern void WindowEvent(int num);
extern void DrawRectangle(int x,int y,int width,int height);
extern void DrawCircle(int x,int y,int r);
extern void DrawDiamond(int x,int y,int height);
extern void ShowResult(int result);

