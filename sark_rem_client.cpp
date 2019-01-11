/**
  ******************************************************************************
  * @file    sark_rem_client.c
  * @author  Melchor Varela - EA4FRB
  * @version V1.0
  * @date    13-March-2018
  * @brief   SARK110 DLL - Commands processing
  ******************************************************************************
  * @copy
  *
  *  This file is a part of the "SARK110 Antenna Vector Impedance Analyzer" software
  *
  *  "SARK110 Antenna Vector Impedance Analyzer software" is free software: you can redistribute it
  *  and/or modify it under the terms of the GNU General Public License as
  *  published by the Free Software Foundation, either version 3 of the License,
  *  or (at your option) any later version.
  *
  *  "SARK110 Antenna Vector Impedance Analyzer" software is distributed in the hope that it will be
  *  useful,  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with "SARK110 Antenna Vector Impedance Analyzer" software.  If not,
  *  see <http://www.gnu.org/licenses/>.
  *
  * <h2><center>&copy; COPYRIGHT 2011-2018 Melchor Varela - EA4FRB </center></h2>
  *  Melchor Varela, Madrid, Spain.
  *  melchor.varela@gmail.com
  */

/** @addtogroup SARK110
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include <winsock2.h>
#include <ws2tcpip.h>
#include "hid.h"
#include "sock_cli.h"
#include "sark_cmd_defs.h"
#include "sark_rem_client.h"
#include "ble.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define HID_TX_TIMEOUT		100
#define HID_RX_TIMEOUT		220

/* Private macro -------------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static CRITICAL_SECTION txrx_mutex;
static bool bInitMutex = FALSE;
static int16 gi16Itfz = ITFZ_HID;

/* Private function prototypes -----------------------------------------------*/
static void Float2Buf (uint8 tu8Buf[4], float fVal);
static void Int2Buf (uint8 tu8Buf[4], uint32 u32Val);
static void Buf2Int (uint32 *pu32Val, uint8 tu8Buf[4]);
static void Buf2Float (float *pfVal, uint8 tu8Buf[4]);
static void Buf2Short (uint16 *pu16Val, uint8 tu8Buf[4]);
static void Short2Buf (uint8 tu8Buf[4], uint16 u16Val);
static int SendReceive (int16 num, uint8 *tx, uint8 *rx);
static uint16 Float2Half(float value);
static float Half2Float(uint16 value);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief Connects to the SARK-110 device
  *
  * @param  itfz        interface with the sark
  * @param  maxDev		maximum number of devices to detect (HID only)
  * @param  serverAddr	server address (sockets only)
  * @retval
  *			@li >=1: 	number of devices detected
  *			@li -1: 	device not detected
  */
int Sark_Connect (int16 itfz, int16 maxDev, char *serverAddr)
{
	int iRc = -1;

	if (bInitMutex == FALSE)
	{
		InitializeCriticalSection(&txrx_mutex);
		bInitMutex = TRUE;
	}

	gi16Itfz = itfz;
	if (gi16Itfz == ITFZ_SOCK)
		return Sock_Connect(serverAddr);
	else if (gi16Itfz == ITFZ_BT)
	{
#ifndef _NO_BLE_SUPPORT_
		iRc = ble_open();
#endif
	}
	else  /* HID */
	{
		iRc = rawhid_open(maxDev, 0x0483, 0x5750, 0xFFB0, 0x0300);
		if (iRc <= 0)
			return -1;
	}
	return iRc;
}

/**
  * @brief Close connection with the device
  *
  * @param  num		device number (starting by zero)
  * @retval
  *			@li 1: Ok
  */
int Sark_Close (int16 num)
{
	if (gi16Itfz == ITFZ_SOCK)
		return Sock_Close();
	else if (gi16Itfz == ITFZ_BT)
	{
#ifndef _NO_BLE_SUPPORT_
		ble_close();
#endif
	}
	else  /* HID */
	{
		rawhid_close(num);
	}
	return 1;
}

