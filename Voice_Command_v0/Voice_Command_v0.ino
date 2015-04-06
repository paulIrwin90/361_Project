/********************************************************
* This is the initial voice command file with the display 
* implemented.
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
#define  TFT_CS     10		// CS
#define  TFT_RESET   9		// RESET
#define  RF_CE       8          // 
#define  RF_CSN      7          // 
#define  VR_RX       6          // 
#define  VR_TX       5          // 

#define  LLED        2          // Listening LED

//#define  LIGHT       4          // Output to light
//#define  DOOR        3          // Output to door
//#define  FAN         2          // Output to fan

// Color definitions
#define	 BLACK      0x0000
#define	 BLUE       0x001F
#define	 RED        0xF800
#define	 GREEN      0x07E0
#define  CYAN       0x07FF
#define  MAGENTA    0xF81F
#define  YELLOW     0xFFE0  
#define  WHITE      0xFFFF
#define  TURQUOISE  0x33CC

// Instances of Objects
Adafruit_HX8340B display(TFT_RESET, TFT_CS); // Set up TFT Display
SoftwareSerial port(VR_RX, VR_TX);           // Arduino RX pin, TX pin
EasyVR easyvr(port);
RF24 radio(RF_CE, RF_CSN);

// Global Variables
int fanState = 0;
int doorState = 0;
int lightState = 0;
bool drawn = 0;
int8_t wordSet, idx;
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
  display.begin();    // LCD screen
  radio.begin();
  radio.openWritingPipe(pipe);

  drawLogo();         // Arduino splash Screen
  setupPins();

  if (easyvr.detect())
  {
    display.setTextSize(2);
    display.setCursor(15,80);           
    display.setTextColor(WHITE);
    display.println("EasyVR detected!"); // draw detection
    delay(2000);
    display.setCursor(15,80);          
    display.setTextColor(BLACK);
    display.println("EasyVR detected!"); // erase detection
  }

  easyvr.setTimeout(5);      // Timeout after 5 seconds of inactivity
  easyvr.setLanguage(0);     // English
  wordSet = EasyVR::TRIGGER; //<-- start wordSet (customize)

  drawMenu();
}

/********************************************************
* Program
********************************************************/
void loop()
{
  Serial.print("Say a command in Wordset ");
  Serial.println(wordSet);
  easyvr.recognizeWord(wordSet);

  do 
  {
    //DISPLAY THE DOTS FOR WAITING COMMAND
    idle();
  }
  while (!easyvr.hasFinished());
  idx = easyvr.getWord();

  // Understand trigger word, built-in trigger (ROBOT)
  if (wordSet == 0 && idx >= 0)
  {
    digitalWrite(8,HIGH);       // turn on LED
    
    display.setTextSize(1);
    display.setCursor(10, 160);  
    display.setTextColor(BLACK);
    display.println("Waiting For Trigger"); // Erase 
    
    display.setCursor(10, 160);
    display.setTextColor(WHITE);
    display.println("Listening For Command"); // Waiting on a command
    wordSet = WORDSET_1;
    return;
  }

  // Print to serial monitor which command was given
  if (idx >= 0)
  { 
    // print debug message
    uint8_t train = 0;
    char name[32];
    Serial.print("Command: ");
    Serial.print(idx);

    if (easyvr.dumpCommand(wordSet, idx, name, train))
    {
      Serial.print(" = ");
      Serial.println(name);
    }
    
    else
      Serial.println();
      
    // perform some action
    action();
  }

  // whenever nothing is understood
  else // errors or timeout
  {
    if (easyvr.isTimeout())
    {
      digitalWrite(8,LOW);          // turn off LED
      display.setCursor(10, 160);
      display.setTextColor(BLACK);
      display.println("Listening For Command"); // erase
      
      display.setCursor(10, 160);
      display.setTextColor(WHITE);
      display.println("Waiting For Trigger");  // draw

      Serial.println("Timed out, try again...");
    }
    int16_t err = easyvr.getError();
    if (err >= 0)
    {
      digitalWrite(8,LOW);         // turn off LED
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
    switch (idx)
    {
      case Robot:
        wordSet = WORDSET_1;
        break;
    }
    break;

  case WORDSET_1:
    switch (idx)
    {
      case Action:
        digitalWrite(5,HIGH);
        break;
        
      case Move:
        digitalWrite(5,LOW);
        break;
    }
    break;
  }
}

/********************************************************
* Draw the Arduino Logo
********************************************************/
void drawLogo()
{
  display.fillScreen(BLACK);

  // DISPLAY THE CIRCLES - x coord, y coord, radius, color

  // DRAW THE BIG CIRCLE
  display.fillCircle(110,109,60,TURQUOISE);

  // DRAW THE SMALL CIRCLES INSIDE
  display.fillCircle(110,85,25,WHITE);
  display.fillCircle(110,85,18,TURQUOISE);
  display.fillCircle(110,133,25,WHITE);
  display.fillCircle(110,133,18,TURQUOISE);

  // DISPLAY THE VOICE RECOGNITION
  display.setRotation(display.getRotation()+1);
  display.setTextSize(2);
  display.setCursor(5,135);
  display.println("Voice Recognition");
  display.setCursor(5,155);
  display.print("     Software");

  // DISPLAY THE - & +
  display.setCursor(128,59);
  display.print("+");
  display.setCursor(80,59);
  display.print("-");
  delay(3000);
  display.fillScreen(BLACK);
}

/********************************************************
* Draw the menu
********************************************************/
void drawMenu()
{ 
  // Draw the lines
  display.setTextSize(1);
  display.setTextColor(WHITE);


  // WHILE THE MENU BUTTON IS PRESSED, MONITOR DEVICES & DISPLAY MENU
  display.setTextSize(1);
  display.drawLine(68,0,68, 150, WHITE);
  display.drawLine(148,0,148, 150, WHITE);

  // Draw Dash lines
  display.setCursor(3,27);
  for (int i = 0; i < 35; i++)
    display.print("-");

  display.setCursor(3,149);
  for (int i = 0; i < 35; i++)
    display.print("-");

  // Draw the Device Names
  display.setCursor(15,10);
  display.setTextSize(2);
  display.println("FAN");

  display.setCursor(84,10);
  display.setTextSize(2);
  display.println("DOOR");

  display.setCursor(157,10);
  display.setTextSize(2);
  display.println("LIGHT");

}


/********************************************************
*
********************************************************/
void setupPins()
{ 
  pinMode(8,OUTPUT);
  digitalWrite(8,LOW);
  pinMode(5,OUTPUT);
  digitalWrite(5,LOW);
  /*
  digitalWrite(FAN,HIGH);
  digitalWrite(DOOR,HIGH);
  digitalWrite(LIGHT,HIGH); 
  */
}

/********************************************************
*
********************************************************/
void idle()
{
  if (!drawn)
  {     
    // monitorDevice();
    display.setTextSize(1);
    display.setCursor(10, 160);
    display.println("Waiting For Trigger");
    drawn = 1;
  }
}




