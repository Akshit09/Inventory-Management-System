

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include <EEPROM.h>
//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
int address = 0;
byte value;
int val;
RF24 radio(9,10);

//
// Topology
//
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(115200);
  printf_begin();
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  radio.begin();
  radio.setRetries(15,15);
   radio.openReadingPipe(1,pipes[1]);
   radio.startListening();
   radio.printDetails();
   inputString.reserve(200);
   receiverSetup();
}

void receiverSetup()
{
  role = role_pong_back;
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
}

void senderSetup()
{
  role = role_ping_out;
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
}

void nrfSend(unsigned long msg)
{
  if (role == role_ping_out)
  {
    radio.stopListening();
    unsigned long time = millis();
    printf("Now sending %lu...",time);
    bool ok = radio.write( &msg, sizeof(unsigned long) );
    if (ok)
      printf("ok...");
    else
      printf("failed.\n\r");
  }
} 

int nrfReceive()
{
  if ( role == role_pong_back )
  {
     if ( radio.available() )
     { 
      unsigned long got_time;
      bool done = false;
      while (!done)
      {
        done = radio.read( &got_time, sizeof(unsigned long) );
        return got_time;        
      }      
    }
  }
}
int valueR;
void loop(void)
{
    valueR = nrfReceive();
    if(valueR==1001){
      Serial.println(valueR);
      valueR= 0;
    }   
    if(valueR==1002){
      Serial.println(valueR);
      valueR= 0;
    }   
    if(valueR==1011){
      Serial.println(valueR);
      valueR= 0;
    }   
    if(valueR==1012){
      Serial.println(valueR);
      valueR= 0;
    }   

  
   
  

}
// vim:cin:ai:sts=2 sw=2 ft=cpp
