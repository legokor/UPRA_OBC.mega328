uint8_t SetupUBLOX(void)
{
  uint8_t GPS_ERROR=0;
  GPS.listen();
   _delay_ms(1000);
  if( setGPS_DynamicModel6() != 0)
  {
    GPS_ERROR += 1;
  }
  //else GPS_ERROR = 0;

  return GPS_ERROR;  
}

uint8_t CheckGPS()
{
  uint8_t inByte;
  uint8_t error=10;
  while (GPS.available() > 0)
  {
    inByte = GPS.read();
 
    if ((inByte =='$') || (GPSIndex >= 80))
    {
      GPSIndex = 0;
    }
 
    if (inByte != '\r')
    {
      GPSBuffer[GPSIndex++] = inByte;
    }
 
    if (inByte == '\n')
    {
      error=ProcessGPSLine();
      GPSIndex = 0;
    }
  }
  return error;
}
uint8_t ProcessGPSLine()
{
  if ((GPSBuffer[1] == 'G') && (GPSBuffer[2] == 'P') && (GPSBuffer[3] == 'G') && (GPSBuffer[4] == 'G') && (GPSBuffer[5] == 'A'))
  {
/*    Serial.println("gga detect");
    for(int v=0;v<82;v++)
    {
      Serial.print(char(GPSBuffer[v]));
    }
    Serial.println("");*/
    ProcessGPGGACommand();
//    CheckGPSFix();
    return 0;
  }
//  Serial.println("no gga");
  return 1;
}

void ProcessGPGGACommand()
{
  int i, j, k, IntegerPart;
  unsigned int Altitude;
  int time_msec = 0;

  char _GPS_time[10] = {'1', '2', '3', '4', '5', '6', '\0'};
  char _GPS_lati[12] = {'+', '4', '8', '0', '7', '.', '0', '3', '8', '\0'};
  char _GPS_long[13] = {'+', '0', '1', '1', '3', '1', '.', '0', '0', '0', '\0'};


 
  // $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
  //                                               =====  <-- altitude in field 8
 
  IntegerPart = 1;
  GPS_Satellites = 0;
  Altitude = 0;
  GPS_Fix =0;
 
  for (i=0, j=0, k=0; (i<GPSIndex) && (j<10); i++) // We start at 7 so we ignore the '$GPGGA,'
  {
    if (GPSBuffer[i] == ',')
    {
      j++;    // Segment index
      k=0;    // Index into target variable
      IntegerPart = 1;
    }
    else
    {
      if (j == 1)
      {
        if ((GPSBuffer[i] >= '0') && (GPSBuffer[i] <= '9') && (time_msec == 0))
        {        
          _GPS_time[k] = GPSBuffer[i];
          k++;
        }
        if(GPSBuffer[i] == '.')
        {
          time_msec = 1;
        }
      }
      else if (j == 2)
      {
        k++;
        _GPS_lati[k] = GPSBuffer[i];
      }
      else if (j == 3)
      {
        _GPS_lati[0] = CheckNSEW(GPSBuffer[i]);
      }
      else if (j == 4)
      {
        k++;
        _GPS_long[k] = GPSBuffer[i];
      }
      else if (j == 5)
      {
        _GPS_long[0] = CheckNSEW(GPSBuffer[i]);
      }
      else if (j == 6)
      {
        // Fix read
        if ((GPSBuffer[i] >= '0') && (GPSBuffer[i] <= '9'))
        {
          GPS_Fix = GPS_Fix * 10;
          GPS_Fix += (unsigned int)(GPSBuffer[i] - '0');
          //GPS_Fix=1; //debug
        }
        //GPS_Fix=1; //debug
      }
      else if (j == 7)
      {
        // Satellite Count
        if ((GPSBuffer[i] >= '0') && (GPSBuffer[i] <= '9'))
        {
          GPS_Satellites = GPS_Satellites * 10;
          GPS_Satellites += (unsigned int)(GPSBuffer[i] - '0');
        }
      }
      else if (j == 9)
      {
        // Altitude
        if ((GPSBuffer[i] >= '0') && (GPSBuffer[i] <= '9') && IntegerPart)
        {
          Altitude = Altitude * 10;
          Altitude += (unsigned int)(GPSBuffer[i] - '0');
        }
        else
        {
          IntegerPart = 0;
        }
      }
    }
    
   // GPS_Altitude = Altitude;
  }
  if( GPS_Fix != 0)
  {
    GPS_Altitude = Altitude;
    sprintf(GPS_lati, "%s", _GPS_lati); 
    sprintf(GPS_long, "%s", _GPS_long); 
    sprintf(GPS_time, "%s", _GPS_time); 
    GPS_lati[11] ='\0';
    GPS_long[12] ='\0';
    GPS_time[7] ='\0';    
  }  
}


