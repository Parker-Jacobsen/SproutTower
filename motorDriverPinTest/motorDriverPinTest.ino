
const int motor1Speed =3;
const int motor1Pin1  =4;
const int motor1Pin2  =5;

const int motor2Speed =9;
const int motor2Pin1  =6;
const int motor2Pin2  =7;

void setup() 
{
  Serial.begin(9600);     //Start Serial monitor for debugging

  pinMode(motor1Speed,OUTPUT);
  pinMode(motor1Pin1,OUTPUT);
  pinMode(motor1Pin2,OUTPUT);
  pinMode(motor2Pin1,OUTPUT);
  pinMode(motor2Pin2,OUTPUT);
  pinMode(motor2Speed,OUTPUT);

}

void loop() 
{
  digitalWrite(motor1Speed,HIGH);
  digitalWrite(motor1Pin1,HIGH);
  digitalWrite(motor1Pin2,HIGH);
  digitalWrite(motor2Pin1,HIGH);
  digitalWrite(motor2Pin2,HIGH);
  digitalWrite(motor2Speed,HIGH);
  delay(500);
  digitalWrite(motor1Speed,LOW);
  digitalWrite(motor1Pin1,LOW);
  digitalWrite(motor1Pin2,LOW);
  digitalWrite(motor2Pin1,LOW);
  digitalWrite(motor2Pin2,LOW);
  digitalWrite(motor2Speed,LOW);
  delay(500);

}
