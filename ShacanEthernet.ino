void initEthernet()
{
  Serial.println("Init Ethernet");
  
  Ethernet.begin(mac,ip, IPAddress(127,0,1,1), IPAddress(10,42,0,1), IPAddress(255,255,0,0));
  Udp.begin(ETHERNET_LISTEN_PORT);
}

// Checks if a UDP Segment has arrived. If yes, it processes it.
void receiveUDPSegment()
{
  int packetSize = Udp.parsePacket();
  if(packetSize)
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i =0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
        Serial.print(".");
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());
    
    if (packetSize >= MIN_ETH2CAN_SEGMENT_SIZE)
    {
      char header[4];
      Udp.read(header, 3);
      header[3] = '\0';
      if (!strncmp(header, "CAN", 3))
      {
        Udp.read(header, 4);
        char id[4];
        id[0] = header[3];
        id[1] = header[2];
        id[2] = header[1];
        id[3] = header[0];
        memcpy(&message.id, id, 4);
        
        Udp.read(&message.srr, 1);
        Udp.read(&message.rtr, 1);
        Udp.read(&message.ide, 1);
        Udp.read(&message.dlc, 1);
        Udp.read(message.data, 8);
        
        sendCANMessage();
      }
      else
      {
        Serial.print("Ignoring segment that does not start with 'CAN' Actual=");
        Serial.println(header);
      }
    }
    else
      Serial.println("Ignoring segment, because it is too small");
  }
}

void message2Eth()
{
  Udp.beginPacket(shacanServerIP, SHACAN_SERVER_PORT);
  Udp.write("can2eth $");
  uint8_t id[4];
  memcpy(id, &message.id, 4);
  Udp.write(&id[3], 1);
  Udp.write(&id[2], 1);
  Udp.write(&id[1], 1);
  Udp.write(&id[0], 1);
  Udp.write(message.srr);
  Udp.write(message.rtr);
  Udp.write(message.ide);
  Udp.write(message.dlc);
  Udp.write(message.data, message.dlc);
  Udp.endPacket();
  delay(10);
  Serial.println("CAN2ETH sent");
}
