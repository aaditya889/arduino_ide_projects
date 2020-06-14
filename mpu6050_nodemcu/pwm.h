#include <BasicLinearAlgebra.h>
#ifndef __constants_h__
#define __constants_h__
#endif


const uint8_t FRONT_PIN_1 = D0;
const uint8_t FRONT_PIN_2 = D1;
const uint8_t REAR_PIN_1 = D2;
const uint8_t REAR_PIN_2 = D3;

Servo ESC_FRONT_1;
Servo ESC_FRONT_2;
Servo ESC_REAR_1;
Servo ESC_REAR_2;

//  function definitions
extern void send_udp(char *);

void calibrate_esc()
{
  char udp_message[150];
  send_udp("Calibrating esc...\n");
  ESC_FRONT_1.attach(FRONT_PIN_1, 1000, 2000);
  ESC_FRONT_2.attach(FRONT_PIN_2, 1000, 2000);
  ESC_REAR_1.attach(REAR_PIN_1, 1000, 2000);
  ESC_REAR_2.attach(REAR_PIN_2, 1000, 2000);
  Serial.println("Sending max pulse...");
  sprintf(udp_message, "Sending max pulse = %d...", MAX_THRUST);
  send_udp(udp_message);
  ESC_FRONT_1.write(MAX_THRUST);
  ESC_FRONT_2.write(MAX_THRUST);
  ESC_REAR_1.write(MAX_THRUST);
  ESC_REAR_2.write(MAX_THRUST);
  delay(3000);
  Serial.println("Sending min pulse...");
  sprintf(udp_message, "Sending min pulse = %d...", MIN_THRUST);
  send_udp(udp_message);
  ESC_FRONT_1.write(MIN_THRUST);
  ESC_FRONT_2.write(MIN_THRUST);
  ESC_REAR_1.write(MIN_THRUST);
  ESC_REAR_2.write(MIN_THRUST);
  delay(5000);
  Serial.println("ESC calibrated (check it manually)!");
  send_udp("ESC calibrated (check it manually)!");
}


void update_esc_power(BLA::Matrix<4> power_matrix)
{
//  Serial << "Updating the power matrix: " << power_matrix << "\n";
  ESC_FRONT_1.write((uint8_t)power_matrix(FRONTMA));
  ESC_FRONT_2.write((uint8_t)power_matrix(FRONTMB));
  ESC_REAR_1.write((uint8_t)power_matrix(REARMA));
  ESC_REAR_2.write((uint8_t)power_matrix(REARMB)); 
}
