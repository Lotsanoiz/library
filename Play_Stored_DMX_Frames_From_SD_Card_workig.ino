/*
 * This sketch is used to read the stored LED lighting effects from a SD card on a 
 * nodemcu or a ESP8266 module.  'Next' button will play the file or effect.  Previous
 * button will play the previous effect.
 * 
 * WiFi connection is not needed for this sketch to work.
 *  
 * https://github.com/tangophi/Artnet_DMX_SD_Card
*/

#include <SPI.h>
#include <SD.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>
#include <Adafruit_NeoPixel.h>

#define PIN_PREVIOUS_BUTTON   4
#define PIN_NEXT_BUTTON       5
#define PIN_SD_CS             15
#define PIN_LED               2

//Wifi settings
const char* ssid = "NETGEAR32";
const char* password = "sweetzoo919";

// Neopixel settings
const int numLeds = 768  ; // change for your setup

Adafruit_NeoPixel leds = Adafruit_NeoPixel(numLeds, PIN_LED, NEO_GRB + NEO_KHZ800);

// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as zero.
const int numberOfChannels = numLeds * 3; // Total number of channels you want to receive (1 led = 3 channels)
byte channelBuffer[numberOfChannels]; // Combined universes into a single array

// SD card
File datafile;

char fileNameFull[10] = "";
int  fileNameSuffix   = 0;
int  totalFilesCount  = 0;

volatile bool prevFile = false;
volatile bool nextFile = false;


// Check if we got all universes
const int maxUniverses = numberOfChannels / 510 + ((numberOfChannels % 510 ) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;

// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");
  
  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  
  return state;
}

void buttonHandlerPrevious()
{
  if (!prevFile)
  {
    prevFile = true;
    Serial.println("Previous button pressed.");
  }
}

void buttonHandlerNext()
{
  if (!nextFile)
  {
    nextFile = true;
    Serial.println("Next button pressed.");
  }
}

void setup()
{
  Serial.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  if (!SD.begin(PIN_SD_CS)) {
    Serial.println("initialization failed!");
  }
  else
    Serial.println("initialization done.");

//  ConnectWifi();
//  artnet.begin();
  leds.begin();
//  initTest();
  
  attachInterrupt (PIN_PREVIOUS_BUTTON,  buttonHandlerPrevious,  RISING);
  attachInterrupt (PIN_NEXT_BUTTON,      buttonHandlerNext,      RISING);

  // Get the total number of 'dataXXX' files on the SD card.
  for (int i=0;i<10000;i++)
  {
    sprintf(fileNameFull, "data%d", i);
    if(SD.exists(fileNameFull))
    {
      totalFilesCount = i + 1;
    }
    else
    {
      break;
    }
  }

  Serial.print("Total number of data files on the SD card: ");
  Serial.println(totalFilesCount);

  memset(fileNameFull, 0, 10);
  sprintf(fileNameFull, "data%d", fileNameSuffix);
  datafile = SD.open(fileNameFull, FILE_READ);
}

void loop()
{
  // Close the current file and open the previous file
  if (prevFile)
  {
    datafile.close();

    if (--fileNameSuffix < 0)
    {
      fileNameSuffix = totalFilesCount - 1;
    }

    memset(fileNameFull, 0, 10);
    sprintf(fileNameFull, "data%d", fileNameSuffix);

    Serial.print("Opening ");
    Serial.println(fileNameFull);
    
    datafile = SD.open(fileNameFull, FILE_READ);

    delay(100);
    prevFile = false;
  }

  // Close the curent file and open the next file
  if (nextFile)
  {
    datafile.close();

    if (++fileNameSuffix >= totalFilesCount)
    {
      fileNameSuffix = 0;
    }

    memset(fileNameFull, 0, 10);
    sprintf(fileNameFull, "data%d", fileNameSuffix);

    Serial.print("Opening ");
    Serial.println(fileNameFull);
    
    datafile = SD.open(fileNameFull, FILE_READ);

    delay(100);
    nextFile = false;
  }

  // Keep reading data from the file and setting the LEDs.  If the full file has been read, then reset
  // the file pointer to the beginning of the file.
  if (datafile.available())
  {
    datafile.read(channelBuffer, numberOfChannels);
    for (int i = 0; i < numLeds; i++)
      leds.setPixelColor(i, channelBuffer[(i) * 3], channelBuffer[(i * 3) + 1], channelBuffer[(i * 3) + 2]);
      
    leds.show();
    delay(20);
  }
  else
  {
    datafile.seek(0);
  }
}

void initTest()
{
  for (int i = 0 ; i < numLeds ; i++)
    leds.setPixelColor(i, 127, 0, 0);
  leds.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
    leds.setPixelColor(i, 0, 127, 0);
  leds.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
    leds.setPixelColor(i, 0, 0, 127);
  leds.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
    leds.setPixelColor(i, 0, 0, 0);
  leds.show();
}
