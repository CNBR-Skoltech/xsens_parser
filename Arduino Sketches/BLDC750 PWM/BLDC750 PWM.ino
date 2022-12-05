long long tim = 0;
bool dir = 0, speed_dir = 1;
int speed = 0;

void setup()
{
  pinMode(2, OUTPUT); // Direction
  pinMode(3, OUTPUT); // Speed
  pinMode(4, OUTPUT); // Break
  pinMode(5, OUTPUT); // ???

  Serial.begin(115200);
}

void loop()
{
  if (millis() - tim > 10)
  {
    if (speed_dir)
      analogWrite(3, speed++);
    else
      analogWrite(3, speed--);

    if (speed == 255)
    {
      speed_dir = !speed_dir;
      digitalWrite(4, 1);
      delay(1000);
      digitalWrite(4, 0);
    }

    if (speed == 0)
    {
      speed_dir = !speed_dir;
      dir = !dir;
      digitalWrite(2, dir);
    }

    tim = millis();
  }

  Serial.print(speed);
  Serial.print(' ');
  Serial.println(dir);
}