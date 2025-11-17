#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>   // socket(), accept() など
#include <sys/select.h>  // select(), fd_set, FD_SET, FD_ZERO, FD_ISSET


#include "constants.h"

static CLIENT clients[MAX_NUM_CLIENTS];
static int num_clients = MAX_NUM_CLIENTS;
static int num_socks;


static void handle_error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;
    char addr_str[MAX_LEN_ADDR];

    printf("=== Server starting ===\n");

    // ソケット作成
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        handle_error("socket()");

    // 再利用設定
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // アドレス設定
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        handle_error("bind()");

    // listen
    if (listen(server_sock, num_clients) < 0)
        handle_error("listen()");

    printf("Server listening on port %d...\n", PORT);

    // 4人分受け入れ
    for (int i = 0; i < num_clients; i++) {
        len = sizeof(client_addr);
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &len)) < 0)
            handle_error("accept()");

        clients[i].cid = i;
        clients[i].sock = client_sock;
        clients[i].addr = client_addr;

        // クライアント名を受信
        read(client_sock, clients[i].name, MAX_LEN_NAME);

        inet_ntop(AF_INET, &(client_addr.sin_addr), addr_str, sizeof(addr_str));
        printf("Client %d connected: %s (%s)\n", i, clients[i].name, addr_str);
    }

    // 全員に接続情報を送信
    for (int i = 0; i < num_clients; i++) {
        write(clients[i].sock, &num_clients, sizeof(int));
        for (int j = 0; j < num_clients; j++) {
            write(clients[i].sock, &clients[j], sizeof(CLIENT));
        }
    }

    printf("All clients connected! Info sent to everyone.\n");

    /*KEY_EVENT event;
event.cid = 1;  // 自分のID
event.key = 'A';      // テスト用に固定文字

write(BROADCAST, &event, sizeof(KEY_EVENT));

// サーバーからの返送（全員に通知）を受け取る
KEY_EVENT recv_event;
read(BROADCAST, &recv_event, sizeof(KEY_EVENT));
printf("Client[%d] received: client[%d] key=%c\n", 1, recv_event.cid, recv_event.key);*/


    close(server_sock);
    return 0;
}
