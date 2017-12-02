#include <SPI.h>
#include <SD.h>

#include <SoftwareSerial.h>
//#include <AltSoftSerial.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

/*
 * Timing macros
 */
 #define MEASURE      1   // activater measurement every MEASURE)*5s  (0-> never, 1-> every 5second)
 #define RADIO        3   // activater telemetry downlink every RADIO*5s  (0-> never, 1-> every 5second)
 #define TERMINATION  cut_down_alt

/*
 * Temperature macros
 */

#define AVRG 10
#define INTERNALOFFSET 0
#define INTERNALCALIB 102.71
#define INTERNALGAIN 2.69

/*
 * PIN macros
 */

#define BUZZ    5         //BUZZER
#define FTU     4         //FLIGHT TERMINATION ENABLE
#define GPS_RX  3         //GPS TO OBC            
#define GPS_TX  2         //OBC TO GPS
#define SDA     18        //I2C SDA        
#define SCL     19        //I2C SCL

//SD CARD - SPI
#define MOSI    11        //MOSI
#define MISO    12        //MISO
#define CLK     13        //CLK        
#define CS      10        //CHIP SELECT

/*
 * COM module macros
 * uncomment the COM module in use
 */
//#define HW_COM            //COM hardware connected
#define SW_COM1           //COM simulation w/o handshake
//#define SE_COM2           //COM simulation w/ handshake

/*
 * SICL Baudrate
 */
#define SICL_BAUD 9600

/*
 * GPS module definition
 * uncomment the GPS module connected to the board
 */

#define NEO6_GPS        //UBLOX NEO 6M
//#define NVS8C_GPS       //NVS-8C
//#define L86_GPS         //QUECTEL L86
//#define TYCO            //Tyco A10xx seires
//#define Soft_GPS        //SERIAL SIMULATOR

/*
 * Radio macros
 */

//#define CALLSIGN 

/*
 * GPS variables
 */

SoftwareSerial GPS(GPS_RX, GPS_TX); // RX, TX
SoftwareSerial _Serial(0,1);
//AltSoftSerial GPS;

byte              GPSBuffer[82];
byte              GPSIndex=0;
unsigned int      GPS_Satellites=0;
unsigned int      GPS_Altitude=110;

unsigned int      GPS_Fix=0;
char              GPS_time[10];
char              GPS_lati[12];
char              GPS_long[13];

/*
 * SD Card variables
 */

byte card_present=0;

/*
 * Temperature variables
 */

volatile float ext_temp = 0.0;
volatile double int_temp =0.0;

/*
 * Radio variables
 */

byte MSGindex=0;
char radio_hk_data[64];
//char radio_handshake[65];
String radio_voltage ="";
char radio_temp[5];


/*
 * Timing variables
 */

unsigned long now;
int is_measure=0;
int is_radio=0;

bool is_climb=false;
bool is_landing=false;
bool is_FTU_on=false;

bool is_com_present=true;

int cut_down_alt = 17800;

void timing()
{
  now = millis();
  if( !is_landing )
  {
    is_measure++;
    is_radio++;
  }
}

/*
 * Logging variables
 */

//String dataString = "";
String radioString = "";
const String callsign PROGMEM = "HAxUPRA";
File dataFile;
/*
 * Configuration variables
 */

//bool is_config_read=false;
//char config_data[100];
 
