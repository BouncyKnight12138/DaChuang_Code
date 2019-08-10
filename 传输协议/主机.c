#include <reg51.h>

#define uchar unsigned char
#define uint unsigned int

sbit PIN_RXD = P2; //�������Ŷ���
sbit PIN_TXD = P3^1; //�������Ŷ���
sbit START = P3^4;//ʹ��λ
sbit choose = P1; //ѡ����λ������

bit RxdOrTxd = 0; //ָʾ��ǰ״̬Ϊ���ջ��Ƿ���
bit RxdEnd = 0; //���ս�����־
bit TxdEnd = 0; //���ͽ�����־

uchar RxdBuf = 0; //���ջ�����
uchar TxdBuf = 0; //���ͻ�����

uint j = 0;

//��Ӧѡ��λ�ô�������
uint select[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
//����8������������
uchar datas[8];

void delay(uint x);
void ConfigTX(uint baud);
void StartTXD(uchar dat);
void StartRXD();

void main(){
  EA = 1; //�����ж�
	EX0 = 1;//�����ⲿ�ж�0
	IT0 = 1;//���ش���
  ConfigTX(9600);//���ò�����Ϊ 9600
	for(j=0;j<8;j++){
		choose = select[j];
		while (PIN_RXD&select[j] == 1); //�ȴ��������ų��ֵ͵�ƽ������ʼλ
     StartRXD(); //��������
     while (!RxdEnd); //�ȴ��������
		datas[j] = RxdBuf;
		delay(500);
	}
}
/* �������ú�����baud-ͨ�Ų����� */
void ConfigTX(uint baud){
    TMOD &= 0xF0; //���� T0 �Ŀ���λ
    TMOD |= 0x02; //���� T0 Ϊģʽ 2
    TH0 = 256 - (11059200/12)/baud; //���� T0 ����ֵ
}
/* �������н��� */
void StartRXD(){
    TL0 = 256 - ((256-TH0)>>1); //��������ʱ�� T0 ��ʱΪ�������������
    ET0 = 1; //ʹ�� T0 �ж�
    TR0 = 1; //���� T0
    RxdEnd = 0; //������ս�����־
    RxdOrTxd = 0; //���õ�ǰ״̬Ϊ����
}
/* �������з��ͣ�dat-�������ֽ����� */
void StartTXD(uchar dat){
    TxdBuf = dat; //���������ݱ��浽���ͻ�����
    TL0 = TH0; //T0 ������ֵΪ����ֵ
    ET0 = 1; //ʹ�� T0 �ж�
    TR0 = 1; //���� T0
    PIN_TXD = 0; //������ʼλ
    TxdEnd = 0; //���㷢�ͽ�����־
    RxdOrTxd = 1; //���õ�ǰ״̬Ϊ����
}
/* T0 �жϷ������������з��ͺͽ��� */
void InterruptTimer0() interrupt 1{
    static unsigned char cnt = 0; //λ���ջ��ͼ���
    if (RxdOrTxd){ //���з��ʹ���
        cnt++;
        if (cnt <= 27){ //��λ�������η��� 27bit ����λ
            PIN_TXD = TxdBuf & 0x01;
            TxdBuf >>= 1;
        }else if (cnt == 28){ //����ֹͣλ
            PIN_TXD = 1;
        }else{ //���ͽ���
            cnt = 0; //��λ bit ������
            TR0 = 0; //�ر� T0
            TxdEnd = 1; //�÷��ͽ�����־
        }
    }else{ //���н��մ���
        if (cnt == 0){ //������ʼλ
            if (!PIN_RXD){ //��ʼλΪ 0 ʱ��������ջ�������׼����������λ
                RxdBuf = 0;
                cnt++;
            }else{ //��ʼλ��Ϊ 0 ʱ����ֹ����
								TR0 = 0; //�ر� T0
						}
        }else if (cnt <= 27){ //���� 8 λ����λ
            RxdBuf >>= 1; //��λ���ȣ����Խ�֮ǰ���յ�λ������
            //���ս�Ϊ 1 ʱ�����������λ�� 1��
            //��Ϊ 0 ʱ�������Ա�����λ��� 0
            if (PIN_RXD){
                RxdBuf |= 0x8000000;
            }
            cnt++;
        }else{ //ֹͣλ����
            cnt = 0; //��λ bit ������
            TR0 = 0; //�ر� T0
            if (PIN_RXD){ //ֹͣλΪ 1 ʱ��������Ϊ������Ч
                RxdEnd = 1; //�ý��ս�����־
            }
        }
    }
}
/*����߼������������ݵĺ���*/
void int0() interrupt 0 using 0{
	uint i;
	for(i=0;i<8;i++){
		StartTXD(datas[i]); //���յ������ݺ󣬷��ͻ�����
		while (!TxdEnd); //�ȴ��������
		delay(500);
	}
}
/*��ʱ����*/
void delay(uint x) 
{
	uint y,z;
	for(z=x;z>0;z--)
	for(y=110;y>0;y--);
}