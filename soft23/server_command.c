/*****************************************************************
�ե�����̾	: server_command.c
��ǽ		: �����С��Υ��ޥ�ɽ���
*****************************************************************/

#include"server_common.h"
#include"server_func.h"

static void SetIntData2DataBlock(void *data,int intData,int *dataSize);
static void SetCharData2DataBlock(void *data,char charData,int *dataSize);
static int GetRandomInt(int n);

/*****************************************************************
�ؿ�̾	: ExecuteCommand
��ǽ	: ���饤����Ȥ��������Ƥ������ޥ�ɤ򸵤ˡ�
		  ����������������¹Ԥ���
����	: char	command		: ���ޥ��
		  int	pos			: ���ޥ�ɤ����ä����饤������ֹ�
����	: �ץ�����ཪλ���ޥ�ɤ������Ƥ������ˤ�0���֤���
		  ����ʳ���1���֤�
*****************************************************************/
int ExecuteCommand(char command,int pos)
{
    unsigned char	data[MAX_DATA];
    int			dataSize,intData;
    int			endFlag = 1;

    /* �����������å� */
    assert(0<=pos && pos<MAX_CLIENTS);

#ifndef NDEBUG
    printf("#####\n");
    printf("ExecuteCommand()\n");
    printf("Get command %c\n",command);
#endif
    switch(command){
	    case END_COMMAND:
			dataSize = 0;
			/* ���ޥ�ɤΥ��å� */
			SetCharData2DataBlock(data,command,&dataSize);

			/* ���桼���������� */
			SendData(ALL_CLIENTS,data,dataSize);

			endFlag = 0;
			break;
		case 'A':
			SetCharData2DataBlock(data,'Z',&dataSize);
			SendData(ALL_CLIENTS,data,dataSize);
	    case CIRCLE_COMMAND:
			/* �ߤ�ɽ�����륯�饤������ֹ��������� */
			RecvIntData(pos,&intData);

			dataSize = 0;
			/* ���ޥ�ɤΥ��å� */
			SetCharData2DataBlock(data,command,&dataSize);
			/* ����� x ��ɸ�Υ��å� */
			SetIntData2DataBlock(data,GetRandomInt(500),&dataSize);
			/* ����� y ��ɸ�Υ��å� */
			SetIntData2DataBlock(data,GetRandomInt(300),&dataSize);
			/* ľ�¤Υ��å� */
			SetIntData2DataBlock(data,GetRandomInt(100),&dataSize);

			/* ���ꤵ�줿���饤����Ȥ����� */
			SendData(intData,data,dataSize);
			break;
	    default:
			/* ̤�ΤΥ��ޥ�ɤ������Ƥ��� */
			fprintf(stderr,"0x%02x is not command!\n",command);
    }
    return endFlag;
}

/*****************************************************************
�ؿ�̾	: SendDiamondCommand
��ǽ	: ���饤����Ȥ�ɩ����ɽ�������뤿��˥ǡ���������
����	: �ʤ�
����	: �ʤ�
*****************************************************************/
void SendDiamondCommand(void)
{
    unsigned char data[MAX_DATA];
    int           dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendDiamondCommand\n");
#endif
    dataSize = 0;
    /* ���ޥ�ɤΥ��å� */
    SetCharData2DataBlock(data,DIAMOND_COMMAND,&dataSize);
    /* ɩ���κ���� x ��ɸ */
    SetIntData2DataBlock(data,GetRandomInt(500),&dataSize);
    /* ɩ���κ���� y ��ɸ */
    SetIntData2DataBlock(data,GetRandomInt(300),&dataSize);
    /* ɩ���ι⤵ */
    SetIntData2DataBlock(data,GetRandomInt(100),&dataSize);

    /* ���饤����Ȥ����� */
    SendData(ALL_CLIENTS,data,dataSize);
}

/*****
static
*****/
/*****************************************************************
�ؿ�̾	: SetIntData2DataBlock
��ǽ	: int ���Υǡ����������ѥǡ����κǸ�˥��åȤ���
����	: void		*data		: �����ѥǡ���
		  int		intData		: ���åȤ���ǡ���
		  int		*dataSize	: �����ѥǡ����θ��ߤΥ�����
����	: �ʤ�
*****************************************************************/
static void SetIntData2DataBlock(void *data,int intData,int *dataSize)
{
    int tmp;

    /* �����������å� */
    assert(data!=NULL);
    assert(0<=(*dataSize));

    tmp = htonl(intData);

    /* int ���Υǡ����������ѥǡ����κǸ�˥��ԡ����� */
    memcpy(data + (*dataSize),&tmp,sizeof(int));
    /* �ǡ��������������䤹 */
    (*dataSize) += sizeof(int);
}

/*****************************************************************
�ؿ�̾	: SetCharData2DataBlock
��ǽ	: char ���Υǡ����������ѥǡ����κǸ�˥��åȤ���
����	: void		*data		: �����ѥǡ���
		  int		intData		: ���åȤ���ǡ���
		  int		*dataSize	: �����ѥǡ����θ��ߤΥ�����
����	: �ʤ�
*****************************************************************/
static void SetCharData2DataBlock(void *data,char charData,int *dataSize)
{
    /* �����������å� */
    assert(data!=NULL);
    assert(0<=(*dataSize));

    /* int ���Υǡ����������ѥǡ����κǸ�˥��ԡ����� */
    *(char *)(data + (*dataSize)) = charData;
    /* �ǡ��������������䤹 */
    (*dataSize) += sizeof(char);
}

/*****************************************************************
�ؿ�̾	: GetRandomInt
��ǽ	: ���������������
����	: int		n	: ����κ�����
����	: �����
*****************************************************************/
static int GetRandomInt(int n)
{
    return rand()%n;
}
