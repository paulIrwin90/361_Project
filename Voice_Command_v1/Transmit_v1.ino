#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <EEPROM.h>

//       Name    Pin #
#define   CSN      10    // yellow
#define   CE       9    // purple

RF24 radio(CE, CSN);

int msg[1];
int reset = 0;
int addr = 0;
bool locked;
const uint64_t pipe = 0xE8E8F0F0E1LL;

String thisString = String();
char var;

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(57600);
  radio.begin();
  radio.openWritingPipe(pipe);
  radio.enableAckPayload();

  msg[0] = 0;
  var = -1;

  radio.write(msg,1);
  radio.write(msg,1);
  radio.write(msg,1);
}

// the loop function runs over and over again forever
void loop()
{
  static uint32_t message_count = 0;

  delay(400);
  while (Serial.available())
  {
    var = Serial.read();
    thisString += var;
  }

  if (thisString == "lock")
  {
    locked = EEPROM.read(addr);
    if (locked)
    {
      msg[0] = 120;
      locked = false;
    }
    else if(!locked)
    {
      msg[0] = 121;
      locked = true;
    }
    EEPROM.write(addr, locked);
  }
  
  Serial.println(msg[0]);

  thisString = "";

  if (msg[0] == 121 || msg[0] == 120)
  {
    radio.write(msg,1);
    radio.write(msg,1);

    Serial.print("Sending....");
    Serial.println(msg[0]);
  }

  delay(200);
  if (radio.isAckPayloadAvailable())
  {
    Serial.print("Acking...");
    Serial.println(message_count);
    radio.read(&message_count, sizeof(message_count));
    msg[0] = 0;
  }

  if (reset == 120)
  {
    Serial.println("resetting");
    //soft_restart();
  }

  reset++;
}





