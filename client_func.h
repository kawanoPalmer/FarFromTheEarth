/*****************************************************************
�ե�����̾	: client_func.h
��ǽ		: ���饤����Ȥγ����ؿ������
*****************************************************************/

#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

#include"common.h"

/* client_net.c */
extern int SetUpClient(char* hostName,int *clientID,int *num,char clientName[][MAX_NAME_SIZE]);
extern void CloseSoc(void);
extern int RecvIntData(int *intData);
extern void SendData(void *data,int dataSize);
extern int SendRecvManager(void);

/* client_win.c */
extern int InitWindow(int clientID,int num,char name[][MAX_NAME_SIZE]);
extern void DestroyWindow(void);
extern void WindowEvent(int num);
extern void RenderWindow(void);


/* client_command.c */
void SendClientCommand(int client_id);
Direction GetJoyConStick(int clientID);
FloatPoint DirToVector(Direction d);

extern int ExecuteCommand(char command);
extern void SendEndCommand(void);
extern void Sendinfo(int pos);
int RecvData(void *data,int dataSize);
void RecvInfo(GameInfo *info);

#endif
