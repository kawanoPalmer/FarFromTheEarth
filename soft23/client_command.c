/*****************************************************************
�ե�����̾	: client_command.c
��ǽ		: ���饤����ȤΥ��ޥ�ɽ���
*****************************************************************/

#include"common.h"
#include"client_func.h"

static void SetIntData2DataBlock(void *data,int intData,int *dataSize);
static void SetCharData2DataBlock(void *data,char charData,int *dataSize);
static void RecvRectangleData(void);

/*****************************************************************
�ؿ�̾	: ExecuteCommand
��ǽ	: �����С����������Ƥ������ޥ�ɤ򸵤ˡ�
		  ����������������¹Ԥ���
����	: char	command		: ���ޥ��
����	: �ץ�����ཪλ���ޥ�ɤ��������Ƥ������ˤ�0���֤���
		  ����ʳ���1���֤�
*****************************************************************/
int ExecuteCommand(char command)
{
    int	endFlag = 1;
#ifndef NDEBUG
    printf("#####\n");
    printf("ExecuteCommand()\n");
    printf("command = %c\n",command);
#endif
    switch(command){
		case END_COMMAND:
			endFlag = 0;
			break;
        case 'Z':
            printf("kita");
            DrawRectangle(rand()%400,rand()%400,rand()%400,rand()%400);
            break;
    }
    return endFlag;
}

/*****************************************************************
�ؿ�̾	: SendEndCommand
��ǽ	: �ץ������ν�λ���Τ餻�뤿��ˡ�
		  �����С��˥ǡ���������
����	: �ʤ�
����	: �ʤ�
*****************************************************************/
void SendEndCommand(void)
{
    unsigned char	data[MAX_DATA];
    int			dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendEndCommand()\n");
#endif
    dataSize = 0;
    /* ���ޥ�ɤΥ��å� */
    SetCharData2DataBlock(data,END_COMMAND,&dataSize);

    /* �ǡ��������� */
    SendData(data,dataSize);
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

    /* char ���Υǡ����������ѥǡ����κǸ�˥��ԡ����� */
    *(char *)(data + (*dataSize)) = charData;
    /* �ǡ��������������䤹 */
    (*dataSize) += sizeof(char);
}

/*****************************************************************
�ؿ�̾	: RecvRectangleData
��ǽ	: �ͳѤ�ɽ�����뤿��Υǡ������������ɽ������
����	: �ʤ�
����	: �ʤ�
*****************************************************************/
static void RecvRectangleData(void)
{
    int	x,y,width,height;

    /* �ͳѥ��ޥ�ɤ��Ф����������������� */
    RecvIntData(&x);
    RecvIntData(&y);
    RecvIntData(&width);
    RecvIntData(&height);

    /* �ͳѤ�ɽ������ */
    DrawRectangle(x,y,width,height);
}

/*****************************************************************
�ؿ�̾	: SendCircleCommand
��ǽ	: ���饤����Ȥ˱ߤ�ɽ�������뤿��ˡ�
		  �����С��˥ǡ���������
����	: int		pos	    : �ߤ�ɽ�������륯�饤������ֹ�
����	: �ʤ�
*****************************************************************/
void Sendinfo(int pos)
{
    unsigned char	data[MAX_DATA];
    int			dataSize;

    /* �����������å� */
    assert(0<=pos && pos<MAX_CLIENTS);

#ifndef NDEBUG
    printf("#####\n");
    printf("SendCircleCommand()\n");
    printf("Send Circle Command to %d\n",pos);
#endif

    dataSize = 0;
    /* ���ޥ�ɤΥ��å� */
    SetCharData2DataBlock(data, 'A',&dataSize);
        //�Ȥꤢ������ʸ��A������ǡ����˥��åȤ���ʥ����������䤹��
    /* ���饤������ֹ�Υ��å� */
    SetIntData2DataBlock(data,pos,&dataSize);
        //���饤������ֹ�򤽤θ���˥��åȤ��ƥ����������䤹
    /* �ǡ��������� */
    SendData(data,dataSize);
}