/**
  * @brief Get protocol version
  *
  * @param  num			device number (starting by zero)
  * @param  pu16Ver		protocol version
  * @param  pu8Fw		FW version
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Version (int16 num, uint16 *pu16Ver, uint8 *pu8FW)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_SARK_VERSION;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	Buf2Short(pu16Ver, &tu8Rx[1]);
	memcpy(pu8FW, &tu8Rx[3], SARKCMD_RX_SIZE-3);

	return 1;
}

/**
  * @brief Measure R and X
  *
  * @param  num			device number (starting by zero)
  * @param  u32Freq		frequency
  * @param  bCal		{TRUE: OSL calibrated measurement; FALSE: not calibrated}
  * @param  u8Samples	Number of samples to average
  * @param  pfR			return R (real Z)
  * @param  pfX			return X (imag Z)
  * @param  pfS21re		return S21 real (SARK110 MK1)
  * @param  pfS21im		return S21 imag (SARK110 MK1)
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Meas_Rx (int16 num, uint32 u32Freq, bool bCal, uint8 u8Samples, float *pfR, float *pfX, float *pfS21re, float *pfS21im)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_SARK_MEAS_RX;
	Int2Buf(&tu8Tx[1], u32Freq);
	if (bCal)
		tu8Tx[5] = PAR_SARK_CAL;
	else
		tu8Tx[5] = PAR_SARK_UNCAL;
	tu8Tx[6] = u8Samples;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	Buf2Float(pfR, &tu8Rx[1]);
	Buf2Float(pfX, &tu8Rx[5]);
	Buf2Float(pfS21re, &tu8Rx[9]);
	Buf2Float(pfS21im, &tu8Rx[13]);

	return 1;
}

/**
  * @brief Measure R and X - efficient
  *
  * @param  num			device number (starting by zero)
  * @param  u32Freq		frequency
  * @param  u32Step		step
  * @param  bCal		{TRUE: OSL calibrated measurement; FALSE: not calibrated}
  * @param  u8Samples	Number of samples to average
  * @param  pfR			return R (real Z)
  * @param  pfX			return X (imag Z)
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Meas_Rx_Eff (int16 num, uint32 u32Freq, uint32 u32Step, bool bCal, uint8 u8Samples,
	float *pfR1, float *pfX1,
	float *pfR2, float *pfX2,
	float *pfR3, float *pfX3,
	float *pfR4, float *pfX4
	)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_SARK_MEAS_RX_EFF;
	Int2Buf(&tu8Tx[1], u32Freq);
	Int2Buf(&tu8Tx[7], u32Step);
	if (bCal)
		tu8Tx[5] = PAR_SARK_CAL;
	else
		tu8Tx[5] = PAR_SARK_UNCAL;
	tu8Tx[6] = u8Samples;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}

	uint16 u16R, u16X;
	float fR, fX;
	int offset = 1;

	Buf2Short(&u16R, &tu8Rx[offset]);
	fR = Half2Float(u16R);
	Buf2Short(&u16X, &tu8Rx[offset+2]);
	fX = Half2Float(u16X);
	*pfR1 = fR;
	*pfX1 = fX;
	offset += 4;

	Buf2Short(&u16R, &tu8Rx[offset]);
	fR = Half2Float(u16R);
	Buf2Short(&u16X, &tu8Rx[offset+2]);
	fX = Half2Float(u16X);
	*pfR2 = fR;
	*pfX2 = fX;
	offset += 4;

	Buf2Short(&u16R, &tu8Rx[offset]);
	fR = Half2Float(u16R);
	Buf2Short(&u16X, &tu8Rx[offset+2]);
	fX = Half2Float(u16X);
	*pfR3 = fR;
	*pfX3 = fX;
	offset += 4;

	Buf2Short(&u16R, &tu8Rx[offset]);
	fR = Half2Float(u16R);
	Buf2Short(&u16X, &tu8Rx[offset+2]);
	fX = Half2Float(u16X);
	*pfR4 = fR;
	*pfX4 = fX;
	offset += 4;

	return 1;
}

/**
  * @brief Measure raw vector
  *
  * @param  num			device number (starting by zero)
  * @param  u32Freq		frequency
  * @param  pfMagV		magnitude voltage
  * @param  pfPhV		phase voltage
  * @param  pfMagI		magnitude current
  * @param  pfPhI		phase current
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Meas_Vect (int16 num, uint32 u32Freq, float *pfMagV, float *pfPhV, float *pfMagI, float *pfPhI )
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_SARK_MEAS_VECTOR;
	Int2Buf(&tu8Tx[1], u32Freq);
	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	Buf2Float(pfMagV, &tu8Rx[1]);
	Buf2Float(pfPhV, &tu8Rx[5]);
	Buf2Float(pfMagI, &tu8Rx[9]);
	Buf2Float(pfPhI, &tu8Rx[13]);

	return 1;
}

/**
  * @brief Measure RF
  *
  * @param  num			device number (starting by zero)
  * @param  u32Freq		frequency
  * @param  pfMagV		magnitude voltage
  * @param  pfPhV		phase voltage
  * @param  pfMagI		magnitude current
  * @param  pfPhI		phase current
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Meas_RF (int16 num, uint32 u32Freq, float *pfMagV, float *pfPhV, float *pfMagI, float *pfPhI )
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_SARK_MEAS_RF;
	Int2Buf(&tu8Tx[1], u32Freq);
	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	Buf2Float(pfMagV, &tu8Rx[1]);
	Buf2Float(pfPhV, &tu8Rx[5]);
	Buf2Float(pfMagI, &tu8Rx[9]);
	Buf2Float(pfPhI, &tu8Rx[13]);

	return 1;
}

/**
  * @brief Measure raw vector Thru (SARK110 MK1)
  *
  * @param  num			device number (starting by zero)
  * @param  u32Freq		frequency
  * @param  pfMagVout	magnitude voltage out
  * @param  pfPhVout	phase voltage out
  * @param  pfMagVin	magnitude voltage in
  * @param  pfPhVin		phase voltage in
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Meas_Vect_Thru (int16 num, uint32 u32Freq, float *pfMagVout, float *pfPhVout, float *pfMagVin, float *pfPhVin )
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_SARK_MEAS_VEC_THRU;
	Int2Buf(&tu8Tx[1], u32Freq);
	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	Buf2Float(pfMagVout, &tu8Rx[1]);
	Buf2Float(pfPhVout, &tu8Rx[5]);
	Buf2Float(pfMagVin, &tu8Rx[9]);
	Buf2Float(pfPhVin, &tu8Rx[13]);

	return 1;
}

/**
  * @brief Signal generator
  *
  * @param  num			device number (starting by zero)
  * @param  u32Freq		frequency
  * @param  u16Level	output level
  * @param  u8Gain		gain multiplier
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Signal_Gen (int16 num, uint32 u32Freq, uint16 u16Level, uint8 u8Gain)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_SARK_SIGNAL_GEN;
	Int2Buf(&tu8Tx[1], u32Freq);
	Short2Buf(&tu8Tx[5], u16Level);
	tu8Tx[7] = u8Gain;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	return 1;
}

/**
  * @brief Battery status
  *
  * @param  num			device number (starting by zero)
  * @param  pu8Vbus		USB vbus value
  * @param  pu16Volt	Battery voltage
  * @param  pu8Chr		Charger status
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_BatteryStatus (int16 num, uint8 *pu8Vbus, uint16 *pu16Volt, uint8 *pu8Chr)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_BATT_STAT;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	*pu8Vbus = tu8Rx[1];
	Buf2Short(pu16Volt, &tu8Rx[2]);
	*pu8Chr = tu8Rx[4];

	return 1;
}

/**
  * @brief Get key press
  *
  * @param  num		device number (starting by zero)
  * @param  pu8Key
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_GetKey (int16 num, uint8 *pu8Key)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_GET_KEY;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	*pu8Key = tu8Rx[1];

	return 1;
}

/**
  * @brief Resets device
  *
  * @param  num		device number (starting by zero)
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Device_Reset (int16 num)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_DEV_RST;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	return 1;
}

/**
  * @brief Disk information
  *
  * @param  num		device number (starting by zero)
  * @param  pu32Tot
  * @param  pu32Fre
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_DiskInfo (int16 num, uint32 *pu32Tot, uint32 *pu32Fre)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_DISK_INFO;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	Buf2Int(pu32Tot, &tu8Rx[1]);
	Buf2Int(pu32Fre, &tu8Rx[5]);

	return 1;
}

/**
  * @brief Disk volume
  *
  * @param  num			device number (starting by zero)
  * @param  pu8Volume	volume name
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_DiskVolume (int16 num, uint8 *pu8Volume)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_DISK_VOLUME;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	memcpy(pu8Volume, &tu8Rx[1], SARKCMD_RX_SIZE-1);

	return 1;
}

/**
  * @brief Buzzer
  *
  * @param  num		device number (starting by zero)
  * @param  u16Freq		frequency
  * @param  u16Duration
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_Buzzer (int16 num, uint16 u16Freq, uint16 u16Duration)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_BUZZER;
	Short2Buf(&tu8Tx[1], u16Freq);
	Short2Buf(&tu8Tx[3], u16Duration);

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	return 1;
}

/**
  * @brief Controls the extra GPIO ports (supported by specific board)
  *
  * @param  num		device number (starting by zero)
  * @param  u8Cmd	{GPIO_MODE, GPIO_WRITE, GPIO_READ}
  * @param  u8Port  port number
  * @param  u8In  
  *					GPIO_MODE: {GPIO_MODE_IPU, GPIO_MODE_IPD, GPIO_MODE_IN_FLOAT, GPIO_MODE_OUT_PP}
  *					GPIO_WRITE: in  {0, 1}
  * @param  pu8Out  
  *					GPIO_READ:  out {0, 1}
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_GPIO (int16 num, uint8 u8Cmd, uint8 u8Port, uint8 u8In, uint8 *pu8Out)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_GPIO;
	tu8Tx[1] = u8Cmd;
	tu8Tx[2] = u8Port;
	tu8Tx[3] = u8In;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	*pu8Out = tu8Rx[1];
	return 1;
}

/**
  * @brief Change setting
  *
  * @param  num		device number (starting by zero)
  * @param  u8Reg	{SETTING_SAMPLING, SETTING_FILTER, SETTING_RUN}
  * @param  u8Val	value to configure
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_SetSetting (int16 num, uint8 u8Reg, uint8 u8Val)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_SET_SETTING;
	tu8Tx[1] = u8Reg;
	tu8Tx[2] = u8Val;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	return 1;
}

/**
  * @brief Read setting
  *
  * @param  num		device number (starting by zero)
  * @param  u8Reg	{SETTING_SAMPLING, SETTING_FILTER, SETTING_RUN}
  * @param  pu8Val	read value
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
int Sark_GetSetting (int16 num, uint8 u8Reg, uint8 *pu8Val)
{
	uint8 tu8Rx[SARKCMD_RX_SIZE];
	uint8 tu8Tx[SARKCMD_TX_SIZE];
	int rc;

	memset(tu8Tx, 0, SARKCMD_TX_SIZE);
	tu8Tx[0] = CMD_GET_SETTING;
	tu8Tx[1] = u8Reg;

	rc = SendReceive (num, tu8Tx, tu8Rx);
	if (rc < 0)
	{
		return -1;
	}
	if (tu8Rx[0]!=ANS_SARK_OK)
	{
		return -2;
	}
	*pu8Val = tu8Rx[1];
	return 1;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Float2Buf (uint8 tu8Buf[4], float fVal)
{
	uint32 u32Val = *((uint32*)(&fVal));
	Int2Buf(tu8Buf, u32Val);
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Int2Buf (uint8 tu8Buf[4], uint32 u32Val)
{
	tu8Buf[3] = (uint8)((u32Val&0xff000000)>>24);
	tu8Buf[2] = (uint8)((u32Val&0x00ff0000)>>16);
	tu8Buf[1] = (uint8)((u32Val&0x0000ff00)>>8);
	tu8Buf[0] = (uint8)((u32Val&0x000000ff)>>0);
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Buf2Float (float *pfVal, uint8 tu8Buf[4])
{
	uint32 u32Val;
	Buf2Int(&u32Val, tu8Buf);
	*((uint32*)(pfVal)) = u32Val;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Buf2Int (uint32 *pu32Val, uint8 tu8Buf[4])
{
	uint32 u32Val;

	u32Val = tu8Buf[3] << 24;
	u32Val += tu8Buf[2] << 16;
	u32Val += tu8Buf[1] << 8;
	u32Val += tu8Buf[0] << 0;

	*pu32Val = u32Val;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Buf2Short (uint16 *pu16Val, uint8 tu8Buf[4])
{
	uint16 u16Val;

	u16Val = tu8Buf[1] << 8;
	u16Val += tu8Buf[0] << 0;

	*pu16Val = u16Val;
}

/**
  * @brief
  * @param  None
  * @retval None
  */
