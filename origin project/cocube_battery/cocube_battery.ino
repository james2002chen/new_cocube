#define BAT_DET 34
char buffer[30];

void setup() {
  // put your setup code here, to run once:
  pinMode(BAT_DET, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  double battery = 4.21 * analogRead(BAT_DET) / 2435;
  char str_battery[5];   //浮点类型转化存储缓冲数组
  dtostrf(battery, 3,2, str_battery);  // 3,2 表示转化的精度
  sprintf(buffer, "The battery voltage is %s V", str_battery);
  Serial.println(buffer);
  delay(1000);
}
