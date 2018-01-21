int ProcessBUSmsg(void)
{
  if ((bus_msg[1] == 'T') && (bus_msg[2] == 'C') && (bus_msg[3] == 'H') && (bus_msg[4] == 'K') && (bus_msg[5] == 'D'))
  {
    parseRadioHK();
//    CheckGPSFix();
    return 0;
  }
  if ((bus_msg[1] == 'C') && (bus_msg[2] == 'M') && (bus_msg[3] == 'D') && (bus_msg[4] == 'T') && (bus_msg[5] == 'A'))
  {
    return processCMDTAmsg();
  }
  return 1;  
}

int GetBusMSG(void)
{
  char inByte;
  int error=10;
  
 // _Serial.listen(); 
  while (_Serial.available() > 0)
  {
    inByte = _Serial.read();
 
    if ((inByte =='$') || (MSGindex >= 30))
    {
      MSGindex = 0;
    }
 
    if (inByte != '\r')
    {
      bus_msg[MSGindex++] = inByte;
    }
 
    if (inByte == '\n')
    {
    //  _Serial.println("endline");
      error=ProcessBUSmsg();
      MSGindex = 0;
    }
  }
  return error;  
}
