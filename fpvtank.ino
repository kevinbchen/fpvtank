#include <Servo.h>

#define SERVO_LEFT_PIN 9
#define SERVO_RIGHT_PIN 10
#define SERVO_ZERO 1480

#define LED_RED_PIN 11
#define LED_BLUE_PIN 6
#define LED_GREEN_PIN 5

#define CH1_PIN 2
#define CH2_PIN 3
#define CH3_PIN 4
#define CH4_PIN 7
#define PWM_ZERO 1500
#define PWM_MIN 1000

#define BATT_PIN A0

Servo servoLeft, servoRight;

volatile int prevPinD;
volatile int lastRiseTime[8];
volatile int pwmValue[8];

ISR(PCINT2_vect) {
  int pinD = PIND;
  int t = micros();
  for (int i = 0; i < 8; i++) {
    int value = pinD & (1 << i);
    int oldValue = prevPinD & (1 << i);
    if (value == oldValue) continue;
    
    if (value == 0) {
      pwmValue[i] = t - lastRiseTime[i];
    } else {
      lastRiseTime[i] = t;
    }
  }
  prevPinD = pinD;
}

void setServo(Servo& servo, int pin, int value) {
  if (abs(value) > 50) {
    if (!servo.attached()) {
      servo.attach(pin);
    }
    servo.writeMicroseconds(SERVO_ZERO + value);
  } else {
    if (servo.attached()) {
      servo.detach();
    }
  }
}
void setLED(float f, float intensity) {
  int r = 255, g = 255, b = 255;  
  if (f > 0.1f) {
    f = (f - 0.1f) / 0.9f;
    float a = (1 - f) / 0.25;
    int x = floor(a);
    int y = floor(255 * (a - x));
    switch (x) {
        case 0: r=255;g=y;b=0;break;
        case 1: r=255-y;g=255;b=0;break;
        case 2: r=0;g=255;b=y;break;
        case 3: r=0;g=255-y;b=255;break;
        case 4: r=0;g=0;b=255;break;
    }
  }  
  analogWrite(LED_RED_PIN, r * intensity);
  analogWrite(LED_GREEN_PIN, g * intensity);
  analogWrite(LED_BLUE_PIN, b * intensity);
}

void setup() {      
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT); 
  pinMode(LED_GREEN_PIN, OUTPUT);
  
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);

  pinMode(BATT_PIN, INPUT);
 
  prevPinD = 0;  
  for (int i = 0; i < 8; i++) {
    lastRiseTime[i] = micros();
    pwmValue[i] = PWM_ZERO;
  }
  
  cli();
  PCICR |= _BV(PCIE2);
  PCMSK2 |= _BV(PCINT18) |  _BV(PCINT19) |  _BV(PCINT20) |  _BV(PCINT23);
  sei(); 
}

void loop() {
  float intensity = min(max(pwmValue[CH3_PIN] - PWM_MIN, 0), 900) / 900.0f;
  float color = min(max(pwmValue[CH4_PIN] - PWM_MIN, 0), 900) / 900.0f;
  int voltage = analogRead(BATT_PIN); // 0 - 1024
  if (voltage < 3.3f / 5 * 1024) {
    color = 1.0f; // red
    intensity = 1.0f;
  }
  setLED(color, intensity);
  
  int left = pwmValue[CH1_PIN] - PWM_ZERO;
  int right = pwmValue[CH2_PIN] - PWM_ZERO;
  setServo(servoLeft, SERVO_LEFT_PIN, left);
  setServo(servoRight, SERVO_RIGHT_PIN, right);
  
  delay(20);
}
