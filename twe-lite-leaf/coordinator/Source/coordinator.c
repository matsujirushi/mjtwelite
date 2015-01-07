////////////////////////////////////////////////////////////////////////////////
// coordinator.c
//
// 2015/01/07 1.0 Takashi Matsuoka
//

#include <jendefs.h>				// JN516x
#include <AppHardwareApi.h>			// JN516x

#include "config.h"

// Select ToCoNet modules.
// -> ToCoNet_SDK_manual_201406.pdf P86.

#include "ToCoNet.h"				// ToCoNet
#include "ToCoNet_mod_prototype.h"	// ToCoNet
#include "ToCoNet_event.h"			// ToCoNet

#include "utils.h"					// ToCoNetUtils
#include "serial.h"					// ToCoNetUtils

#include "../../../library/system.h"
#include "../../../library/uart.h"

typedef enum
{
	E_STATE_APP_BASE = ToCoNet_STATE_APP_BASE,
	E_STATE_APP_STARTUP_COLD,
} teStateApp;

static tsFILE uart;
static uint32 LedOn = 0;

static void vProcessEvCore(tsEvent *pEv, teEvent eEvent, uint32 u32evarg);

static void InitializeHardware(bool_t fWarmStart)
{
	// Brown outを設定。
	if (!fWarmStart)
	{
		vAHI_BrownOutConfigure(0, FALSE, FALSE, FALSE, FALSE);
	}

	// vfPrintf()用tsFILEを設定。
	uart0_init(115200, 8, UART_PARITY_NONE, 1);
	uart0_link_tsFILE(&uart);

	vPortAsOutput(9);
}

////////////////////////////////////////
// 電源投入時コールバック関数
//
void cbAppColdStart(bool_t bAfterAhiInit)
{
	if (!bAfterAhiInit)
	{
		ToCoNet_REG_MOD_ALL();
	}
	else
	{
		InitializeHardware(FALSE);

		// ユーザ定義イベント処理関数を登録。
		ToCoNet_Event_Register_State_Machine(vProcessEvCore);

		// sToCoNet_AppContextを設定。
		// -> ToCoNet_SDK_manual_201406.pdf P89.
		sToCoNet_AppContext.u32AppId = APP_ID;		// アプリケーションID
		sToCoNet_AppContext.u32ChMask = CH_MASK;	// 利用チャネル群
		sToCoNet_AppContext.u8TxMacRetry = 0;		// MAC層の再送回数
		sToCoNet_AppContext.bRxOnIdle = TRUE;		// 受信要否
	}
}

////////////////////////////////////////
// スリープ復帰時コールバック関数
//
void cbAppWarmStart(bool_t bAfterAhiInit)
{
	if (!bAfterAhiInit)
	{
	}
	else
	{
		InitializeHardware(TRUE);
	}
}

////////////////////////////////////////
// ハードウェア割り込みハンドラ
//
uint8 cbToCoNet_u8HwInt(uint32 u32DeviceId, uint32 u32ItemBitmap)
{
	return FALSE;
}

////////////////////////////////////////
// 送信完了コールバック関数
//
void cbToCoNet_vTxEvent(uint8 u8CbId, uint8 bStatus)
{
}

////////////////////////////////////////
// ネットワークイベントコールバック関数
//
void cbToCoNet_vNwkEvent(teEvent eEvent, uint32 u32arg)
{
	switch (eEvent)
	{
	case E_EVENT_TOCONET_NWK_START:
		ToCoNet_Event_Process(eEvent, u32arg, vProcessEvCore);
		break;
	}
}

////////////////////////////////////////
// ユーザ定義イベント処理関数
//
PRSEV_HANDLER_DEF(E_STATE_IDLE, tsEvent *pEv, teEvent eEvent, uint32 u32evarg)
{
	if (eEvent == E_EVENT_NEW_STATE)
	{
		vfPrintf(&uart, "#E_STATE_IDLE"LB);
	}
	else if (eEvent == E_EVENT_START_UP)
	{
		vfPrintf(&uart, "#E_EVENT_START_UP"LB);

		ToCoNet_Event_SetState(pEv, E_STATE_APP_STARTUP_COLD);
	}
}

PRSEV_HANDLER_DEF(E_STATE_APP_STARTUP_COLD, tsEvent *pEv, teEvent eEvent, uint32 u32evarg)
{
	if (eEvent == E_EVENT_NEW_STATE)
	{
		vfPrintf(&uart, "#E_STATE_APP_STARTUP_COLD"LB);

		ToCoNet_vMacStart();

		ToCoNet_Event_SetState(pEv, E_STATE_RUNNING);
	}
}

PRSEV_HANDLER_DEF(E_STATE_RUNNING, tsEvent *pEv, teEvent eEvent, uint32 u32evarg)
{
	if (eEvent == E_EVENT_NEW_STATE)
	{
		vfPrintf(&uart, "#E_STATE_RUNNING"LB);
	}
	else if (eEvent == E_EVENT_TICK_SECOND)
	{
		vPutChar(&uart, '.');
	}
	else if (eEvent == E_EVENT_TICK_TIMER)
	{
		if (LedOn > 0)
		{
			vPortSetHi(9);
			LedOn--;
		}
		else
		{
			vPortSetLo(9);
		}
	}
}

static const tsToCoNet_Event_StateHandler asStateFuncTbl[] =
{
	PRSEV_HANDLER_TBL_DEF(E_STATE_IDLE),
	PRSEV_HANDLER_TBL_DEF(E_STATE_APP_STARTUP_COLD),
	PRSEV_HANDLER_TBL_DEF(E_STATE_RUNNING),
};

static void vProcessEvCore(tsEvent *pEv, teEvent eEvent, uint32 u32evarg)
{
	ToCoNet_Event_StateExec(asStateFuncTbl, pEv, eEvent, u32evarg);
}

////////////////////////////////////////
// ハードウェア割り込み遅延実行部コールバック関数
//
void cbToCoNet_vHwEvent(uint32 u32DeviceId, uint32 u32ItemBitmap)
{
}

////////////////////////////////////////
// 受信コールバック関数
//
void cbToCoNet_vRxEvent(tsRxDataApp *pRx)
{
	vfPrintf(&uart, "%6u,", u32TickCount_ms / 1000);
	vfPrintf(&uart, "%08x,", pRx->u32SrcAddr);
	vfPrintf(&uart, "%3u,", pRx->u8Lqi);
	vfPrintf(&uart, "%3u,", pRx->u8Seq);
	vfPrintf(&uart, LB);

	LedOn = 500 / 4;
}

////////////////////////////////////////
// メインループコールバック関数
//
void cbToCoNet_vMain()
{
}

////////////////////////////////////////////////////////////////////////////////
