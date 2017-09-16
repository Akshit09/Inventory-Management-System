#include <Keypad.h>
#include <SPI.h>
#include "RF24.h"
#include "printf.h"
#include <LiquidCrystal.h>
#include <MFRC522.h>
#include <EEPROM.h>
#define RST_PIN 13
#define SS_PIN 20
//
// Hardware configuration
//
// initialize the library with the numbers of the interface pins
int rftag = 0;
int tag88 = EEPROM.read(0);
int tag197 = EEPROM.read(1);/*
int tag0  = EEPROM.read(2);
int tag1 = EEPROM.read(3);
int tag2 = EEPROM.read(4);*/
void Initialize()
{
  EEPROM.write(0, 50);
  EEPROM.write(1, 50);
  tag88 = 50;
  tag197 = 50;
  /*EEPROM.write(0, 0);
  EEPROM.write(0, 0);
  */
}
void ResetRfid()
{
  //digitalWrite(40, 0);
  digitalWrite(RST_PIN, 0);
  digitalWrite(SS_PIN, 0);
}
int scanRfid()
{
  MFRC522 mfrc522(SS_PIN, RST_PIN);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  digitalWrite(40, 1);
  while(1){
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  Serial.println(mfrc522.uid.uidByte[0]);
  if(mfrc522.uid.uidByte[0]!=0){
   ResetRfid();
   SPI.end();
   SPI.endTransaction();
   return (mfrc522.uid.uidByte[0]);
   }
  }
}

LiquidCrystal lcd(22, 23, 24, 25, 26, 27);//RS,EN,D4,D5,D6,D7
const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad

//keymap defines the key pressed according to the row and columns just as appears on the keypad
char keymap[numRows][numCols]=
{
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};

//Code that shows the the keypad connections to the arduino terminals
byte rowPins[numRows] = {9,8,7,6}; //Rows 0 to 3
byte colPins[numCols]= {5,4,3,2}; //Columns 0 to 3
char pwd[4] = {'0','0','0','0'};
char pwdE[4];

//initializes an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

RF24 radio(19,18);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
typedef enum { role_ping_out = 1, role_pong_back } role_e;
// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};
// The role of the current running sketch
role_e role = role_pong_back;
int passcode()
{
  int flag = 0,j = 0, i =0;
  
  lcd.print("Enter password\n");
  //lcd.autoscroll();
  lcd.setCursor(0,1);
  while(i<4)
  {
    char keypressed = myKeypad.getKey();
    if(keypressed != NO_KEY){
      lcd.print(keypressed);
        pwdE[i] = keypressed;
        i++;
   }
  }
      if(i==4)
        for(j = 0; j<4; j++)
          if(pwd[j] == pwdE[j])
            continue;
          else
            {
              flag = 1;
              break;
            }
       lcd.clear();
        lcd.setCursor(0,0);
       if(flag == 1)
        {
          lcd.print("Access Denied");i=0;flag=passcode();}
       else
        {lcd.print("Access Granted");
        return flag;}
}
void senderSetup()
{
  role = role_ping_out;
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
}
void setup(void)
{
  Initialize();
  pinMode(40 , OUTPUT);
  digitalWrite(40, 1);
  Serial.begin(115200);
  printf_begin();
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
  radio.begin();
  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();
  radio.printDetails();
}
void radioSetup()
{
  radio.begin();
  radio.setRetries(15,15);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();
//  radio.printDetails();
}

//If key is pressed, this key is stored in 'keypressed' variable
//If key is not equal to 'NO_KEY', then this key is printed out
//if count=17, then count is reset back to 0 (this means no key is pressed during the whole keypad scan process
void lcdPrint(String info, int cursorPoint)
{
  lcd.clear();
  lcd.setCursor(0, cursorPoint);
  lcd.print(info);
}
void nrfSend(unsigned long msg)
{
   if (role == role_ping_out)
  {
     radio.stopListening();
    lcdPrint("Sending:", 0);
    lcd.print(msg);
    delay(1000);
    bool ok = radio.write( &msg, sizeof(unsigned long) );
    
    if (ok)
      printf("ok...");
    else
      printf("failed.\n\r");
    //receiverSetup();
   }
}
int Nrfreceive()
{
  if(role == role_pong_back)
  {
    if(radio.available())
    {
      
      unsigned long val;
      bool done = false;
      while(!done)
      {
        done = radio.read(&val, sizeof(unsigned long));
        lcdPrint("Received Val:", 0);
        lcd.print(val);
        delay(1000);
        return(val);
      }
    }
  }
}
    
