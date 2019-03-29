#include <SPI.h>
#include <SdFat.h>

#include <SoftwareSerial.h>
//#include <AltSoftSerial.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

/*
 * Timing macros
 */
 #define GETGPS       1   // activate GPS read every GETGPS)*5s                   (0-> never, 1-> every 5second)
 #define GETTEMP      15  // activate Temperature measurement every GETTEMP)*5s   (0-> never, 1-> every 5second)
 #define GETICS       2  // activate ICS capture every GETICS)*5s                (0-> never, 1-> every 5second)
 #define RADIO        3   // activate telemetry downlink every RADIO*5s           (0-> never, 1-> every 5second)
 //#define TERMINATION  cut_down_alt

/*
 * Temperature macros
 */

#define AVRG 10

#define INTERNALCALIB         ((double)(90.0 ))
#define INTERNALGAIN          ((double)(1.51))
#define CALIBRATION_OFFSET    ((double)(-23.33))
#define CALIBRATION_GAIN      ((double)(1.35))

/*
 * PIN macros
 */

#define BUZZ    5         //BUZZER
#define FTU     4         //FLIGHT TERMINATION ENABLE
#define GPS_RX  3         //GPS TO OBC            
#define GPS_TX  2         //OBC TO GPS
#define SDA     18        //I2C SDA        
#define SCL     19        //I2C SCL

//EXTERNAL CAMERA
#define MENU    18
#define SHTR    19

//SD CARD - SPI
#define MOSI    11        //MOSI
#define MISO    12        //MISO
#define CLK     13        //CLK        
#define CS      10        //CHIP SELECT

//UPRA BUS
#define SICL_BAUD 9600
#define SICLRX    0
#define SICLTX    1
#define BUSBUSY   A0


/*
 * GPS variables
 */

SoftwareSerial GPS(GPS_RX, GPS_TX); // RX, TX
SoftwareSerial _Serial(SICLRX,SICLTX);
//AltSoftSerial GPS;

byte              GPSBuffer[82];
byte              GPSIndex=0;
uint8_t           GPS_Satellites=0;
long              GPS_Altitude=0;

uint8_t           GPS_Fix=0;
char              GPS_time[10];
char              GPS_lati[12];
char              GPS_long[13];

bool              GPS_first_fix;

/*
 * SD Card variables
 */

bool card_present=false;

SdFat sd;
SdFile dataFile;

/*
 * Temperature variables
 */

volatile float ext_temp = 0.0;
volatile double int_temp =0.0;

/*
 * Radio variables
 */

byte MSGindex=0;
char bus_msg[30];
//char radio_handshake[65];

char radio_temp[4];


/*
 * Timing variables
 */

unsigned long now;
uint8_t is_gps=0;
uint8_t is_temp=0;
uint8_t is_radio=0;
uint8_t is_ics=0;

uint8_t ecam_timer=0;
bool is_ecam_on=false;

bool is_climb=false;
bool is_landing=false;
//bool is_FTU_on=false;

bool is_com_present=true;

uint8_t init_error;

void timing()
{
  now = millis();
  if( !is_landing )
  {
    is_gps++;
    is_temp++;
    is_radio++;
    is_ics++;
  }
}

/*
 * Logging variables
 */