static void Short2Buf (uint8 tu8Buf[4], uint16 u16Val)
{
	tu8Buf[1] = (uint8)((u16Val&0xff00)>>8);
	tu8Buf[0] = (uint8)((u16Val&0x00ff)>>0);
}


/**
  * @brief Send receive
  *
  * @param  None
  * @retval
  *			@li 1: Ok
  *			@li -1: error
  */
static int SendReceive (int16 num, uint8 *tx, uint8 *rx)
{
	int i;
	int rc;

	if (gi16Itfz == ITFZ_SOCK)
		return Sock_SendReceive(tx, rx);
	else if (gi16Itfz == ITFZ_BT)
	{
#ifndef _NO_BLE_SUPPORT_

		int retryCmd;
		int retryGbl;
		int numRetry;

		if (tx[0] == CMD_SARK_VERSION)
			numRetry = 1;
		else
			numRetry = 5;

		EnterCriticalSection(&txrx_mutex);
		for (retryGbl = 0; retryGbl < numRetry; retryGbl++)
		{
			for (retryCmd=0; retryCmd < 2; retryCmd++)
			{
				rc = ble_send(tx, SARKCMD_TX_SIZE);
				if (rc < 0)
					break;
				rc = ble_recv(rx, SARKCMD_RX_SIZE);
				if (rc < 0)
					break;
				if (rx[0]==ANS_SARK_OK || rx[0]==ANS_SARK_ERR)
					break;
				else
					rc = -1;
			}
			if (rc > 0)
				break;
			ble_close();
			Sleep(50);
			ble_open();
		}
		LeaveCriticalSection(&txrx_mutex);
#endif
	}
	else  /* HID */
	{
		EnterCriticalSection(&txrx_mutex);
		for (i=0; i < 5; i++)
		{
			rc = rawhid_send(num, tx, SARKCMD_TX_SIZE, HID_TX_TIMEOUT);
			if (rc < 0)
				break;
			rc = rawhid_recv(num, rx, SARKCMD_RX_SIZE, HID_RX_TIMEOUT);
			if (rc < 0)
				break;
			if (rx[0]==ANS_SARK_OK || rx[0]==ANS_SARK_ERR)
				break;
			else
				rc = -1;
		}
		LeaveCriticalSection(&txrx_mutex);
	}

	return rc;
}

