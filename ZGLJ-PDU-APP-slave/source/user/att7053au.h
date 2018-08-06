#ifndef _ATT7053AU_H
#define _ATT7053AU_H



#define  ATT_SPI_FRAME		4		// ATT SPIͨ������֡����
#define  ATT_EMU_FREQ		921000	// ATT7053 femu = 921kHz 
#define  ATT_EC				3200	//������峣��


/** @defgroup ATT7022E data address define
  * @{
  */
/****************EPR*******************/
#define  EPADR_Spl_I1		0x00	// 3bytes	����ͨ��1��ADC��������
#define  EPADR_Spl_I2		0x01	// 3bytes	����ͨ��2��ADC��������
#define  EPADR_Spl_U		0x02	// 3bytes	��ѹͨ����ADC��������
#define  EPADR_Spl_P		0x03	// 3bytes	�й����ʲ�������
#define  EPADR_Spl_Q		0x04	// 3bytes	�޹����ʲ�������
#define  EPADR_Spl_S		0x05	// 3bytes	���ڹ��ʲ�������
#define  EPADR_Rms_I1		0x06	// 3bytes	����ͨ��1����Чֵ
#define  EPADR_Rms_I2		0x07	// 3bytes	����ͨ��2����Чֵ
#define  EPADR_Rms_U		0x08	// 3bytes	��ѹͨ������Чֵ
#define  EPADR_Freq_U		0x09	// 2bytes	��ѹƵ��
#define  EPADR_Power_P1		0x0A	// 3bytes	ͨ��1�й�����
#define  EPADR_Power_Q1		0x0B	// 3bytes	ͨ��1�޹�����
#define  EPADR_Power_S		0x0C	// 3bytes	���ڹ���
#define  EPADR_Energy_P		0x0D	// 3bytes	�й�����
#define  EPADR_Energy_Q		0x0E	// 3bytes	�޹�����
#define  EPADR_Energy_S		0x0F	// 3bytes	��������
#define  EPADR_Power_P2		0x10	// 3bytes	ͨ��2�й�����
#define  EPADR_Power_Q2		0x11	// 3bytes	ͨ��2�޹�����
#define  EPADR_I1_Angle		0x12	// 3bytes	ͨ��1���ѹ�н�
#define  EPADR_I2_Angle		0x13	// 3bytes	ͨ��2���ѹ�н�
#define  EPADR_TempData 	0x14	// 1byte	�¶�����
/**
  * @}
  */


/** @defgroup ATT7022E command define
  * @note     Some parameters need to modify reference to real value(--)
  * @{
  */
///******************************ECR*****************************************/ 
//#define	EMUIE			30H // 0000 2(15bit) EMU �ж�ʹ�ܼĴ���
//#define	EMUIF			31H // 0000 2(16bit) EMU �жϱ�־�Ĵ���
//#define	WPREG			32H // 00   1(8bit) д�����Ĵ���
//#define	SRSTREG			33H // 00   1(8bit) �����λ�Ĵ���	  
//
//#define	EMUCFG 			40H // 0010 2(15bit) EMU ���üĴ���
//#define	FreqCFG			41H // 0088 2(9bit) ʱ��/����Ƶ�����üĴ���
//#define	ModuleEn		42H // 007E 2(14bit) EMU ģ��ʹ�ܼĴ���
//#define	ANAEN			43H // 3B 	1(7bit) ģ��ģ��ʹ�ܼĴ���
//#define	X				44H // 0000 2(11bit) REV
//#define	IOCFG			45H // 0024 2(10bit) IO ������üĴ���
//        
//#define	GP1				50H // 0000 2(16bit) ͨ�� 1 ���й�����У��
//#define	GQ1				51H // 0000 2(16bit) ͨ�� 1 ���޹�����У��
//#define	GS1				52H // 0000 2(16bit) ͨ�� 1 �����ڹ���У��
//#define	Phase1			53H // 00 	1(8bit) ͨ�� 1 ����λУ�����Ʋ����㷽ʽ��
//#define	GP2				54H // 0000 2(16bit) ͨ�� 2 ���й�����У��
//#define	GQ2				55H // 0000 2(16bit) ͨ�� 2 ���޹�����У��
//#define	GS2				56H // 0000 2(16bit) ͨ�� 2 �����ڹ���У��
//#define	Phase2			57H // 00	1(8bit) ͨ�� 2 ����λУ�����Ʋ����㷽ʽ��
//#define	QPhsCal			58H // FF00 2(16bit) �޹���λ����
//#define	ADCCON			59H // 0000 2(12bit) ADC ͨ������ѡ��
//#define	AllGain			5AH // 0000 2(16bit) 3�� ADC ͨ����������Ĵ���
//#define	I2Gain			5BH // 0000 2(16bit) ����ͨ�� 2 ���油��
//#define	I1Off			5CH // 0000 2(16bit) ����ͨ�� 1 ��ƫ��У��
//#define	I2Off			5DH // 0000 2(16bit) ����ͨ�� 2 ��ƫ��У��
//#define	UOff			5EH // 0000 2(16bit) ��ѹͨ����ƫ��У��
//#define	PQStart			5FH // 0040 2(16bit) �𶯹�������
//#define	RMSStart		60H // 0040 2(16bit) ��Чֵ����ֵ���üĴ���
//#define	HFConst			61H // 0040 2(15bit) �������Ƶ������
//#define	CHK				62H // 10 	1(8bit) �Ե���ֵ����
//#define	IPTAMP			63H // 0020 2(16bit) �Ե��������ֵ
//#define	UCONST			64H // 0000 2(16bit) ʧѹ����²�������ĵ�ѹ��������Ե�
//#define	P1OFFSET		65H // 00 	1(8bit) ͨ�� 1 �й�����ƫִУ��������Ϊ 8bit ����
//#define	P2OFFSET		66H // 00 	1(8bit) ͨ�� 2 �й�����ƫִУ��������Ϊ 8bit ����
//#define	Q1OFFSET		67H // 00 	1(8bit) ͨ�� 1 �޹�����ƫִУ��������Ϊ 8bit ����
//#define	Q2OFFSET		68H // 00	1(8bit) ͨ�� 2 �޹�����ƫִУ��������Ϊ 8bit ����
//#define	I1RMSOFFSET		69H // 00 	1(8bit) ͨ�� 1 ��Чֵ�����Ĵ�����Ϊ 8bit �޷�����
//#define	I2RMSOFFSET		6AH // 00 	1(8bit) ͨ�� 2 ��Чֵ�����Ĵ�����Ϊ 8bit �޷�����
//#define	URMSOFFSET		6BH // 00 	1(8bit) ��ѹͨ����ͨ�� 3����Чֵ�����Ĵ�����Ϊ 8bit�޷�����
//#define	ZCrossCurrent	6CH // 0004 2(16bit) ����������ֵ���üĴ���
//#define	GPhs1			6DH // 0000 2(16bit) ͨ�� 1 ����λУ���� PQ ��ʽ��
//#define	GPhs2			6EH // 0000 2(16bit) ͨ�� 2 ����λУ���� PQ ��ʽ��
//#define	PFCnt			6FH // 0000 2(16bit) �����й��������
//#define	QFCnt			70H // 0000 2(16bit) �����޹��������
//#define	SFCnt			71H // 0000 2(16bit) ���������������

#define ATT_WPROTECT    0xb2000000  //�Ĵ���д����

#define ATT_WPROTECT1   0xb20000BC  //�Ĵ���д����,ֻ�ܲ���0x40H����0x45H(0xc0����0xc5)
#define ATT_CON_EMU     0xc0002010  //EMU��Ԫ����, Ĭ��ֵ
#define ATT_CON_FREQ	0xc1000088	//ʱ��/����Ƶ�����üĴ���, Ĭ��ֵ
#define ATT_EN_MODULE   0xc20000fe  //ģ��ʹ�����ã�����ͨ��1��2�����͵�ѹͨ���ǶȲ������ر�ͨ��2���ʲ���
#define ATT_EN_ANMODE   0xc3000037  //ģ��ģ��ʹ������
#define ATT_CON_IOCFG   0xc5000024  //IO������ã� Ĭ��ֵ

/**************************************************************************
	HFConst��5.75*Vu*Vi*10^10/(EC*Un*Ib)
	Vu�� ���ѹ����ʱ����ѹͨ���ĵ�ѹ�������ϵ�ѹ���Ŵ�����
	Vi�� ���������ʱ������ͨ���ĵ�ѹ �������ϵ�ѹ���Ŵ�����
	Un�������ĵ�ѹ
	Ib�� �����ĵ���
	EC�������,�˴���Ϊ3200
	�˴�:HFConst��5.75*0.44*0.4*10^10/(3200*220*10)=1437(0x059D)
**************************************************************************/
#define ATT_WPROTECT2   0xb20000A6  //�Ĵ���д����,ֻ�ܲ���0x50H����0x71H(0xd0����0xf1)
#define ATT_HF_CONST    0xe1001440  //��Ƶ������峣��
#define R_ATT_HF_CONST  0x61000000  //��ȡ��Ƶ������峣��

#define ATT_CAL_GP1     0xd00002da  //ͨ��1�й�����У��--
#define ATT_CAL_GQ1     0xd10002da  //ͨ��1�޹�����У��--
#define ATT_CAL_GS1     0xd20002da  //ͨ��1���ڹ���У��--
#define ATT_CAL_GPH1    0xd300000a  //ͨ��1��λУ��--
	  
//#define ATT_CAL_GP2     0xd40002da  //ͨ��2�й�����У��--
//#define ATT_CAL_GQ2     0xd50002da  //ͨ��2�޹�����У��--
//#define ATT_CAL_GS2     0xd60002da  //ͨ��2���ڹ���У��--
//#define ATT_CAL_GPH2    0xd700000a  //ͨ��2��λУ��--
	  
//#define ATT_I2_GAIN     0xdb004ac2  //����ͨ��2��������У��
	  
#define ATT_P1_OFFSET   0xe50000f5  //ͨ��1С�ź��й�����У��--
//#define ATT_P2_OFFSET   0xe60000f5  //ͨ��2С�ź��й�����У��--
#define ATT_Q1_OFFSET   0xe70000f5  //ͨ��1С�ź��޹�����У��--
//#define ATT_Q2_OFFSET   0xe80000f5  //ͨ��2С�ź��޹�����У��--

//#define ATT_I1_OFFSET   0xe9000000  //����ͨ��1��ЧֵУ��--
//#define ATT_I2_OFFSET   0xea000000  //����ͨ��2��ЧֵУ��--
//#define ATT_U_OFFSET    0xeb000000  //��ѹͨ����ЧֵУ��--
/**
  * @}
  */


	  
	  
void ATT7053_Calib(SPI_TypeDef* SPIx);

float Current1_Sample(SPI_TypeDef* SPIx);
float Current2_Sample(SPI_TypeDef* SPIx);
float Voltage_Sample(SPI_TypeDef* SPIx);

float Frequency_Sample(SPI_TypeDef* SPIx);
float PowerFac_Sample(SPI_TypeDef* SPIx);
float PowerP1_Sample(SPI_TypeDef* SPIx);
float PowerP2_Sample(SPI_TypeDef* SPIx);
float EnergyP_Sample(SPI_TypeDef* SPIx);


#endif