void setup() 
{
  // put your setup code here, to run once:

  GPS_time[0] = '3';
  GPS_time[1] = '3';
  GPS_time[2] = '6';
  GPS_time[3] = '6';
  GPS_time[4] = '7';
  GPS_time[5] = '7';
  GPS_time[6] = '\0';

  //GPS_lati[12] = {'+', '4', '8', '0', '7', '.', '0', '3', '8', '\0'};
  GPS_lati[0] = '+';
  GPS_lati[1] = '4';
  GPS_lati[2] = '8';
  GPS_lati[3] = '0';
  GPS_lati[4] = '7';
  GPS_lati[5] = '.';
  GPS_lati[6] = '0';
  GPS_lati[7] = '3';
  GPS_lati[8] = '8';
  GPS_lati[9] = '\0';

  //GPS_long[13] = {'+', '0', '1', '1', '3', '1', '.', '0', '0', '0', '\0'};
  GPS_long[0] = '+';
  GPS_long[1] = '0';
  GPS_long[2] = '1';
  GPS_long[3] = '1';
  GPS_long[4] = '3';
  GPS_long[5] = '1';
  GPS_long[6] = '.';
  GPS_long[7] = '0';
  GPS_long[8] = '0';
  GPS_long[9] = '0';
  GPS_long[10] = '\0';
  
  //UART port for COM module
  _Serial.begin(SICL_BAUD);
  _Serial.println(F("OBC: startup"));
  _Serial.print(F("OBC: init COM..."));
  _Serial.listen();
  
  //wait for COM startup or timeout
  now = millis();
  while(!_Serial.available())
  {
    if((millis() - now) > 10000)
    {
      is_com_present=false;
      break;
    }
  }
  now=0;
  if(is_com_present)
  {
    delay(1000);
    _Serial.println(F("OK"));
  }
  else
  {
    _Serial.println(F("COM TIMEOUT"));
  }

  
  //UART port for GPS module
  //todo: GPS related configuration for GPS
  _Serial.print(F("OBC: init GPS..."));
#ifdef NEO6_GPS  
  GPS.begin(9600);
  int GPS_stat = SetupUBLOX();
  if(GPS_stat == 1)
  {
    _Serial.listen();
    _Serial.println(F("ERROR!: Airborne mode"));
  }
  else if(GPS_stat == 2)
  {
    _Serial.listen();
    _Serial.println(F("ERROR!: Port config"));
  }
  else if(GPS_stat == 3)
  {
    _Serial.listen();
    _Serial.println(F("ERROR!: Airborne + Port"));
  }
  else
  {
    _Serial.listen();
    _Serial.println(F("OK"));
  }
#elif defined(TYCO)
  GPS.begin(4800);
#elif defined(Soft_GPS)
  GPS.begin(9600);

#endif
  
  delay(500);
  _Serial.listen();
  //Pin configurations
  pinMode(BUZZ, OUTPUT);
  pinMode(FTU, OUTPUT);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  digitalWrite(FTU, LOW);
  
  delay(500);

  //Test Buzzer
  _Serial.print(F("OBC: BUZZER TEST..."));
  buzzerTest();
  _Serial.println(F("OK"));
  //Debug FTU
/*        digitalWrite(FTU, HIGH);
        delay(1000);
        digitalWrite(FTU, LOW);  
  while(1);
  */
  //SD CARD init
  if (!SD.begin(CS)) 
  {
    _Serial.println(F("OBC: NO SD Card"));
    card_present=0;
  }
  else
  {
    _Serial.print(F("OBC: create log file..."));
    card_present=1;
    dataFile = SD.open("datalog.csv", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) 
    {
      dataFile.println(F("time,latitude,longitude,altitude,ext_temp,OBC_temp,COM_temp"));
      dataFile.close();
      _Serial.println(F("OK"));
      // print to the serial port too:
    }
    // if the file isn't open, pop up an error:
    else 
    {
      card_present=2;
      _Serial.println(F("File error!"));
    }    
  }
/*
  //flush config data
  for(int i=0; i<100; i++)
  {
    config_data[i] = 0;
  }

  //set config from SD Card
  _Serial.print("READ CONFIG...");
  if(read_config()!=0)
  {
    is_config_read = false;
     _Serial.println("FAILED");
     callsign="HAxUPRA";
  }
  else
  {
    is_config_read = true;
     _Serial.println("OK");
  }

  if( is_config_read )
  {
    parseConfig();
  }
*/
  
  //send startup msg
#ifdef SW_COM1
  _Serial.println(F("OBC: Init done"));
  _Serial.print(F("OBC: CRD: "));
  _Serial.println(card_present);
  _Serial.print(F("OBC: SGN: "));
  _Serial.println(callsign);
  _Serial.print(F("OBC: FTU: "));
  _Serial.println(TERMINATION);
#endif  
  
  //set startup time
  now = millis();  

  //Startup measurement and radio

  getGPSMeasurement();
  delay(50);
  lowSpeedTelemetry();
}

void loop() 
{
  // put your main code here, to run repeatedly:
  if( ((millis() - now) > 5000) && (!is_landing))
  {
    timing();
    //_Serial.println(is_measure);
    //_Serial.println(MEASURE);
    if(is_measure == MEASURE)
    {
      getMeasurements();
 
      getGPSMeasurement();
      delay(50);
/*      _Serial.print("DATA LOG...");
      if( dumpLog() == 0)
      {
        _Serial.println("OK"); 
      }
      else
      {
        _Serial.println("FAILED");
      }*/
      dumpLog();

//---------------datalog---------------

      
 
 /*   dataFile = SD.open("datalog.csv", FILE_WRITE);
    if (dataFile) 
    {
      dataFile.print(GPS_time);
      dataFile.print(",");
      dataFile.print(GPS_lati);
      dataFile.print(",");
      dataFile.print(GPS_long);
      dataFile.print(",");
      dataFile.print(GPS_Altitude);
      dataFile.print(",");
      dataFile.print((int)(ext_temp*10.0));
      dataFile.print(",");
      dataFile.print((int)(int_temp*10.0));
      dataFile.print(",");
      dataFile.println(radio_temp);      
      dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else 
    {
    }
    */


//---------------datalogend------------
      
      is_measure=0;
    }
    if(is_radio == RADIO)
    {
      lowSpeedTelemetry();
      is_radio=0;
    }
    if(( GPS_Altitude > TERMINATION) && (!is_FTU_on))
    {
        digitalWrite(FTU, HIGH);
        delay(1000);
        digitalWrite(FTU, LOW);
        is_FTU_on = true;
    }
   // timing();
  }

// Delay-es megoldás
  
  if(is_landing)
  {
    _Serial.println(F("OBC: BEACON MODE"));
    buzzer();
    getGPSMeasurement();
    lowSpeedTelemetry();
   // delay(1000);
  }


}
