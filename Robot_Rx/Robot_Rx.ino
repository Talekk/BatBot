/**
   August 28, 2020
   Battle Bot Radio Reciever
   Gareth Koch

   nRF24 Wiring
   GND  -----  GND
   VCC  ----- 3.3V
   CE   -----    8
   CSN  -----    9
   SCK  -----   13
   MOSI -----   11
   MISO -----   12

   L298N Wiring
   +12V -----   9V
   GND  -----  GND
   +5V  -----  +5V
   ENA  -----   10
   IN1  -----    7
   IN2  -----    6
   IN3  -----    5
   IN4  -----    4
   ENB  -----    3

*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

// Define left motor pins
#define Len 10
#define L1 6
#define L2 7

// Define right motor pins
#define Ren 3
#define R1 4
#define R2 5

//Define servo pins
#define Servo1 2

// Set address for radio
// This must be the same for both the transmitter and receiver
const byte address[6] = {'P', 'U', 'R', 'P', 'L', 'E'};

// Create radio and set CE and CSE pins to 8 and 9 respectively
RF24 radio(8, 9);

// Joystick data format to be read
struct Joystick
{
  byte x;
  byte y;
  byte servo;
};

// Variable of above struct
Joystick joystick;

//Set up servo
Servo servo1;
int servoPos = 90;
int prevPos = 90;

int timer;
int lsatUpdate;

void setup()
{
  Serial.begin(9600);

  //Start radio
  radio.begin();
  Serial.println("Radio begin");
  // Set data rate
  radio.setDataRate(RF24_250KBPS);
  Serial.println("Data rate: 250kbps");
  // Set address
  radio.openReadingPipe(1, address);
  Serial.println("Address set");
  // Set radio to be receiver
  radio.startListening();

  // Set channel values to "middle" of 0-255
  joystick.x = 127;
  joystick.y = 127;

  // Set pinouts for motor controller
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(Len, OUTPUT);
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(Ren, OUTPUT);
}

void loop() {
  while (radio.available()) {
    // Read joystick data
    radio.read(&joystick, sizeof(Joystick));

    /*
      // Test data received
      x_value = joystick.x;
      y_value = joystick.y;
      Serial.print("x: ");
      Serial.print(x_value);
      Serial.print(" y: ");
      Serial.println(y_value);
    */

    // Actually move
    control(joystick.x, joystick.y);
    // Control actuator servo
    // servoControl(joystick.servo);
    // Use this instead for a continuous servo
    continuousServoControl(joystick.servo);
  }
}

// Check if a joystick axis is in the neutral position
bool isCentered(int value) {
  return (value > 120 && value < 135);
}

void control (int x, int y) {
  // Define forward velocity in terms of x axis
  int fwSpeed = map(x, 0, 255, -255, 255);
  // Define left and right motor velocity in terms of forward velocity
  int lSpeed = fwSpeed;
  int rSpeed = fwSpeed;

  // If joystick is in neutral position, do not move
  if (isCentered(x) && isCentered(y)) {
    lSpeed = 0;
    rSpeed = 0;
    // Turn in place if no forward velocity
  } else if (isCentered(x)) {
    lSpeed = map(y, 0, 255, -255, 255);
    rSpeed = - lSpeed;
    // Scale forward velocity of right and left motors relative to y axis
  } else {
    if (y < 120) {
      // left turn
      lSpeed = fwSpeed + map(y, 135, 255, 0, fwSpeed);
    }
    else if (y > 135) {
      rSpeed = fwSpeed + map(y, 0, 120, fwSpeed, 0);
    }
  }

  /*
    // Test motor input/output
    Serial.print("x: ");
    Serial.print(x);
    Serial.print(" ");
    Serial.print("y: ");
    Serial.print(y);
    Serial.print(" ");
    Serial.print("L: ");
    Serial.print(lSpeed);
    Serial.print(" ");
    Serial.print("R: ");
    Serial.println(rSpeed);
  */

  // Process final "speed" values to movement
  trackMove(lSpeed, rSpeed);
}

void trackMove(int lSpe, int rSpe) {
  if (lSpe > 8) {
    // Move forward
    digitalWrite(L1, HIGH);
    digitalWrite(L2, LOW);
    analogWrite(Len, lSpe);
  } else if (lSpe < -8) {
    // Move backwards
    digitalWrite(L1, LOW);
    digitalWrite(L2, HIGH);
    analogWrite(Len, -lSpe);
  } else {
    digitalWrite(L1, LOW);
    digitalWrite(L2, LOW);
    analogWrite(Len, 0);
  }

  if (rSpe > 8) {
    // Move forward
    digitalWrite(R1, HIGH);
    digitalWrite(R2, LOW);
    analogWrite(Ren, rSpe);
  } else if (rSpe < -8) {
    // Move backwards
    digitalWrite(R1, LOW);
    digitalWrite(R2, HIGH);
    analogWrite(Ren, -rSpe);
  } else {
    digitalWrite(R1, LOW);
    digitalWrite(R2, LOW);
    analogWrite(Ren, 0);
  }
}

void halt() {
  // Stop
  digitalWrite(L1, LOW);
  digitalWrite(L2, LOW);
  analogWrite(Len, 0);
  digitalWrite(R1, LOW);
  digitalWrite(R2, LOW);
  analogWrite(Ren, 0);
}

void servoControl(int dServo) {
  // raise servo while joystick up
  if (dServo > 199 && servoPos < 159) {
    servo1.attach(Servo1);
    servoPos += 2;
    servo1.write(servoPos);
    delay(10);
    // lower servo while joystick down
  } else if (dServo < 55 && servoPos > 21) {
    servo1.attach(Servo1);
    servoPos -= 2;
    servo1.write(servoPos);
    delay(10);
    // stop servo if not being used
  } else {
    servo1.detach();
  }

  // Test servo input/output
  prevPos = servo1.read();
  Serial.print("control: ");
  Serial.print(dServo);
  Serial.print(" assigned position: ");
  Serial.print(servoPos);
  Serial.print(" read position: ");
  Serial.println(prevPos);
}

void continuousServoControl(int dServo) {
  // raise servo while joystick up
  if (dServo > 199) {
    servo1.attach(Servo1);
    servoPos = 0;
    servo1.write(servoPos);
    // lower servo while joystick down
  } else if (dServo < 55) {
    servo1.attach(Servo1);
    servoPos = 180;
    servo1.write(servoPos);
    // stop servo if not being used
  } else {
    servo1.detach();
  }
  // Test servo direction
  Serial.print("control: ");
  Serial.print(dServo);
  Serial.print(" cw/ccw ");
  Serial.println(servoPos);
}
