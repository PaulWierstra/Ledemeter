#include <Ethernet.h>
#include <TimerOne.h>
#include "LPD6803_thiez.h"
#include <math.h>
#include "ProgressBar.h"
#include <SPI.h>
#include <Time.h>

#define WAITTIME 2000
#define METER_LONG_MAX 1000.0f  // Don't forget the .0f at the end. Change it to whatever the amount of liters to show.
#define METER_SHORT_MAX 1000.0f   // Second strand, this should be a lot lower so you can see progress
#define NUM_LEDS_LONG 20         // This is the amount of leds (or individual parts of the ledstrip) to control
#define NUM_LEDS_SHORT 20        // To test this, you need to change the input, a value too low will result in the upper part of the strip being white, too high will mean
                                 // that if you give a value lower than the max amount, the string will be full.

#define DEBUG_PRINTING

#ifdef DEBUG_PRINTING
#define DEBUG_PRINT_INIT() Serial.begin(9600)
#define DEBUG_PRINT(m) Serial.println(m)
#else
#define DEBUG_PRINT_INIT()
#define DEBUG_PRINT(m)
#endif

float curr_liters = 0.0;
int knipperd = 0;

int w = 1000; //wait time for debug

// Choose which 2 pins you will use for output.
// Can be any valid output pins.
int dataPinLong = 2;       // 'yellow' wire
int clockPinLong = 3;      // 'green' wire
int dataPinShort = 4;
int clockPinShort = 5;
// Don't forget to connect 'blue' to ground and 'red' to +5V

// Timer 1 is also used by the strip to send pixel clocks

// Set the first variable to the NUMBER of pixels. 20 = 20 pixels in a row
LPD6803 stripLong; //= LPD6803(NUM_LEDS_LONG, dataPinLong, clockPinLong);
LPD6803 stripShort; //= LPD6803(NUM_LEDS_SHORT, dataPinShort, clockPinShort);
ProgressBar meterLong; //= ProgressBar(&stripLong,METER_LONG_MAX);
ProgressBar meterShort; //= ProgressBar(&stripShort,METER_SHORT_MAX);

//Mac address
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x0D, 0x78, 0x62 };
byte server[] = { 
  130,89,149,242}; //vestingbar.nl

EthernetClient client;

//dhcp renewal info
int h;
time_t t;

void setup(){
  DEBUG_PRINT_INIT();
  DEBUG_PRINT("Hello world!");
  /*if(strip.getCurrentStrip()==NULL)
    Serial.println("Klaag");
  else
    Serial.println("Praise");
    delay(100);*/
  DEBUG_PRINT("Creating strips");
  stripLong = LPD6803(NUM_LEDS_LONG, dataPinLong, clockPinLong);
  stripShort = LPD6803(NUM_LEDS_SHORT, dataPinShort, clockPinShort);
  stripLong.setCPUmax(50);  // start with 50% CPU usage. up this if the strand flickers or is slow
  DEBUG_PRINT("Strip begin");
  delay(100);
  stripLong.begin();
  stripShort.begin();
  DEBUG_PRINT("Strip show");
  delay(100);
  stripLong.show();
  stripShort.show();
  DEBUG_PRINT("Getting IP");
  delay(100); 
    //Returns 1 on succesfull DHCP request, 0 on failure
  if(Ethernet.begin(mac) == 0){
    DEBUG_PRINT("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  // print your local IP address:
  DEBUG_PRINT(Ethernet.localIP());
  DEBUG_PRINT("Setting time");
  delay(100);
  // get info for dhcp renewal
  t = now();
  DEBUG_PRINT("Getting hour");
  delay(100);
  h = hour();
  DEBUG_PRINT("Fixed time");
  delay(1000);
  DEBUG_PRINT("Initializing biermeters");
  delay(1000);
  meterLong = ProgressBar(&stripLong,METER_LONG_MAX);
  meterShort = ProgressBar(&stripShort,METER_SHORT_MAX);
  DEBUG_PRINT("Setting biermeters");
  meterLong.setColor(26,25,0);
  meterShort.setColor(26,25,0);
  //meterLong.setMarks(1000.0, 1000.0);
  //meterShort.setMarks(50.0, 50.0);
}

void loop(){
  DEBUG_PRINT("DHCP check.");
  delay(500);
  dhcpCheck();
  DEBUG_PRINT("Beer check.");
  delay(500);
  getBierInfo();
  //Serial.println("Init litermeter");
  //delay(500);
  //ProgressBar literMeter = ProgressBar(&strip, LITER_MAX);
  Serial.print("Bierstand: "); Serial.println(curr_liters);
  meterLong.setProgress(curr_liters);
  meterShort.setProgress( fmod(curr_liters, METER_SHORT_MAX) );
  meterLong.update();
  meterShort.update();
  delay(WAITTIME);
}

void getBierInfo(){
  //Server connection
  connectToVB();
  int bytesAvail = client.available();
  if(bytesAvail){
    readPastHeader(&client);
    Serial.print("Read: ");
    String result = "";
    char c = client.read();
    while(c != -1){
      Serial.print(c);
      result.concat(c);
      c = client.read();
    }
    char charBuf[50];
    result.toCharArray(charBuf,50);
    curr_liters = atof(charBuf);
    Serial.print("Done getting beer info: ");
    Serial.println(curr_liters);
  }


}

int connectToVB(){
  client.stop(); 
  delay(2000);
  if(client.connect(server, 80)){
    Serial.println("Connected");
    client.println("GET /docs/bierstanden.txt HTTP/1.0");
    client.println();
    delay(2000);
    Serial.println("Received.");
    return 1;
  }
  Serial.println("Connection failed");
  return 0;
}

void dhcpCheck(){
  time_t n = now();
  if((hour(n)- h) > 23){
    byte i = Ethernet.maintain();
    Serial.print("Requested DHCP update, ");
    switch (i){
    case 0:
      Serial.println("nothing happened");  
      break;
    case 1:
      Serial.println("renew failed");
      break;
    case 2:
      Serial.println("renew success");
      h = hour(n);
      break;
    case 3:
      Serial.println("rebind fail");
      break;
    case 4:
      Serial.println("rebind success");
      h = hour(n);
      break;
    default:
      Serial.println("Error!");
      break;
    }
  }
} 

bool readPastHeader(Client *pClient)
{
  bool bIsBlank = true;
  while(true)
  {
    if (pClient->available()) 
    {
      char c = pClient->read();
      if(c=='\r' && bIsBlank)
      {
        // throw away the /n
        c = pClient->read();
        return true;
      }
      if(c=='\n')
        bIsBlank = true;
      else if(c!='\r')
        bIsBlank = false;
    }
  }
}










