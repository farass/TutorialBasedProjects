// ------------------- blog.make2explore.com---------------------------------------------------------------//
// Project           - Secure Door lock using RFID, PassCode, Fingerprint and Android App method
// Created By        - admin@make2explore.com
// Version - 1.0
// Software Platform - C/C++, Arduino IDE, Libraries
// Hardware Platform - Arduino Nano, 16x2LCD display, RC522 RFID Reader module, HC-05 Blutooth module, 
//                     4x4 kepad, Solenoid Lock, Relay Module etc.                 
// Sensors Used      - RC522 RFID Reader, GT511C3 Fingerprint Sensor
// Last Modified     - 24/02/2020
// -------------------------------------------------------------------------------------------------------//


#include <SPI.h>

#include <MFRC522.h>

#include <Wire.h>

#include <LiquidCrystal_I2C.h>

#include <Keypad.h>

#include<EEPROM.h>


#include "FPS_GT511C3.h"

#include "SoftwareSerial.h"


FPS_GT511C3 fps(10, 11); // (Arduino SS_RX = pin 24, Arduino SS_TX = pin 25)

int ID=0;

int relPin;

int state_bt=1;

int state=0;

byte  COD[10];

byte  AUX[10];

int k=0;

String accessCode="*123456#";

String codpairing="*654321#";

String resetID="*789789#"; 

//RFID

#define RST_PIN 48

#define SS_PIN 53 

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

//#define NEW_UID {0x50, 0xEF, 0xB4, 0x4F}

MFRC522::MIFARE_Key key;



//LCD

LiquidCrystal_I2C  lcd(0x27,16,2);



//KEYPAD

const byte numRows= 4; //number of rows on the keypad

const byte numCols= 4; //number of columns on the keypad



char keymap[numRows][numCols]=

{

{'1', '2', '3', 'A'},

{'4', '5', '6', 'B'},

{'7', '8', '9', 'C'},

{'*', '0', '#', 'D'}

};


#define wipeB A8     // Button pin for WipeMode
#define relayPin A7  // RelayPin
#define RedLED A0
#define GreenLED A1
#define BlueLED A2

//Code that shows the the keypad connections to the arduino terminals
byte rowPins[numRows] = {2,3,4,5}; //Rows 0 to 3
byte colPins[numCols]= {6,7,8,9}; //Columns 0 to 3


//initializes an instance of the Keypad class

Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);



void setup() {

  fps.Open();         //send serial command to initialize fps
  fps.SetLED(true);   //turn on LED so fps can see fingerprint

  pinMode(RedLED,OUTPUT);

  digitalWrite(RedLED,LOW);
  

  pinMode(GreenLED,OUTPUT);

  digitalWrite(GreenLED,LOW);
  

  pinMode(BlueLED,OUTPUT);

  digitalWrite(BlueLED,LOW);
  

  pinMode(relayPin,OUTPUT);

  digitalWrite(relayPin,HIGH);
  

  pinMode(wipeB, INPUT_PULLUP);   // Enable pin's pull up resistor

  //RFID

  Serial.begin(9600);            // Initialize serial communications with the PC

  while (!Serial);               // Do nothing if no serial port is opened

  Serial1.begin(9600);
  
  SPI.begin(); // Init SPI bus

  mfrc522.PCD_Init();     // Init MFRC522 card



 for (byte i = 0; i < 6; i++) {        

  key.keyByte[i] = 0xFF;

 }


lcd.init();

lcd.backlight();

lcd.clear();

lcd.setCursor(0,0);

lcd.print( "LOCKED !!!" );

digitalWrite(RedLED,HIGH);

}


//-----------------------This function will read the code stored RFID----------------------------------//

void  readNFC(){

 for (byte i = 0; i<(mfrc522.uid.size); i++) {  // the  UID

   COD[i]=mfrc522.uid.uidByte[i];

 }

}


//---------------------------Enrolling RFID Card--------------------------------------------------------//

