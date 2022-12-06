void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  delay(5000);
}

void loop() {
  if (Serial.available()){
    digitalWrite(13, Serial.read()>0);
  }
}
