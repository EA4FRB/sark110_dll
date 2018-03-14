// main.c : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "sark110_dll.h"

int _tmain(int argc, _TCHAR* argv[])
{
	uint32 u32Freq;
	float fR, fX;
	float fS21re, fS21im;
	int rc;
	uint16 u16Ver;
	uint8 tu8FW[25];
	uint8 tu8Volume[25];

	rc = SARK110_Connect(0, 1, NULL);
	if (rc < 0)
	{
		printf("Cannot connect to SARK-110\n");
		return rc;
	}
	printf("SARK-110 Connected\n");

	SARK110_Version(0, &u16Ver, tu8FW);
	SARK110_DiskVolume(0, tu8Volume);
	printf("Protocol Version: %04x, Firmware Version: %s, Disk Volume: %s\n", u16Ver, tu8FW, tu8Volume);

	for (u32Freq = 1000000; u32Freq <= 30000000; u32Freq += 500000)
	{
		rc = SARK110_Meas_Rx (0, u32Freq, TRUE, 0, &fR, &fX, &fS21re, &fS21im);
		if (rc <= 0)
			return rc;
		printf("Freq:%d, R:%f, X:%f\n", u32Freq, fR, fX);
	}
	SARK110_Close(1);
	printf("** End **\n");

	return 0;
}

