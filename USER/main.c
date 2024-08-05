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



/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	


//电机正向转动任务
//设置任务优先级
#define PWM_TASK_PRIO       			7
//设置任务堆栈大小
#define PWM_STK_SIZE  					64
//任务堆栈
OS_STK PWM_TASK_STK[PWM_STK_SIZE];
//任务函数
void pwm_task(void *pdata);


//电机反向转动
#define Re_TASK_PRIO							6
#define	Re_STK_SIZE								64
OS_STK	Re_TASK_STK[Re_STK_SIZE];
void 	reverse_task(void *pdata);


//电机转速显示任务
#define Motoshow_task_prio   				4
#define Motoshow_task_size					64
OS_STK	Motoshow_task_stk[Motoshow_task_size];
void Motoshow_task(void *pdata);


//us105测量距离任务
#define us105_task_prio       		  5    
#define us105_task_size							128
OS_STK	us105_task_stk[us105_task_size];
void us105_task(void *pdata);

//us_015启动任务
#define st_task_prio                 9
#define st_task_size								32
OS_STK	st_task_stk[st_task_size];
void st_task(void *pdata);

//MPU6050测量倾斜角度任务						
#define mpu6050_task_prio						 8
#define	mpu6050_task_size						512
OS_STK	mpu6050_task_stk[mpu6050_task_size];
void mpu6050_task(void *pdata);


//定义需要用到的全局变量
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
	delay_init();	    //延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级 	
	MPU_Init();
	mpu_dmp_init();
	LED_Init(&led1,GPIOC,GPIO_Pin_13);			 //初始化与LED连接的硬件接口
	Beep_Init(&bep,GPIOB,GPIO_Pin_15);
	us105Init(&us,GPIOA,GPIO_Pin_12);
	TIM3_PWM_Init(99,14399);
	TIM2_Int_Init(4999,14399);
	TIM1_Cap_Init(84,71999);//计数频率也不能太高，否则程序也会跑飞，因为会频繁触发中断占用cpu资源
	EXTIX_Init();
	LCD_Init();	
	uart_init(115200);
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	  	 
}
 
	  
//开始任务
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
  OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    				   
 	OSTaskCreate(pwm_task,(void *)0,(OS_STK*)&PWM_TASK_STK[PWM_STK_SIZE-1],PWM_TASK_PRIO);	//而R0寄存器是堆栈指针PSP，如果PSP=0，代表任务第一次进行任务切换，所以
	OSTaskCreate(reverse_task,(void *)0,(OS_STK*)&Re_TASK_STK[Re_STK_SIZE-1],Re_TASK_PRIO);
	OSTaskCreate(Motoshow_task,(void *)0,(OS_STK*)&Motoshow_task_stk[Motoshow_task_size-1],Motoshow_task_prio);
	OSTaskCreate(us105_task,(void *)0,(OS_STK*)&us105_task_stk[us105_task_size-1],us105_task_prio);
	OSTaskCreate(st_task,(void *)0,(OS_STK*)&st_task_stk[st_task_size-1],st_task_prio);
	OSTaskCreate(mpu6050_task,(void *)0,(OS_STK*)&mpu6050_task_stk[mpu6050_task_size-1],mpu6050_task_prio);
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}



//电机正向转动任务
void pwm_task(void *pdata)
{	u8 err;
	while(1)
	{
		OSSemPend(sig1,0,&err);
		TIM_SetCompare3(TIM3,flag);
	}
}
//电机反转任务
void reverse_task(void *pdata)
{
	u8 err;
	while(1)
	{
		OSSemPend(sig2,0,&err);
		TIM_SetCompare3(TIM3,mark);
	}
}

//电机转速显示任务
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


//us105测量距离任务
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

//us-015启动任务
void st_task(void *pdata)
{
	while(1)
	{
		us105_Start(&us);
		delay_ms(1000);//这个延时一定要加，因为这是us015的触发信号，但是触发信号的频率越高，输出的脉冲频率也越高，导致进入中断的频率也会越高，会让中断过于频繁，让cpu资源耗尽
//导致程序跑飞，当我延时了1s时，此时显示距离都很正常了。
	}
}
	

//mpu6050测量倾斜角度任务
void mpu6050_task(void *pdata)
{
	float pitch,roll,yaw; 
	int tmp;
	while(1)
	{
		if(mpu_dmp_get_data(&pitch,&roll,&yaw)!=0)//防止FIFO队列溢出，队列溢出是指你处理的速度过慢，导致你处理的跟不上人家发送过来的速度，就会堆积在缓冲区，造成缓冲区堵塞
		{//因此建议把延时弄低一些，加快处理速度。
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
			delay_ms(300);//延时尽量短，保证读取数据的频率足够高，否则时间长了会看不到数据变化和FIFO溢出,最高延时不能超过300ms
		}
	}
}



void EXTI1_IRQHandler(void)//外部中断1,对应蜂鸣器按键，按下加速
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


void EXTI15_10_IRQHandler(void)//外部中断PB12,PC14用于控制电机反转/电机转速计数
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


void TIM2_IRQHandler(void)//定时器中断函数，定时1s计算此时的转速,需要注意的是中断函数里边不能写太复杂的东西，不然就会被卡住
{
	OSIntEnter();
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查 TIM2 更新中断发生与否
	{
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update ); //清除 TIM2 更新中断标志
			OSSemPost(sig3);
	}
	OSIntExit();
}



//以下是us105测距模块，捕获传回来的高电平脉冲,捕获中断
void TIM1_CC_IRQHandler(void)
{ 
	OSIntEnter();
 	if((STA&0X80)==0)//还未成功捕获	
	{	  
		if (TIM_GetITStatus(TIM1, TIM_IT_CC1) != RESET)//捕获1发生捕获事件
			{	
				if(STA&0X40)		//捕获到一个下降沿 		
				{	  			
					STA|=0X80;		//标记成功捕获到一次高电平脉宽
					PluseWidth=TIM_GetCapture1(TIM1);
					TIM_OC1PolarityConfig(TIM1,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
				}else  								//还未开始,第一次捕获上升沿
				{
					STA=0;			//清空
					PluseWidth=0;
					TIM_SetCounter(TIM1,0);
					STA|=0X40;		//因此程序实际上是等到上升沿中断的时候，在此处主动把这个比寄存器的值变成不是0的
					TIM_OC1PolarityConfig(TIM1,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
				}		    
			}			     	    					   
 	}
	 OSSemPost(sig4);
   TIM_ClearITPendingBit(TIM1, TIM_IT_CC1); //清除中断标志位
	 OSIntExit();
}


