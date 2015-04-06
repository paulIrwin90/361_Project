#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define CSN   9  // Yellow
#define CE    8  // Purple
#define relay 7

RF24 radio(CE, CSN);

// GLOBAL VARIABLES
int msg[1];
int reset = 0;
int device_state;
const uint64_t pipe = 0xE8E8F0F0E1LL;

/********************************************************
 * SET UP.
 ********************************************************/
void setup(void)
{
  // PIN SETUP
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
  
  // LET'S BEGIN
  Serial.begin(57600);  
  radio.begin();
  radio.openReadingPipe(1, pipe);
  radio.startListening();
  radio.enableAckPayload();
  delay(20); // just to ensure a good reading
}

/********************************************************
* Program
********************************************************/
void loop()
{
  static uint32_t message_count = 0; // used for ACK
  Serial.println(msg[0]);
  delay(100); // Just to get a solid reading

/****************************************/
  // LET'S SEE IF ANYONE IS TALKING
  if ( radio.available() )
  {
    bool done = false;
    while (!done)
    {
      done = radio.read(msg, 1);
      Serial.print("message is ");
      Serial.println(msg[0]);
    }

    // WRITE AN ACK SO THE SENDER KNOWS WE RECEIVED .. INCREASE COUNT
    radio.writeAckPayload( 1, &message_count, sizeof(message_count));
    ++message_count;
    Serial.print("Writing ack...");
    Serial.println(message_count);
  }

/****************************************/
  // IF THE MESSAGE IS THE DESIRED ONE TURN ON OR OFF DEVICE
  if (msg[0] == 110)
  {
    msg[0] = 0;
    Serial.println("MESSAGE BEING EXCECUTED");
    device_state = !digitalRead(relay);
    //Serial.println(device_state);
    digitalWrite(relay, device_state);
    delay(100);
  } 
}


