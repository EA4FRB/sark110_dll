/**
  ******************************************************************************
  * @file    ble_windows.cpp
  * @author  Melchor Varela - EA4FRB
  * @version V1.0
  * @date    13-March-2018
  * @brief   SARK110 DLL - BLE Functions
  *          Ensure that you have paired the SARK-110 with the computer!
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

#include <stdio.h>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <bthdef.h>
#include <Bluetoothleapis.h>
#include "ble.h"

#pragma comment(lib, "SetupAPI.lib")
#pragma comment(lib, "BluetoothApis.lib")


#define TO_SEARCH_DEVICE_UUID "{49535343-fe7d-4ae5-8fa9-9fafd205e455}"
#define RECV_BUFF_SIZE	100

static HANDLE GetBLEHandle(__in GUID AGuid);
static void CALLBACK RecvHandler( BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context);

static HANDLE ghLEDevice = NULL;
static PBTH_LE_GATT_CHARACTERISTIC gGattChar;
static char gtRcvBuff[RECV_BUFF_SIZE];
static bool gIsRcvData = FALSE;

int ble_open (void)
{
	int iRc = -1;
	HANDLE hLEDevice = NULL;
	PBTH_LE_GATT_CHARACTERISTIC pCharBuffer = NULL;
	PBTH_LE_GATT_SERVICE pServiceBuffer = NULL;
	PBTH_LE_GATT_DESCRIPTOR pDescriptorBuffer = NULL;
	PBTH_LE_GATT_DESCRIPTOR_VALUE pDescValueBuffer = NULL;

	ble_close();
	do
	{
		//Step 1: find the BLE device handle from its GUID
		GUID AGuid;
		//GUID can be constructed from "{xxx....}" string using CLSID
		CLSIDFromString(TEXT(TO_SEARCH_DEVICE_UUID), &AGuid);

		//now get the handle
		hLEDevice = GetBLEHandle(AGuid);

		//Step 2: Get a list of services that the device advertises
		// first send 0,NULL as the parameters to BluetoothGATTServices inorder to get the number of
		// services in serviceBufferCount
		USHORT serviceBufferCount;
		////////////////////////////////////////////////////////////////////////////
		// Determine Services Buffer Size
		////////////////////////////////////////////////////////////////////////////
		HRESULT hr = BluetoothGATTGetServices(
			hLEDevice,
			0,
			NULL,
			&serviceBufferCount,
			BLUETOOTH_GATT_FLAG_NONE);

		if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
			printf("BluetoothGATTGetServices - Buffer Size %d", hr);
			break;
		}

		pServiceBuffer = (PBTH_LE_GATT_SERVICE)
			malloc(sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);

		if (NULL == pServiceBuffer) {
			printf("pServiceBuffer out of memory\r\n");
			break;
		} else {
			RtlZeroMemory(pServiceBuffer,
				sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);
		}

		////////////////////////////////////////////////////////////////////////////
		// Retrieve Services
		////////////////////////////////////////////////////////////////////////////
		USHORT numServices;
		hr = BluetoothGATTGetServices(
			hLEDevice,
			serviceBufferCount,
			pServiceBuffer,
			&numServices,
			BLUETOOTH_GATT_FLAG_NONE);

		if (S_OK != hr) {
			printf("BluetoothGATTGetServices - Buffer Size %d", hr);
			break;
		}


		//Step 3: now get the list of charactersitics. note how the pServiceBuffer is required from step 2
		////////////////////////////////////////////////////////////////////////////
		// Determine Characteristic Buffer Size
		////////////////////////////////////////////////////////////////////////////

		USHORT charBufferSize;
		hr = BluetoothGATTGetCharacteristics(
			hLEDevice,
			pServiceBuffer,
			0,
			NULL,
			&charBufferSize,
			BLUETOOTH_GATT_FLAG_NONE);

		if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
			printf("BluetoothGATTGetCharacteristics - Buffer Size %d", hr);
			break;
		}

		if (charBufferSize > 0) {
			pCharBuffer = (PBTH_LE_GATT_CHARACTERISTIC)
				malloc(charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));

			if (NULL == pCharBuffer) {
				printf("pCharBuffer out of memory\r\n");
				break;
			} else {
				RtlZeroMemory(pCharBuffer,
					charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));
			}

			////////////////////////////////////////////////////////////////////////////
			// Retrieve Characteristics
			////////////////////////////////////////////////////////////////////////////
			USHORT numChars;
			hr = BluetoothGATTGetCharacteristics(
				hLEDevice,
				pServiceBuffer,
				charBufferSize,
				pCharBuffer,
				&numChars,
				BLUETOOTH_GATT_FLAG_NONE);

			if (S_OK != hr) {
				printf("BluetoothGATTGetCharacteristics - Actual Data %d", hr);
				break;
			}

			if (numChars != charBufferSize) {
				printf("buffer size and buffer size actual size mismatch\r\n");
				break;
			}
		}

		//Step 4: now get the list of descriptors. note how the pCharBuffer is required from step 3
		//descriptors are required as we descriptors that are notification based will have to be written
		//once IsSubcribeToNotification set to true, we set the appropriate callback function
		//need for setting descriptors for notification according to
		//http://social.msdn.microsoft.com/Forums/en-US/11d3a7ce-182b-4190-bf9d-64fefc3328d9/windows-bluetooth-le-apis-event-callbacks?forum=wdk
		PBTH_LE_GATT_CHARACTERISTIC currGattChar;
		for (int ii=0; ii < 1 /*charBufferSize*/; ii++) {
			currGattChar = &pCharBuffer[ii];
			USHORT charValueDataSize;
			PBTH_LE_GATT_CHARACTERISTIC_VALUE pCharValueBuffer;

			///////////////////////////////////////////////////////////////////////////
			// Determine Descriptor Buffer Size
			////////////////////////////////////////////////////////////////////////////
			USHORT descriptorBufferSize;
			hr = BluetoothGATTGetDescriptors(
				hLEDevice,
				currGattChar,
				0,
				NULL,
				&descriptorBufferSize,
				BLUETOOTH_GATT_FLAG_NONE);

			if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
				printf("BluetoothGATTGetDescriptors - Buffer Size %d", hr);
				break;
			}

			if (descriptorBufferSize > 0) {
				pDescriptorBuffer = (PBTH_LE_GATT_DESCRIPTOR)
					malloc(descriptorBufferSize
					* sizeof(BTH_LE_GATT_DESCRIPTOR));

				if (NULL == pDescriptorBuffer) {
					printf("pDescriptorBuffer out of memory\r\n");
					break;
				} else {
					RtlZeroMemory(pDescriptorBuffer, descriptorBufferSize);
				}

				////////////////////////////////////////////////////////////////////////////
				// Retrieve Descriptors
				////////////////////////////////////////////////////////////////////////////

				USHORT numDescriptors;
				hr = BluetoothGATTGetDescriptors(
					hLEDevice,
					currGattChar,
					descriptorBufferSize,
					pDescriptorBuffer,
					&numDescriptors,
					BLUETOOTH_GATT_FLAG_NONE);

				if (S_OK != hr) {
					printf("BluetoothGATTGetDescriptors - Actual Data %d", hr);
					break;
				}

				if (numDescriptors != descriptorBufferSize) {
					printf("buffer size and buffer size actual size mismatch\r\n");
					break;
				}

				for(int kk=0; kk<numDescriptors; kk++) {
					PBTH_LE_GATT_DESCRIPTOR  currGattDescriptor = &pDescriptorBuffer[kk];
					////////////////////////////////////////////////////////////////////////////
					// Determine Descriptor Value Buffer Size
					////////////////////////////////////////////////////////////////////////////
					USHORT descValueDataSize;
					hr = BluetoothGATTGetDescriptorValue(
						hLEDevice,
						currGattDescriptor,
						0,
						NULL,
						&descValueDataSize,
						BLUETOOTH_GATT_FLAG_NONE);

					if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
						printf("BluetoothGATTGetDescriptorValue - Buffer Size %d", hr);
						break;
					}

					pDescValueBuffer = (PBTH_LE_GATT_DESCRIPTOR_VALUE)malloc(descValueDataSize);

					if (NULL == pDescValueBuffer) {
						printf("pDescValueBuffer out of memory\r\n");
						break;
					} else {
						RtlZeroMemory(pDescValueBuffer, descValueDataSize);
					}

					////////////////////////////////////////////////////////////////////////////
					// Retrieve the Descriptor Value
					////////////////////////////////////////////////////////////////////////////
					hr = BluetoothGATTGetDescriptorValue(
						hLEDevice,
						currGattDescriptor,
						(ULONG)descValueDataSize,
						pDescValueBuffer,
						NULL,
						BLUETOOTH_GATT_FLAG_NONE);
					if (S_OK != hr) {
						printf("BluetoothGATTGetDescriptorValue - Actual Data %d", hr);
						break;
					}
					//you may also get a descriptor that is read (and not notify) andi am guessing the attribute handle is out of limits
					// we set all descriptors that are notifiable to notify us via IsSubstcibeToNotification
					if(currGattDescriptor->AttributeHandle < 255) {
						BTH_LE_GATT_DESCRIPTOR_VALUE newValue;

						RtlZeroMemory(&newValue, sizeof(newValue));

						newValue.DescriptorType = ClientCharacteristicConfiguration;
						newValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;

						hr = BluetoothGATTSetDescriptorValue(
							hLEDevice,
							currGattDescriptor,
							&newValue,
							BLUETOOTH_GATT_FLAG_NONE);
						if (S_OK != hr) {
							printf("BluetoothGATTGetDescriptorValue - Actual Data %d", hr);
							break;
						} else {
							//printf("setting notification for service handle %d\n", currGattDescriptor->ServiceHandle);
						}
					}
				}
			}

			//set the appropriate callback function when the descriptor change value
			BLUETOOTH_GATT_EVENT_HANDLE EventHandle;

			if (currGattChar->IsNotifiable) {
				//printf("Setting Notification for ServiceHandle %d\n",currGattChar->ServiceHandle);
				BTH_LE_GATT_EVENT_TYPE EventType = CharacteristicValueChangedEvent;

				BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION EventParameterIn;
				EventParameterIn.Characteristics[0] = *currGattChar;
				EventParameterIn.NumCharacteristics = 1;

				hr = BluetoothGATTRegisterEvent(
					hLEDevice,
					EventType,
					&EventParameterIn,
					(PFNBLUETOOTH_GATT_EVENT_CALLBACK)RecvHandler,
					NULL,
					&EventHandle,
					BLUETOOTH_GATT_FLAG_NONE);

				if (S_OK != hr) {
					printf("BluetoothGATTRegisterEvent - Actual Data %d", hr);
					break;
				}
			}

			if (currGattChar->IsReadable) {//currGattChar->IsReadable
				////////////////////////////////////////////////////////////////////////////
				// Determine Characteristic Value Buffer Size
				////////////////////////////////////////////////////////////////////////////
				hr = BluetoothGATTGetCharacteristicValue(
					hLEDevice,
					currGattChar,
					0,
					NULL,
					&charValueDataSize,
					BLUETOOTH_GATT_FLAG_NONE);

				if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
					printf("BluetoothGATTGetCharacteristicValue - Buffer Size %d", hr);
					break;
				}

				pCharValueBuffer = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(charValueDataSize);

				if (NULL == pCharValueBuffer) {
					printf("pCharValueBuffer out of memory\r\n");
					break;
				} else {
					RtlZeroMemory(pCharValueBuffer, charValueDataSize);
				}

				////////////////////////////////////////////////////////////////////////////
				// Retrieve the Characteristic Value
				////////////////////////////////////////////////////////////////////////////

				hr = BluetoothGATTGetCharacteristicValue(
					hLEDevice,
					currGattChar,
					(ULONG)charValueDataSize,
					pCharValueBuffer,
					NULL,
					BLUETOOTH_GATT_FLAG_NONE);

				if (S_OK != hr) {
					printf("BluetoothGATTGetCharacteristicValue - Actual Data %d", hr);
					break;
				}

				//print the characeteristic Value
				//for an HR monitor this might be the body sensor location
				printf("\n Printing a read (not notifiable) characterstic (maybe) body sensor value");
				for(int iii=0; iii< pCharValueBuffer->DataSize; iii++) {// ideally check ->DataSize before printing
					printf("%d",pCharValueBuffer->Data[iii]);
				}
				printf("\n");


				// Free before going to next iteration, or memory leak.
				free(pCharValueBuffer);
				pCharValueBuffer = NULL;
			}
		}
		iRc = 1;
	} while (FALSE);

	if (pDescriptorBuffer)
		free(pDescriptorBuffer);
	if (pServiceBuffer)
		free(pServiceBuffer);
	if (pDescValueBuffer)
		free(pDescValueBuffer);
	if (iRc == 1)
	{
		ghLEDevice = hLEDevice;
		gGattChar = &pCharBuffer[0];
		gIsRcvData = FALSE;
		Sleep(1000);
	}
	else
	{
		if (hLEDevice != NULL)
			CloseHandle(hLEDevice);
	}
	return iRc;
}

