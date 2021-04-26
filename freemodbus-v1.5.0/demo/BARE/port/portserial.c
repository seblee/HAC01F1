/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "uart.h"
#include "fifo.h"
#include "sys_conf.h"
/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
//void
//vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
//{
//    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
//     * transmitter empty interrupts.
//     */
//}

void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
	if (xRxEnable)
	{
		/* 485ͨ��ʱ���ȴ�������λ�Ĵ����е����ݷ�����ɺ���ȥʹ��485�Ľ��ա�ʧ��485�ķ���*/
		while (!USART_GetFlagStatus(USART1,USART_FLAG_TC));
		RE485;
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	}
	else
	{
		TE485;
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	}
	if (xTxEnable)
	{
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	}
	else
	{
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	}
}


BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
	uart1_init(ulBaudRate);
	return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
	USART_SendData(USART1, ucByte);
	return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
	*pucByte = USART_ReceiveData(USART1);
	return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}


void USART1_IRQHandler(void)
{
	extern sys_reg_st					g_sys; 
	
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		g_sys.status.Com_error[0]=0;
		g_sys.status.Com_error[1]=0;
		USART_ClearITPendingBit( USART1, USART_IT_RXNE);
		prvvUARTRxISR();
	}
	//�����ж�
	if (USART_GetITStatus(USART1, USART_IT_TXE) == SET)
	{
		USART_ClearITPendingBit( USART1, USART_IT_TXE );
		prvvUARTTxReadyISR();
	}
}
