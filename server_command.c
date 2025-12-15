#include"server_common.h"
#include"server_func.h"
#include <arpa/inet.h>


/*サーバが管理するゲーム全体情報(プレイヤ位置など)*/
GameInfo game_info;
ClientCommand clientsCommand[MAX_CLIENTS];

SDL_Surface* mask;

int ColorDecision(SDL_Surface *surface, int x, int y){
    Uint8 r, g, b;

    if (!surface) return 0;

    //x,yを画像を描画する位置のぶんだけズラす。
    x -= MAX_WINDOW_X/2-SPACESHIP_SIZE/2;
    y -= MAX_WINDOW_Y/2-SPACESHIP_SIZE/2;
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    Uint32 pixel = 0;

    switch (bpp) {
        case 1:
            pixel = *p;
            break;
        case 2:
            pixel = *(Uint16 *)p;
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                pixel = p[0] << 16 | p[1] << 8 | p[2];
            else
                pixel = p[0] | p[1] << 8 | p[2] << 16;
            break;
        case 4:
            pixel = *(Uint32 *)p;
            break;
    }

    SDL_GetRGB(pixel, surface->format, &r, &g, &b);
    fprintf(stderr, "r=%d, g=%d, b=%d\n", r, g, b);

    //以上で座標上のRGBを取得、以下で色によって分岐する。
    if (r == 255 && g == 0 && b == 0) {
        // 赤
        return  IT_MoveR;
    } else if (r == 85 && g == 255 && b == 0) {
        // 緑
        return IT_AttackUpper;
    } else if (r == 0 && g == 85 && b == 255) {
        // 青
        return IT_MoveL;
    } else if (r == 255 && g == 255 && b == 0) {
        // 黄
        return IT_AttackLower;
    } else if (r == 170 && g == 0 && b == 255) {
        // 紫
        return IT_TaskOxy;
    } else if (r == 255 && g == 255 && b == 255) {
        // 白
        return FT_Passable;
    } else if (r == 0 && g == 0 && b == 0) {
        // 黒
        return FT_Unpassible;
    } else 
        return 999;

}

void ExecuteCommand(CharaInfo *ch, const ClientCommand *cmd)
{
    int Interaction = ColorDecision(mask, ch->point.x, ch->point.y);
    fprintf(stderr, "Your Interact: %d\n", Interaction);

    CharaInfo *ship = &game_info.chinf[ID_SHIP];
    //クライアントから送られてきたスティックの方向ベクトルを取得
    FloatPoint stick_vec = cmd->dir;
    float stick_len = sqrtf(stick_vec.x * stick_vec.x + stick_vec.y * stick_vec.y);

    // スティックが傾けられていない場合は処理をスキップ 
    if (stick_len < 0.1f) return; 

    // 船の速度 = 基本速度 * クライアントから送られてきた速度（現在は1.0f固定）
    float actual_speed = SHIP_BASE_SPEED * cmd->velocity;

    static int cooldown = 0;
    if (cooldown > 0) cooldown--;

    switch (Interaction) {
        //IT_MoveR: 左右操作 赤
        case IT_MoveR: 
            // スティックのX軸入力を船のX移動に使う
            ship->point.x += stick_vec.x * actual_speed;
            fprintf(stderr, "[Ship] Move X: %.1f\n", ship->point.x);
            break;
            
        //IT_MoveL: 上下操作 青
        case IT_MoveL: 
            // スティックのY軸入力を船のY移動に使う (SDLではYが増加すると下へ移動)
            ship->point.y += stick_vec.y * actual_speed;
            fprintf(stderr, "[Ship] Move Y: %.1f\n", ship->point.y);
            break;
            
        case IT_AttackUpper:
        if (cmd->act == 'B' && cooldown == 0) {

        }   
             break;
        case IT_AttackLower:
        if (cmd->act == 'B' && cooldown == 0) {
            
        }
             
        default:
            break;
    }
}