/*void CheckGPSFix()
{
  if( GPS_Fix !=0)
  {
    sprintf(_GPS_lati, "%s", GPS_lati); 
    sprintf(_GPS_long, "%s", GPS_long); 
    sprintf(_GPS_time, "%s", GPS_time); 
    _GPS_lati[11] ='\0';
    _GPS_long[12] ='\0';
    _GPS_time[7] ='\0';

  }
  else
  {
    sprintf(GPS_lati, "%s", _GPS_lati); 
    sprintf(GPS_long, "%s", _GPS_long); 
    sprintf(GPS_time, "%s", _GPS_time); 
    GPS_lati[11] ='\0';
    GPS_long[12] ='\0';
    GPS_time[7] ='\0';
  }
  
}*/

char CheckNSEW(char nsew)
{
  if(( nsew == 'N') or ( nsew == 'E'))
  {
    return '+';
  }
  else if(( nsew == 'S') or ( nsew == 'W'))
  {
    return '-';
  }
  else return 0;
}


uint8_t setGPS_DynamicModel6()
{
 int gps_set_sucess=0;
 int wtchdg=0;
 uint8_t setdm6[] = {
 0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06,
 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC };
 
 while(!gps_set_sucess)
 {
   sendUBX(setdm6, sizeof(setdm6)/sizeof(uint8_t));
   //delay(100);
   gps_set_sucess=getUBX_ACK(setdm6);
   wtchdg++;
   if( wtchdg > 3) //32700 
   {
//      _Serial.print("d_error...");
      return 1;
   }
   //delay(100);  
 }
// _Serial.print("d_ok...");
  return 0;
}


void sendUBX(uint8_t *MSG, uint8_t len) 
{
// GPS.flush();
 GPS.write(0xFF);
 _delay_ms(500);
 for(int i=0; i<len; i++) 
 {
   GPS.write(MSG[i]);
 }
}

boolean getUBX_ACK(uint8_t *MSG) 
{
 int wtchdg=0; 
 uint8_t b;
 uint8_t ackByteID = 0;
 uint8_t ackPacket[10];
 unsigned long startTime = millis();
 
// Construct the expected ACK packet
 ackPacket[0] = 0xB5; // header
 ackPacket[1] = 0x62; // header
 ackPacket[2] = 0x05; // class
 ackPacket[3] = 0x01; // id
 ackPacket[4] = 0x02; // length
 ackPacket[5] = 0x00;
 ackPacket[6] = MSG[2]; // ACK class
 ackPacket[7] = MSG[3]; // ACK id
 ackPacket[8] = 0; // CK_A
 ackPacket[9] = 0; // CK_B
 
// Calculate the checksums
 for (uint8_t ubxi=2; ubxi<8; ubxi++) 
 {   
   ackPacket[8] = ackPacket[8] + ackPacket[ubxi];
   ackPacket[9] = ackPacket[9] + ackPacket[8];
 }
 
 while (1) 
 {
// Test for success
   if (ackByteID > 9) 
   {
     // All packets in order!
     return true;
   }
   
    // Timeout if no valid response in 3 seconds
   if (millis() - startTime > 3000) 
   {
     return false;
   }
   
  // Make sure data is available to read
   if (GPS.available()) 
   {
     b = GPS.read();
   
  // Check that bytes arrive in sequence as per expected ACK packet
     if (b == ackPacket[ackByteID]) 
     {
       ackByteID++;
     }
     else 
     {
       ackByteID = 0; // Reset and look again, invalid order
     }
   }
 }
}
