/**
  ******************************************************************************
  * @file    sark110_dll.h
  * @author  Melchor Varela - EA4FRB
  * @version V1.0
  * @date    13-March-2018
  * @brief	 SARK110 DLL interface
  ******************************************************************************
  * @copy
  *
  *  This file is a part of the "SARK110 Antenna Vector Impedance Analyzer" software
  *
  *  "SARK110 Antenna Vector Impedance Analyzer" software is free software: you can redistribute it
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
  *  along with "SARK110 Antenna Vector Impedance Analyzer" firmware.  If not,
  *  see <http://www.gnu.org/licenses/>.
  *
  * <h2><center>&copy; COPYRIGHT 2011-2018 Melchor Varela - EA4FRB </center></h2>
  *  Melchor Varela, Madrid, Spain.
  *  melchor.varela@gmail.com
  */

/** @addtogroup SARK110
  * @{
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SARK110_DLL_H__
#define __SARK110_DLL_H__

/* Includes ------------------------------------------------------------------*/
#include "device.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
	ITFZ_HID,
	ITFZ_BT,
	ITFZ_SOCK
} T_ITFZ;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern int SARK110_Connect(int16 itfz, int16 maxDev, char *serverAddr);
extern int SARK110_Close(int16 num);
extern int SARK110_Version(int16 num, uint16 *pu16Ver, uint8 *pu8FW);
extern int SARK110_Meas_Rx(int16 num, uint32 u32Freq, bool bCal, uint8 u8Samples, float *pfR, float *pfX, float *pfS21re, float *pfS21im);
extern int SARK110_Meas_Rx_Eff (int16 num, uint32 u32Freq, uint32 u32Step, bool bCal, uint8 u8Samples,
	float *pfR1, float *pfX1,
	float *pfR2, float *pfX2,
	float *pfR3, float *pfX3,
	float *pfR4, float *pfX4
	);
extern int SARK110_Meas_Vect(int16 num, uint32 u32Freq, float *pfMagV, float *pfPhV,
										   float *pfMagI, float *pfPhI );
extern int SARK110_Meas_RF(int16 num, uint32 u32Freq, float *pfMagV, float *pfPhV,
										   float *pfMagI, float *pfPhI );
extern int SARK110_Meas_Vect_Thru(int16 num, uint32 u32Freq, float *pfMagVout, float *pfPhVout,
										   float *pfMagVin, float *pfPhVin );
extern int SARK110_Signal_Gen(int16 num, uint32 u32Freq, uint16 u16Level, uint8 u8Gain);
extern int SARK110_Battery_Status(int16 num, uint8 *pu8Vbus, uint16 *pu16Volt, uint8 *pu8Chr);
extern int SARK110_GetKey(int16 num, uint8 *pu8Key);
extern int SARK110_DiskInfo(int16 num, uint32 *pu32Tot, uint32 *pu32Fre);
extern int SARK110_DiskVolume(int16 num, uint8 *pu8Volume);
extern int SARK110_Buzzer(int16 num, uint16 u16Freq, uint16 u16Duration);
extern int SARK110_Device_Reset (int16 num);

#endif	 /* __SARK110_DLL_H__ */

/**
  * @}
  */

/************* (C) COPYRIGHT 2011-2018 Melchor Varela - EA4FRB *****END OF FILE****/