/*クライアントからのコマンド受信
  引数
    buf  : 受信したバイト配列
    size : データサイズ
    cmd  : 復元先の ClientCommand 構造体

  返り値
    0  = 正常に復元完了  
   -1 = データサイズ不足による失敗
*/
int UnpackClientCommand(const unsigned char *buf, int size, ClientCommand *cmd)
{
    // 必要バイト数（client_id + x + y + velocity）が揃っているか確認
    if (size < sizeof(int32_t) + 3 * sizeof(uint32_t)) return -1;

    int offset = 0;

    int32_t cid;
    memcpy(&cid, buf + offset, sizeof(cid));
    cmd->client_id = ntohl(cid);
    offset += sizeof(cid);

    int32_t act_bits;
    memcpy(&act_bits, buf + offset, sizeof(act_bits));
    act_bits = ntohl(act_bits);
    cmd->act = (ActionType)act_bits;
    offset += sizeof(act_bits);

    uint32_t x_bits;
    memcpy(&x_bits, buf + offset, sizeof(x_bits));
    x_bits = ntohl(x_bits);
    memcpy(&cmd->dir.x, &x_bits, sizeof(float));
    offset += sizeof(x_bits);

    uint32_t y_bits;
    memcpy(&y_bits, buf + offset, sizeof(y_bits));
    y_bits = ntohl(y_bits);
    memcpy(&cmd->dir.y, &y_bits, sizeof(float));
    offset += sizeof(y_bits);

    uint32_t v_bits;
    memcpy(&v_bits, buf + offset, sizeof(v_bits));
    v_bits = ntohl(v_bits);
    memcpy(&cmd->velocity, &v_bits, sizeof(float));
    offset += sizeof(v_bits);

    // act は未使用（コメントアウト中）
   // cmd->act = AT_OpX; 

    return 0;
}

/*クライアントの移動をサーバ側座標を更新
  引数
    cmd : クライアントから届いた入力コマンド
*/
void UpdateCharaPosition(const ClientCommand *cmd)
{
    int id = cmd -> client_id;
    if (id < 0 || id >= MAX_CLIENTS) return;

    CharaInfo *ch = &game_info.chinf[id];

    // 移動量計算
    if(ColorDecision(mask, ch->point.x+cmd->dir.x*cmd->velocity, ch->point.y+cmd->dir.y * cmd->velocity) != FT_Unpassible){
    ch->point.x += cmd->dir.x * cmd->velocity;
    ch->point.y += cmd->dir.y * cmd->velocity;
    }

    // 画面端チェック
    if (ch->point.x < 0.0f) ch->point.x = 0.0f;
    if (ch->point.x > MAX_WINDOW_X) ch->point.x = MAX_WINDOW_X;
    if (ch->point.y < 0.0f) ch->point.y = 0.0f;
    if (ch->point.y > MAX_WINDOW_Y) ch->point.y = MAX_WINDOW_Y;
}

/*ゲーム情報を全クライアントに送信*/
void BroadcastGameInfo(void)
{
    unsigned char buf[MAX_DATA];
    int size = sizeof(GameInfo);

    // そのままメモリコピで送る
    memcpy(buf, &game_info, size);
    
    SendData(ALL_CLIENTS, &buf, size);
}

void InteractManeger(const ClientCommand *cmd)
{
    switch (cmd->act)
    {
    case 'X':
        game_info.chinf[cmd->client_id].stts = CS_Action;
        break;
    case 'A':
    game_info.chinf[cmd->client_id].stts = CS_Normal;
    break;
    default:
        break;
    }
}

/*クライアントからの受信データを処理
  引数
    data     : クライアントから受信したバイト配列
    dataSize : データサイズ
*/
void ProcessClientData(const unsigned char *data, int dataSize)
{
    ClientCommand cmd;
    if (UnpackClientCommand(data, dataSize, &cmd) == 0) {

            InteractManeger(&cmd);
        if (game_info.chinf[cmd.client_id].stts == CS_Action)
            ExecuteCommand(&game_info.chinf[cmd.client_id], &cmd);
        // サーバ側のキャラ位置を更新
        if (game_info.chinf[cmd.client_id].stts == CS_Normal)
            UpdateCharaPosition(&cmd);
            

        #ifndef NDEBUG
        printf("Server: client %d moved to (%.2f, %.2f)\n",
            cmd.client_id,
            game_info.chinf[cmd.client_id].point.x,
            game_info.chinf[cmd.client_id].point.y);
        #endif
    }
}

/*サーバ起動時にゲーム情報を初期化する*/
void InitGameInfo(void)
{
    memset(&game_info, 0, sizeof(GameInfo));

    for (int i = 0; i < MAX_CLIENTS; i++) {
        game_info.chinf[i].stts = CS_Normal;
        game_info.chinf[i].type = CT_Player;
        game_info.chinf[i].point.x = 695;
        game_info.chinf[i].point.y = 370+2*i;
        game_info.chinf[i].w       = 20;
        game_info.chinf[i].h       = 30;
    }

    game_info.chinf[ID_SHIP].type = CT_Ship;
    game_info.chinf[ID_SHIP].stts = CS_Normal;
    game_info.chinf[ID_SHIP].point.x = 0.0f; 
    game_info.chinf[ID_SHIP].point.y = 0.0f;

    SDL_Surface* src = IMG_Load("materials_win/spaceship_proto2_mask.png");
    mask = SDL_CreateRGBSurface(
        0, 500, 500,
        src->format->BitsPerPixel,
        src->format->Rmask,
        src->format->Gmask,
        src->format->Bmask,
        src->format->Amask
    );
    SDL_BlitScaled(src, NULL, mask, NULL);
}