void pairNFC(){

  long  r=0;

  int c=0;

  for(int i=1;i<=EEPROM.read(0);i++){                       //The UID cannot be stored on

      switch(i%4){                                                     // one variable, it was needed to be

      case 1 :{AUX[0]=EEPROM.read(i); break;}     // split

      case 2 :{AUX[1]=EEPROM.read(i); break;}

      case 3 :{AUX[2]=EEPROM.read(i); break;}

      case 0 :{AUX[3]=EEPROM.read(i); break;}

}

if((i)%4==0)

   {//Serial.println(r);

    if( AUX[0]== COD[0] && AUX[1]==COD[1] && AUX[2]==COD[2] && AUX[3]==COD[3] ){                                      //Verify if the code is in EEPROM

         lcd.clear();

         lcd.setCursor(0,0);

         lcd.print("CARD ALREADY IN");

         lcd.setCursor(0,1);

         lcd.print("SYSTEM");

         delay(2000);

         c=1;

      break;}

  }

  }



if(c==0){
  int aux2=EEPROM.read(0);

  EEPROM.write(aux2+1,COD[0]);  //Writing code in EEPROM

  EEPROM.write(aux2+2,COD[1]);

  EEPROM.write(aux2+3,COD[2]);

  EEPROM.write(aux2+4,COD[3]);



  aux2=aux2+4; // Position for a new code

  EEPROM.write(0,0);

  EEPROM.write(0,aux2);    

  lcd.clear();

  lcd.setCursor(0,0);

  lcd.print("CARD ENROLLED");
  digitalWrite(BlueLED,HIGH); //The red blue will be on
  delay(1000);
  digitalWrite(BlueLED,LOW); //The red blue will be on
  }   

 }


//-------------------------------ValidateRFID saved code in EEPROM--------------------------------------//
boolean validationNFC(){

  boolean c=false;

  for(int i=1;i<=EEPROM.read(0);i++){   //Read the EEPROM

      switch(i%4){

      case 1 :{AUX[0]=EEPROM.read(i); break;}

      case 2 :{AUX[1]=EEPROM.read(i); break;}

      case 3 :{AUX[2]=EEPROM.read(i); break;}

      case 0 :{AUX[3]=EEPROM.read(i); break;}

    }

if((i)%4==0)

   {

    if( (AUX[0]==COD[0]) && (AUX[1]==COD[1]) && (AUX[2]==COD[2]) &&  (AUX[3]==COD[3]))

      c=true; //Verify if the code is in EEPROM and make flag=true;

     }

}       

  return c;  

}


//-----------------------------We type a code on keypad and this will be compared------------------------------
int compareCODE(String a)

{ 
  //with the accessCode;

  if(a.equals(accessCode)){

    return 1;
    }
    
    else if(a.equals(codpairing)){
    
     
    return 2;
    }

    else if(a.equals(resetID)){
    
    return 3;
    }

    else return 0;  

}


//-----------------------display on the LCD the code typed on KeyPad with 'X'---------------------------------------
String takeCode(char x)

{ char vec[10];

 vec[0]=x;

 lcd.setCursor(0,0);

 lcd.clear();

 lcd.print('X');

 for(int i=1;i<8;i++)

    {vec[i]=myKeypad.waitForKey(); //Waits for 8 keys to be pressed and after that  

     lcd.print('X');} //is taking the decision

 vec[8]=NULL;

 String str(vec);

 return str;

}  



