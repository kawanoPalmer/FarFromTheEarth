#include"server_common.h"
#include"server_func.h"
#include <arpa/inet.h>

/*サーバが管理するゲーム全体情報(プレイヤ位置など)*/
GameInfo game_info;
ClientCommand clientsCommand[MAX_CLIENTS];

void ExecuteCommand(ClientCommand *cmd)
{
    if(game_info.chinf[cmd->client_id].stts == CS_Normal)   
        fprintf(stderr, "Interact OFF!\n");
    else if(game_info.chinf[cmd->client_id].stts == CS_Action)
        fprintf(stderr, "Interact ON!\n");
    else
        fprintf(stderr, "wrong\n");
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
    ch->point.x += cmd->dir.x * cmd->velocity;
    ch->point.y += cmd->dir.y * cmd->velocity;

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
            ExecuteCommand(&cmd);
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
        game_info.chinf[i].point.x = 50*i + 30;
        game_info.chinf[i].point.y = 100;
        game_info.chinf[i].w       = 20;
        game_info.chinf[i].h       = 30;
    }
}