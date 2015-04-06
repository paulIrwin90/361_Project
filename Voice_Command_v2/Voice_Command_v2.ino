/********************************************************
 *
 ********************************************************/
#include "SoftwareSerial.h"
#include "EasyVR.h"
#include "nRF24L01.h"
#include "RF24.h"
#include <Adafruit_GFX.h>
#include <Adafruit_HX8340B.h>
#include <SPI.h>
#include <avr/wdt.h> // used for software reset
#include <SoftReset.h> // used for software reset
#include <EEPROM.h>


//       Name      Pin #
#define  SCK        13
#define  MISO       12		
#define  MOSI       11		
#define  RF_CSN     10     // Yellow
#define  RF_CE       9     // Purple
#define  LED         8
#define  VR_RX       6     // White
#define  VR_TX       5     // Blue


// SET UP EVERYTHING
SoftwareSerial port(VR_RX, VR_TX);
EasyVR easyvr(port);
RF24 radio(RF_CE, RF_CSN);

// GLOBAL VARIABLES
bool drawn = 0;
int8_t wordSet, index;
int msg[1];
bool var;
int addr = 0;
bool locked;
int reset = 0;
const uint64_t pipe = 0xE8E8F0F0E1LL;

// wordSets and Commands
enum Wordset
{
  WORDSET_0 = 0,
  WORDSET_1 = 1,
  WORDSET_2 = 2,
  WORDSET_3 = 3,
};

enum Wordset0
{
  Robot = 0, 
};

enum Wordset3
{
  One = 1,
  Two = 2,
  Three = 3,
};

/********************************************************
 * Set up the serial monitor, serial port, and the 
 * display. Display splash screen. Initialize VR.
 ********************************************************/
void setup()
{
  // BEGIN AND INITIALIZE EVERYTHING
  Serial.begin(9600); // serial monitor
  port.begin(9600);   // serial port from arduino to VR
  radio.begin();
  radio.openWritingPipe(pipe);
  radio.enableAckPayload();
  delay(20); // to ensure that everything is read properly

  pinMode(LED, OUTPUT);  // LED
  pinMode(LED, LOW);

  msg[0] = -1;
  for(int i = 0; i < 5;i++)
    radio.write(msg,1);

  /****************************************/
  // SEE IF THE EASY_VR IS CONNECTED
  if (easyvr.detect())
    Serial.println("Easy VR Detected.");   

  easyvr.setTimeout(5);      // Timeout after 5 seconds of inactivity
  easyvr.setLanguage(0);     // English
  wordSet = EasyVR::TRIGGER; //<-- start wordSet (customize)

  var = radio.setDataRate(RF24_1MBPS);
  Serial.print("Rate changed? ");
  Serial.println(var);

}

/********************************************************
 * Program
 ********************************************************/
void loop()
{
  // MESSAGE COUNT IS USED FOR THE ACK_PAYLOAD
  static uint32_t message_count = 0;
  Serial.print("Say a command in Wordset ");
  Serial.println(wordSet);
  easyvr.recognizeWord(wordSet);
  do
  {
    
  } // GET THE WORD AND STORE IT IN INDEX
  while (!easyvr.hasFinished());

  index = easyvr.getWord();
  /****************************************/
  // Understand trigger word, built-in trigger (ROBOT)
  if (wordSet == 0 && index >= 0)
  {
    Serial.println("Listening..."); // for a command?
    digitalWrite(LED, HIGH);
    wordSet = WORDSET_3;
    return; // Restart loop waiting for trigger work      
  }

  /****************************************/
  // Recognized a command
  if (index >= 0)
  {
    Serial.print("Command: ");
    Serial.println(index);

    // ACTION() JUST SETS THE APPROPRIATE NUMBER FOR MSG
    action();
  }

  //  NOTHING WAS RECOGNIZED
  else
  {
    Serial.print("Message is ");
    Serial.println(msg[0]);
    //jordan wants to remove this   
    if (easyvr.isTimeout())
    {
      Serial.println("Timed out, try again...");
    } 

    // Turn off the Trigger led and get error.
    digitalWrite(8, LOW);


    // Set it back to wordset 0 to recognize the trigger.
    wordSet = WORDSET_0;

  }

  /****************************************/
  // THIS STATEMENT RESENDS THE MESSAGE IF ACK WASN'T
  // RECEIVED AND IF IT'S A MSG WE WANT TO SEND

  // IF ACK IS AVAILABLE, READ IT OUT AND SET MSG BACK TO -1
  if (radio.isAckPayloadAvailable())
  {
    radio.read(&message_count, sizeof(message_count));
    msg[0] = -1;
    return;

  }
  //****************************************/
  // THIS IS JUST TO DO A SOFT RESET AFTER A CERTAIN COUNT
  if (reset == 15)
  {
    soft_restart();
  }
  reset++;
}

/********************************************************
 * SEE IF ITS A RECOGNIZED WORD. IF SO, GIVE MSG A VALUE
 ********************************************************/
void action()
{
  switch (wordSet)
  {
  case WORDSET_3:
    switch (index)
    {
    case One:
      msg[0] = 110;
      break;

    case Two:
      msg[0] = 115;
      break;

    case Three:
      {
        locked = EEPROM.read(addr);
        if (locked)
        {
          msg[0] = 121;
          locked = false;
        }

        else if(!locked)
        {
          msg[0] = 120;
          locked = true;
        }
        EEPROM.write(addr, locked);
        break;
      }
    }
    break;
  }

  for (int i = 0; i < 3; i++)
  {
    radio.write(msg,1);
  }
}




















