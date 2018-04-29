#include <string.h>
#include <math.h>
#include "board.h"

#include "LoRaMac.h"
#include "Region.h"

#include "LoRaMacTest.h"

/*!
 * Device address on the network (big endian)
 */
static uint32_t DevAddr = 0x26011A7A;

/*!
 * AES encryption/decryption cipher network session key (MSB)
 */
static uint8_t NwkSKey[] = { 0xF3, 0x84, 0x72, 0x5D, 0x61, 0xCC, 0xD1, 0x9F, 0xA9, 0x3E, 0x59, 0x09, 0x1C, 0x8D, 0xB9, 0xB8 };

/*!
 * AES encryption/decryption cipher application session key (MSB)
 */
static uint8_t AppSKey[] = { 0x88, 0x6A, 0x5B, 0x82, 0x42, 0xEA, 0x4F, 0x06, 0xBB, 0xAF, 0x44, 0xFC, 0xF6, 0xAA, 0xC5, 0x91 };


#define LC4                { 867100000, 0, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 867300000, 0, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 867500000, 0, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 867700000, 0, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 867900000, 0, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC9                { 868800000, 0, { ( ( DR_7 << 4 ) | DR_7 ) }, 2 }
#define LC10               { 868300000, 0, { ( ( DR_6 << 4 ) | DR_6 ) }, 1 }

/*!
 * User application data buffer size
 */
#define LORAWAN_DEFAULT_DATARATE                    DR_5


/*!
 * Application port
 */
static uint8_t AppPort = 2;

/*!
 * User application data size
 */
const uint8_t AppDataSize = 9;

/*!
 * User application data
 */
static uint8_t AppData[10];

/*!
 * Indicates if a new packet can be sent
 */
volatile bool transmitting = false;

bool hasFix = false;


void dump_hex2str(uint8_t *buf, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		printf("%02X ", buf[i]);
	}
	printf("\r\n");
}

bool PrepareTxFrame() {
	double latitude, longitude, hdopGps = 0;
	int16_t altitudeGps = 0xFFFF;

	if (GpsGetLatestGpsPositionDouble(&latitude, &longitude) == SUCCESS) {
		altitudeGps = GpsGetLatestGpsAltitude(); // in m
		hdopGps = GpsGetLatestGpsHorizontalDilution();
		uint32_t lat = ((latitude + 90) / 180.0) * 16777215;
		uint32_t lon = ((longitude + 180) / 360.0) * 16777215;
		uint16_t alt = altitudeGps;
		uint8_t hdev = hdopGps * 10;

		printf("Lat: %d Lon: %d, Alt: %d\r\n", (int)lat, (int)lon, alt);

		AppData[0] = lat >> 16;
		AppData[1] = lat >> 8;
		AppData[2] = lat;

		AppData[3] = lon >> 16;
		AppData[4] = lon >> 8;
		AppData[5] = lon;

		AppData[6] = alt >> 8;
		AppData[7] = alt;

		AppData[8] = hdev;

		return true;

	} else {
		printf("No GPS fix.\r\n");

		return false;
	}
}

static int SendFrame(void) {
	McpsReq_t mcpsReq;
	int err;
	mcpsReq.Type = MCPS_UNCONFIRMED;
	mcpsReq.Req.Confirmed.fPort = AppPort;
	mcpsReq.Req.Confirmed.fBuffer = AppData;
	mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
	mcpsReq.Req.Confirmed.NbTrials = 8;
	mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;

	err = LoRaMacMcpsRequest(&mcpsReq);
	return err; // LORAMAC_STATUS_OK is zero.
}

static void McpsIndication(McpsIndication_t *mcpsIndication) {
	printf(" McpsIndication");
}

static void McpsConfirm(McpsConfirm_t *mcpsConfirm) {
	printf(" McpsConfirm");
	transmitting = false;
}

static void MlmeConfirm(MlmeConfirm_t *mlmeConfirm) {
	printf(" MlmeConfirm");
}

/**
 * Main application entry point.
 */
int main(void) {
	LoRaMacPrimitives_t LoRaMacPrimitives;
	LoRaMacCallback_t LoRaMacCallbacks;
	MibRequestConfirm_t mibReq;

	BoardInitMcu();
	BoardInitPeriph();

	printf("RAK811 BreakBoard soft version: 1.0.2\r\n");

	LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
	LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
	LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
	LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
	LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks, LORAMAC_REGION_EU868 );

	mibReq.Type = MIB_ADR;
	mibReq.Param.AdrEnable = false;
	LoRaMacMibSetRequestConfirm(&mibReq);

	mibReq.Type = MIB_PUBLIC_NETWORK;
	mibReq.Param.EnablePublicNetwork = true;
	LoRaMacMibSetRequestConfirm(&mibReq);

	printf("ABP: \r\n");
	printf("DevAddr: %016X\r\n", DevAddr);
	printf("NwkSKey: ");
	dump_hex2str(NwkSKey , 16);
	printf("AppSKey: ");
	dump_hex2str(AppSKey , 16);

	mibReq.Type = MIB_NET_ID;
	mibReq.Param.NetID = 0;
	LoRaMacMibSetRequestConfirm( &mibReq );

	mibReq.Type = MIB_DEV_ADDR;
	mibReq.Param.DevAddr = DevAddr;
	LoRaMacMibSetRequestConfirm( &mibReq );

	mibReq.Type = MIB_NWK_SKEY;
	mibReq.Param.NwkSKey = NwkSKey;
	LoRaMacMibSetRequestConfirm( &mibReq );

	mibReq.Type = MIB_APP_SKEY;
	mibReq.Param.AppSKey = AppSKey;
	LoRaMacMibSetRequestConfirm( &mibReq );

	mibReq.Type = MIB_NETWORK_JOINED;
	mibReq.Param.IsNetworkJoined = true;
	LoRaMacMibSetRequestConfirm( &mibReq );

	// Switch LED 1 ON
	GpioWrite( &Led1, 0 );

	while (1) {
		
		if (PrepareTxFrame()) {
			hasFix = true;

			printf("Waiting for tx complete");
			while (transmitting == true) {
				printf(".");
				Delay(1);
				if (hasFix) GpioWrite( &Led1,  !GpioRead(&Led1) );
			}
			printf("Tx complete\n");
			
			int err;
			err = SendFrame();
			if (err == LORAMAC_STATUS_OK) {
				// TX enqueued
				transmitting = true;
			}
			printf("SendFrame: %d\r\n", err);
		} else {
			Delay(1);
			//delay to give the gps time to get a fix
		}

		if (GpsGetPpsDetectedState() == true) {
			// Switch LED 2 ON
			// GpioWrite(&Led2, 0);
		}
		if (Lis3dhGetIntState() == true) {
			Lis3dh_IntEventClear();
			for (uint8_t index = 0; index < 6; index++) {
				LIS3DH_ReadReg(LIS3DH_OUT_X_L + index, AppData + 2 + index);
				DelayMs(1);
			}
			//printf("[Debug]: ACC X:%04X Y:%04X Z:%04X\r\n", AppData[3]<<8 | AppData[2], AppData[5]<<8 | AppData[4], AppData[7]<<8 | AppData[6]);
		}
	}
}