/**
  * @brief
  *
  * @param
  * @retval None
  */
union Bits
{
    float f;
    int32 si;
    uint32 ui;
};

#define C_SHIFT         13
#define C_SHIFTSIGN     16

#define C_INFN  0x7F800000  // flt32 infinity
#define C_MAXN  0x477FE000  // max flt16 normal as a flt32
#define C_MINN  0x38800000  // min flt16 normal as a flt32
#define C_SIGNN 0x80000000  // flt32 sign bit

#define C_INFC (C_INFN >> C_SHIFT)
#define C_NANN ((C_INFC + 1) << C_SHIFT)    // minimum flt16 nan as a flt32
#define C_MAXC (C_MAXN >> C_SHIFT)
#define C_MINC (C_MINN >> C_SHIFT)
#define C_SIGNC (C_SIGNN >> C_SHIFTSIGN)    // flt16 sign bit

#define C_MULN 0x52000000   // (1 << 23) / C_MINN
#define C_MULC 0x33800000   // C_MINN / (1 << (23 - C_SHIFT))

#define C_SUBC 0x003FF      // max flt32 subnormal down shifted
#define C_NORC 0x00400      // min flt32 normal down shifted

static int32 const C_MAXD = C_INFC - C_MAXC - 1;
static int32 const C_MIND = C_MINC - C_SUBC - 1;

