void setup() {
  // put your setup code here, to run once:
  pinMode(16, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  digitalWrite(16, LOW);
  delay(1000);
  digitalWrite(16, HIGH);
}
