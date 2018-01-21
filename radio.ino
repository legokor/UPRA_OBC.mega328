void serialFlush(){
  while(_Serial.available() > 0) {
    char t = _Serial.read();
  }
  for(int i=0; i<30; i++)
  {
    bus_msg[i]=0;
  }
} 

void parseRadioHK()
{
  int i, j, k, IntegerPart;


 
  // $TCHKD,0123,336*47
  //        temp  vcc
  //           1   2
 
  IntegerPart = 1;

 
  for (i=0, j=0, k=0; (i<MSGindex) && (j<10); i++) // We start at 7 so we ignore the '$GPGGA,'
  {
    if (bus_msg[i] == ',')
    {
      j++;    // Segment index
      k=0;    // Index into target variable
      IntegerPart = 1;
    }
    else
    {
      if (j == 1)
      {
        if (((bus_msg[i] >= '0') && (bus_msg[i] <= '9')) ||(bus_msg[i] == '-') )
        {        
          radio_temp[k] = bus_msg[i];
          k++;
        }
      }
      else if (j == 2)
      {
        k++;
      }
    }
    
   // GPS_Altitude = Altitude;
  }
  radio_temp[3] ='\0';  

}

int parseRequestHandshake(void)
{
 
}

int parseEOTHandshake(void)
{

}


int GetRadioHousekeeping(void)
{
  int inByte;
  int i=0;
  int msg_index=0;
  int msg_code = 0;
  int timer=0;
  int nowtime=0;
  int getmsg=10;
  
  _Serial.listen();  
  _Serial.println(F("OBC: Get Radio Housekeeping"));  
  delay(100);
  _Serial.println(F("$TMHKR,C,,,,*47"));

  timer=millis();
  //_Serial.println(timer);

  _Serial.listen();  
  serialFlush();
  while(getmsg !=0)
  {
    getmsg=GetBusMSG();
    
    nowtime=millis();
    if(nowtime - timer >4000)
    {
      _Serial.println(F("OBC: COM HK timeout"));
      break;
    }
    
        
   // _Serial.print(".");
  }
  
}

int SendRadioTelemetry(void)
{
 
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

void lowSpeedTelemetry(void)
{
      unsigned long timer=0;
      long  GPS_Alt_tmp;
      GetRadioHousekeeping();
      _Serial.print(F("OBC: COM temp: "));
      _Serial.println(radio_temp);
//      Serial.begin(57600);
      _Serial.listen(); 
      getMeasurements();
      
      delay(500);
//      Serial.println("");
//      debugLOG();
      

        GPS_Alt_tmp = GPS_Altitude;
        delay(200);
        _Serial.print(F("$TMLTM,"));
        _Serial.print(GPS_time);
        _Serial.print(F(","));
        _Serial.print(GPS_lati);
        _Serial.print(F(","));
        _Serial.print(GPS_long);
        _Serial.print(F(","));
        //_Serial.print(GPS_Alt_ch);
        _Serial.printf("%05ld",GPS_Alt_tmp);
        _Serial.print(F(","));
        _Serial.printf("%04d",(int)(ext_temp*10.0));
        _Serial.print(F(","));
        _Serial.printf("%04d",(int)(int_temp*10.0));
        _Serial.println(F("*47"));    
        
}

/*void lowSpeedStartup(void)
{
      unsigned long timer=0;
     
//      _Serial.begin(57600); 
      _Serial.listen();
      GetRadioHousekeeping();
      
      
      delay(1000);
//      _Serial.println("");
//      debugLOG();
      
      if ( SendRadioTelemetry() != 0)
      {
        _Serial.print("##");
        _Serial.print(callsign);
        _Serial.print(",MATeF-STARTUP------------------------------------------");   

        timer=millis();       
        while( GetEOTHandshake() == 0)
        {
            if( millis()-timer > 6000)
           {
              break;
           }   
        }
      }
      _Serial.println("");    
}*/
