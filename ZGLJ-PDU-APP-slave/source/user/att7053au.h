#ifndef _ATT7053AU_H
#define _ATT7053AU_H



#define  ATT_SPI_FRAME		4		// ATT SPI通信数据帧长度
#define  ATT_EMU_FREQ		921000	// ATT7053 femu = 921kHz 
#define  ATT_EC				3200	//电表脉冲常数


/** @defgroup ATT7022E data address define
  * @{
  */
/****************EPR*******************/
#define  EPADR_Spl_I1		0x00	// 3bytes	电流通道1的ADC采样数据
#define  EPADR_Spl_I2		0x01	// 3bytes	电流通道2的ADC采样数据
#define  EPADR_Spl_U		0x02	// 3bytes	电压通道的ADC采样数据
#define  EPADR_Spl_P		0x03	// 3bytes	有功功率波形数据
#define  EPADR_Spl_Q		0x04	// 3bytes	无功功率波形数据
#define  EPADR_Spl_S		0x05	// 3bytes	视在功率波形数据
#define  EPADR_Rms_I1		0x06	// 3bytes	电流通道1的有效值
#define  EPADR_Rms_I2		0x07	// 3bytes	电流通道2的有效值
#define  EPADR_Rms_U		0x08	// 3bytes	电压通道的有效值
#define  EPADR_Freq_U		0x09	// 2bytes	电压频率
#define  EPADR_Power_P1		0x0A	// 3bytes	通道1有功功率
#define  EPADR_Power_Q1		0x0B	// 3bytes	通道1无功功率
#define  EPADR_Power_S		0x0C	// 3bytes	视在功率
#define  EPADR_Energy_P		0x0D	// 3bytes	有功能量
#define  EPADR_Energy_Q		0x0E	// 3bytes	无功能量
#define  EPADR_Energy_S		0x0F	// 3bytes	视在能量
#define  EPADR_Power_P2		0x10	// 3bytes	通道2有功功率
#define  EPADR_Power_Q2		0x11	// 3bytes	通道2无功功率
#define  EPADR_I1_Angle		0x12	// 3bytes	通道1与电压夹角
#define  EPADR_I2_Angle		0x13	// 3bytes	通道2与电压夹角
#define  EPADR_TempData 	0x14	// 1byte	温度数据
/**
  * @}
  */


/** @defgroup ATT7022E command define
  * @note     Some parameters need to modify reference to real value(--)
  * @{
  */
///******************************ECR*****************************************/ 
//#define	EMUIE			30H // 0000 2(15bit) EMU 中断使能寄存器
//#define	EMUIF			31H // 0000 2(16bit) EMU 中断标志寄存器
//#define	WPREG			32H // 00   1(8bit) 写保护寄存器
//#define	SRSTREG			33H // 00   1(8bit) 软件复位寄存器	  
//
//#define	EMUCFG 			40H // 0010 2(15bit) EMU 配置寄存器
//#define	FreqCFG			41H // 0088 2(9bit) 时钟/更新频率配置寄存器
//#define	ModuleEn		42H // 007E 2(14bit) EMU 模块使能寄存器
//#define	ANAEN			43H // 3B 	1(7bit) 模拟模块使能寄存器
//#define	X				44H // 0000 2(11bit) REV
//#define	IOCFG			45H // 0024 2(10bit) IO 输出配置寄存器
//        
//#define	GP1				50H // 0000 2(16bit) 通道 1 的有功功率校正
//#define	GQ1				51H // 0000 2(16bit) 通道 1 的无功功率校正
//#define	GS1				52H // 0000 2(16bit) 通道 1 的视在功率校正
//#define	Phase1			53H // 00 	1(8bit) 通道 1 的相位校正（移采样点方式）
//#define	GP2				54H // 0000 2(16bit) 通道 2 的有功功率校正
//#define	GQ2				55H // 0000 2(16bit) 通道 2 的无功功率校正
//#define	GS2				56H // 0000 2(16bit) 通道 2 的视在功率校正
//#define	Phase2			57H // 00	1(8bit) 通道 2 的相位校正（移采样点方式）
//#define	QPhsCal			58H // FF00 2(16bit) 无功相位补偿
//#define	ADCCON			59H // 0000 2(12bit) ADC 通道增益选择
//#define	AllGain			5AH // 0000 2(16bit) 3个 ADC 通道整体增益寄存器
//#define	I2Gain			5BH // 0000 2(16bit) 电流通道 2 增益补偿
//#define	I1Off			5CH // 0000 2(16bit) 电流通道 1 的偏置校正
//#define	I2Off			5DH // 0000 2(16bit) 电流通道 2 的偏置校正
//#define	UOff			5EH // 0000 2(16bit) 电压通道的偏置校正
//#define	PQStart			5FH // 0040 2(16bit) 起动功率设置
//#define	RMSStart		60H // 0040 2(16bit) 有效值启动值设置寄存器
//#define	HFConst			61H // 0040 2(15bit) 输出脉冲频率设置
//#define	CHK				62H // 10 	1(8bit) 窃电阈值设置
//#define	IPTAMP			63H // 0020 2(16bit) 窃电检测电流域值
//#define	UCONST			64H // 0000 2(16bit) 失压情况下参与计量的电压，断相仿窃电
//#define	P1OFFSET		65H // 00 	1(8bit) 通道 1 有功功率偏执校正参数，为 8bit 补码
//#define	P2OFFSET		66H // 00 	1(8bit) 通道 2 有功功率偏执校正参数，为 8bit 补码
//#define	Q1OFFSET		67H // 00 	1(8bit) 通道 1 无功功率偏执校正参数，为 8bit 补码
//#define	Q2OFFSET		68H // 00	1(8bit) 通道 2 无功功率偏执校正参数，为 8bit 补码
//#define	I1RMSOFFSET		69H // 00 	1(8bit) 通道 1 有效值补偿寄存器，为 8bit 无符号数
//#define	I2RMSOFFSET		6AH // 00 	1(8bit) 通道 2 有效值补偿寄存器，为 8bit 无符号数
//#define	URMSOFFSET		6BH // 00 	1(8bit) 电压通道（通道 3）有效值补偿寄存器，为 8bit无符号数
//#define	ZCrossCurrent	6CH // 0004 2(16bit) 电流过零阈值设置寄存器
//#define	GPhs1			6DH // 0000 2(16bit) 通道 1 的相位校正（ PQ 方式）
//#define	GPhs2			6EH // 0000 2(16bit) 通道 2 的相位校正（ PQ 方式）
//#define	PFCnt			6FH // 0000 2(16bit) 快速有功脉冲计数
//#define	QFCnt			70H // 0000 2(16bit) 快速无功脉冲计数
//#define	SFCnt			71H // 0000 2(16bit) 快速视在脉冲计数