void loop() {

  if (fps.IsPressFinger())
  {
    fps.CaptureFinger(false);
    int id = fps.Identify1_N();
    
    if (id <200) //<- change id value depending model you are using
    {//if the fingerprint matches, provide the matching template ID
      //Serial.print("Verified ID:");
      //Serial.println(id);
    lcd.clear();

    lcd.setCursor(0,0);

    lcd.print( "UNLOCKED" );

    digitalWrite(relayPin,LOW);

    digitalWrite(RedLED,LOW); //The red LED will be off

    digitalWrite(GreenLED,HIGH); //The green LED will be on

    //tone(6,3000,5010); //The buzzer will make a sound

    delay(5000); //After 5 seconds the system will be blocked

    digitalWrite(relayPin,HIGH);

    digitalWrite(RedLED,HIGH);

    digitalWrite(GreenLED,LOW);

    lcd.setCursor(0,0);

    lcd.clear();

    lcd.print( "LOCKED !!!" );
      
    }

  }

  //Start BT autentification
if(Serial1.available())
{
char c=Serial1.read();
switch (state_bt) {
 
case 1:
if(c=='*') state_bt=2;
else state_bt=1;
break;
 
case 2:
if(c=='1') state_bt=3;
else state_bt=1;
break;
 
case 3:
if(c=='2') state_bt=4;
else state_bt=1;
break;
 
case 4:
if(c=='3') state_bt=5;
else state_bt=1;
break;
 
case 5:
if(c=='4') state_bt=6;
else state_bt=1;
break;
 
case 6:
if(c=='5') state_bt=7;
else state_bt=1;
break;
 
case 7:
if(c=='6') state_bt=8;
else state_bt=1;
break;
 
case 8:
if(c=='#') state_bt=9;
else state_bt=1;
break;
 
case 9:
 
    lcd.clear();

    lcd.setCursor(0,0);

    lcd.print( "UNLOCKED" );

    digitalWrite(relayPin,LOW);

    digitalWrite(RedLED,LOW); //The red LED will be off

    digitalWrite(GreenLED,HIGH); //The green LED will be on

    //tone(6,3000,5010); //The buzzer will make a sound

    delay(5000); //After 5 seconds the system will be blocked

    digitalWrite(relayPin,HIGH);

    digitalWrite(RedLED,HIGH);

    digitalWrite(GreenLED,LOW);

    lcd.setCursor(0,0);

    lcd.clear();

    lcd.print( "LOCKED !!!" );

state_bt=1;
break;
 
default:
 
break;
}
}
  
  switch(state){

  case 0: {

    mfrc522.PCD_Init();

    if (  mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() ){

   

    readNFC(); //It will read the card and it will search for UID in its

    if(validationNFC()) //memory

       { state=1;

        lcd.clear();

        lcd.setCursor(0,0);

        lcd.print( "VALID RFID CARD" ); //The door will be opened
        

        lcd.setCursor(0,1);

        lcd.print( "Access Granted" ); //The door will be opened

        delay(2000);

        return;

       }

    else{

         lcd.clear();

         lcd.setCursor(0,0);

         lcd.print( "INVALID ID CARD" ); //If the code was wrongblocked

         lcd.setCursor(0,1);

         lcd.print( "Access Denied" ); //The door will be opened

         delay(2000);

         lcd.setCursor(0,0);

         lcd.clear();

         lcd.print( "LOCKED !!!" );

         return;

        }

    }      

 

char c=myKeypad.getKey();

  if(c != NO_KEY){

   

String codcurent=takeCode(c);

int A=compareCODE(codcurent);

  if(A==0){                                //A is a variable that stores the current code

   lcd.clear();
   
   lcd.setCursor(0,0);
   lcd.print("INVALID CODE");

   lcd.setCursor(0,1);
   lcd.print( "Access Denied" ); //The door will be opened
   

   delay(2000);

   lcd.setCursor(0,0);

   lcd.clear();

   lcd.print("LOCKED !!!");

   return;

}

  if(A==1){

   lcd.clear();
   lcd.setCursor(0,0);
   lcd.print( "VALID CODE " );

   lcd.setCursor(0,1);
   lcd.print( "Access Granted" ); //The door will be opened

   delay(2000);

   state = 1;

   return;

}

  if(A==2){

         state=2;

         lcd.clear();

         lcd.setCursor(0,0);

         lcd.print( "Enrolling..." );
         digitalWrite(A0,HIGH); //The blue led will be on

      delay(2000);

    return;}


   if(A==3){

         state=3;

         lcd.clear();

         lcd.setCursor(0,0);

         lcd.print( "RESET RFID" );
         lcd.setCursor(0,1);
         lcd.print( "Record" );
         
         digitalWrite(A0,HIGH); //The blue led will be on

      delay(2000);

    return;}

  }

  }

break;



 case 1:{

    lcd.clear();

    lcd.setCursor(0,0);

    lcd.print( "UNLOCKED" );

    digitalWrite(relayPin,LOW);

    digitalWrite(RedLED,LOW); //The red LED will be off

    digitalWrite(GreenLED,HIGH); //The green LED will be on

    delay(5000); //After 5 seconds the system will be blocked

    digitalWrite(relayPin,HIGH);

    digitalWrite(RedLED,HIGH);

    digitalWrite(GreenLED,LOW);

    state=0;

    lcd.setCursor(0,0);

    lcd.clear();

    lcd.print( "LOCKED !!!" );

return;

    }



  case 2:{

   mfrc522.PCD_Init();

      if (  mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() ){

      readNFC();

      pairNFC();

      state=0;

      delay(2000);

      lcd.clear();

      lcd.setCursor(0,0);

      lcd.print( "LOCKED !!!" );   }

 

break;

}

 case 3:{

   mfrc522.PCD_Init();

      clearRecord();

      state=0;

      delay(2000);

      lcd.clear();

      lcd.setCursor(0,0);

      lcd.print( "LOCKED !!!" );   }

      break;

}

}