int ble_close (void)
{
	if (ghLEDevice == NULL)
		return -1;

	CloseHandle(ghLEDevice);
	ghLEDevice = NULL;

	if (gGattChar)
		free(gGattChar);
	gGattChar = NULL;
	return 1;
}

int ble_send(void *buf, int len)
{
	HRESULT hr;

	if (ghLEDevice == NULL)
		return -1;

	gIsRcvData = FALSE;

	size_t required_size = sizeof(BTH_LE_GATT_CHARACTERISTIC_VALUE) + len;
	PBTH_LE_GATT_CHARACTERISTIC_VALUE gatt_value = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(required_size);
	if (gatt_value == NULL)
		return -2;
    ZeroMemory(gatt_value, required_size);
    gatt_value->DataSize = (ULONG)len;
    memcpy(gatt_value->Data, buf, len);
	hr = BluetoothGATTSetCharacteristicValue(ghLEDevice,
										gGattChar,
										gatt_value,
										NULL,
										BLUETOOTH_GATT_FLAG_NONE);
	free(gatt_value);
	if (S_OK != hr) {
		printf("BluetoothGATTSetCharacteristicValue - Actual Data %d", hr);
		return -2;
	}

	return 1;
}

int ble_recv(void *buf, int len)
{
	int ii;
	if (ghLEDevice == NULL)
		return -1;
	for (ii = 0; ii < 200; ii++)
	{
		if (gIsRcvData)
			break;
		Sleep(1);
	}
	if (gIsRcvData == FALSE)
		return -2;
	gIsRcvData = FALSE;
	memcpy(buf, gtRcvBuff, len);

	return 1;
}

