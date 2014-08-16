/*
 Copyright (C) 2012 - 2014 Nils Weiss, Patrick Bruenn.

 This file is part of Wifly_Light.

 Wifly_Light is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Wifly_Light is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Wifly_Light.  If not, see <http://www.gnu.org/licenses/>. */


//*****************************************************************************
//
//! \addtogroup wylight
//! @{
//
//*****************************************************************************

#include <stdint.h>

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "utils.h"

//Free_rtos/ti-rtos includes
#include "osi.h"
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

//Common interface includes
#include "network_if.h"

//Application Includes
#define extern
#include "broadcast.h"
#undef extern

//*****************************************************************************
//
//! \addtogroup serial_wifi
//! @{
//
//*****************************************************************************

//
// GLOBAL VARIABLES -- Start
//
extern unsigned long g_ulStatus;

struct __attribute__((__packed__)) BroadcastMessage {
	uint8_t MAC[6];
	uint8_t channel;
	uint8_t rssi;
	uint16_t port;
	uint32_t rtc;
	uint16_t battery;
	uint16_t gpio;
	uint8_t asciiTime[14];
	uint8_t version[28];
	uint8_t deviceId[32];
	uint16_t boottime;
	uint8_t sensors[16];

};

static struct BroadcastMessage mBroadcastMessage;
//
// GLOBAL VARIABLES -- End
//

//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
void Broadcast_Task(void *pvParameters);

//*****************************************************************************
//
//! Broadcast_Task
//!
//!  \param  pvParameters
//!
//!  \return none
//!
//!  \brief Task handler function to handle the Broadcast Messages
//
//*****************************************************************************

void Broadcast_Task(void *pvParameters) {

	memset(&mBroadcastMessage, 0, sizeof(struct BroadcastMessage));

	// Inital Wait to give the main Task time to establish the wifi connection
	osi_Sleep(5000);

	while (!IS_CONNECTED(g_ulStatus)) {
		osi_Sleep(500);
	}

	// Get MAC-Address for Broadcast Message
	unsigned char macAddressLen = SL_MAC_ADDR_LEN;
	sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &macAddressLen, (unsigned char *) &(mBroadcastMessage.MAC));

	// Set Client Port
	mBroadcastMessage.port = htons(2000);

	// Set Device ID
	memset(&(mBroadcastMessage.deviceId), 0, sizeof(mBroadcastMessage.deviceId));
	const char tempDeviceId[] = "WyLightCC3200";
	mem_copy(&(mBroadcastMessage.deviceId), (void *) tempDeviceId, sizeof(mBroadcastMessage.deviceId));

	// Set Version
	const char tempVersion[] = "wifly-EZX Ver 4.00.1, Apr 19";
	mem_copy(&(mBroadcastMessage.version), (void *) tempVersion, sizeof(mBroadcastMessage.version));

	while (1) {

		while (!IS_CONNECTED(g_ulStatus)) {
			osi_Sleep(500);
		}

		SlSockAddrIn_t sAddr;
		int iAddrSize;
		int iSockID;
		int iStatus;

		//filling the UDP server socket address
		sAddr.sin_family = SL_AF_INET;
		sAddr.sin_port = sl_Htons((unsigned short) PORT_NUM);
		sAddr.sin_addr.s_addr = sl_Htonl((unsigned int) IP_ADDR);

		iAddrSize = sizeof(SlSockAddrIn_t);

		// creating a UDP socket
		iSockID = sl_Socket(SL_AF_INET, SL_SOCK_DGRAM, 0);
		if (iSockID < 0) {
			// error
			UART_PRINT("ERROR: Couldn't aquire socket for Broadcast transmit\r\n");
			LOOP_FOREVER(__LINE__);
		}

		do {

			SlGetRxStatResponse_t wlanStatistics;

			if (sl_WlanRxStatGet(&wlanStatistics, 0) != 0) {
				UART_PRINT("ERROR: Couldn't get wlan statistics\r\n");
				break;
			}

			mBroadcastMessage.rssi = (uint8_t) wlanStatistics.AvarageMgMntRssi;
			mBroadcastMessage.rtc = htonl(wlanStatistics.GetTimeStamp);

			// Send Broadcast Message
			iStatus = sl_SendTo(iSockID, &mBroadcastMessage, sizeof(struct BroadcastMessage), 0, (SlSockAddr_t *) &sAddr, iAddrSize);

			if (iStatus <= 0) {
				UART_PRINT("ERROR: Failure during Broadcast transmit\r\n");
				break;
			}
			osi_Sleep(1500);
		} while (IS_CONNECTED(g_ulStatus) && iStatus > 0);

		// Close socket in case of any error's and try to open a new socket in the next loop
		sl_Close(iSockID);
	}
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
