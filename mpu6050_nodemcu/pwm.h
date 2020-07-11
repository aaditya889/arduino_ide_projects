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

//  function declarations
extern void send_udp(char *);
void update_esc_power(BLA::Matrix<4> power_matrix);

void calibrate_esc()
{
  BLA::Matrix<4> max_power, min_power;
  max_power.Fill(MAX_PULSE);
  min_power.Fill(MIN_PULSE);
  char udp_message[150];
  send_udp("Calibrating esc...\n");
  sprintf(udp_message, "Sending max pulse (%d) in 5 seconds...", MAX_PULSE);
  send_udp(udp_message);
  Serial.println("Sending max pulse in 5 seconds...");
  delay(5000);
  
  ESC_FRONT_1.attach(FRONT_PIN_1, 1000, 2000);
  ESC_FRONT_2.attach(FRONT_PIN_2, 1000, 2000);
  ESC_REAR_1.attach(REAR_PIN_1, 1000, 2000);
  ESC_REAR_2.attach(REAR_PIN_2, 1000, 2000);
  Serial.println("Sending max pulse...");
  sprintf(udp_message, "Sending max pulse = %d...", MAX_PULSE);
  send_udp(udp_message);
  update_esc_power(max_power);
  delay(3000);
  Serial.println("Sending min pulse...");
  sprintf(udp_message, "Sending min pulse = %d...", MIN_PULSE);
  send_udp(udp_message);
  update_esc_power(min_power);
  delay(5000);
  Serial.println("ESC calibrated (check it manually)!");
  send_udp("ESC calibrated (check it manually)!");
}


void update_esc_power(BLA::Matrix<4> power_matrix)
{
  using namespace BLA;
//  Serial << "Updating the power matrix: " << power_matrix << "\n";
  ESC_FRONT_1.write((uint8_t)power_matrix(FRONTMA));
  ESC_FRONT_2.write((uint8_t)power_matrix(FRONTMB));
  ESC_REAR_1.write((uint8_t)power_matrix(REARMA));
  ESC_REAR_2.write((uint8_t)power_matrix(REARMB)); 
}
