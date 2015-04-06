#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

//       Name    Pin #
#define   CE       8     // purple
#define   CSN      9     // yellow


int msg[1];
RF24 radio(CE,CSN);
const uint64_t pipe = 0xE8E8F0F0E1LL;

String thisString = String();
char var;

// the setup function runs once when you press reset or power the board
void setup() 
{
  Serial.begin(57600); 
  radio.begin();
  radio.openWritingPipe(pipe);
  radio.setRetries(15,15);
  
  pinMode(8, OUTPUT);
  digitalWrite(8,HIGH);
  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);
  
  // initialize digital pin 13 as an output. 
   msg[0] = -1;
   var = -1; 
}

// the loop function runs over and over again forever
void loop() 
{
   digitalWrite(9,HIGH);
     delay(1000);
     digitalWrite(9,LOW);
  delay(400);
  while(Serial.available())
  {
    var = Serial.read();
    thisString += var;
  } 
  
  if (thisString == "unlock")
  {
      msg[0] = 101;
  }
  
  if (thisString == "lock")
  {
      msg[0] = 100;
  }
      Serial.println(msg[0]);

  thisString = "";
  
  if (msg[0] == 100 || msg[0] == 101)
  {
    Serial.print("Sending ....");
    Serial.println(msg[0]);

    for (int i = 0; i < 5; i++) 
      radio.write(msg,1);
      
      msg[0] = -1;
  } 
}


