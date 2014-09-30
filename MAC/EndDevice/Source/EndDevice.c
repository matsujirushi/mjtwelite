////////////////////////////////////////////////////////////////////////////////
// EndDevice.c
//
// 2014/09/30 1.0 Takashi Matsuoka
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
	E_STATE_APP_STARTUP_WARM,
	E_STATE_APP_TX,
	E_STATE_APP_SLEEP,
} teStateApp;

typedef enum
{
	E_EVENT_APP_BASE = ToCoNet_EVENT_APP_BASE,
	E_EVENT_APP_FINISH_TX_SUCCESS,
	E_EVENT_APP_FINISH_TX_ERROR,
} teEventApp;

static tsFILE uart;

static void vProcessEvCore(tsEvent *pEv, teEvent eEvent, uint32 u32evarg);

static void Reset()
{
	vfPrintf(&uart, "!Reset."LB);
	uart0_flush();
	delay_us(10e6);

	vAHI_SwReset();
}

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
		sToCoNet_AppContext.bRxOnIdle = FALSE;		// 受信要否
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
	if (!bStatus)
	{
		ToCoNet_Event_Process(E_EVENT_APP_FINISH_TX_ERROR, 0, vProcessEvCore);
	}
	else
	{
		ToCoNet_Event_Process(E_EVENT_APP_FINISH_TX_SUCCESS, 0, vProcessEvCore);
	}
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
		if (!(u32evarg & EVARG_START_UP_WAKEUP_RAMHOLD_MASK))
		{
			ToCoNet_Event_SetState(pEv, E_STATE_APP_STARTUP_COLD);
		}
		else
		{
			ToCoNet_Event_SetState(pEv, E_STATE_APP_STARTUP_WARM);
		}
	}
}

PRSEV_HANDLER_DEF(E_STATE_APP_STARTUP_COLD, tsEvent *pEv, teEvent eEvent, uint32 u32evarg)
{
	if (eEvent == E_EVENT_NEW_STATE)
	{
		vfPrintf(&uart, "#E_STATE_APP_STARTUP_COLD"LB);

		ToCoNet_vMacStart();

		ToCoNet_Event_SetState(pEv, E_STATE_APP_TX);
	}
}

PRSEV_HANDLER_DEF(E_STATE_APP_STARTUP_WARM, tsEvent *pEv, teEvent eEvent, uint32 u32evarg)
{
	if (eEvent == E_EVENT_NEW_STATE)
	{
		vfPrintf(&uart, "#E_STATE_APP_STARTUP_WARM"LB);

		ToCoNet_vMacStart();

		ToCoNet_Event_SetState(pEv, E_STATE_APP_TX);
	}
}

PRSEV_HANDLER_DEF(E_STATE_APP_TX, tsEvent *pEv, teEvent eEvent, uint32 u32evarg)
{
	if (eEvent == E_EVENT_NEW_STATE)
	{
		vfPrintf(&uart, "#E_STATE_APP_TX"LB);

		tsTxDataApp tsTx;
		zero_memory(&tsTx, sizeof (tsTx));

		tsTx.u8CbId = 0;
		tsTx.u8Retry = 0x80;

		tsTx.u8Cmd = TOCONET_PACKET_CMD_APP_DATA;
		tsTx.bAckReq = FALSE;
		tsTx.u8Seq = 0;
		tsTx.u32DstAddr = TOCONET_MAC_ADDR_BROADCAST;
		tsTx.u32SrcAddr = ToCoNet_u32GetSerial();
		tsTx.u8Len = 1;
		tsTx.auData[0] = '@';

		if (!ToCoNet_bMacTxReq(&tsTx))
		{
			vfPrintf(&uart, "!Tx."LB);
			Reset();
		}
		ToCoNet_Tx_vProcessQueue();
	}
	else if (eEvent == E_EVENT_APP_FINISH_TX_SUCCESS || eEvent == E_EVENT_APP_FINISH_TX_ERROR)
	{
		ToCoNet_Event_SetState(pEv, E_STATE_APP_SLEEP);
	}
}

PRSEV_HANDLER_DEF(E_STATE_APP_SLEEP, tsEvent *pEv, teEvent eEvent, uint32 u32evarg)
{
	if (eEvent == E_EVENT_NEW_STATE)
	{
		vfPrintf(&uart, "#E_STATE_APP_SLEEP"LB);

		uart0_flush();
		ToCoNet_vSleep(E_AHI_WAKE_TIMER_0, 2000, TRUE, FALSE);
	}
}

static const tsToCoNet_Event_StateHandler asStateFuncTbl[] =
{
	PRSEV_HANDLER_TBL_DEF(E_STATE_IDLE),
	PRSEV_HANDLER_TBL_DEF(E_STATE_APP_STARTUP_COLD),
	PRSEV_HANDLER_TBL_DEF(E_STATE_APP_STARTUP_WARM),
	PRSEV_HANDLER_TBL_DEF(E_STATE_APP_TX),
	PRSEV_HANDLER_TBL_DEF(E_STATE_APP_SLEEP),
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
}

////////////////////////////////////////
// メインループコールバック関数
//
void cbToCoNet_vMain()
{
}

////////////////////////////////////////////////////////////////////////////////
