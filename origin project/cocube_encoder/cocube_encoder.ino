#define EC_L 25
#define EC_R 17

#define LEFT_AHEAD 18
#define LEFT_BACK 19
#define RIGHT_AHEAD 32
#define RIGHT_BACK 33
#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  pinMode(LEFT_AHEAD, OUTPUT);
  pinMode(LEFT_BACK, OUTPUT);
  pinMode(RIGHT_AHEAD, OUTPUT);
  pinMode(RIGHT_BACK, OUTPUT);
  pinMode(EC_L,INPUT);
  pinMode(EC_R,INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LEFT_AHEAD, HIGH);
  digitalWrite(LEFT_BACK, LOW);
  digitalWrite(RIGHT_AHEAD, HIGH);
  digitalWrite(RIGHT_BACK, LOW);
  Serial.println(digitalRead(EC_L));
//  Serial.println("loop");
}
