////////////////////////////////////////////////////////////////////////////////
// matsujirushi.c
//
// 2014/01/04 0.1 Takashi Matsuoka
//                ・新規作成。
//

#include <jendefs.h>				// JN516x
#include <AppHardwareApi.h>			// JN516x

#include "matsujirushi.h"

static int TweLiteDipPinToDioNumber(int pin)
{
	if (pin < 1 || 28 < pin)
	{
		return -1;
	}

	static const int mapTweLiteDipPinToDioNumber[28] =
	{
		-1,	// 1
		14,	// 2
		7,	// 3
		5,	// 4
		18,	// 5
		-1,	// 6
		-1,	// 7
		19,	// 8
		4,	// 9
		6,	// 10
		8,	// 11
		9,	// 12
		10,	// 13
		-1,	// 14
		12,	// 15
		13,	// 16
		11,	// 17
		16,	// 18
		15,	// 19
		17,	// 20
		-1,	// 21
		-1,	// 22
		0,	// 23
		-1,	// 24
		1,	// 25
		2,	// 26
		3,	// 27
		-1,	// 28
	};

	return mapTweLiteDipPinToDioNumber[pin - 1];
}

void pinMode(int pin, enum pinmode_t mode)
{
	int dioNumber = TweLiteDipPinToDioNumber(pin);
	if (dioNumber < 0)
	{
		return;
	}

	switch (mode)
	{
	case INPUT:
		vAHI_DioSetDirection(1 << dioNumber, 0);
		vAHI_DioSetPullup(0, 1 << dioNumber);
		break;
	case OUTPUT:
		vAHI_DioSetDirection(0, 1 << dioNumber);
		break;
	case INPUT_PULLUP:
		vAHI_DioSetDirection(1 << dioNumber, 0);
		vAHI_DioSetPullup(1 << dioNumber, 0);
		break;
	}
}

void digitalWrite(int pin, enum digital_t value)
{
	int dioNumber = TweLiteDipPinToDioNumber(pin);
	if (dioNumber < 0)
	{
		return;
	}

	switch (value)
	{
	case LOW:
		vAHI_DioSetOutput(0, 1 << dioNumber);
		break;
	case HIGH:
		vAHI_DioSetOutput(1 << dioNumber, 0);
		break;
	}
}

enum digital_t digitalRead(int pin)
{
	int dioNumber = TweLiteDipPinToDioNumber(pin);
	if (dioNumber < 0)
	{
		return LOW;
	}
	return (u32AHI_DioReadInput() & 1 << dioNumber) != 0 ? HIGH : LOW;
}

int analogRead(int pin)
{
	if (!bAHI_APRegulatorEnabled())
	{
		vAHI_ApConfigure(E_AHI_AP_REGULATOR_ENABLE, E_AHI_AP_INT_DISABLE, E_AHI_AP_SAMPLE_2, E_AHI_AP_CLOCKDIV_500KHZ, E_AHI_AP_INTREF);
		while (!bAHI_APRegulatorEnabled())
		{
		}
	}

	uint8 source;
	switch (pin)
	{
	case 22:
		source = E_AHI_ADC_SRC_ADC_1;
		break;
	case 24:
		source = E_AHI_ADC_SRC_ADC_2;
		break;
	case 23:
		source = E_AHI_ADC_SRC_ADC_3;
		break;
	case 25:
		source = E_AHI_ADC_SRC_ADC_4;
		break;
	case 28:
		source = E_AHI_ADC_SRC_VOLT;
		break;
	default:
		return 0;
	}
	vAHI_AdcEnable(E_AHI_ADC_SINGLE_SHOT, E_AHI_AP_INPUT_RANGE_2, source);

	vAHI_AdcStartSample();
	while (bAHI_AdcPoll())
	{
	}
	return u16AHI_AdcRead();
}

////////////////////////////////////////////////////////////////////////////////
