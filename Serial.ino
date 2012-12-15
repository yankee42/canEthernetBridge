// Command buffer
char serialBuffer[COMMAND_BUFFER_SIZE];
char* serialCommandSegments[COMMAND_MAX_SEGMENTS];
int serialBufferPos = 0;
int serialCommandSegmentsCount = 1;

void initSerial()
{
  serialCommandSegments[0] = serialBuffer;
  Serial.begin(9600);
  Serial.println("Serial interface initialized");
}

// Reads commands from the serial interface
void serialEvent()
{
  while (Serial.available())
  {
    char c = Serial.read();
    if (c == '\r'); // always ignore \r
    else if (c == '\n')
    {
      Serial.println("endOfLine");
      if (serialBufferPos > COMMAND_BUFFER_SIZE)
      {
        Serial.print("Your command was ignored because it is to big. Maximum allowed: ");
        Serial.print(COMMAND_BUFFER_SIZE);
        Serial.print(" actual size: ");
        Serial.println(serialBufferPos);
      }
      else
      {
        serialLineEvent();
      }
      serialBufferPos = 0;
      serialCommandSegmentsCount = 1;
    }
    else if (c == ' ')
    {
      serialBuffer[serialBufferPos] = '\0';
      ++serialBufferPos;
      if (serialCommandSegmentsCount < COMMAND_MAX_SEGMENTS)
        serialCommandSegments[serialCommandSegmentsCount] = serialBuffer + serialBufferPos;
      ++serialCommandSegmentsCount;
    }
    else
    {
      if (serialBufferPos < COMMAND_BUFFER_SIZE)
        serialBuffer[serialBufferPos] = c;
      serialBufferPos++;
    }
  }
}

void serialLineEvent()
{
  if (!strcmp("canSend", serialCommandSegments[0]))
    commandCanSend(serialCommandSegments, serialCommandSegmentsCount);
  else
  {
    Serial.print("Unknown command ('");
    Serial.print(serialCommandSegments[0]);
    Serial.print("'), full command ");
    Serial.print(serialBufferPos);
    Serial.print(" chars): '");
    for(int i = 0; i < serialBufferPos; ++i)
    {
      if (serialBuffer[i] == '\0')
        Serial.print(' ');
      else
        Serial.print(serialBuffer[i]);
    }
    Serial.println("'");
  }
}

int commandCanSend(char** params, int paramCount)
{
  Serial.print("command canSend, ParamCount=");
  Serial.println(paramCount);
  if (paramCount < 6)
    return 1;

  message.id = atoi(params[1]);
  message.srr = atoi(params[2]);
  message.rtr = atoi(params[3]);
  message.ide = atoi(params[4]);
  message.dlc = atoi(params[5]);
  Serial.print("Data length=");
  Serial.println(message.dlc);
  for(int i = 0; i < message.dlc; ++i)
    message.data[i] = atoi(params[6+i]);

  Serial.print("Sending message... ");
  sendCANMessage();
  Serial.println("done.");
  return 0;
}
