////////////////////////////////////////////////////////////////////////////////
// uart.c
//

#include <jendefs.h>				// JN516x
#include <AppHardwareApi.h>			// JN516x

#include "serial.h"					// ToCoNetUtils

#include "uart.h"

PUBLIC bool uart0_init(uint32 baud_rate, uint8 data_bits, enum UART_PARITY parity, uint8 stop_bits)
{
	static uint8 tx_buffer[256];
	static uint8 rx_buffer[256];

	tsSerialPortSetup setup;
	setup.pu8SerialRxQueueBuffer = rx_buffer;
	setup.pu8SerialTxQueueBuffer = tx_buffer;
	setup.u32BaudRate = baud_rate;
	setup.u16AHI_UART_RTS_LOW = 0xffff;
	setup.u16AHI_UART_RTS_HIGH = 0xffff;
	setup.u16SerialRxQueueSize = sizeof (rx_buffer);
	setup.u16SerialTxQueueSize = sizeof (tx_buffer);
	setup.u8SerialPort = E_AHI_UART_0;
	setup.u8RX_FIFO_LEVEL = E_AHI_UART_FIFO_LEVEL_1;

	tsUartOpt opt;
	opt.bHwFlowEnabled = FALSE;
	switch (parity)
	{
	case UART_PARITY_NONE:
		opt.bParityEnabled = FALSE;
		break;
	case UART_PARITY_EVEN:
		opt.bParityEnabled = TRUE;
		opt.u8ParityType = E_AHI_UART_EVEN_PARITY;
		break;
	case UART_PARITY_ODD:
		opt.bParityEnabled = TRUE;
		opt.u8ParityType = E_AHI_UART_ODD_PARITY;
		break;
	default:
		return FALSE;
	}
	switch (stop_bits)
	{
	case 1:
		opt.u8StopBit = E_AHI_UART_1_STOP_BIT;
		break;
	case 2:
		opt.u8StopBit = E_AHI_UART_2_STOP_BITS;
		break;
	default:
		return FALSE;
	}
	if (data_bits < 5 || 8 < data_bits)
	{
		return FALSE;
	}
	opt.u8WordLen = data_bits;

	SERIAL_vInitEx(&setup, &opt);

	return TRUE;
}

PUBLIC void uart0_link_tsFILE(tsFILE* file)
{
	file->bPutChar = SERIAL_bTxChar;
	file->u8Device = E_AHI_UART_0;
}

PUBLIC void uart0_flush()
{
	SERIAL_vFlush(E_AHI_UART_0);
}

////////////////////////////////////////////////////////////////////////////////
