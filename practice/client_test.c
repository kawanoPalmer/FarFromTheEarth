#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h> 
#include <sys/types.h>   // socket(), accept() など
#include <sys/socket.h>  // socket(), bind(), listen(), accept() など
#include <netinet/in.h>  // sockaddr_in

#include "constants.h"

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    CLIENT clients[MAX_NUM_CLIENTS];
    int num_clients;
    CLIENT me;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <name>\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    const char *name = argv[2];

    // ソケット作成
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // サーバー接続
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(1);
    }

    // 名前を送信
    write(sock, name, MAX_LEN_NAME);

    printf("Connected to server as '%s'\n", name);

    // 全員の情報を受信
    read(sock, &num_clients, sizeof(int));
    for (int i = 0; i < num_clients; i++) {
        read(sock, &clients[i], sizeof(CLIENT));
    }

    // 自分の情報を表示
    for (int i = 0; i < num_clients; i++) {
        printf("Client[%d]: name=%s, sock=%d, port=%d\n",
               clients[i].cid,
               clients[i].name,
               clients[i].sock,
               ntohs(clients[i].addr.sin_port));
    }

    printf("All clients received.\n");

    /*KEY_EVENT event;
event.cid = 1;  // 自分のID
event.key = 'A';      // テスト用に固定文字

write(sock, &event, sizeof(KEY_EVENT));

// サーバーからの返送（全員に通知）を受け取る
KEY_EVENT recv_event;
read(sock, &recv_event, sizeof(KEY_EVENT));
printf("Client[%d] received: client[%d] key=%c\n", 1, recv_event.cid, recv_event.key);*/


      close(sock);
    return 0;
}
