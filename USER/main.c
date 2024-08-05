#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "includes.h"
#include "adc.h"
#include "exti.h"
#include "lcd.h"
#include "gui.h"
#include "beep.h"
#include "us105.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h" 



/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	


//�������ת������
//�����������ȼ�
#define PWM_TASK_PRIO       			7
//���������ջ��С
#define PWM_STK_SIZE  					64
//�����ջ
OS_STK PWM_TASK_STK[PWM_STK_SIZE];
//������
void pwm_task(void *pdata);


//�������ת��
#define Re_TASK_PRIO							6
#define	Re_STK_SIZE								64
OS_STK	Re_TASK_STK[Re_STK_SIZE];
void 	reverse_task(void *pdata);


//���ת����ʾ����
#define Motoshow_task_prio   				4
#define Motoshow_task_size					64
OS_STK	Motoshow_task_stk[Motoshow_task_size];
void Motoshow_task(void *pdata);


//us105������������
#define us105_task_prio       		  5    
#define us105_task_size							128
OS_STK	us105_task_stk[us105_task_size];
void us105_task(void *pdata);

//us_015��������
#define st_task_prio                 9
#define st_task_size								32
OS_STK	st_task_stk[st_task_size];
void st_task(void *pdata);

//MPU6050������б�Ƕ�����						
#define mpu6050_task_prio						 8
#define	mpu6050_task_size						512
OS_STK	mpu6050_task_stk[mpu6050_task_size];
void mpu6050_task(void *pdata);


//������Ҫ�õ���ȫ�ֱ���
//OS_EVENT *Bsem; 
//OS_EVENT *email;
OS_EVENT *sig1;
OS_EVENT *sig2;
OS_EVENT *sig3;
OS_EVENT *sig4;
led_d led1;
led_d bep;
led_d us;
volatile u8 flag=8;
volatile u8 mark=8;
volatile u32 rate=0;
volatile u32 buff;
volatile u8  STA=0;
volatile u16 PluseWidth;
u32 				 num;



 int main(void)
 {	
	delay_init();	    //��ʱ������ʼ��	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ� 	
	MPU_Init();
	mpu_dmp_init();
	LED_Init(&led1,GPIOC,GPIO_Pin_13);			 //��ʼ����LED���ӵ�Ӳ���ӿ�
	Beep_Init(&bep,GPIOB,GPIO_Pin_15);
	us105Init(&us,GPIOA,GPIO_Pin_12);
	TIM3_PWM_Init(99,14399);
	TIM2_Int_Init(4999,14399);
	TIM1_Cap_Init(84,71999);//����Ƶ��Ҳ����̫�ߣ��������Ҳ���ܷɣ���Ϊ��Ƶ�������ж�ռ��cpu��Դ
	EXTIX_Init();
	LCD_Init();	
	uart_init(115200);
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	  	 
}
 
	  
//��ʼ����
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
	//Bsem=OSSemCreate(0);
	sig1=OSSemCreate(0);
	sig2=OSSemCreate(0);
	sig3=OSSemCreate(0);
	sig4=OSSemCreate(0);
	//email=OSMboxCreate((void *)0);
  OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    				   
 	OSTaskCreate(pwm_task,(void *)0,(OS_STK*)&PWM_TASK_STK[PWM_STK_SIZE-1],PWM_TASK_PRIO);	//��R0�Ĵ����Ƕ�ջָ��PSP�����PSP=0�����������һ�ν��������л�������
	OSTaskCreate(reverse_task,(void *)0,(OS_STK*)&Re_TASK_STK[Re_STK_SIZE-1],Re_TASK_PRIO);
	OSTaskCreate(Motoshow_task,(void *)0,(OS_STK*)&Motoshow_task_stk[Motoshow_task_size-1],Motoshow_task_prio);
	OSTaskCreate(us105_task,(void *)0,(OS_STK*)&us105_task_stk[us105_task_size-1],us105_task_prio);
	OSTaskCreate(st_task,(void *)0,(OS_STK*)&st_task_stk[st_task_size-1],st_task_prio);
	OSTaskCreate(mpu6050_task,(void *)0,(OS_STK*)&mpu6050_task_stk[mpu6050_task_size-1],mpu6050_task_prio);
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}



//�������ת������
void pwm_task(void *pdata)
{	u8 err;
	while(1)
	{
		OSSemPend(sig1,0,&err);
		TIM_SetCompare3(TIM3,flag);
	}
}
//�����ת����
void reverse_task(void *pdata)
{
	u8 err;
	while(1)
	{
		OSSemPend(sig2,0,&err);
		TIM_SetCompare3(TIM3,mark);
	}
}

//���ת����ʾ����
void Motoshow_task(void *pdata)
{
	u8 err;
	while(1)
	{
		OSSemPend(sig3,0,&err);
		buff=rate;
		Show_Str(0,80,BLUE,WHITE,"Speed:00r/min",16,0);
		LCD_ShowNum(0+6*8,80,60*buff/2,2,16);
		rate=0;
	}
}


//us105������������
void us105_task(void *pdata)
{
	u8 err;
	while(1)
	{
		OSSemPend(sig4,0,&err);
		if(STA&0x80)
		{
			num=0.005*340*PluseWidth;
			Show_Str(0,60,BLUE,WHITE,"juli:00cm",16,0);
			LCD_ShowNum(0+5*8,60,num,2,16);
			if(num<=5)
			{
				Beep_on(&bep);
				led_on(&led1);
				TIM_SetCompare3(TIM3,8);
			}	
			else
			{
				led_off(&led1);
				Beep_off(&bep);
			}
			STA=0;
	  }
	}
}

