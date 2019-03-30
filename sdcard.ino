//header: time,latitude,longitude,altitude,ext_temp,OBC_temp,COM_temp,CAM_temp,CAM_sensorA,CAM_sensorB,CAM_sdcard,CAM_imagenum,CAM_intervalometer

int dumpLog(void)
{
 // File dataFile;
  if(card_present)
  {  
    _Serial.print(F("OBC: Store data..."));
    // if the file is available, write to it:
    if (dataFile.open("data.csv", O_RDWR | O_CREAT | O_AT_END)) 
    {
      dataFile.print(GPS_time);
      dataFile.print(F(","));
      dataFile.print(GPS_lati);
      dataFile.print(F(","));
      dataFile.print(GPS_long);
      dataFile.print(F(","));
      dataFile.print(GPS_Altitude);
      dataFile.print(F(","));
      dataFile.print((int)(ext_temp*10.0));
      dataFile.print(F(","));
      dataFile.print((int32_t)(int_temp*10.0));
      dataFile.print(F(","));
     // dataFile.println(F("0123"));      
      dataFile.print(radio_temp);      
      dataFile.print(F(","));
      dataFile.print(payload1_temp);   
      dataFile.print(F(","));
      dataFile.print(cam_sensorA_status, DEC);   
      dataFile.print(F(","));
      dataFile.print(cam_sensorB_status, DEC);   
      dataFile.print(F(","));
      dataFile.print(cam_sd_status, DEC);   
      dataFile.print(F(","));
      dataFile.print(cam_images_taken, DEC);   
      dataFile.print(F(","));
      dataFile.println(cam_intervalometer_period, DEC);   
         
      
      dataFile.close();
  
      _Serial.println(F("OK"));
      return 0;
    }
    // if the file isn't open, pop up an error:
    else 
    {
      dataFile.close();
      _Serial.println(F("file error!"));
      return 2;
    }
  }
  dataFile.close();
  _Serial.println(F("OBC: No SD CARD!"));
  return 1;  
}
/*
int read_config(void)
{
  File dataFile = SD.open("config1.cfg");

  if (dataFile) 
  {
    // read from the file until there's nothing else in it:
    int i=0;
    while (dataFile.available()) 
    {
      config_data[i]=dataFile.read();
      i++;
    }
    // close the file:
    dataFile.close();
    return 0;
  } else 
  {
    return 2;
    // if the file didn't open, print an error:
    
  }
  return 1;
}

void parseConfig(void)
{
    TERMINATION=0;
    callsign = "";
    int j=0;
    for(int i=0; config_data[i] != 'e'; i++)
    {
      if( config_data[i] == '\r')
      {
        j++;
        i+=2;
      }
      if( j == 0)
      {
        callsign += String(config_data[i]);        
      }
      else if( j == 1)
      {
        TERMINATION = TERMINATION * 10;
        TERMINATION += (config_data[i] - '0');
      }
    }  
}*/

