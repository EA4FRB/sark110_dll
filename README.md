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

.NET Applications
-----------------
Following code snippets show the SARK110_DLL invoke functions from .NET programs.

using System.Runtime.InteropServices;     // DLL support

namespace xxx
{
    class yyy
    {
	...
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
