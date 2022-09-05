// 时间：2022年4月28日
// 作者：梁帅
// 程序：CoCube蓝牙遥控程序
// 功能：前后左右移动、WS2812显示、IP5306唤醒

#include <Arduino.h>
#include "BluetoothSerial.h"

//esp32的计时器，用于唤醒IP5306
#include <Ticker.h>;
Ticker tickerkey; //声明Ticker对象

//WS2812灯板库
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define EC_L 25
#define EC_R 17

#define LEFT_AHEAD 18
#define LEFT_BACK 19
#define RIGHT_AHEAD 32
#define RIGHT_BACK 33
#define KEY 23

#define WS2812PIN 16

Adafruit_NeoPixel strip = Adafruit_NeoPixel(64, WS2812PIN, NEO_GRB + NEO_KHZ800);

//0428 新增 Apriltag图案
char matrix_0[] = "1111111110000001100000011011110110100101100101011000000111111111";
//Aruco ID = 1
char aruco1[] = "1111111110000001100000011011110110100101100101011000000111111111";
//Bilibili
char bilibili[] = "0100001000100100011111101000000110100101101001011000000101111110";
//点赞
char dianzan[] = "0000000000110000001100000011100011111101111111011111110101111101";
//投币
char toubi[] = "0111111011000011111001111100001110100101101001011110011101111110";
//转发
char zhuanfa[] = "0001000000110000011111001111111001111111001100110001000100000000";



int matrix_color[8][8][3] = 
  {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 
  {{0, 0, 0}, {6, 30, 216}, {6, 30, 216}, {6, 30, 216}, {6, 30, 216}, {6, 30, 216}, {6, 30, 216}, {0, 0, 0}}, 
  {{0, 0, 0}, {0, 0, 0}, {6, 30, 216}, {6, 30, 216}, {6, 30, 216}, {6, 30, 216}, {0, 0, 0}, {0, 0, 0}}, 
  {{0, 0, 0}, {0, 0, 0}, {6, 30, 216}, {6, 30, 216}, {6, 30, 216}, {6, 30, 216}, {0, 0, 0}, {0, 0, 0}}, 
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {6, 30, 216}, {6, 30, 216}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};

