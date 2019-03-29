void getGPSMeasurement(void)
{
  uint8_t gps_error=10;
  volatile int gps_wtchdg=0;
  int nowtime=0;

/*#ifdef NEO6_GPS  
  GPS.begin(9600);
  SetupUBLOX(void);
#elif defined(TYCO)
  GPS.begin(4800);
#elif defined(Soft_GPS)
  GPS.begin(9600);

#endif
*/
  gps_wtchdg = millis();
//  Serial.println(gps_wtchdg);
  _Serial.print("OBC: GPS read...");
  GPS.listen(); 
  while((gps_error !=0))// || (gps_wtchdg < 10000))
  {
//    Serial.print(".");
    gps_error=CheckGPS();
    //gps_wtchdg++;
    nowtime=millis();
    if(nowtime - gps_wtchdg >3000)
    {
      _Serial.print("timeout");
      break;
    }

  //  Serial.println(nowtime);
  }
  _Serial.println("...end");
  gps_wtchdg = 0;  
  if( GPS_Altitude > 1000)
  {
    is_climb = true; 
    is_landing = false;
  }
  if( (GPS_Altitude < 500) && (is_climb))
  {
    is_landing = true;
  }
  if( (GPS_Altitude > 500) && (is_landing))
  {
    is_landing = false;
  }

  UDR0=0;
  UCSR0A &= !(1<<UDRE0);
  
//  GPS.flush();
  _Serial.flush();
  serialFlush();
}

void getTemperatures(void)
{
  _Serial.println(F("OBC: COLLECT SENSOR DATA"));
  //getGPSMeasurement();
  
  ext_temp = getExtTemp();
  int_temp = getIntTemp();
}

void debugLOG(void)
{
  _Serial.print("time:      ");_Serial.println(GPS_time);
  _Serial.print("latitude:  ");_Serial.println(GPS_lati);
  _Serial.print("longitude: ");_Serial.println(GPS_long);
  _Serial.print("altitude:  ");_Serial.println(GPS_Altitude);
  _Serial.print("external:  ");_Serial.println(ext_temp);
  _Serial.print("internal:  ");_Serial.println(int_temp);
  _Serial.print("radio:  ");_Serial.println(radio_temp);
}

