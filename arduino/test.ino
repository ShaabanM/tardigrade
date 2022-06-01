

void setup()

{
  pinMode(2, INPUT);
  pinMode(3, OUTPUT);

  pinMode(4, INPUT);
  pinMode(5, OUTPUT);

  pinMode(6, OUTPUT);
  pinMode(7, INPUT);

  pinMode(8, INPUT);
  pinMode(9, OUTPUT);

  pinMode(10, OUTPUT);
  pinMode(12, INPUT);

  pinMode(11, OUTPUT);
  pinMode(13, INPUT);

  Serial.begin(115200);
}

void loop()

{

  // Write output to all pwm pins at varying duty cycles
  analogWrite(3, 0);
  analogWrite(5, 50);
  analogWrite(6, 100);
  analogWrite(9, 150);
  analogWrite(10, 200);
  analogWrite(11, 255);

  pwm_value = pulseIn(2, HIGH);
  Serial.println(pwm_value);
  pwm_value = pulseIn(4, HIGH);
  Serial.println(pwm_value);
  pwm_value = pulseIn(7, HIGH);
  Serial.println(pwm_value);
  pwm_value = pulseIn(8, HIGH);
  Serial.println(pwm_value);
  pwm_value = pulseIn(12, HIGH);
  Serial.println(pwm_value);
  pwm_value = pulseIn(12, HIGH);
  Serial.println(pwm_value);
}