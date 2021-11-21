#include <Servo.h> 
// Note: servo.h library disables analogWrite on pins 9 and 10

#define INPUT_PIN A2

Servo servos[5];
const int SERVO_PINS[] = {5, 6, 9, 10, 11};

int i; // variable for loops

void zeroOutServos() 
{
  for (i = 0; i < sizeof(servos)/sizeof(*servos); i++) // would've used range-based for loop, but its not supported on C++98
  { 
    servos[i].write(0);
  }
}

void setup() {
  pinMode(INPUT_PIN, INPUT);
  
  for(i = 0; i < sizeof(servos)/sizeof(*servos); i++) 
  {
    pinMode(SERVO_PINS[i], OUTPUT);
    servos[i].attach(SERVO_PINS[i]);
  }
  
  delay(1500);
  zeroOutServos();
}

void dispensePill(int servoNumber) 
{
  if (servoNumber > 0) {
    servos[servoNumber - 1].write((servos[servoNumber - 1].read() == 180)?0:180);
    delay(1500);
  }
}

void loop()
{ 
  dispensePill(pulseIn(INPUT_PIN, HIGH) / 1000);
  
  delay(100);
}
