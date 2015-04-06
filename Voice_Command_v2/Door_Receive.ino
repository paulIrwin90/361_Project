
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <EEPROM.h> // used to store info after turned off
#include <avr/wdt.h> // used for software reset
#include <SoftReset.h> // used for software reset

RF24 radio(9, 10);

// GLOBAL VARIABLES
int msg[1];
int reset = 0;
int var;
bool locked; // value that will be stored in address below
const int IN1 = 8;
const int IN2 = 7;

int addr = 0; // address where bool lock is stored
const uint64_t pipe = 0xE8E8F0F0E1LL;

/********************************************************
 * SET UP.
 ********************************************************/
void setup(void)
{
  // PIN SETUP
  pinMode(IN1, OUTPUT);
  digitalWrite(IN1, HIGH);
  pinMode(IN2, OUTPUT);
  digitalWrite(IN2, HIGH);

  // LET'S BEGIN
  Serial.begin(57600);
  radio.begin();
  radio.openReadingPipe(1, pipe);
  radio.startListening();
  radio.enableAckPayload();
  
  delay(20); // to ensure that everything is read properly
}

/********************************************************
 * Program
 ********************************************************/
void loop()
{
  static uint32_t message_count = 0; // used for ACK
  Serial.println(msg[0]);
  delay(300); // Just to get a solid reading on the role pin

  /****************************************/
  // LET'S SEE IF ANYONE IS TALKING
  if ( radio.available() )
  {
    bool done = false;
    while (!done)
    {
      done = radio.read(msg, 1);
    }
    var = radio.getDataRate();
    Serial.print("Rate = ");
    Serial.println(var);
    // WRITE AN ACK SO THE SENDER KNOWS WE RECEIVED .. INCREASE COUNT
    if(msg[0] == 120 || msg[0] == 121)
    {
    radio.writeAckPayload( 1, &message_count, sizeof(message_count));
    ++message_count;
    }
    
  }

  /****************************************/
  // IF THE MESSAGE IS THE DESIRED ONE TURN ON OR OFF DEVICE
  if (msg[0] == 120 || msg[0] == 121)
  {
    Serial.println("COMMAND BEING EXECUTED");

    if (msg[0] == 121)
    {
      unlock();
    }
    if (msg[0] == 120)
    {
      lock();
    }
        msg[0] = 0;

    delay(100); // to ensure everything is written properly
  }

  /****************************************/
  // THIS IS JUST TO DO A SOFT RESET AFTER A CERTAIN COUNT
  if (reset == 25)
  {
    Serial.println("resetting");
    soft_restart();
  }
  reset++;
}

/********************************************************
 * UNLOCK
 ********************************************************/
void unlock()
{
  digitalWrite(IN1, LOW);
  delay(450);
  digitalWrite(IN1, HIGH);
}

/********************************************************
 * LOCK
 ********************************************************/
void lock()
{
  digitalWrite(IN2, LOW);
  delay(550);
  digitalWrite(IN2, HIGH);
}