#define ATT_WPROTECT    0xb2000000  //寄存器写保护

#define ATT_WPROTECT1   0xb20000BC  //寄存器写保护,只能操作0x40H——0x45H(0xc0——0xc5)
#define ATT_CON_EMU     0xc0002010  //EMU单元配置, 默认值
#define ATT_CON_FREQ	0xc1000088	//时钟/更新频率配置寄存器, 默认值
#define ATT_EN_MODULE   0xc20000fe  //模块使能配置，开启通道1和2电流和电压通道角度测量，关闭通道2功率测量
#define ATT_EN_ANMODE   0xc3000037  //模拟模块使能配置
#define ATT_CON_IOCFG   0xc5000024  //IO输出配置， 默认值

/**************************************************************************
	HFConst＝5.75*Vu*Vi*10^10/(EC*Un*Ib)
	Vu： 额定电压输入时，电压通道的电压（引脚上电压×放大倍数）
	Vi： 额定电流输入时，电流通道的电压 （引脚上电压×放大倍数）
	Un：额定输入的电压
	Ib： 额定输入的电流
	EC：电表常数,此处设为3200
	此处:HFConst＝5.75*0.44*0.4*10^10/(3200*220*10)=1437(0x059D)
**************************************************************************/
#define ATT_WPROTECT2   0xb20000A6  //寄存器写保护,只能操作0x50H——0x71H(0xd0——0xf1)
#define ATT_HF_CONST    0xe1001440  //高频输出脉冲常数
#define R_ATT_HF_CONST  0x61000000  //读取高频输出脉冲常数

#define ATT_CAL_GP1     0xd00002da  //通道1有功功率校正--
#define ATT_CAL_GQ1     0xd10002da  //通道1无功功率校正--
#define ATT_CAL_GS1     0xd20002da  //通道1视在功率校正--
#define ATT_CAL_GPH1    0xd300000a  //通道1相位校正--
	  
//#define ATT_CAL_GP2     0xd40002da  //通道2有功功率校正--
//#define ATT_CAL_GQ2     0xd50002da  //通道2无功功率校正--
//#define ATT_CAL_GS2     0xd60002da  //通道2视在功率校正--
//#define ATT_CAL_GPH2    0xd700000a  //通道2相位校正--
	  
//#define ATT_I2_GAIN     0xdb004ac2  //电流通道2电流增益校正
	  
#define ATT_P1_OFFSET   0xe50000f5  //通道1小信号有功功率校正--
//#define ATT_P2_OFFSET   0xe60000f5  //通道2小信号有功功率校正--
#define ATT_Q1_OFFSET   0xe70000f5  //通道1小信号无功功率校正--
//#define ATT_Q2_OFFSET   0xe80000f5  //通道2小信号无功功率校正--

//#define ATT_I1_OFFSET   0xe9000000  //电流通道1有效值校正--
//#define ATT_I2_OFFSET   0xea000000  //电流通道2有效值校正--
//#define ATT_U_OFFSET    0xeb000000  //电压通道有效值校正--
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