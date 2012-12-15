#include <Arduino.h>
#include <SPI.h> // Arduino SPI Library

void initCAN()
{
  // Set up SPI Communication
  // dataMode can be SPI_MODE0 or SPI_MODE3 only for MCP2515
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.begin();

  // Initialise MCP2515 CAN controller at the specified speed and clock frequency
  // In this case 125kbps with a 16MHz oscillator
  // (Note:  This is the oscillator attached to the MCP2515, not the Arduino oscillaltor)
  int baudRate=CAN.Init(125,20);
  if(baudRate>0)
  {
    Serial.println("MCP2515 Init OK ...");
    Serial.print("RXStatus: ");
    Serial.println(CAN.RXStatus(), 2);
    Serial.print("Baud Rate (kbps): ");
    Serial.println(baudRate,DEC);
  }
  else
    Serial.println("MCP2515 Init Failed ...");
}

// Checks if the MCP2515 has anything to tell and takes appropriate action
void receiveCANMessage()
{
   message.id = 0;

  // This implementation utilizes the MCP2515 INT pin to flag received messages or other events
  if(CAN.Interrupt())
  {
    // determine which interrupt flags have been set
    byte interruptFlags = CAN.Read(CANINTF);
    Serial.print("Got a message. InterruptFlags=");
    Serial.println(interruptFlags);

    if(interruptFlags & RX0IF)
    {
        Serial.println("Message is on RX buffer 0");
	message = CAN.ReadBuffer(RXB0);
        processIncomingCANMessage();
    }
    if(interruptFlags & RX1IF)
    {
        Serial.println("Message is on RX buffer 1");
	message = CAN.ReadBuffer(RXB1);
        processIncomingCANMessage();
    }
    if(interruptFlags & TX0IF)
    {
        Serial.println("TX buffer 0 sent");
    }
    if(interruptFlags & TX1IF)
    {
	Serial.println("TX buffer 1 sent");
    }
    if(interruptFlags & TX2IF)
    {
	Serial.println("TX buffer 2 sent");
    }
    if(interruptFlags & ERRIF)
    {
	Serial.println("Error - ERRIF");
    }
    if(interruptFlags & MERRF)
    {
	// error handling code
	// if TXBnCTRL.TXERR set then transmission error
	// if message is lost TXBnCTRL.MLOA will be set
        Serial.println("Error - MERRF");
    }
  }
  
  CAN.Write(CANINTF, 0);
}

// If the global var "message" contains an incoming CAN message, this function can be called to process it.
void processIncomingCANMessage()
{
  // Print message
  Serial.print("ID: ");
  Serial.println(message.id,HEX);
  Serial.print("Extended: ");
  if (message.ide)
	Serial.println("Yes");
  else
	Serial.println("No");
  Serial.print("DLC: ");
  Serial.println(message.dlc,DEC);
  for(int i=0; i<message.dlc; i++)
  {
	Serial.print(message.data[i],HEX);
	Serial.print(" ");
  }
  Serial.println();
  message2Eth();
}

// Sends the message stored in the global var "message", and notifies the shacan-Server
void sendCANMessage()
{
  Serial.print("Sending CAN message with id=");
  Serial.println(message.id);
  byte bufferToUse = -1;
  
  while(true){
    byte canStatus = CAN.Status();
    if (!(canStatus & (1<<2)))
      bufferToUse = TXB0;
    else if (!(canStatus & (1<<4)))
      bufferToUse = TXB1;
    else if (!(canStatus & (1<<6)))
      bufferToUse = TXB2;
    
    if (bufferToUse == -1)
    {
      Serial.println("All buffers full. Will retry.");
      delay(1);
    }
    else
      break;
  }

  CAN.LoadBuffer(bufferToUse, message);
  CAN.SendBuffer(bufferToUse);
  message2Eth();
}
