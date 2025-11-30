#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <netinet/in.h>

#define MAX_NUM_CLIENTS 4
#define MAX_LEN_NAME 32
#define MAX_LEN_ADDR 32

#define BROADCAST -1

typedef struct {
    int cid;                          // クライアントID（0〜3）
    int sock;                         // ソケット番号
    struct sockaddr_in addr;          // アドレス情報
    char name[MAX_LEN_NAME];          // 名前
} CLIENT;

typedef struct {
    int cid;    // 送信者ID
    char key;   // 押したキー
} KEY_EVENT;

#endif
