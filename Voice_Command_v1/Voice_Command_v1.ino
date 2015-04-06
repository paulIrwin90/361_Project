/********************************************************
* In this iteration we included the voice recognition set
********************************************************/
#include "SoftwareSerial.h"
#include "EasyVR.h"
#include "nRF24L01.h"
#include "RF24.h"
#include <Adafruit_GFX.h>
#include <Adafruit_HX8340B.h>
#include <SPI.h>

//       Name      Pin #
#define  SCK        13
#define  MISO       12		
#define  MOSI       11		
#define  TFT_CS     10     // CS
#define  TFT_RESET   9	   // RESET
#define  RF_CE       8     // 
#define  RF_CSN      7     // 
#define  VR_RX       6     // 
#define  VR_TX       5     // 

SoftwareSerial port(VR_RX, VR_TX);
EasyVR easyvr(port);
RF24 radio(RF_CE, RF_CSN);

bool drawn = 0;
int8_t wordSet, index;
int msg[1];
const uint64_t pipe = 0xE8E8F0F0E1LL;

// wordSets and Commands
enum Wordset
{
  WORDSET_0 = 0,
  WORDSET_1 = 1,
};

enum Wordset0
{
  Robot = 0, 
};

enum Wordset1
{
  Action = 0,
  Move = 1,
};

/********************************************************
* Set up the serial monitor, serial port, and the 
* display. Display splash screen. Initialize VR.
********************************************************/
void setup()
{
   Serial.begin(9600); // serial monitor
   port.begin(9600);   // serial port from arduino to VR
   
   radio.begin();
   radio.enableAckPayload();
   radio.openWritingPipe(pipe);
   
   if (easyvr.detect())
      Serial.println("Easy VR Detected.");   

   easyvr.setTimeout(5);      // Timeout after 5 seconds of inactivity
   easyvr.setLanguage(0);     // English
   wordSet = EasyVR::TRIGGER; //<-- start wordSet (customize)
}

/********************************************************
* Program
********************************************************/
void loop()
{
  Serial.print("Say a command in Wordset ");
  Serial.println(wordSet);
  easyvr.recognizeWord(wordSet);

  msg[0] = 1;
  do
  {
    Serial.println("Doing...");
    radio.write(msg, 1);
    Serial.println(msg[0]);
    delay(2000);
  } 
  while (!easyvr.hasFinished());
  Serial.println("Not Doing...");
  index = easyvr.getWord();

  // Understand trigger word, built-in trigger (ROBOT)
  if (wordSet == 0 && index >= 0)
  {
    Serial.println("Listening...");
    wordSet = WORDSET_1;
    return;                        // Kick out of loop() and start again
  }
  
  // Recognized word other than trigger
  if (index >= 0)
  {
    uint8_t train = 0;
    char name[32];
    Serial.print("Command: ");
    Serial.print(index);
    
    if (easyvr.dumpCommand(wordSet, index, name, train))
    {
      Serial.print(" = ");
      Serial.println(name);
    }
    
    else
      Serial.println("?");
    
    action();
  }
  
  // Nothing was recognized
  else
  {
    if (easyvr.isTimeout())
    {
      Serial.println("Timed out, try again...");
    }     
    int16_t err = easyvr.getError();
    if (err >= 0)
    {
      Serial.print("Error ");
      Serial.println(err, HEX);   
    }
    wordSet = WORDSET_0;
  }
}

/********************************************************
*
********************************************************/
void action()
{
  
  switch (wordSet)
  {
  case WORDSET_0:
    switch (index)
    {
      case Robot:
        wordSet = WORDSET_1;
        break;
    }
    break;

  case WORDSET_1:
    switch (index)
    {
      case Action:
        Serial.println("Action");
        msg[0] = index;
        radio.write(msg, 1);
        delay(2000);
        break;
        
      case Move:
        Serial.println("Move");
        msg[0] = index;
        radio.write(msg, 1);
        delay(2000);
        break;
    }
    break;
  }
}


