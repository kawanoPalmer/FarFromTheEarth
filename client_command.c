#include "common.h"
#include"client_func.h"
#include <arpa/inet.h>
#include <joyconlib.h>

extern joyconlib_t jc;

/*JoyCon入力し、Directionから正規化
サーバへ送信*/

static void SetIntData2DataBlock(void *data,int intData,int *dataSize);
static void SetCharData2DataBlock(void *data,char charData,int *dataSize);
static void PackClientCommand(const ClientCommand *cmd, unsigned char *buf, int *size);

void SendClientCommand(int client_id)
{
    unsigned char data[MAX_DATA];
    int dataSize = 0;

    // JoyConのスティック入力
    Direction d = GetJoyConStick(client_id);
    // 正規化された方向ベクトル
    FloatPoint vec = DirToVector(d);

    // サーバに送るクライアント入力情報の作成
    ClientCommand cmd;
    cmd.client_id = client_id; // クライアントのID
    cmd.dir       = vec;         // 移動方向ベクトル
    cmd.velocity  = 1.0f;        // 速度固定

    // 構造体から送信用バイト列に変換
    PackClientCommand(&cmd, data, &dataSize);

    #ifndef NDEBUG
    printf("SendClientCommand(): id=%d vec=(%.2f, %.2f)\n",
        cmd.client_id, cmd.dir.x, cmd.dir.y);
    #endif
    // サーバーに送信
    SendData(data, dataSize);
}

/*JoyConのスティック入力状態を読み取り、Direction構造体に変換
　返り値:Direction
   up/down/left/right の4方向を bool で保持した構造体
  DEADZONE 未満の場合は無視して 0 とする
*/
Direction GetJoyConStick(int clientID)
{
    // 全ての方向をfalseで初期化
    Direction dir = {SDL_FALSE, SDL_FALSE, SDL_FALSE, SDL_FALSE}; 
    // JoyConの状態更新
    joycon_get_state(&jc);

    float x = jc.stick.x; //左右入力
    float y = jc.stick.y; //上下入力
    const float DEADZONE = 0.3f;

    float s_x = y;
    float s_y = x;
    // 左右判定
    if (s_x < -DEADZONE) dir.right = SDL_TRUE;
    if (s_x > DEADZONE) dir.left = SDL_TRUE;

    //上下判定
    if (s_y > DEADZONE) dir.down = SDL_TRUE;
    if (s_y < -DEADZONE) dir.up = SDL_TRUE;    

    return dir;
}

/*Direction → 方向ベクトルに変換
  返り値：FloatPoint
   x, y の正規化した2D移動ベクトルを返す
   入力が0の場合は (0,0) を返す
*/
FloatPoint DirToVector(Direction d)
{
    FloatPoint v = {0.0f, 0.0f};
    // 左右方向
    if (d.right) v.x = 1.0f;
    if (d.left)  v.x = -1.0f;
    // 上下方向
    if (d.down) v.y = 1.0f;
    if (d.up)   v.y = -1.0f;

    // ベクトルの長さ
    float len = sqrtf(v.x * v.x + v.y * v.y);

    // 斜め方向など、ベクトル長が0でなければ正規化する
    if (len > 0.0f) {
    v.x /= len;
    v.y /= len;
    }
    return v;
}

/*ClientCommand 構造体を「送信用バイト列」に変換する関数
引数：
    cmd  - クライアント入力コマンド
    buf  - 変換後のバイト列を書き込む配列
    size - 書き込んだバイト数（out）
*/
static void PackClientCommand(const ClientCommand *cmd, unsigned char *buf, int *size)
{
    int offset = 0; // 初期化

    int32_t cid = htonl(cmd->client_id);
    memcpy(buf + offset, &cid, sizeof(cid));
    offset += sizeof(cid);
    
    int32_t act = htonl((int32_t)cmd->act);
    memcpy(buf + offset, &act, sizeof(act));
    offset += sizeof(act);

    uint32_t x_bits;
    memcpy(&x_bits, &cmd->dir.x, sizeof(float));
    x_bits = htonl(x_bits);
    memcpy(buf + offset, &x_bits, sizeof(x_bits));
    offset += sizeof(x_bits);

    uint32_t y_bits;
    memcpy(&y_bits, &cmd->dir.y, sizeof(float));
    y_bits = htonl(y_bits);
    memcpy(buf + offset, &y_bits, sizeof(y_bits));
    offset += sizeof(y_bits);

    uint32_t v_bits;
    memcpy(&v_bits, &cmd->velocity, sizeof(float));
    v_bits = htonl(v_bits);
    memcpy(buf + offset, &v_bits, sizeof(v_bits));
    offset += sizeof(v_bits);

    // 実際のデータサイズを呼び出し元へ返す
    *size = offset;
}