/**
 * August 11, 2020
 * Battle Bot Radio Reciever
 * Gareth Koch
 * 
 * nRF24 Wiring
 * GND  -----  GND
 * VCC  ----- 3.3V
 * CE   -----    8
 * CSN  -----    9
 * SCK  -----   13
 * MOSI -----   11
 * MISO -----   12
 * 
 * Joystick Wiring
 * GND  -----  GND
 * VCC  -----  +5V
 * VRX  -----   A0
 * VRY  -----   A1
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const byte address[6] = {'T', 'A', 'L', 'E', 'K', 'K'};

// Create radio and set CE and CSE pins to 8 and 9 respectively
RF24 radio(8, 9);

struct Joystick
{
  byte x;
  byte y;
};

// Variable of above struct
Joystick joystick;

void setup()
{
  Serial.begin(9600);
  
  // Start radio
  radio.begin();
  Serial.println("Radio begin");
  // Set data rate
  radio.setDataRate(RF24_250KBPS);
  Serial.println("Data rate: 250kbps");
  // Set address
  radio.openWritingPipe(address);
  Serial.print("Address set");

  // Set channel values to "middle" of 0-255
  joystick.x = 127;
  joystick.y = 127;
}

void loop()
{
  joystick.x = map (analogRead(A0), 0, 1023, 0, 255);
  joystick.y = map (analogRead(A1), 0, 1023, 0, 255);

  radio.write(&joystick, sizeof(Joystick));
  Serial.print("x: ");
  Serial.print(joystick.x);
  Serial.print(" y: ");
  Serial.print(joystick.y);
  Serial.print(" A0: ");
  Serial.print(analogRead(A0));
  Serial.print(" A1: ");
  Serial.println(analogRead(A1));
}
