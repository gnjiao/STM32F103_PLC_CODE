
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dac.h"
#include "stm32f10x_dma.h"
#include <stdio.h>       

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ADC1_DR_Address    ((uint32_t)0x4001244C)  //ADC1 DR�Ĵ�������ַ
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#define N  100  
#define M  2    
// ע��ADCΪ12λģ��ת������ֻ��ADCConvertedValue�ĵ�12λ��Ч
__IO uint16_t ADCConvertedValue[N][M];

//extern u16 PLC_16BIT[12100];		 
extern u16  all_data[16600];
extern void DAC_out_init(void);


void RCC_Configuration(void)  
{ 
	ADC_InitTypeDef  ADC_InitStructure;      //ADC��ʼ���ṹ������
	DMA_InitTypeDef  DMA_InitStructure;      //DMA��ʼ���ṹ������  
	
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable ADC1 and GPIOC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);  
	/* Enable DMA1 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	/* Configure PC.00 (ADC Channel10) as analog input -------------------------*/
	//PC0 ��Ϊģ��ͨ��10��������                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;                // DMA��Ӧ���������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValue;       // �ڴ�洢����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	                       // DMA��ת��ģʽΪSRCģʽ����������Ƶ��ڴ�
	DMA_InitStructure.DMA_BufferSize = N*M;		                                 // DMA�����С����λΪDMA_MemoryDataSize
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	         // ����һ�����ݺ��豸��ַ��ֹ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	                   // ����һ�����ݺ�Ŀ���ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;// �����������ݿ���Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;        // DMA�����ݳߴ磬HalfWord����Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                            // ת��ģʽ��ѭ������ģʽ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;	                       // DMA���ȼ���
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		                           // M2Mģʽ����
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);         
	
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                       // ������ת��ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;		                         // ����ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                       // ��������ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	     // ADC�ⲿ���أ��ر�״̬
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                   // ���뷽ʽ,ADCΪ12λ�У��Ҷ��뷽ʽ
	ADC_InitStructure.ADC_NbrOfChannel = M;	                                 // ����ͨ������4��
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channel10 configuration ADCͨ���飬 ��10��ͨ�� ����˳��1��ת��ʱ�� */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_239Cycles5);

	
	/* ADC���ʹ�� */
	ADC_DMACmd(ADC1, ENABLE);	  
	/* ����ADC1 */
	ADC_Cmd(ADC1, ENABLE);  
	/* ����У׼ */   
	ADC_ResetCalibration(ADC1);	  
	/* �ȴ�����У׼��� */
	while(ADC_GetResetCalibrationStatus(ADC1));
	/* ��ʼУ׼ */
	ADC_StartCalibration(ADC1);		
	/* �ȴ�У׼��� */
	while(ADC_GetCalibrationStatus(ADC1));	  
	/* ����ת����ʼ��ADCͨ��DMA��ʽ���ϵĸ���RAM */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	
}


	 
void ADC_init(void)
{	
	RCC_Configuration();
	DAC_out_init();
}

void filter(void)
{ 
	int sum=0;
	u8  count,i;
	for(i=0;i<M;i++)		 //
	{ 
		for(count=0;count<N;count++)
		{
			sum+=ADCConvertedValue[count][i];
		}
		all_data[0x378E+i] =sum/N;	
		sum=0;
	}
}