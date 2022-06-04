int pwm_value;
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

  // Write output to all pwm pins at varying duty cycles
  analogWrite(3, 5);
  analogWrite(5, 50);
  analogWrite(6, 100);
  analogWrite(9, 150);
  analogWrite(10, 200);
  analogWrite(11, 250);

  Serial.begin(115200);
}

void report_adc(int channel) {
  int sensorValue = analogRead(channel);
  float voltage = sensorValue * (5.0 / 1023.0);
  String msg = "a" + String(channel) + ": ";
  Serial.print(msg);
  Serial.println(voltage);
}

void loop()

{


  // Get pulse lengths in micro seconds and spit them out to serial
  
  pwm_value = pulseIn(2, HIGH);
  Serial.write("pwm_5: ");
  Serial.println(pwm_value);
  pwm_value = pulseIn(4, HIGH);
  Serial.write("pwm_50: ");
  Serial.println(pwm_value);
  pwm_value = pulseIn(7, HIGH);
  Serial.write("pwm_100: ");
  Serial.println(pwm_value);
  pwm_value = pulseIn(8, HIGH);
  Serial.write("pwm_150: ");
  Serial.println(pwm_value);
  pwm_value = pulseIn(12, HIGH);
  Serial.write("pwm_200: ");
  Serial.println(pwm_value);
  pwm_value = pulseIn(12, HIGH);
  Serial.write("pwm_250: ");
  Serial.println(pwm_value);
  
  report_adc(A0);
  report_adc(A1);
  report_adc(A2);
  report_adc(A3);
  report_adc(A4);
  report_adc(A5);

}