static uint16 Float2Half(float value)
{
    union Bits v, s;
	v.f = value;
	uint32 sign = v.si & C_SIGNN;
	v.si ^= sign;
	sign >>= C_SHIFTSIGN; // logical shift
	s.si = C_MULN;
	s.si = s.f * v.f; // correct subnormals
	v.si ^= (s.si ^ v.si) & -(C_MINN > v.si);
	v.si ^= (C_INFN ^ v.si) & -((C_INFN > v.si) & (v.si > C_MAXN));
	v.si ^= (C_NANN ^ v.si) & -((C_NANN > v.si) & (v.si > C_INFN));
	v.ui >>= C_SHIFT; // logical shift
	v.si ^= ((v.si - C_MAXD) ^ v.si) & -(v.si > C_MAXC);
	v.si ^= ((v.si - C_MIND) ^ v.si) & -(v.si > C_SUBC);
	return v.ui | sign;
}

static float Half2Float(uint16 value)
{
	union Bits v;
	v.ui = value;
	int32 sign = v.si & C_SIGNC;
	v.si ^= sign;
	sign <<= C_SHIFTSIGN;
	v.si ^= ((v.si + C_MIND) ^ v.si) & -(v.si > C_SUBC);
	v.si ^= ((v.si + C_MAXD) ^ v.si) & -(v.si > C_MAXC);
	union Bits s;
	s.si = C_MULC;
	s.f *= v.si;
	int32 mask = -(C_NORC > v.si);
	v.si <<= C_SHIFT;
	v.si ^= (s.si ^ v.si) & mask;
	v.si |= sign;
	return v.f;
}

/**
  * @}
  */

/************* (C) COPYRIGHT 2011-2018 Melchor Varela - EA4FRB *****END OF FILE****/
