int32_t camera_get_hk(void)
{
  int inByte;
  int i=0;
  int msg_index=0;
  int msg_code = 0;
  int timer=0;
  int nowtime=0;
  int getmsg=10;
  
  _Serial.listen();  
  _Serial.println(F("OBC: Get Camera Housekeeping"));  
  delay(100);
  setBusBusy();
  delay(500);
  _Serial.println(F("$TMHKR,P1*32"));

  timer=millis();
  //_Serial.println(timer);

  _Serial.listen();  
  serialFlush();
  while(getmsg !=0)
  {
    getmsg=GetBusMSG();
    
    nowtime=millis();
    if(nowtime - timer >6000)
    {
      _Serial.println(F("OBC: UPRA-CAM HK timeout"));
      clrBusBusy();
      return -1;
    }
    
        
   // _Serial.print(".");
  }
  clrBusBusy();
  return 0;
}

void parse_camera_hk()
{
  int i, j, k, IntegerPart;


 
  // $TCHKD,0123,336*47
  //        temp  vcc
  //           1   2
 
  IntegerPart = 1;

  cam_sensorA_status = 0;
  cam_sensorB_status = 0;
  cam_sd_status = 0;
  cam_images_taken = 0;
  cam_intervalometer_period = 0;

  for(i = 0; i < MSGindex; i++)
  {
    _Serial.print(bus_msg[i]);
  }
  _Serial.println("");
   
  for (i=0, j=0, k=0; (i<MSGindex) && (j<10); i++) // We start at 7 so we ignore the '$GPGGA,'
  {
    if ((bus_msg[i] == ',') || (bus_msg[i] == '*'))
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
          payload1_temp[k] = bus_msg[i];
          k++;
        }
      }
      else if (j == 2)
      {
        cam_sensorA_status = (uint8_t)(bus_msg[i] - '0');
      }
      else if (j == 3)
      {
        cam_sensorB_status = (uint8_t)(bus_msg[i] - '0');
      }
      else if (j == 4)
      {
        cam_sd_status = (uint8_t)(bus_msg[i] - '0');
      }
      else if (j == 4)
      {
        if ((bus_msg[i] >= '0') && (bus_msg[i] <= '9'))
        {
          cam_images_taken = cam_images_taken * 10;
          cam_images_taken += (uint8_t)(bus_msg[i] - '0');
        }
      }
      else if (j == 5)
      {
        if ((bus_msg[i] >= '0') && (bus_msg[i] <= '9'))
        {
          cam_intervalometer_period = cam_intervalometer_period * 10;
          cam_intervalometer_period += (uint8_t)(bus_msg[i] - '0');
        }
      }
    }
    
   // GPS_Altitude = Altitude;
  }
  payload1_temp[3] ='\0';  

}

int processCMDTAmsg(void)
{
  int i, j, k, IntegerPart;
  char tmp[3];
  int pic=0;
 
  // $TMHKR,C,,,,*47
  //       COM 
  //        1  
 
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
        tmp[k] = bus_msg[i];
        k++;
      }

    }
    
   
  }

  if( tmp[0] == 'S')
  {
    //_Serial.println("save");
    pic = savePICsd();
    _Serial.println(F(""));
    if(pic == 0)
    {
      _Serial.println(F("Success"));
      return 0;
      delay(500);
    }
    else if(pic == 1)
    {
      _Serial.println(F("Data overflow"));
    }
    else if(pic == 2)
    {
      _Serial.println(F("CAM timeout"));
    }
    return 0;
  }
  if( tmp[0] == 'E')
  {
    _Serial.println(F("$TMCAM,,END*47"));
    return 0;
  }  
  
}

int savePICsd(void)
{
  char str[10];
  byte buf[256];
  static int i = 0;
  static int k = 0;
  uint8_t temp = 0,temp_last=0;
  bool is_data=false;
  volatile int cam_wtchdg=0;
  int nowtime=0; 
  
  //itoa(picnum, str, 10);
  str[0]='\0';
  strcat(str, GPS_time);
  strcat(str, ".jpg");

  //Open the new dataFile
  //dataFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
  if(!dataFile.open(str, O_RDWR | O_CREAT ))
  {
    _Serial.println(F("open dataFile faild"));
    dataFile.close();
    return 2;
  }
  //picnum++;

  
 i = 0;
 cam_wtchdg = millis();
 while(!_Serial.available())
 {
    nowtime=millis();
    if(nowtime - cam_wtchdg >6000)
    {        
      dataFile.close();
      return 2;
    }    
 }
/*  while (_Serial.available() > 0)
  {
    temp = _Serial.read();
  }*/
  temp=0; 
 //Read JPEG data from FIFO
 cam_wtchdg = millis();
 while ( (temp !=0xD9) | (temp_last !=0xFF))
 {
  nowtime=millis();
  if(nowtime - cam_wtchdg >3000)
  {        
    dataFile.close();
    return 2;
  }  
  if (_Serial.available() > 0)
  {
    cam_wtchdg = millis();
    temp_last = temp;
    temp = _Serial.read();  //Write image data to buffer if not full

      if( i < 256)
       buf[i++] = temp;
       else{
        //Write 256 bytes image data to dataFile
    
        dataFile.write(buf ,256);
        i = 0;

        //timeout in case of corrupt dadaflow; picture data larger than 80kB -> must be tested and calibrated
        if(k>320)
        {
          dataFile.close(); //close dataFile
          return 1;
        }
        
        buf[i++] = temp;
    
       }
 
     _Serial.print(F("o")); 
     delay(0);  
  }
 }
 
 //Write the remain bytes in the buffer
 if(i > 0){

  dataFile.write(buf,i);
 }
 //Close the dataFile
 dataFile.close();
  

  return 0;  
}

void getPICuart()
{
  int timer=0;
  int nowtime=0;
  int getmsg=10;
  

  _Serial.listen();
  //_Serial.println("d");
  _Serial.println(F("$TMCAM,1,CAP*47"));
  
  timer=millis();
  while(getmsg !=0)
  {
    getmsg=GetBusMSG();
    
    nowtime=millis();
    if(nowtime - timer >6000)
    {
      _Serial.println(F("timeout"));
      break;
    }
    
        
   // _Serial.print(".");
  }
  
}
