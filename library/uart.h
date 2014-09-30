////////////////////////////////////////////////////////////////////////////////
// uart.h
//
// 2014/09/30 1.0 Takashi Matsuoka
//

#ifndef UART_H_
#define UART_H_

#include "fprintf.h"				// ToCoNetUtils

enum UART_PARITY
{
	UART_PARITY_NONE,
	UART_PARITY_EVEN,
	UART_PARITY_ODD,
};

PUBLIC bool uart0_init(uint32 baud_rate, uint8 data_bits, enum UART_PARITY parity, uint8 stop_bits);
PUBLIC void uart0_link_tsFILE(tsFILE* file);
PUBLIC void uart0_flush();

#endif /* UART_H_ */

////////////////////////////////////////////////////////////////////////////////