void receiverSetup()
{
  role = role_pong_back;
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
}
int menu()
{
  delay(200);
  lcdPrint("A. Entry and Exit", 0);
  lcd.setCursor(0,1);
  lcd.print("B. Details");
  delay(2000);
  lcdPrint("C. Location", 0);
  delay(2000);
  lcdPrint("Your Choice:", 0);
  while(1){
  char option = myKeypad.getKey();
  if(option)
    lcd.print(option);
  switch (option)
  {
    case 'A':
      lcdPrint("1. Entry", 0);
      lcd.setCursor(0,1);
      lcd.print("2. Exit");
      option = 'j';
      while(1)
      {
        char option2 = myKeypad.getKey();
        switch(option2)
        {
          case '1':
            lcdPrint("Enter", 0);
            option2 = 'j';
            while(rftag==0){
              lcdPrint("no tag", 0);
              delay(100);
              rftag = scanRfid();}
            lcdPrint("tag found", 0);
            return 100;
            
          case '2':
            lcdPrint("Exit", 0);
            option2 = 'j';
            rftag = scanRfid();
            while(rftag==0){
              lcdPrint("no tag", 0);
              delay(100);
              rftag = scanRfid();}
            lcdPrint("tag found", 0);
            return 101;}
      }
    case 'B':
      for(int i = 0; i<=1; i++){
        lcdPrint("1. Pencil ", 0);
        lcd.setCursor(0, 1);
        lcd.print("2 Pen ");
        delay(1000);
        lcdPrint("3. Eraser", 0);
        lcd.setCursor(0,1);
        lcd.print("4. Scale");
        delay(1000);
        lcdPrint("5. NB", 0);
        delay(1000);}
        lcdPrint("Enter Choice:", 0);
        while(1)
          {char option2 = myKeypad.getKey();
          if(option2){
            lcd.print(option2);}
            switch (option2)
            {
              case '1':
              lcdPrint("Pencil",0);
              option2 = 'j';
              return 102;

              case '2':
                lcdPrint("Pen",1);
                option2 = 'j';
                return 103;

              case '3':
                lcdPrint("Eraser", 0);
                option2 = 'j';
                return 104;

              case '4':
                lcdPrint("Scale", 1);
                option2 = 'j';
                return 105;

              case '5':
                lcdPrint("Notebooks", 0);
                option2 = 'j';
                return 106;
             }
          }
    case 'C':
      for(int i = 0; i<=1; i++){
        lcdPrint("1. Pencil ", 0);
        lcd.setCursor(0, 1);
        lcd.print("2 Pen ");
        delay(800);
        lcdPrint("3. Eraser", 0);
        lcd.setCursor(0,1);
        lcd.print("4. Scale");
        delay(800);
        lcdPrint("5. NB", 0);
        delay(1000);}
        lcdPrint("Enter Choice:", 0);
        while(1)
          {char option2 = myKeypad.getKey();
          if(option2){
            lcd.print(option2);}
            switch (option2)
            {
              case '1':
              lcdPrint("Pencil",0);
              option2 = 'j';
              lcdPrint("Location:A1", 0);
              delay(2000);
              return 0;
              case '2':
                lcdPrint("Pen",1);
                option2 = 'j';
                lcdPrint("Location:A2", 0);
                delay(2000);
                return 0;
              case '3':
                lcdPrint("Eraser", 0);
                option2 = 'j';
                lcdPrint("Location:B1", 0);
                delay(2000);
                return 0;
              case '4':
                lcdPrint("Scale", 1);
                option2 = 'j';
                lcdPrint("Location:B2", 0);
                delay(2000);
                return 0;
              case '5':
                lcdPrint("Notebooks", 0);
                option2 = 'j';
                lcdPrint("Location:C1", 0);
                delay(2000);
                return 0;
             }
          }
           
        }
    }  
}

void updateVal()
{
  EEPROM.write(0, tag197);
  EEPROM.write(1, tag88);
}
void loop(void)
{
  int flag = passcode();
  int val =1;
  while(flag == 0)
  {
    radioSetup();
    senderSetup();
    rftag = 0;
    val = menu();
   switch (val)
      {
        case 100:
          switch(rftag){
            case 197:
              if(tag197 >= 250){
                lcdPrint("MAX Stock", 0);
                delay(1000);}
              else{
                nrfSend(1001);
                tag197 = 10 + tag197;
                lcdPrint("10 Pencils added", 0);
                updateVal();
                delay(1000);}
                break;
            case 88:
              if(tag88 >= 250){
                lcdPrint("MAX Stock", 0);
                delay(1000);}
              else{
                lcdPrint("10 Pen added", 0);
                tag88 = 10 + tag88;
                nrfSend(1002);
                updateVal();
                delay(1000);}
                break;
          }
          break;           
        case 101:
          switch(rftag){
            case 197:
              if(tag197 <= 40){
                lcdPrint("Low:", 0);
                lcd.print(tag197);}
              else{
                tag197 = tag197 - 10;
                lcdPrint("Removing 10", 0);
                updateVal();
                nrfSend(1011);}
              break;
            case 88:
              if(tag88 <= 40){
                lcdPrint("Low:", 0);
                lcd.print(tag88);}
              else{
                tag88 = tag88 - 10;
                updateVal();
                nrfSend(1012);
              }
              break;
          }
          break;
        case 102:
          delay(1000);
           nrfSend(102);
           tag197 = EEPROM.read(0);
           lcdPrint("value(Rs. 2):", 0);
           lcd.print(tag197);
           delay(2000);
           break;
           case 103:
           nrfSend(103);
           tag88 = EEPROM.read(1);
           lcdPrint("value(Rs. 10):", 0);
           lcd.print(tag88);
           delay(2000);
           break;
      /*  case 104:
           nrfSend(104);
           while(1){
            int receivedVal = Nrfreceive();
            lcdPrint("R:", 0);
            lcd.print(receivedVal);
            delay(5000);
            if(receivedVal)
            break;}*/
            
        }
    }
}