//String dataString = "";
//String radioString = "";
//const String callsign PROGMEM = "HAxUPRA";
//File dataFile;
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

  GPS_lati[0] = '+';
  GPS_lati[1] = '9';
  GPS_lati[2] = '5';
  GPS_lati[3] = '0';
  GPS_lati[4] = '0';
  GPS_lati[5] = '.';
  GPS_lati[6] = '0';
  GPS_lati[7] = '0';
  GPS_lati[8] = '0';
  GPS_lati[9] = '\0';

  GPS_long[0] = '+';
  GPS_long[1] = '1';
  GPS_long[2] = '8';
  GPS_long[3] = '8';
  GPS_long[4] = '8';
  GPS_long[5] = '8';
  GPS_long[6] = '.';
  GPS_long[7] = '0';
  GPS_long[8] = '0';
  GPS_long[9] = '0';
  GPS_long[10] = '\0';

  GPS_first_fix = false;
  
  radio_temp[0] = 'N';
  radio_temp[1] = '/';
  radio_temp[2] = 'A';
  radio_temp[3] = '\0';
  
  //UART port for COM module
  _Serial.begin(SICL_BAUD);
  _Serial.println(F("OBC: startup"));

  ecam_init();
  ecam_ON();
  

  
  _Serial.print(F("OBC: init GPS..."));

  GPS.begin(9600);
  int GPS_stat = SetupUBLOX();
  if(GPS_stat == 1)
  {
    _Serial.listen();
    _Serial.println(F("ERROR!: Airborne mode"));
    init_error = 3;
  }
  else
  {
    _Serial.listen();
    _Serial.println(F("OK"));
    init_error = 5;
  }

  
  delay(500);
  _Serial.listen();
  //Pin configurations
  pinMode(BUSBUSY, INPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(FTU, OUTPUT);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  digitalWrite(FTU, LOW);
  digitalWrite(BUZZ, HIGH);
  delay(500);

  //Debug FTU
/*        digitalWrite(FTU, HIGH);
        delay(1000);
        digitalWrite(FTU, LOW);  
  while(1);
  */
  //SD CARD init
  if (!sd.begin(CS, SPI_HALF_SPEED)) 
  {
    _Serial.println(F("OBC: NO SD Card"));
    card_present=false;
    init_error = 0;
  }
  else
  {
    _Serial.print(F("OBC: create log file..."));
    card_present=true;
    if (dataFile.open("data.csv", O_RDWR | O_CREAT | O_AT_END))
    {
      dataFile.println(F("time,latitude,longitude,altitude,ext_temp,OBC_temp,COM_temp"));
      dataFile.close();
      _Serial.println(F("OK"));      
    }   
    else 
    {
      card_present=false;
      dataFile.close();
      _Serial.println(F("File error!"));
    }    
  }
  dataFile.close();

  //Test Buzzer
  _Serial.print(F("OBC: BUZZER TEST..."));
  buzzerTest(init_error);
  _Serial.println(F("OK"));
  
  //send startup msg
  _Serial.println(F("OBC: Init done"));

  //start external camera video
  _Serial.println(F("OBC: ECAM START"));
  ecam_PictureMode();
  ecam_pressSHTR(500);

   
  //set startup time
  now = millis();  

  delay(1000);
  //Startup measurement and radio
  getTemperatures();
  getGPSMeasurement();
  delay(50);
  lowSpeedTelemetry();
}

void loop() 
{
  // put your main code here, to run repeatedly:
  if(busBusy_interrupt())
  {
    _Serial.println(F("OBC: BUS INTERRUPT"));
    //send ACK message
    //get BUS message
  }
  
  if( ((millis() - now) > 5000) && (!is_landing))
  {
    ecam_timer=0;
    timing();
    if(is_temp == GETTEMP)
    {
      getTemperatures();
      is_temp = 0;
    }

    if(is_gps == GETGPS)
    {
      //getTemperatures();
 
      getGPSMeasurement();
      delay(50);

//---------------datalog---------------

      dumpLog();
          
//---------------datalogend------------

      ecam_pressSHTR(500);
      
      is_gps=0;
    }
    if(is_radio == RADIO)
    {
      lowSpeedTelemetry();
      is_radio=0;
    }

    if(is_ics == GETICS)
    {
      getPICuart();
      is_ics = 0;
    }
    
/*    if(( GPS_Altitude > TERMINATION) && (!is_FTU_on))
    {
        digitalWrite(FTU, HIGH);
        delay(1000);
        digitalWrite(FTU, LOW);
        is_FTU_on = true;
    }*/
   // timing();
  }
  
  if(is_landing)
  {
    _Serial.println(F("OBC: BEACON MODE"));
    buzzer();
    getGPSMeasurement();
    dumpLog();
    lowSpeedTelemetry();
    if( ((millis() - now) > 5000) && (is_ecam_on))
    {
      timing();
      ecam_pressSHTR(500);
      ecam_timer++;
    }
    if( (ecam_timer > 120 ) && (is_ecam_on)) //120 for 10mins
    {
      ecam_OFF();
    }
   // delay(1000);
  }


}
