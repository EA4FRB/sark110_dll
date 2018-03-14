/**
  ******************************************************************************
  * @file    sock_cli.cpp
  * @author  Melchor Varela - EA4FRB
  * @version V1.0
  * @date    13-March-2018
  * @brief   SARK110 DLL - TCP Client sockets
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
#include <stdio.h>
#include "sock_cli.h"
#include "sark_cmd_defs.h"

#pragma comment (lib,"ws2_32.lib") 		// Winsock Library
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define NUM_RETRIES			5
//#define SERVER 				"192.168.1.66"
#define PORT 				8888
#define TIMEOUT_CONNECT		3000	/* in ms */

/* Private macro -------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static SOCKET ConnectSocket = INVALID_SOCKET;

/* Private function prototypes -----------------------------------------------*/
static void CALLBACK TimerProc(PVOID lpParameter, BOOLEAN TimerOrWaitFired);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief Open connection
  *
  * @param
  * @retval
  *			@li 1: 	   	Ok
  *			@li <0: 	Error
  */
int Sock_Connect (char* serverAddr)
{
    WSADATA wsa;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;

    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
		return -1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
	char portStr[6];
    sprintf(portStr, "%u",PORT);
    iResult = getaddrinfo(serverAddr, portStr, &hints, &result);
    if ( iResult != 0 ) {
        WSACleanup();
        return -1;
    }
    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
		{
            WSACleanup();
            return -1;
        }

        /*
		* Connect to server.
		*/
		// connect is blocking, so create a timeout timer with shorter wait time
		HANDLE timer_handle;
		CreateTimerQueueTimer(&timer_handle, NULL, TimerProc, NULL, TIMEOUT_CONNECT, 0, WT_EXECUTEDEFAULT);

		iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

		DeleteTimerQueueTimer(NULL, timer_handle, NULL);

        if (iResult == SOCKET_ERROR)
		{
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
	{
        WSACleanup();
        return -2;
    }

	return 1;
}

/**
  * @brief Close connection
  *
  * @retval
  *			@li 1: Ok
  */
int Sock_Close (void)
{
	int iResult;
	uint8 tx[SARKCMD_TX_SIZE];
	uint8 rx[SARKCMD_RX_SIZE];

	/* Inform server about disconnection */
	memset(tx, 0xff, SARKCMD_TX_SIZE);
	Sock_SendReceive( tx, rx);

	iResult = shutdown(ConnectSocket, SD_SEND);
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

	return 1;
}

/**
  * @brief Send receive
  *
  * @param  None
  * @retval
  *			@li 1: Ok
  *			@li -1: error
  */
int Sock_SendReceive (uint8 *tx, uint8 *rx)
{
	int iResult;

	iResult = send( ConnectSocket, (const char*)tx, SARKCMD_TX_SIZE, 0 );
    if (iResult == SOCKET_ERROR) {
        return -1;
    }
	memset(rx, 0, SARKCMD_RX_SIZE);
	iResult = recv(ConnectSocket, (char*)rx, SARKCMD_RX_SIZE, 0);
	if (iResult <= 0)
		return -2;
	return 1;
}

/**
  * @brief Connect timer timeout callback
  *
  */
static void CALLBACK TimerProc(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	closesocket(ConnectSocket);
}

/**
  * @}
  */

/************* (C) COPYRIGHT 2011-2018 Melchor Varela - EA4FRB *****END OF FILE****/


