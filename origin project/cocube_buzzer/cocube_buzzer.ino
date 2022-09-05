# define BUZZ 4

void setup() {
  // put your setup code here, to run once:
  ledcSetup(0, 20000, 10);
  ledcAttachPin(BUZZ, 0);
  }

void loop() {
  // put your main code here, to run repeatedly:
  ledcWriteTone(0, 440);
  delay(800);
  ledcWriteTone(0, 600);
  delay(500);
}
