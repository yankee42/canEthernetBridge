#include "MCP2515.h"
#include <Ethernet.h>
#include <EthernetUdp.h>

// Pin definitions specific to how the MCP2515 is wired up.
#define CS_PIN 7
#define INT_PIN 6
#define PIN_RESET 2

#define COMMAND_BUFFER_SIZE 64
#define COMMAND_MAX_SEGMENTS 16

#define ETHERNET_LISTEN_PORT 1543
#define MIN_ETH2CAN_SEGMENT_SIZE 11
#define SHACAN_SERVER_PORT 1543

// Ethernet stuff
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x0D };
IPAddress ip(10, 42, 5, 1);
EthernetUDP Udp;
IPAddress shacanServerIP(10, 42, 0, 207);

// Create CAN object with pins as defined
MCP2515 CAN(CS_PIN, INT_PIN);

void setup()
{
  pinMode(PIN_RESET, OUTPUT);
  digitalWrite(PIN_RESET, HIGH);
  initSerial();
  //initPowerMonitor();
  initCAN();
  initEthernet();
  
  Serial.println("Ready.");
}

// CAN message frame (actually just the parts that are exposed by the MCP2515 RX/TX buffers)
Frame message;

void loop()
{
  receiveCANMessage();
  receiveUDPSegment();
  //powerMonitorCheck();
}