// 收藏
int shoucang[8][8][3] = 
{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 
{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {219, 150, 18}, {219, 150, 18}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 
{{0, 0, 0}, {0, 0, 0}, {110, 75, 9}, {219, 150, 18}, {219, 150, 18}, {110, 75, 9}, {0, 0, 0}, {0, 0, 0}}, 
{{110, 74, 9}, {219, 150, 18}, {219, 150, 18}, {219, 150, 18}, {219, 150, 18}, {219, 150, 18}, {219, 150, 18}, {162, 107, 9}}, 
{{0, 0, 0}, {164, 112, 14}, {219, 150, 18}, {219, 150, 18}, {219, 150, 18}, {219, 150, 18}, {218, 148, 17}, {0, 0, 0}}, 
{{0, 0, 0}, {0, 0, 0}, {219, 150, 18}, {219, 150, 18}, {219, 150, 18}, {219, 150, 18}, {0, 0, 0}, {0, 0, 0}}, 
{{0, 0, 0}, {0, 0, 0}, {219, 150, 18}, {219, 150, 18}, {219, 150, 18}, {219, 150, 18}, {0, 0, 0}, {0, 0, 0}}, 
{{0, 0, 0}, {0, 0, 0}, {219, 150, 18}, {0, 0, 0}, {0, 0, 0}, {219, 150, 18}, {0, 0, 0}, {0, 0, 0}}};



char incomingByte = 'P';
BluetoothSerial SerialBT;

//中断，每隔9s把IP5310的Key拉高一次，保证正常供电。
void flash() //中断处理函数
{                        
  digitalWrite(KEY, HIGH);
  delayMicroseconds(100000);//Key电平保持需要在60ms-2s内，设置为70ms
  digitalWrite(KEY, LOW);
}

void setup() {
  Serial.begin(9600);
  SerialBT.begin("ESP32test3"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  pinMode(KEY,OUTPUT);
  pinMode(LEFT_AHEAD,OUTPUT);
  pinMode(LEFT_BACK,OUTPUT);
  pinMode(RIGHT_AHEAD,OUTPUT);
  pinMode(RIGHT_BACK,OUTPUT);
  digitalWrite(LEFT_AHEAD,LOW);
  digitalWrite(LEFT_BACK,LOW);
  digitalWrite(RIGHT_AHEAD,LOW);
  digitalWrite(RIGHT_BACK,LOW);
//  pinMode(EC_L,INPUT);
//  pinMode(EC_R,INPUT);
  

  tickerkey.attach(9, flash);   //每9秒调用flash

  //WS2812灯板部分
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
  strip.begin();
  strip.setBrightness(5);
  strip.show(); // Initialize all pixels to 'off'
}


void loop() {
//  Serial.println(digitalRead(EC_L));
//  Serial.println(digitalRead(EC_R));
//  Serial.println("");
  
  if (Serial.available()) {
    SerialBT.write(Serial.read());
     incomingByte = Serial.read();
     Serial.println(incomingByte);
  }
  if (SerialBT.available()) {
    incomingByte = SerialBT.read();
    //Serial.write(SerialBT.read());
 if(incomingByte == 'P') {Serial.println("park"); park();}
 else if(incomingByte == 'A') {Serial.println("ahead"); ahead();}
 else if(incomingByte =='L') {Serial.println("trun left"); turnLeft();}
 else if(incomingByte =='R') {Serial.println("turn right"); turnRight();}
 else if(incomingByte =='B') {Serial.println("go back"); back();}
 else if(incomingByte =='W') {Serial.println("WS2812 Begin"); colorWipe(aruco1, strip.Color(100,100,100), 1);}
 else if(incomingByte =='S') {Serial.println("WS2812 Stop"); strip.clear();strip.show();}
 else if(incomingByte =='O') {Serial.println("Power Off"); powerOff();}
 else if(incomingByte =='C') {Serial.println("MultiColor"); multiColor(matrix_color);}
 //点赞、投币、收藏、转发、小电视
  else if(incomingByte =='1') {Serial.println("MultiColor"); colorWipe(dianzan, strip.Color(42, 149, 208), 1);}
  else if(incomingByte =='2') {Serial.println("MultiColor"); colorWipe(toubi, strip.Color(42, 149, 208), 1);}
  else if(incomingByte =='3') {Serial.println("MultiColor"); multiColor(shoucang);}
  else if(incomingByte =='4') {Serial.println("MultiColor"); colorWipe(zhuanfa, strip.Color(42, 149, 208), 1);}
  else if(incomingByte =='5') {Serial.println("MultiColor"); colorWipe(bilibili, strip.Color(42, 149, 208), 1);}
  else if(incomingByte =='6') {Serial.println("MultiColor"); colorWipe(bilibili, 0xcc0000, 1);}
  delay(20);}
}

void park(){
  digitalWrite(LEFT_AHEAD,LOW);
  digitalWrite(LEFT_BACK,LOW);
  digitalWrite(RIGHT_AHEAD,LOW);
  digitalWrite(RIGHT_BACK,LOW);
}

void ahead(){
  digitalWrite(LEFT_AHEAD,LOW);
  digitalWrite(LEFT_BACK,HIGH);
  digitalWrite(RIGHT_AHEAD,HIGH);
  digitalWrite(RIGHT_BACK,LOW);
  }

void turnLeft(){
  digitalWrite(LEFT_AHEAD,HIGH);
  digitalWrite(LEFT_BACK,LOW);
  digitalWrite(RIGHT_AHEAD,HIGH);
  digitalWrite(RIGHT_BACK,LOW);
  }
  
void turnRight(){
  digitalWrite(LEFT_AHEAD,LOW);
  digitalWrite(LEFT_BACK,HIGH);
  digitalWrite(RIGHT_AHEAD,LOW);
  digitalWrite(RIGHT_BACK,HIGH);
  }
  
void back(){
  digitalWrite(LEFT_AHEAD,HIGH);
  digitalWrite(LEFT_BACK,LOW);
  digitalWrite(RIGHT_AHEAD,LOW);
  digitalWrite(RIGHT_BACK,HIGH);
  }

//void colorWipe(uint32_t color, int wait) {
//  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
//    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
//                              //  Update strip to match
//    //delay(wait);                           //  Pause for a moment
//  }
//  strip.show();
//}


void colorWipe(char matrix[64], uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    if (matrix[i] == '1') 
    {
      strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    }
    else {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  }
  strip.show();
}


// 让灯板显示彩色。注意两点：CV2中是BGR，要转为Arduino中的RGB；WS2812灯板硬件左右反向，需要调整。
void multiColor(int color[8][8][3]){
  for(int i = 0; i <8 ; i++ ){
    for(int j = 0; j < 8; j++){
      strip.setPixelColor(i*8+j, strip.Color(color[i][7-j][2],color[i][7-j][1],color[i][7-j][0]));
    }
  }
  strip.show();
}

//强制关机
void powerOff(){
  digitalWrite(KEY, HIGH);
  delay(3000);
}