//--------------------------------Check for Reset Button is pressed or Not------------------------------------------//
bool monitorWipeButton(uint32_t interval) {
  uint32_t now = (uint32_t)millis();
  while ((uint32_t)millis() - now < interval)  {
    // check on every half a second
    if (((uint32_t)millis() % 500) == 0) {
      if (digitalRead(wipeB) != LOW)
        return false;
    }
  }
  return true;
}

//--------------------------------------RESET RFID Record-------------------------------------------------------------//
void clearRecord()
{
    digitalWrite(A0, HIGH); // Red Led stays on to inform user we are going to wipe
    lcd.setCursor(0, 0);
    lcd.print("Button Pressed");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("This will remove");
    lcd.setCursor(0, 1);
    lcd.print("all records");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("You have 3 ");
    lcd.setCursor(0, 1);
    lcd.print("secs to Cancel");
    delay(1000);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unpres to cancel");
    lcd.setCursor(0, 1);
    lcd.print("Counting: ");
    lcd.setCursor(10, 1);
    lcd.print("1");
    delay(1000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unpres to cancel");
    lcd.setCursor(0, 1);
    lcd.print("Counting: ");
    lcd.setCursor(10, 1);
    lcd.print("2");
    delay(1000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unpres to cancel");
    lcd.setCursor(0, 1);
    lcd.print("Counting: ");
    lcd.setCursor(10, 1);
    lcd.print("3");
    delay(1000);
    
    bool buttonState = monitorWipeButton(3000); // Give user enough time to cancel operation
    if (buttonState == true && digitalRead(wipeB) == LOW) {

       lcd.print("Wiping EEPROM...");
       for (uint16_t x = 0; x < EEPROM.length(); x = x + 1) {    //Loop end of EEPROM address
        if (EEPROM.read(x) == 0) {              //If EEPROM address 0
          // do nothing, already clear, go to the next address in order to save time and reduce writes to EEPROM
        }
        else {
          EEPROM.write(x, 0);       // if not write 0 to clear, it takes 3.3mS
        }

      }

      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wiping Done");
      // visualize a successful wipe
      digitalWrite(BlueLED, LOW);
      delay(200);
      digitalWrite(BlueLED, HIGH);
      delay(200);
      digitalWrite(BlueLED, LOW);
      delay(200);
      digitalWrite(BlueLED, HIGH);
      delay(200);
      digitalWrite(BlueLED, LOW);
      
    } 
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wiping Cancelled"); // Show some feedback that the wipe button did not pressed for 10 seconds
      delay(2000);
      digitalWrite(BlueLED, LOW);
      // If button still be pressed, wipe EEPROM
       }

  }

// ------------------- blog.make2explore.com---------------------------------------------------------------//