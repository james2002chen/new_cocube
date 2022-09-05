// 时间：2022年5月1日
// 作者：梁帅
// 程序：延续自cocube_udp，在此基础上增加了#include <ArduinoJson.h>，实现了对json格式数据包的解码
// 功能：在Python上切换不同的Aruco，通过UDP发送给esp32并显示

#include <Arduino.h>

//调用WIFIUDP库
#include <WiFi.h>
#include <WiFiUdp.h>//调用WIFIUDP库
#include <ArduinoJson.h>


unsigned int localPort = 1234;      // 可以自定义端口号
WiFiUDP Udp;
char packetBuffer[255]; //buffer to hold incoming packet,

#ifndef STASSID
#define STASSID "CoCubeServer"    
#define STAPSK  "CoCube2022"
#endif
const char* ssid     = STASSID;
const char* password = STAPSK;

//esp32的计时器，用于唤醒IP5306
#include <Ticker.h>;
Ticker tickerkey; //声明Ticker对象

//WS2812灯板库
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LEFT_AHEAD 18
#define LEFT_BACK 19
#define RIGHT_AHEAD 32
#define RIGHT_BACK 33
#define KEY 23

#define BUZZ 4
#define WS2812PIN 16

Adafruit_NeoPixel strip = Adafruit_NeoPixel(64, WS2812PIN, NEO_GRB + NEO_KHZ800);

char bilibili[] = "0100001000100100011111101000000110100101101001011000000101111110";

//新增一个matrix，用于接受udp的aruco数据。
char matrix[] = "0000000000000000000000000000000000000000000000000000000000000000";

//中断，每隔9s把IP5310的Key拉高一次，保证正常供电。
void flash() //中断处理函数
{                        
  digitalWrite(KEY, HIGH);
  delayMicroseconds(100000);//Key电平保持需要在60ms-2s内，设置为70ms
  digitalWrite(KEY, LOW);
}

void setup() {
  Serial.begin(9600);
//  SerialBT.begin("ESP32test3"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  pinMode(KEY,OUTPUT);
  pinMode(LEFT_AHEAD,OUTPUT);
  pinMode(LEFT_BACK,OUTPUT);
  pinMode(RIGHT_AHEAD,OUTPUT);
  pinMode(RIGHT_BACK,OUTPUT);
  pinMode(BUZZ,OUTPUT);
  digitalWrite(LEFT_AHEAD,LOW);
  digitalWrite(LEFT_BACK,LOW);
  digitalWrite(RIGHT_AHEAD,LOW);
  digitalWrite(RIGHT_BACK,LOW);
//  蜂鸣器是无源蜂鸣器，不能这么玩。
  digitalWrite(BUZZ,HIGH);
  delay(1000);
  digitalWrite(BUZZ,LOW);

  tickerkey.attach(9, flash);   //每9秒调用flash

  //WS2812灯板部分
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
  strip.begin();
  strip.setBrightness(5);
  strip.clear();
  strip.show(); // Initialize all pixels to 'off'


  //连接WiFi部分
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Udp.begin(localPort);//打开端口
}

void loop() {
  int packetSize = Udp.parsePacket();
  
  if (packetSize) {
    int n = Udp.read(packetBuffer, 255);
    packetBuffer[n] = 0;
    Serial.print("收到数据:");
    Serial.println(packetBuffer);
    Serial.print("发送端IP：");
    Serial.print(Udp.remoteIP().toString().c_str());
    Serial.println(Udp.remotePort());

//    String line = packetBuffer;
////    Serial.println(line);
//    if(line =="W") {
//        Serial.println("Bilibili");
//        colorWipe(bilibili, strip.Color(42, 149, 208), 1);
//    }
//    else if(line =="S") {
//        Serial.println("Clear");
//        strip.clear();
//        strip.show();
//    }
    //Json部分
    String json = packetBuffer;
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);

//    matrix[] = root["matrix"];
    strcpy(matrix, root["matrix"]);
    colorWipe(matrix, strip.Color(200, 20, 20), 1);
    Serial.print("收到matrix：");
    Serial.println(matrix);
  }
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
