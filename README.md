# sark110_dll
Windows DLL client for the SARK-110 Antenna Analyzer

About
-----
Complete source code of the Windows DLL client for the [SARK-110](www.sark110.com) Antenna Analyzer. This DLL is used by SARK Plots client software for communicating with the SARK-110 and it can be used for developing your own client.

The client supports the following communication interfaces:
- USB custom HID 
- Bluetooth LE (future device)
- Network connection with a server; see [sark110-srv-linux](https://github.com/EA4FRB/sark110-srv-linux)

Pre-requisites
--------------
- Microsoft Visual Studio Express 2010

Usage
-----
A basic C++ project for demonstrating the DLL usage is available in SARK110_DLL_Call_Demo folder.

API
-----
```C++
/**
  * @brief Connects to the SARK-110 device
  *
  * @param  itfz        interface with the sark
  * @param  maxDev	maximum number of devices to detect (HID only)
  * @param  serverAddr	server address (sockets only)
  * @retval
  *			@li >=1: 	number of devices detected
  *			@li -1: 	device not detected
  */
extern int Sark_Connect (int16 itfz, int16 maxDev, char *serverAddr);

/**
  * @brief Close connection with the device
  *
  * @param  num		device number (starting by zero)
  * @retval
  *			@li 1: Ok
  */
extern int Sark_Close (int16 num);

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
int Sark_Version (int16 num, uint16 *pu16Ver, uint8 *pu8FW);

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
extern int Sark_Meas_Rx (int16 num, uint32 u32Freq, bool bCal, uint8 u8Samples, float *pfR, float *pfX, float *pfS21re, float *pfS21im);

/**
  * @brief Measure R and X - Four readings in single command
  *
  * @param  num			device number (starting by zero)
  * @param  u32Freq		frequency
  * @param  u32Step		step
  * @param  bCal		{TRUE: OSL calibrated measurement; FALSE: not calibrated}
  * @param  u8Samples		Number of samples to average
  * @param  pfR			return R (real Z)
  * @param  pfX			return X (imag Z)
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
extern int Sark_Meas_Rx_Eff (int16 num, uint32 u32Freq, uint32 u32Step, bool bCal, uint8 u8Samples, 
	float *pfR1, float *pfX1, 
	float *pfR2, float *pfX2, 
	float *pfR3, float *pfX3, 
	float *pfR4, float *pfX4
	);
	
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
extern int Sark_Meas_Vect (int16 num, uint32 u32Freq, float *pfMagV, float *pfPhV, float *pfMagI, float *pfPhI );

/**
  * @brief Measure RF (main generator disabled)
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
extern int Sark_Meas_RF (int16 num, uint32 u32Freq, float *pfMagV, float *pfPhV, float *pfMagI, float *pfPhI );

/**
  * @brief Signal generator
  *
  * @param  num			device number (starting by zero)
  * @param  u32Freq		frequency
  * @param  u16Level		output level
  * @param  u8Gain		gain multiplier
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
extern int Sark_Signal_Gen (int16 num, uint32 u32Freq, uint16 u16Level, uint8 u8Gain);

/**
  * @brief Battery status
  *
  * @param  num			device number (starting by zero)
  * @param  pu8Vbus		USB vbus value
  * @param  pu16Volt	 	Battery voltage
  * @param  pu8Chr		Charger status
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
extern int Sark_BatteryStatus (int16 num, uint8 *pu8Vbus, uint16 *pu16Volt, uint8 *pu8Chr);

/**
  * @brief Get button press
  *
  * @param  num		device number (starting by zero)
  * @param  pu8Key
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
extern int Sark_GetKey (int16 num, uint8 *pu8Key);

/**
  * @brief Resets device
  *
  * @param  num		device number (starting by zero)
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
extern int Sark_Device_Reset (int16 num);

/**
  * @brief Get disk information
  *
  * @param  num		device number (starting by zero)
  * @param  pu32Tot
  * @param  pu32Fre
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
extern int Sark_DiskInfo (int16 num, uint32 *pu32Tot, uint32 *pu32Fre);

/**
  * @brief Get disk volume name
  *
  * @param  num			device number (starting by zero)
  * @param  pu8Volume	volume name
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
extern int Sark_DiskVolume (int16 num, uint8 *pu8Volume);

/**
  * @brief Sounds the buzzer
  *
  * @param  num		device number (starting by zero)
  * @param  u16Freq		frequency
  * @param  u16Duration
  * @retval None
  *			@li 1: Ok
  *			@li -1: comm error
  *			@li -2: device answered error
  */
extern int Sark_Buzzer (int16 num, uint16 u16Freq, uint16 u16Duration);
```

.NET Applications
-----------------
Following code snippets show the SARK110_DLL invoke functions from .NET programs.

```C#
using System.Runtime.InteropServices;     // DLL support

namespace xxx
{
    class yyy
    {
	..
	[DllImport("SARK110_DLL.dll", CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_Connect(Int16 itfz, Int16 num, String serverAddr);

	[DllImport("SARK110_DLL.dll", CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_Close(Int16 num);

	[DllImport("SARK110_DLL.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_Version(Int16 num, out UInt16 ver, StringBuilder strFw);

	[DllImport("SARK110_DLL.dll", CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_Meas_Rx(Int16 num, UInt32 u32Freq, byte bCal, byte u8Samples, out float pfR, out float pfX, out float pfS21re, out float pfS21im);
	[DllImport("SARK110_DLL.dll", CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_Meas_Rx_Eff(Int16 num, UInt32 u32Freq, UInt32 u32Step, byte bCal, byte u8Samples,
	    out float pfR1, out float pfX1,
	    out float pfR2, out float pfX2,
	    out float pfR3, out float pfX3,
	    out float pfR4, out float pfX4
	    );

	[DllImport("SARK110_DLL.dll", CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_Meas_Vect(Int16 num, UInt32 u32Freq, out float pfV, out float pfPhV, out float pfI, out float pfPhI);

	[DllImport("SARK110_DLL.dll", CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_Meas_Vect_Thru(Int16 num, UInt32 u32Freq, out float pfVout, out float pfPhVout, out float pfVin, out float pfPhVin);

	[DllImport("SARK110_DLL.dll", CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_Battery_Status(Int16 num, out byte pbVbus, out UInt16 pu16Volt, out byte pbChr);

	[DllImport("SARK110_DLL.dll", CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_GetKey(Int16 num, out byte pu8Key);

	[DllImport("SARK110_DLL.dll", CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_DiskInfo(Int16 num, out UInt32 u32Tot, out UInt32 u32Fre);

	[DllImport("SARK110_DLL.dll", CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_Buzzer(Int16 num, UInt16 u16Freq, UInt16 u16Duration);

	[DllImport("SARK110_DLL.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
	public static extern int SARK110_DiskVolume(Int16 num, StringBuilder strVolume);
	...
    }
}
```