static HANDLE GetBLEHandle(__in GUID AGuid)
{
	HDEVINFO hDI;
	SP_DEVICE_INTERFACE_DATA did;
	SP_DEVINFO_DATA dd;
	GUID BluetoothInterfaceGUID = AGuid;
	HANDLE hComm = NULL;

	hDI = SetupDiGetClassDevs(&BluetoothInterfaceGUID, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

	if( hDI == INVALID_HANDLE_VALUE ) return NULL;

	did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	dd.cbSize = sizeof(SP_DEVINFO_DATA);

	for(DWORD i = 0; SetupDiEnumDeviceInterfaces(hDI, NULL, &BluetoothInterfaceGUID, i, &did); i++)
	{
		SP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;

		DeviceInterfaceDetailData.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		DWORD size = 0;

		if(!SetupDiGetDeviceInterfaceDetail(hDI, &did, NULL, 0, &size, 0) )
		{
			int err = GetLastError();

			if( err == ERROR_NO_MORE_ITEMS ) break;

			PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR , size);

			pInterfaceDetailData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

			if( !SetupDiGetDeviceInterfaceDetail(hDI, &did, pInterfaceDetailData, size, &size, &dd) )
				break;

			hComm = CreateFile(
				pInterfaceDetailData->DevicePath,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

			GlobalFree(pInterfaceDetailData);
		}
	}

	SetupDiDestroyDeviceInfoList(hDI);
	return hComm;
}

static void CALLBACK RecvHandler( BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context)
{
	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)EventOutParameter;

	if (ValueChangedEventParameters->CharacteristicValue->DataSize) {
		if (ValueChangedEventParameters->CharacteristicValue->DataSize > RECV_BUFF_SIZE)
			ValueChangedEventParameters->CharacteristicValue->DataSize = RECV_BUFF_SIZE;
		memcpy(gtRcvBuff, ValueChangedEventParameters->CharacteristicValue->Data, ValueChangedEventParameters->CharacteristicValue->DataSize);
		gIsRcvData = TRUE;
	}
}

/**
  * @}
  */

/************* (C) COPYRIGHT 2011-2018 Melchor Varela - EA4FRB *****END OF FILE****/


