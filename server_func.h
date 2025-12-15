#ifndef _SERVER_FUMC_H_
#define _SERVER_FUMC_H_

#include"server_common.h"

/*server_command.c*/
int UnpackClientCommand(const unsigned char *buf, int size, ClientCommand *cmd);
int ProcessClientData(const unsigned char *data, int dataSize);
void UpdateCharaPosition(const ClientCommand *cmd);
void BroadcastGameInfo(void);
void InitGameInfo(void);

/*server_net.c*/
int SetUpServer(int num);
int SendRecvManager(void);
void Ending(void);
void SendDiamondCommand(void);
void SendData(int pos,void *data,int dataSize);

#endif