#include <Servo.h>
#include <BasicLinearAlgebra.h>
//#include "constants.h"
const uint8_t FRONT_PIN_1 = D0;
const uint8_t FRONT_PIN_2 = D1;
const uint8_t REAR_PIN_1 = D2;
const uint8_t REAR_PIN_2 = D3;

Servo ESC_FRONT_1;
Servo ESC_FRONT_2;
Servo ESC_REAR_1;
Servo ESC_REAR_2;


void calibrate_esc()
{
  ESC_FRONT_1.attach(FRONT_PIN_1, 1000, 2000);
  ESC_FRONT_2.attach(FRONT_PIN_2, 1000, 2000);
  ESC_REAR_1.attach(REAR_PIN_1, 1000, 2000);
  ESC_REAR_2.attach(REAR_PIN_2, 1000, 2000);
  Serial.println("Sending max pulse...");
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
  delay(3000);
  Serial.println("ESC calibrated (check it manually)!");
}


void update_esc_power(BLA::Matrix<4> power_matrix)
{
  using namespace BLA;
  Serial << "Updating the power matrix: " << power_matrix << "\n";
  ESC_FRONT_1.write(power_matrix(FRONTMA));
  ESC_FRONT_2.write(power_matrix(FRONTMB));
  ESC_REAR_1.write(power_matrix(REARMA));
  ESC_REAR_2.write(power_matrix(REARMB)); 
}
