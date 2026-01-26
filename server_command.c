#include"server_common.h"
#include"server_func.h"
#include <arpa/inet.h>
#include<time.h>

/*サーバが管理するゲーム全体情報(プレイヤ位置など)*/
GameInfo game_info;
ClientCommand clientsCommand[MAX_CLIENTS];
static CharaInfo ObstaclesInfo[OBSTACLE_MAXNUM];

extern int gClientNum;
static int gReady[MAX_CLIENTS] = {0,0,0,0};
static int obstacles_num = 0;
static int obstacles_loaded = 0;

SDL_Surface* mask;

int CheckStartCondition(void) {
    int cnt = 0;
    for (int i = 0; i < gClientNum; ++i) {
        if (gReady[i]) ++cnt;
    }
    if (cnt >= gClientNum) {
        // 全員準備OKになった -> ゲーム開始を通知
        game_info.stts = GS_Playing;
        return 0;
    }
    return 1;
}

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
    } else if (r == 45 && g == 173 && b == 49) {
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

int CollisionInSpace(CharaInfo *ship, FloatPoint delta)
{             
     // 移動後の船の予測位置（世界座標）
    float future_ship_x = ship->point.x + delta.x;
    float future_ship_y = ship->point.y + delta.y;

    for(int i=0; i<obstacles_num; i++){
        FloatPoint obstacle_pos = ObstaclesInfo[i].point;
        float rad_sum = ship->r + ObstaclesInfo[i].r;


        // 障害物との距離の二乗を計算
        float dx = obstacle_pos.x - future_ship_x;
        float dy = obstacle_pos.y - future_ship_y;
        float distSq = dx * dx + dy * dy;

        if(distSq <= (rad_sum * rad_sum)){
            fprintf(stderr, "Collide! with Obstacle %d\n", i);
            ship->hp--;
            if(ship->hp  < 0){
                game_info.stts = GS_End;
            }
            fprintf(stderr, "hp%d\n", ship->hp);
            return 0; // 衝突
        }
    }

    for (int i = 0; i < MAX_ENEMY; i++) {
        int id = ENEMY_ID + i;
        CharaInfo *enemy = &game_info.chinf[id];

        if (enemy->stts != CS_Alive) continue;

        float enemy_r = enemy->w / 2.0f;
        float rad_sum = ship->r + enemy_r;
        float rad_sum_sq = rad_sum * rad_sum;

        // 未来の距離
        float dx_future = enemy->point.x - future_ship_x;
        float dy_future = enemy->point.y - future_ship_y;
        float distSq_future = dx_future * dx_future + dy_future * dy_future;

        if (distSq_future <= rad_sum_sq) {
            
            // 現在の距離
            float dx_current = enemy->point.x - ship->point.x;
            float dy_current = enemy->point.y - ship->point.y;
            float distSq_current = dx_current * dx_current + dy_current * dy_current;

            if (distSq_future < distSq_current) {
                return 0; 
            }
        }
    }
    
    float rad_sum_goal = ship->r + GOAL_POSITION_R;
    float dx_G = GOAL_POSITION_X - future_ship_x;
    float dy_G = GOAL_POSITION_Y - future_ship_y;
    float distSq_G = dx_G * dx_G + dy_G * dy_G;

    if(distSq_G <= (rad_sum_goal * rad_sum_goal)){
        fprintf(stderr, "Goal!\n");
        game_info.stts = GS_Result;
        return 0;
    }

    return 1;
}

void ExecuteCommand(CharaInfo *ch, const ClientCommand *cmd)
{
    int Interaction = ColorDecision(mask, ch->point.x, ch->point.y);
    fprintf(stderr, "Your Interact: %d\n", Interaction);
    ch->act = Interaction;

    CharaInfo *ship = &game_info.chinf[ID_SHIP];

    const float accel = 0.5f;
    const float Brake = 4.0f;
    //クライアントから送られてきたスティックの方向ベクトルを取得
    FloatPoint stick_vec = cmd->dir;
    float stick_len = sqrtf(stick_vec.x * stick_vec.x + stick_vec.y * stick_vec.y);

    float center_x = ship->point.x;
    float center_y = ship->point.y;

    // 砲台の位置設定
    const float OFFSET_MAIN = 170.0f; // 中心から砲口までの距離
    const float OFFSET_SIDE = 110.0f; // 2砲台の間隔
    static int cooldown = 0;
    if (cooldown > 0) cooldown--;

    switch (Interaction) {
        //IT_MoveR: 左右操作 赤
        case IT_MoveR: 

            if (stick_len > 0.1f) {
                float dir_x = stick_vec.x / stick_len;
                if (fabsf(dir_x) > 0.5) {
                    float current_vx = ship->Velocity.x;
                    float thrust = accel;
                    if(dir_x >= 0.0f)
                        game_info.fireEffect = 0;
                    else
                        game_info.fireEffect = 1;
                    
                    if (dir_x * current_vx < 0) {
                        thrust = Brake;
                    }
                    ship->Velocity.x += dir_x * thrust;
                }
            }
            break;
            
        //IT_MoveL: 上下操作 青
        case IT_MoveL: 
            if (stick_len > 0.1f) {
                float dir_y = stick_vec.y / stick_len;
                if (fabsf(dir_y) > 0.5) {
                    float current_vy = ship->Velocity.y;
                    float thrust = accel;
                    game_info.fireEffect = (dir_y >= 0.0f) ? 2 : 3;
                    if (dir_y * current_vy < 0) {
                        thrust = Brake;
                    }
                    ship->Velocity.y += dir_y * thrust;
                }
            }
            break;
        
        case IT_TaskOxy:
        if (cmd->act == 'B') {
            /*game_info.oxy_progress++;
            if (game_info.oxy_progress >= game_info.oxy_required) {
                game_info.oxy_amount= game_info.oxy_max;
                fprintf(stderr, "Oxygen Task Progress: %d\n", game_info.oxy_progress);
                game_info.oxy_progress = 0;
                fprintf(stderr, "Oxygen fully replenished!\n");
            }*/
            const float Recovery_Speed = 0.5f;
            game_info.oxy_amount += Recovery_Speed;
            if (game_info.oxy_amount > game_info.oxy_max) {
            game_info.oxy_amount = game_info.oxy_max;
            }
        }
        break;

        case IT_AttackUpper: // Y軸
        if (cmd->act == 'B' && cooldown == 0 && stick_len > 0.1f) {
            if (fabsf(stick_vec.y) > fabsf(stick_vec.x)) {
                    
                    int fired_count = 0;
                    
                    //　上
                    if (stick_vec.y < 0) {
                        for(int i=0; i<MAX_BULLETS; i++){
                            if(game_info.bullets[i].active == 0){
                                game_info.bullets[i].active = 1;
                                game_info.bullets[i].vec.x = 0;
                                game_info.bullets[i].vec.y = -BULLET_SPEED; // 上へ

                                // 位置: 上にズラす
                                game_info.bullets[i].point.y = center_y - OFFSET_MAIN; 
                                // 左右に振り分け
                                if (fired_count == 0) game_info.bullets[i].point.x = center_x - OFFSET_SIDE;
                                else                  game_info.bullets[i].point.x = center_x + OFFSET_SIDE;

                                fired_count++;
                                if(fired_count >= 2) break;
                            }
                        }
                    }
                    //下
                    else {
                        for(int i=0; i<MAX_BULLETS; i++){
                            if(game_info.bullets[i].active == 0){
                                game_info.bullets[i].active = 1;
                                game_info.bullets[i].vec.x = 0;
                                game_info.bullets[i].vec.y = BULLET_SPEED; // 下へ

                                // 位置: 下にズラす
                                game_info.bullets[i].point.y = center_y + OFFSET_MAIN;
                                // 左右に振り分け
                                if (fired_count == 0) game_info.bullets[i].point.x = center_x - OFFSET_SIDE;
                                else                  game_info.bullets[i].point.x = center_x + OFFSET_SIDE;

                                fired_count++;
                                if(fired_count >= 2) break;
                            }
                        }
                    }
                    
                    cooldown = 10; // 発射したらクールダウン
                }
            }
            break;

        case IT_AttackLower: // X軸
       if (cmd->act == 'B' && cooldown == 0 && stick_len > 0.1f) {
        if (fabsf(stick_vec.x) > fabsf(stick_vec.y)) {
                    
                    int fired_count = 0;

                    // 左
                    if (stick_vec.x < 0) {
                        for(int i=0; i<MAX_BULLETS; i++){
                            if(game_info.bullets[i].active == 0){
                                game_info.bullets[i].active = 1;
                                game_info.bullets[i].vec.x = -BULLET_SPEED; // 左へ
                                game_info.bullets[i].vec.y = 0;

                                // 位置: 左にズラす
                                game_info.bullets[i].point.x = center_x - OFFSET_MAIN;
                                // 上下に振り分け
                                if (fired_count == 0) game_info.bullets[i].point.y = center_y - OFFSET_SIDE;
                                else                  game_info.bullets[i].point.y = center_y + OFFSET_SIDE;

                                fired_count++;
                                if(fired_count >= 2) break;
                            }
                        }
                    }
                    // 右
                    else {
                        for(int i=0; i<MAX_BULLETS; i++){
                            if(game_info.bullets[i].active == 0){
                                game_info.bullets[i].active = 1;
                                game_info.bullets[i].vec.x = BULLET_SPEED; // 右へ
                                game_info.bullets[i].vec.y = 0;

                                // 位置: 右にズラす
                                game_info.bullets[i].point.x = center_x + OFFSET_MAIN;
                                // 上下に振り分け
                                if (fired_count == 0) game_info.bullets[i].point.y = center_y - OFFSET_SIDE;
                                else                  game_info.bullets[i].point.y = center_y + OFFSET_SIDE;

                                fired_count++;
                                if(fired_count >= 2) break;
                            }
                        }
                    }

                    cooldown = 10;
                }
            }
            break;
             
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
    cmd->act = (InteractType)act_bits;
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
    float oxy_ratio = game_info.oxy_amount / game_info.oxy_max;
    if (oxy_ratio < 0.0f) oxy_ratio = 0.0f;
    if (oxy_ratio > 1.0f) oxy_ratio = 1.0f;

    const float min_speed = 0.05f;
    float speed_multiplier = min_speed + (1.0f - min_speed) * oxy_ratio;
    float actual_speed = cmd->velocity * speed_multiplier;

    // 移動量計算
    if(ColorDecision(mask, ch->point.x+cmd->dir.x * actual_speed, ch->point.y+cmd->dir.y * actual_speed) != FT_Unpassible){
    ch->point.x += cmd->dir.x * actual_speed;
    ch->point.y += cmd->dir.y * actual_speed;
    }

    // 画面端チェック
    if (ch->point.x < 0.0f) ch->point.x = 0.0f;
    if (ch->point.x > MAX_WINDOW_X) ch->point.x = MAX_WINDOW_X;
    if (ch->point.y < 0.0f) ch->point.y = 0.0f;
    if (ch->point.y > MAX_WINDOW_Y) ch->point.y = MAX_WINDOW_Y;
}

void UpdateOxygen(void)
{
    game_info.oxy_amount -= OXY_DEPLETION;

    if (game_info.oxy_amount < 0.0f) {
        game_info.oxy_amount = 0.0f;
    }
    if (game_info.oxy_amount <= 0.0f) {
        // 0以下ならゲームオーバ
        game_info.stts = GS_End; 
        
        // サーバーのログに表示（確認用）
        fprintf(stderr, "GAME OVER: Oxygen depleted.\n");
    }
}

void UpdateEnemy(void)
{
    CharaInfo *ship = &game_info.chinf[ID_SHIP];
    const float HALF_WIDTH = 1500.0f / 2.0f;
    const float HALF_HEIGHT = 800.0f / 2.0f;

    for (int i = 0; i < MAX_ENEMY; i++) {
        int id = ENEMY_ID + i;
        CharaInfo *enemy = &game_info.chinf[id];

        if (enemy->stts != CS_Alive) continue;
        float dx = ship->point.x - enemy->point.x;
        float dy = ship->point.y - enemy->point.y;

        if (fabsf(dx) < HALF_WIDTH && fabsf(dy) < HALF_HEIGHT) {
            float dist = sqrtf(dx * dx + dy * dy);
            float stop_dist = ship->r + (enemy->w / 2.0f);
            if (dist > stop_dist) {
            float move_x = (dx / dist) * ENEMY_SPEED;
            float move_y = (dy / dist) * ENEMY_SPEED;

            enemy->point.x += move_x;
            enemy->point.y += move_y;
            }
            else {
                /*if (rand() % 20 == 0) {
                    ship->hp -= 1;
                    fprintf(stderr, "Hit! Ship HP: %d\n", ship->hp);*/
                    if (ship->damage_timer == 0) {
                    ship->hp -= 10; 
                    ship->damage_timer = 60;
                    fprintf(stderr, "Crash! Ship HP: %d\n", ship->hp);
                    enemy->stts = 0;
                    }
                    if (ship->hp <= 0) {
                        game_info.stts = GS_End;
                    }
                }
            }
        }
    }


void UpdateSdhipOperate(void)
{
    CharaInfo *ship = &game_info.chinf[ID_SHIP];

    if (ship->damage_timer > 0) {
    ship->damage_timer--;
}

    const float friction = 0.995f;
    const float max_speed = 8.0f;

    float current_speed = sqrtf(ship->Velocity.x * ship->Velocity.x + ship->Velocity.y * ship->Velocity.y);
    if (current_speed > max_speed) {
        ship->Velocity.x = (ship->Velocity.x / current_speed) * max_speed;
        ship->Velocity.y = (ship->Velocity.y / current_speed) * max_speed;
    }

    FloatPoint delta = {ship->Velocity.x, ship->Velocity.y};

    if (CollisionInSpace(ship, delta)) {
        ship->point.x += ship->Velocity.x;
        ship->point.y += ship->Velocity.y;
    } else {
        // 壁に衝突した場合の作用（ここでは停止させています）
        ship->Velocity.x = 0;
        ship->Velocity.y = 0;
    }
    // 3. 摩擦処理
    // 1.0 を掛けるので、速度は減りません（等速直線運動を維持）
    ship->Velocity.x *= friction;
    ship->Velocity.y *= friction;
}

void UpdateBullets(void)
{
    for(int i=0; i<MAX_BULLETS; i++){
        // 使っていない弾は無視
        if(game_info.bullets[i].active == 0) continue;

        game_info.bullets[i].point.x += game_info.bullets[i].vec.x;
        game_info.bullets[i].point.y += game_info.bullets[i].vec.y;

        if(game_info.bullets[i].point.x < -5000 || game_info.bullets[i].point.x > 5000 ||
           game_info.bullets[i].point.y < -5000 || game_info.bullets[i].point.y > 5000){
            game_info.bullets[i].active = 0;
            continue;
        }

        // 敵との当たり判定
        for(int j=0; j<MAX_ENEMY; j++){
            int eid = ENEMY_ID + j;
            CharaInfo *enemy = &game_info.chinf[eid];

            if(enemy->stts != CS_Alive) continue;

            float dx = game_info.bullets[i].point.x - enemy->point.x;
            float dy = game_info.bullets[i].point.y - enemy->point.y;
            float dist = sqrtf(dx*dx + dy*dy);

            // 敵のサイズ(w/2) + 弾のサイズ
            if(dist < (enemy->w/2 + BULLET_R)){
                enemy->stts = CS_Dead;         // 敵を倒す
                game_info.bullets[i].active = 0; // 弾も消える
                game_info.killCount++;
                break; 
            }
        }
    }
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
    CharaInfo *ch = &game_info.chinf[cmd->client_id];
    int Color = ColorDecision(mask, ch->point.x+cmd->dir.x*cmd->velocity, ch->point.y+cmd->dir.y * cmd->velocity);
    switch (cmd->act)
    {
    case 'X':
        if(Color != FT_Passable && Color != FT_Unpassible)
            game_info.chinf[cmd->client_id].stts = CS_Action;
        break;
    case 'A':
        game_info.chinf[cmd->client_id].stts = CS_Normal;
        break;
    case 'H':
        game_info.stts = GS_End;
    default:
        break;
    }
}

/*クライアントからの受信データを処理
  引数
    data     : クライアントから受信したバイト配列
    dataSize : データサイズ
*/
int ProcessClientData(const unsigned char *data, int dataSize)
{
    int endFlag = 1;
    ClientCommand cmd;
    switch(game_info.stts){
        case GS_Title:
            if (UnpackClientCommand(data, dataSize, &cmd) == 0) {
                    switch(cmd.act){
                            case 'X':
                                gReady[cmd.client_id] = 1;
                                break;
                            case 'A':
                                gReady[cmd.client_id] = 0;
                                break;
                            case 'H':
                                game_info.stts = GS_End;
                            default:
                                break;
                    }
            }
            endFlag = CheckStartCondition();
            return endFlag;
            break;

        case GS_Playing:
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
            if(game_info.stts == GS_End)
                endFlag = GS_End;
            else if(game_info.stts ==GS_Result)
                endFlag = GS_Result;
            return endFlag;
            break;
    }
}

/*サーバ起動時にゲーム情報を初期化する*/
void InitGameInfo(void)
{
    //spaceshipの描画ズレ補正用
    static int offset_x = MAX_WINDOW_X/2-SPACESHIP_SIZE/2;
    static int offset_y = MAX_WINDOW_Y/2-SPACESHIP_SIZE/2;

    srand((unsigned)time(NULL));
    memset(&game_info, 0, sizeof(GameInfo));

    game_info.stts = GS_Title;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        game_info.chinf[i].stts = CS_Normal;
        game_info.chinf[i].type = CT_Player;
        game_info.chinf[i].point.x = 695;
        game_info.chinf[i].point.y = 370+2*i;
        game_info.chinf[i].w       = 20;
        game_info.chinf[i].h       = 30;
    }

    /* 敵の初期化*/
    for (int i = 0; i < MAX_ENEMY; i++) {
        int id = ENEMY_ID + i;
        game_info.chinf[id].type = CT_Enemy;
        game_info.chinf[id].stts = CS_Alive;

        //game_info.chinf[id].point.x = (rand() % 2000) - 1000;
        //game_info.chinf[id].point.y = (rand() % 2000) - 1000;
        game_info.chinf[id].w = 40;
        game_info.chinf[id].h = 40;
        float x, y, dist;

        do {
            x = (rand() % SPAWN_RANGE) - (SPAWN_RANGE / 2);
            y = (rand() % SPAWN_RANGE) - (SPAWN_RANGE / 2);

            dist = sqrtf(x * x + y * y);
        } while (dist < SAFE_RADIUS);

        game_info.chinf[id].point.x = x;
        game_info.chinf[id].point.y = y;
        fprintf(stderr, "Enemy[%d] (%.1f, %.1f)\n", i, x, y);
    }

    // 宇宙船初期化
    game_info.chinf[ID_SHIP].type = CT_Ship;
    game_info.chinf[ID_SHIP].stts = CS_Normal;
    game_info.chinf[ID_SHIP].point.x = offset_x; 
    game_info.chinf[ID_SHIP].point.y = offset_y;
    game_info.chinf[ID_SHIP].r = SPACESHIP_SIZE/2;
    game_info.chinf[ID_SHIP].hp = 1000;

    // 酸素タスク初期化
    game_info.oxy_max = 30.0f;
    game_info.oxy_amount = game_info.oxy_max;
    game_info.oxy_progress = 0;
    game_info.oxy_required = 50;

    game_info.fireEffect = 0;

    game_info.killCount = 0;

    // 弾の初期化
    for(int i=0; i<MAX_BULLETS; i++){
        game_info.bullets[i].active = 0;
    }

    SDL_Surface* src = IMG_Load("materials_win/spaceship_mask.png");
    mask = SDL_CreateRGBSurface(
        0, 500, 500,
        src->format->BitsPerPixel,
        src->format->Rmask,
        src->format->Gmask,
        src->format->Bmask,
        src->format->Amask
    );
    SDL_BlitScaled(src, NULL, mask, NULL);

    /** マップ情報読込 **/
    FILE* fp = fopen("materials_win/obstacles.txt", "r");
    if (fp == NULL) {
        fprintf(stderr, "failed to open map data file.\n");
    }

    /* 前ステージで読み込んだ行数分スキップ */
    for (int i = 0; i < obstacles_loaded; i++) {
        char dummy[256];
        if (!fgets(dummy, sizeof(dummy), fp)) {
            // ファイル終端に到達した場合は break
            break;
        }
    }

    char linebuf[256];
    while (fgets(linebuf, sizeof(linebuf), fp)) {
        if (linebuf[0] == '#') continue;
        if (linebuf[0] == '*'){
            
            break;
        }
        if (obstacles_num < OBSTACLE_MAXNUM) {
            int x, y, r;
            if (sscanf(linebuf, "%d %d %d", &x, &y, &r) == 3) {
                ObstaclesInfo[obstacles_num].point.x = x;
                ObstaclesInfo[obstacles_num].point.y = y;
                ObstaclesInfo[obstacles_num].r       = r;
                obstacles_num++;
                obstacles_loaded++;
            }
        }
    }
    fclose(fp);
}

void AfterPlayingRoop(void)
{
    game_info.stts = GS_Result;	
}