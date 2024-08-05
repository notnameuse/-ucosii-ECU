#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
//#include "beep.h"
extern u8  flag,mark;
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//�ⲿ�ж� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   
//�ⲿ�ж�0�������
void EXTIX_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		EXTI_InitTypeDef EXTI_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;


  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ܸ��ù���ʱ��
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14;//KEY0
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
		GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4

   //GPIOE.4	  �ж����Լ��жϳ�ʼ������  �����ش���	//KEY0
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource1);
		EXTI_InitStructure.EXTI_Line=EXTI_Line1;	//KEY0
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

	//GPIOA.0	  �ж����Լ��жϳ�ʼ������ �����ش��� PA0  WK_UP
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource12); 
  	EXTI_InitStructure.EXTI_Line=EXTI_Line12;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);		//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
		
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource14); 
  	EXTI_InitStructure.EXTI_Line=EXTI_Line14;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);		//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���


  	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;	//��ռ���ȼ�2�� 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//�����ȼ�3
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure); 

		NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;			//ʹ�ܰ���KEY0���ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;	//��ռ���ȼ�2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//�����ȼ�0 
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure);  	  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
}

//�ⲿ�ж�0������� 
//void EXTI0_IRQHandler(void)
//{
//	delay_ms(50);//����
//	if(WK_UP==1)	 	 //WK_UP����
//	{				 
//		LED1=!LED1;	
//	}
//	EXTI_ClearITPendingBit(EXTI_Line0); //���LINE0�ϵ��жϱ�־λ  
//}
 
//�ⲿ�ж�4�������
//void EXTI1_IRQHandler(void)
//{
//	delay_ms(50);//����
//	LED0=!LED0;
//	EXTI_ClearITPendingBit(EXTI_Line9);
//	if(KEY0==1)	 //����KEY0
//	{
//		LED0=!LED0;
//	}		 
//	EXTI_ClearITPendingBit(EXTI_Line4);  //���LINE4�ϵ��жϱ�־λ  
//}

//void EXTI1_IRQHandler(void)//�ⲿ�ж�1
//{
//		delay_ms(50);
//		if(KEY0==0)
//			{
//				flag=1;
//			}
//		EXTI_ClearITPendingBit(EXTI_Line1);
//}


//void EXTI15_10_IRQHandler(void)//�ⲿ�ж�PB12
//{
//		delay_ms(50);
//	  if(KEY1==0)
//	    {
//				mark=1;
//			}
//				
//		EXTI_ClearITPendingBit(EXTI_Line12);
//}

 