//us-015��������
void st_task(void *pdata)
{
	while(1)
	{
		us105_Start(&us);
		delay_ms(1000);//�����ʱһ��Ҫ�ӣ���Ϊ����us015�Ĵ����źţ����Ǵ����źŵ�Ƶ��Խ�ߣ����������Ƶ��ҲԽ�ߣ����½����жϵ�Ƶ��Ҳ��Խ�ߣ������жϹ���Ƶ������cpu��Դ�ľ�
//���³����ܷɣ�������ʱ��1sʱ����ʱ��ʾ���붼�������ˡ�
	}
}
	

//mpu6050������б�Ƕ�����
void mpu6050_task(void *pdata)
{
	float pitch,roll,yaw; 
	int tmp;
	while(1)
	{
		if(mpu_dmp_get_data(&pitch,&roll,&yaw)!=0)//��ֹFIFO������������������ָ�㴦����ٶȹ����������㴦��ĸ������˼ҷ��͹������ٶȣ��ͻ�ѻ��ڻ���������ɻ���������
		{//��˽������ʱŪ��һЩ���ӿ촦���ٶȡ�
			tmp = roll * 10;
			if (tmp < 0)
			{
					tmp = -tmp;
					Show_Str(0,40,BLUE,WHITE,"angle:-00.00d",16,0);
					LCD_ShowNum(0+7*8,40,tmp/10,2,16);
					LCD_ShowNum(0+10*8,40,tmp%10,2,16);
			}
			else
			{
					Show_Str(0,40,BLUE,WHITE,"angle:00.00d",16,0);
					LCD_ShowNum(0+6*8,40,tmp/10,2,16);
					LCD_ShowNum(0+9*8,40,tmp%10,2,16);
			}
			if(tmp>45)
			{
					TIM_SetCompare3(TIM3,8);
					Show_Str(0,20,BLUE,WHITE,"Cuation!",16,0);
			}
			else 	LCD_Fill(0,0,128,35,WHITE);
			delay_ms(300);//��ʱ�����̣���֤��ȡ���ݵ�Ƶ���㹻�ߣ�����ʱ�䳤�˻ῴ�������ݱ仯��FIFO���,�����ʱ���ܳ���300ms
		}
	}
}



void EXTI1_IRQHandler(void)//�ⲿ�ж�1,��Ӧ���������������¼���
{
		OSIntEnter();
		delay_ms(10);
		if(KEY0==0)
			{
				flag-=1;
				if(flag<3)
				{
					flag=8;
				}
				Show_Str(0,100,BLUE,WHITE,"Gear:",16,0);
				LCD_ShowNum(0+5*8,100,flag,2,16);
				OSSemPost(sig1);
			}
		EXTI_ClearITPendingBit(EXTI_Line1);
		OSIntExit();
}


void EXTI15_10_IRQHandler(void)//�ⲿ�ж�PB12,PC14���ڿ��Ƶ����ת/���ת�ټ���
{
		OSIntEnter();
		delay_ms(10);
	if(EXTI_GetITStatus(EXTI_Line12)!=RESET)
	{
	  if(KEY1==0)
	    {
				mark+=1;
				if(mark>13)
				{
					mark=8;
				}
				Show_Str(0,120,BLUE,WHITE,"Gear:",16,0);
				LCD_ShowNum(0+5*8,120,mark,2,16);
				OSSemPost(sig2);
	    }
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
	if(EXTI_GetITStatus(EXTI_Line14)!=RESET)
	{
		if(KEY2==0)
		{
			rate++;
		}
		EXTI_ClearITPendingBit(EXTI_Line14);
	}
		OSIntExit();
}


void TIM2_IRQHandler(void)//��ʱ���жϺ�������ʱ1s�����ʱ��ת��,��Ҫע������жϺ�����߲���д̫���ӵĶ�������Ȼ�ͻᱻ��ס
{
	OSIntEnter();
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //��� TIM2 �����жϷ������
	{
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update ); //��� TIM2 �����жϱ�־
			OSSemPost(sig3);
	}
	OSIntExit();
}



//������us105���ģ�飬���񴫻����ĸߵ�ƽ����,�����ж�
void TIM1_CC_IRQHandler(void)
{ 
	OSIntEnter();
 	if((STA&0X80)==0)//��δ�ɹ�����	
	{	  
		if (TIM_GetITStatus(TIM1, TIM_IT_CC1) != RESET)//����1���������¼�
			{	
				if(STA&0X40)		//����һ���½��� 		
				{	  			
					STA|=0X80;		//��ǳɹ�����һ�θߵ�ƽ����
					PluseWidth=TIM_GetCapture1(TIM1);
					TIM_OC1PolarityConfig(TIM1,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
				}else  								//��δ��ʼ,��һ�β���������
				{
					STA=0;			//���
					PluseWidth=0;
					TIM_SetCounter(TIM1,0);
					STA|=0X40;		//��˳���ʵ�����ǵȵ��������жϵ�ʱ���ڴ˴�����������ȼĴ�����ֵ��ɲ���0��
					TIM_OC1PolarityConfig(TIM1,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
				}		    
			}			     	    					   
 	}
	 OSSemPost(sig4);
   TIM_ClearITPendingBit(TIM1, TIM_IT_CC1); //����жϱ�־λ
	 OSIntExit();
}


