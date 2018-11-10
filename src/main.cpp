/*
 * Layout used:
 * ------------------------------------------------------
 *             MFRC522      Arduino         HC-SR04
 *             Reader/PCD   Uno/101         Ultrasonic
 * Signal      Pin          Pin             Sensor
 * ------------------------------------------------------
 * RST/Reset   RST          9
 * SPI SS      SDA(SS)      10
 * SPI MOSI    MOSI         11 / ICSP-4
 * SPI MISO    MISO         12 / ICSP-1
 * SPI SCK     SCK          13 / ICSP-3
 * 3.3 VCC     VCC
 * 5 VCC                    VCC               VCC
 * GND         GND          GND               GND
 *                          7                 Echo
 *                          6                 Trig
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include "keyboardHelper.h"

#define __DEBUG

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

#define MY_CARD         20435
#define MY_TAG          29468


int trigPin = 6;    // Trigger
int echoPin = 7;    // Echo
long duration, cm, inches;
uint32_t timer = millis();
bool locked = false;
uint8_t buf[8] = {0};

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

/*!
 * @brief mfrc522.PICC_IsNewCardPresent() should be checked before 
 * @return the card UID
 */
unsigned long getID()
{
  unsigned long hex_num;
  hex_num =  mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] <<  8;
  hex_num += mfrc522.uid.uidByte[3];
  mfrc522.PICC_HaltA(); // Stop reading
  return hex_num;
}

void releaseKey() 
{
  buf[0] = 0;
  buf[2] = 0;
  Serial.write(buf, 8);  // Release key  
}

void setup() 
{
	Serial.begin(115200);		// Initialize serial communications with the PC
	while (!Serial);		    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			      // Init SPI bus
	mfrc522.PCD_Init();		  // Init MFRC522

//  Show details of PCD - MFRC522 Card Reader details
//	mfrc522.PCD_DumpVersionToSerial();	

   //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

}

void loop() 
{
  /*!
   * HC-SR04 sensor
   *{*/
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
 
  // Convert the time into a distance
  cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135
  
  if((inches > 80) && ( millis() - timer > 5000)  && (locked == false))//away for more than 5 sec
  {
    #ifdef __DEBUG
      Serial.println(inches);
      Serial.println("Locked");
    #endif

    //Lock computer
    buf[0] = KEY_LEFT_SHIFT; 
    buf[2] = KEY_LEFT_CTRL;
    buf[4] = KEY_POWER;
    Serial.write(buf, 8);
    releaseKey();
    locked = true;
  }
  /*}*/

  /*!
  *@note MFRC522 Reader
  *{*/
	// Look for new cards
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

	// Dump debug info about the card; PICC_HaltA() is automatically called
	#ifdef __DEBUG
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  #endif

  unsigned long uid = getID();

  //This means my card
  if(uid == MY_CARD || uid == MY_TAG && locked == true)
  {
    //Type enter first to open the login screen
    //and wait for a bit

    //Now type the password
    delay(200);
    //Keyboard.print("13");
    delay(500);
    //Keyboard.press(KEY_RETURN);
    delay(200);
    //Keyboard.releaseAll();

    //Reinitialize the counter for the proximity device
    timer = millis();

    locked = false;

    #ifdef __DEBUG
      Serial.println("Unlocked");
    #endif
  }
  /*}*/
}