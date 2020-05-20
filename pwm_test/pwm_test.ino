#include <Ticker.h>
#include <WiFiUdp.h>
#include <wireless_operations.h>
#include <Servo.h>

const uint8_t LED_P1 = D1;
const uint8_t LED_P2 = D2;
const uint8_t LED_P3 = D3;
const uint8_t LED_P4 = D4;
const uint8_t FRONT_PIN_1 = D1;
const uint8_t FRONT_PIN_2 = D2;
const uint8_t REAR_PIN_1 = D3;
const uint8_t REAR_PIN_2 = D4;

const char *ssid = "sharma";
const char *password = "H0m$#@12345";

Servo ESC_FRONT_1;
Servo ESC_FRONT_2;
Servo ESC_REAR_1;
Servo ESC_REAR_2;

uint16_t led_p1_pow = 0;
const uint8_t POW_DIFF = 2;

void setup() 
{
  Serial.begin(115200);
  connect_AP(ssid, password);
//  analogWriteFreq(50);
//  analogWriteRange(255);
  ESC_FRONT_1.attach(FRONT_PIN_1, 1000, 2000);
  ESC_FRONT_2.attach(FRONT_PIN_2, 1000, 2000);
  ESC_REAR_1.attach(REAR_PIN_1, 1000, 2000);
  ESC_REAR_2.attach(REAR_PIN_2, 1000, 2000);
  calibrate_ESC();
}

void calibrate_ESC()
{
  Serial.println("Sending max pulse...");
//  analogWrite(TEST_MOTOR_PIN, MAX_PWM);
  ESC_FRONT_1.write(180);
  ESC_FRONT_2.write(180);
  ESC_REAR_1.write(180);
  ESC_REAR_2.write(180);
  delay(3000);
  Serial.println("Sending min pulse...");
    ESC_FRONT_1.write(0);
  ESC_FRONT_2.write(0);
  ESC_REAR_1.write(0);
  ESC_REAR_2.write(0);
//  analogWrite(TEST_MOTOR_PIN, MIN_PWM);
  delay(3000);
  Serial.println("ESC calibrated (check it manually)!");
}

void loop() 
{
  
}
