// CAN In The Middle (CITM) 02 Dual CAN - BXProject 7th July 2019
// Based on the work of Cory J. Fowler, January 31st 2014
// Set to L320 (#15)

//--------------------------------------------- // Define function(s) to use, remove comments to enable function(s)
#define SerMon                                  // Turn on Serial functions (used to send data to PC)
// #define msgfilter                            // Turn on Message Filters, we can use these to stop messages being passed from Ch0 to Ch1

//--------------------------------------------- // Setup libraries
#include <mcp_can_dfs.h>                        // Include the required libraries from the MCP_CAN Library Master
#include <mcp_can.h>
#include <SPI.h>                                // Include the SPI library so the CANBus shield is accessible

//--------------------------------------------- // Setup some parameters 
// Define Global Values
unsigned long rxId;
unsigned long rxId_Filter;
unsigned char flagRecv = 0;
unsigned char buf[8];
char str[20];

byte len;
byte rxBuf[8];

//byte txBuf0[] = {AA,55,AA,55,AA,55,AA,55};
//byte txBuf1[] = {55,AA,55,AA,55,AA,55,AA};

MCP_CAN CAN0(9);                               // CAN0 interface usins CS on digital pin 10
MCP_CAN CAN1(8);                               // CAN1 interface using CS on digital pin 9

#define CAN0_INT 2                              // Set INT to pin 2
#define CAN1_INT 3                              // Set INT to pin 2

//--------------------------------------------- // Setup the hardware
void setup()
{
#ifdef SerMon
  Serial.begin(115200);                         // open up the serial port if the option is turned on
#endif
                                                // init CAN0 bus, baudrate: 250k@16MHz
  if(CAN0.begin(MCP_ANY, CAN_125KBPS, MCP_8MHZ) == CAN_OK){ //MCP_EXT Vs MCP_ANY?
#ifdef SerMon
	  Serial.print("CAN0: Init OK!\r\n");
#endif
  CAN0.setMode(MCP_NORMAL);
  }
  else {
#ifdef SerMon
	  Serial.print("CAN0: Init Fail!!!\r\n");
#endif
  }
                                                // init CAN1 bus, baudrate: 250k@16MHz
  if(CAN1.begin(MCP_ANY, CAN_125KBPS, MCP_8MHZ) == CAN_OK){
#ifdef SerMon
  Serial.print("CAN1: Init OK!\r\n");
#endif
  CAN1.setMode(MCP_NORMAL);
  }
  else {
#ifdef SerMon
	  Serial.print("CAN1: Init Fail!!!\r\n");
#endif
  }
#ifdef SerMon
 // Serial.end();
#endif

  SPI.setClockDivider(SPI_CLOCK_DIV2);          // Set SPI to run at 8MHz (16MHz / 2 = 8 MHz)

  //CAN0.sendMsgBuf(0x1000000, 1, 8, txBuf0);
  //CAN1.sendMsgBuf(0x1000001, 1, 8, txBuf0);
  rxId_Filter = 0x400;

 #ifdef msgfilter                               //setup message filters if turned on

  // set mask, set both the mask to 0x3ff
  CAN0.init_Mask(0, 0, 0x3ff);                  // there are 2 mask in mcp2515, you need to set both of them
  CAN0.init_Mask(1, 0, 0x3ff);
  // set filter, we can receive id from 0x04 ~ 0x09
  CAN0.init_Filt(0, 0, 0x04);                          // there are 6 filter in mcp2515
  CAN0.init_Filt(1, 0, 0x05);                          // there are 6 filter in mcp2515
  CAN0.init_Filt(2, 0, 0x06);                          // there are 6 filter in mcp2515
  CAN0.init_Filt(3, 0, 0x07);                          // there are 6 filter in mcp2515
  CAN0.init_Filt(4, 0, 0x08);                          // there are 6 filter in mcp2515
  CAN0.init_Filt(5, 0, 0x09);                          // there are 6 filter in mcp2515
#endif
}

//--------------------------------------------- // Run the main program
void loop(){
	if (!digitalRead(CAN0_INT)) {                 // If pin 2 is low, read CAN0 receive buffer
		CAN0.readMsgBuf(&rxId, &len, rxBuf);        // Read data: len = data length, buf = data byte(s)
		if (rxId != rxId_Filter) {
			CAN1.sendMsgBuf(rxId, 0, len, rxBuf);      // Immediately send message out CAN1 interface if not a filtered value
		}
		else
		{
			//Serial.print("Hash 400 \t");
			//		for (int i = 0; i<8; i++)            // print the data
			//		{
			//			Serial.print("0x");
			//			Serial.print(rxBuf[i], HEX);
			//			Serial.print("\t");
			//		}
			//Serial.print("\r\n");

			if (rxBuf[0] == 0x1)
			{
			#ifdef SerMon                             // Send serial data to PC is SerialMon turned on in config section
				Serial.print(rxId);                     // Print to serial port the ID of the received message
				Serial.print("\t ID 400 Buf 1 \t");     

				for (int i = 0; i<8; i++)               // print the data one byte at a time
				{
					Serial.print("0x");
					Serial.print(rxBuf[i], HEX);
					Serial.print("\t");
				}
				Serial.print("\r\n");
			#endif

//--------------------------------------------- // Modify LR CCF parameter to change the vehicle model (to L320)    
				//<CAN ID><DLC><DataBytes>
				//0x400 8		01		0B			02		02		01		B0		02		0A
				//						Veh.Type	Doors	Trans	Not_Bio	Engine	Fuel	Alternator
				byte data[8] = { rxBuf[0], 0x14, rxBuf[2], rxBuf[3], rxBuf[4], rxBuf[5], rxBuf[6], rxBuf[7] };
				CAN1.sendMsgBuf(rxId, 0, len, data );
			}
			else
			{
				CAN1.sendMsgBuf(rxId, 0, len, rxBuf);
			}
		}
	}
//--------------------------------------------- // Send all data from CAN1 to CAN0   
  if(!digitalRead(CAN1_INT)){                   // If pin 3 is low, read CAN1 receive buffer
    CAN1.readMsgBuf(&rxId, &len, rxBuf);        // Read data: len = data length, buf = data byte(s)
    CAN0.sendMsgBuf(rxId, 0, len, rxBuf);       // Immediately send message out CAN0 interface
  }
}
