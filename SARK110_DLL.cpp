// SARKCLIENT.cpp : Defines the exported functions for the DLL application.
//
#include "sark_rem_client.h"

extern "C"
{
__declspec(dllexport) int SARK110_Connect(int16 itfz, int16 maxDev, char *serverAddr)
{
	return Sark_Connect(itfz,  maxDev, serverAddr);
}

__declspec(dllexport) int SARK110_Close(int16 num)
{
	return Sark_Close(num);
}

__declspec(dllexport) int SARK110_Version(int16 num, uint16 *pu16Ver, uint8 *pu8FW)
{
	return Sark_Version (num, pu16Ver, pu8FW);
}

__declspec(dllexport) int SARK110_Meas_Rx(int16 num, uint32 u32Freq, bool bCal, uint8 u8Samples, float *pfR, float *pfX, float *pfS21re, float *pfS21im)
{
	return Sark_Meas_Rx (num, u32Freq, bCal, u8Samples, pfR, pfX, pfS21re, pfS21im);
}

__declspec(dllexport) int SARK110_Meas_Rx_Eff(int16 num, uint32 u32Freq, uint32 u32Step, bool bCal, uint8 u8Samples, 
	float *pfR1, float *pfX1,
	float *pfR2, float *pfX2,
	float *pfR3, float *pfX3,
	float *pfR4, float *pfX4
	)
{
	return Sark_Meas_Rx_Eff (num, u32Freq, u32Step, bCal, u8Samples, 
		pfR1, pfX1,
		pfR2, pfX2,
		pfR3, pfX3,
		pfR4, pfX4
		);
}
__declspec(dllexport) int SARK110_Meas_Vect(int16 num, uint32 u32Freq, float *pfMagV, float *pfPhV,
										   float *pfMagI, float *pfPhI )
{
	return Sark_Meas_Vect (num, u32Freq, pfMagV, pfPhV, pfMagI, pfPhI);
}

__declspec(dllexport) int SARK110_Meas_Vect_Thru(int16 num, uint32 u32Freq, float *pfMagVout, float *pfPhVout,
										   float *pfMagVin, float *pfPhVin )
{
	return Sark_Meas_Vect_Thru (num, u32Freq, pfMagVout, pfPhVout, pfMagVin, pfPhVin);
}

__declspec(dllexport) int SARK110_Signal_Gen(int16 num, uint32 u32Freq, uint16 u16Level, uint8 u8Gain)
{
	return Sark_Signal_Gen (num, u32Freq, u16Level, u8Gain);
}

__declspec(dllexport) int SARK110_Battery_Status(int16 num, uint8 *pu8Vbus, uint16 *pu16Volt, uint8 *pu8Chr)
{
	return Sark_BatteryStatus (num, pu8Vbus, pu16Volt, pu8Chr);
}

__declspec(dllexport) int SARK110_GetKey(int16 num, uint8 *pu8Key)
{
	return Sark_GetKey (num, pu8Key);
}

__declspec(dllexport) int SARK110_DiskInfo(int16 num, uint32 *pu32Tot, uint32 *pu32Fre)
{
	return Sark_DiskInfo (num, pu32Tot, pu32Fre);
}

__declspec(dllexport) int SARK110_DiskVolume(int16 num, uint8 *pu8Volume)
{
	return Sark_DiskVolume (num, pu8Volume);
}

__declspec(dllexport) int SARK110_Buzzer(int16 num, uint16 u16Freq, uint16 u16Duration)
{
	return Sark_Buzzer (num, u16Freq, u16Duration);
}

__declspec(dllexport) int SARK110_Device_Reset(int16 num)
{
	return Sark_Device_Reset (num);
}

__declspec(dllexport) int SARK110_Meas_RF(int16 num, uint32 u32Freq, float *pfMagV, float *pfPhV,
										   float *pfMagI, float *pfPhI )
{
	return Sark_Meas_RF (num, u32Freq, pfMagV, pfPhV, pfMagI, pfPhI);
}

}