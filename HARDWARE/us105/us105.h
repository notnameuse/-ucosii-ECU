#ifndef __US105_H
#define __US105_H
#include "sys.h"
#include  "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//���Ź� ��������		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/5/30
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  

void us105Init(led_d *io,gpioled port,u16 pin);
void us105_config(led_d *io);
void us105_Start(led_d *io);
#endif
