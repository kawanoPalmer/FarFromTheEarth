#ifndef _COMMON_H_
#define _COMMON_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<assert.h>
#include<math.h>
#include <SDL2/SDL.h>
#include <joyconlib.h> 
#include<SDL2/SDL_image.h>
#include<SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#define PORT			(u_short)8888	/* ポート番号 */

#define MAX_CLIENTS		4				/* クライアント数の最大値 */
#define MAX_NAME_SIZE	10 				/* ユーザー名の最大値*/
#define MAX_WINDOW_X 1500
#define MAX_WINDOW_Y 800

#define MAX_DATA		20000				/* 送受信するデータの最大値 */

#define CHARATYPE_MAXNUM 4 // キャラタイプ総数
#define CHARA_NUM 7 //クライアント(4) + ゴール(1) + 敵(1) + 宇宙船(1)

//キャラクターの種類（列挙体）
typedef enum {
    CT_Player  = 0, // プレイヤー
    CT_Goal    = 1, // ゴール（地球）
    CT_Enemy   = 2, // 敵
    CT_Ship    = 3, // 宇宙船
} CharaType;

//キャラクターの状態　(列挙体)
typedef enum {
    CS_Normal = 1, // 通常
    CS_Action = 2, // アクション
    CS_Damege = 4  // 被弾 (宇宙船被弾用)
} CharaStts;

typedef enum{
    GS_End     = 0, /* 終了 */
    GS_Playing = 1, /* ゲーム中 */
    GS_Stay    = 2, //クライアント待機中
    GS_Result  = 3  //いるかわからんけどリザルト画面とか続けて遊ぶとか選べるかも
}GameStts;

typedef enum {
    AT_OpX = 0,//操縦（左右）
    AT_OpY = 1,//操縦（上下）
    AT_AtL = 2,//攻撃（左）
    AT_AtR = 3,//攻撃（右）
    AT_Pet = 4,//燃料
    AT_Oxg = 5 //酸素
} ActionType;

/* 実数座標 */
typedef struct {
    float x;
    float y;
} FloatPoint;

/*追加
スティック入力状態*/
typedef struct {
    SDL_bool up;   //上
    SDL_bool down; //下
    SDL_bool left; //左
    SDL_bool right;//右
}Direction;

/* キャラクターの情報 */
typedef struct {
    int client_id;
    CharaType type;   //キャラタイプ
    CharaStts stts;   //キャラの状態（基本は通常）
    ActionType act;
    float basevel;    /* 速度の大きさの基準値 */
    float velocity;   /* 現在の速度の大きさ */
    FloatPoint dir;   /* 現在の方向（大きさ最大1となる成分） */
    FloatPoint point; /* 現在の座標 */
    int w;            /* キャラの幅 */
    int h;            /* 　　　　高さ */
    int hp;           //hp(宇宙船しかつかわんかも)
} CharaInfo;


/* ゲームの情報 (この構造体をサーバーからブロードキャストで送信したりする想定)*/
typedef struct {
    GameStts stts;
    CharaInfo chinf[CHARA_NUM];
} GameInfo;


//クライアントが変更したものをサーバーに送る
typedef struct {
    int32_t client_id;  // クライアントを識別（サーバーが誰の入力か判定）
    int act;           // 入力ボタン
    FloatPoint dir;     // 入力方向（スティック方向）
    float velocity;     // 速度（移動や操作の強さ）
} ClientCommand;


#endif