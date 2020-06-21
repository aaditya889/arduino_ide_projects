#include <BasicLinearAlgebra.h>

#ifndef __constants_h__
#define __constants_h__
#endif

using namespace BLA;

//  function definitions
uint8_t get_mapped_thrust(uint8_t reference, uint8_t value, uint8_t min_val, uint8_t max_val, boolean throttle);
void update_thrust_vector(BLA::Matrix<3> ypr_delta);
void fix_roll (uint8_t *reference_vector, uint8_t deviation, uint8_t min_value, uint8_t max_value, boolean left_tilt, BLA::Matrix<4>* thrust_vector);
void fix_pitch (uint8_t *reference_vector, uint8_t deviation, uint8_t min_value, uint8_t max_value, boolean backward_lean, BLA::Matrix<4>* thrust_vector);
void calibrate_flight_thrust();
void check_flight_status();

void filter_and_update_thrust()
{
  BLA::Matrix<3> mpu_values[2], ypr_acc, angle_delta, ypr_delta;
  read_mpu_values(mpu_values, mpu_slave_address, MPU6050_REGISTER_ACCEL_XOUT_H, true);

  for (int i = 0; i < mpu_values[0].GetRowCount(); i++) 
  {
    ypr_acc(i) = atan2f((double)(mpu_values[0](i)), (double)mpu_values[0](AZ)) * (180 / PI);
  }

  GYRO_END_TIME = micros();

  angle_delta = (mpu_values[1] * 1.0 * (double)((GYRO_END_TIME - GYRO_START_TIME) / (double) MEGA));
  YPR_GYRO += ADX * (double)angle_delta(GX) + ADY * (double)angle_delta(GY) + ADZ * (double)angle_delta(GZ);
  
  YPR = ypr_acc * (double)(ACC_WEIGHT) + YPR_GYRO * (double)(1 - ACC_WEIGHT);
  YPR_GYRO = YPR;
  ypr_delta = YPR - DES_YPR;

  update_thrust_vector(ypr_delta);
  GYRO_START_TIME = micros();
}


void update_thrust_vector(BLA::Matrix<3> ypr_delta)
{
  uint8_t roll_deviation = abs(ypr_delta(ROLL));
  uint8_t pitch_deviation = abs(ypr_delta(PITCH));
  uint8_t reference_vector[4] = {FLIGHT_THRUST, FLIGHT_THRUST, FLIGHT_THRUST, FLIGHT_THRUST};
  BLA::Matrix<4> thrust_vector;
  
  boolean left_tilt = (ypr_delta(ROLL) < 0);
  boolean backward_lean = (ypr_delta(PITCH) < 0);

  fix_roll(reference_vector, abs(ypr_delta(ROLL)), 0, 2 * FLIGHT_THRUST, left_tilt, &thrust_vector);
  fix_pitch(reference_vector, abs(ypr_delta(PITCH)), 0, 2 * FLIGHT_THRUST, backward_lean, &thrust_vector);
  
  update_esc_power(thrust_vector);
  
}

void fix_roll (uint8_t *reference_vector, uint8_t deviation, uint8_t min_value, uint8_t max_value, boolean left_tilt, BLA::Matrix<4>* thrust_vector)
{
  if (left_tilt)  // left tilt
  {
    (*thrust_vector)(FRONTMA) = reference_vector[FRONTMA] = get_mapped_thrust(reference_vector[FRONTMA], deviation, min_value, max_value, true);
    (*thrust_vector)(REARMA) = reference_vector[REARMA] = get_mapped_thrust(reference_vector[REARMA], deviation, min_value, max_value, true);
    (*thrust_vector)(FRONTMB) = reference_vector[FRONTMB] = get_mapped_thrust(reference_vector[FRONTMB], deviation, min_value, max_value, false);
    (*thrust_vector)(REARMB) = reference_vector[REARMB] = get_mapped_thrust(reference_vector[REARMB], deviation, min_value, max_value, false);
  }
  else    // right tilt
  {
    (*thrust_vector)(FRONTMA) = reference_vector[FRONTMA] = get_mapped_thrust(reference_vector[FRONTMA], deviation, min_value, max_value, false);
    (*thrust_vector)(REARMA) = reference_vector[REARMA] = get_mapped_thrust(reference_vector[REARMA], deviation, min_value, max_value, false);
    (*thrust_vector)(FRONTMB) = reference_vector[FRONTMB] = get_mapped_thrust(reference_vector[FRONTMB], deviation, min_value, max_value, true);
    (*thrust_vector)(REARMB) = reference_vector[REARMB] = get_mapped_thrust(reference_vector[REARMB], deviation, min_value, max_value, true);
  }
}

void fix_pitch (uint8_t *reference_vector, uint8_t deviation, uint8_t min_value, uint8_t max_value, boolean backward_lean, BLA::Matrix<4>* thrust_vector)
{
  if (backward_lean)  // backward lean
  {
    (*thrust_vector)(FRONTMA) = reference_vector[FRONTMA] = get_mapped_thrust(reference_vector[FRONTMA], deviation, min_value, max_value, false);
    (*thrust_vector)(FRONTMB) = reference_vector[FRONTMB] = get_mapped_thrust(reference_vector[FRONTMB], deviation, min_value, max_value, false);
    (*thrust_vector)(REARMA) = reference_vector[REARMA] = get_mapped_thrust(reference_vector[REARMA], deviation, min_value, max_value, true);
    (*thrust_vector)(REARMB) = reference_vector[REARMB] = get_mapped_thrust(reference_vector[REARMB], deviation, min_value, max_value, true);
  }
  else    // forward lean
  {
    (*thrust_vector)(FRONTMA) = reference_vector[FRONTMA] = get_mapped_thrust(reference_vector[FRONTMA], deviation, min_value, max_value, true);
    (*thrust_vector)(FRONTMB) = reference_vector[FRONTMB] = get_mapped_thrust(reference_vector[FRONTMB], deviation, min_value, max_value, true);
    (*thrust_vector)(REARMA) = reference_vector[REARMA] = get_mapped_thrust(reference_vector[REARMA], deviation, min_value, max_value, false);
    (*thrust_vector)(REARMB) = reference_vector[REARMB] = get_mapped_thrust(reference_vector[REARMB], deviation, min_value, max_value, false);
  }
}

uint8_t get_mapped_thrust(uint8_t reference, uint8_t value, uint8_t min_val, uint8_t max_val, boolean throttle) 
{
  return (throttle ? map(value, 0, 180, reference, max_val) : map(value, 90, 0, min_val, reference));
}

void calibrate_flight_thrust()
{
  Serial << "Calibrating thrust...\n";
  char udp_message[150];
  uint8_t min_gyro_delta = 5, delta_thrust, grace_thrust = 0;
  double gyro_z;
  BLA::Matrix<4> thrust_vector;
  BLA::Matrix<3> mpu_values[2];

  FLIGHT_THRUST = 2;
  
  while (!IS_FLIGHT_ACHIEVED)
  {
    FLIGHT_THRUST = (FLIGHT_THRUST + 3) % MAX_THRUST;  
    check_flight_status();
    thrust_vector.Fill(FLIGHT_THRUST); 
    
    Serial << "Trying to achieve flight at thrust = " << FLIGHT_THRUST << ", and thrust_vector = "<< thrust_vector << "...\n";
    sprintf(udp_message, "Trying to achieve flight at thrust = %d...\n", FLIGHT_THRUST);
    send_udp(udp_message);

    delta_thrust = FLIGHT_THRUST / 3;
    thrust_vector(FRONTMA) = FLIGHT_THRUST + delta_thrust;
    thrust_vector(FRONTMB) = FLIGHT_THRUST - delta_thrust;
    thrust_vector(REARMA) = FLIGHT_THRUST - delta_thrust;
    thrust_vector(REARMB) = FLIGHT_THRUST + delta_thrust;
    
    update_esc_power(thrust_vector);
    find_mpu_averages(mpu_values, 200, 10, true);
    gyro_z = mpu_values[1](GZ);
    Serial.print("GOT GYRO Z = "); Serial.println(gyro_z);
    gyro_z = (double)abs(gyro_z);
    sprintf(udp_message, "Current gyro Z delta = %lf, min needed for flight = %d...\n", gyro_z, min_gyro_delta);
    send_udp(udp_message);
    
    if (gyro_z >= min_gyro_delta) IS_FLIGHT_ACHIEVED = true;

    if (FLIGHT_THRUST >= MAX_THRUST / 2) 
    {
      Serial << "Unable to achieve flight, something might be wrong. Aborting...\n";
      send_udp("Unable to achieve flight, something might be wrong. Aborting...\n");
      
      FLIGHT_THRUST = MIN_THRUST;
      thrust_vector.Fill(FLIGHT_THRUST);
      update_esc_power(thrust_vector);
      while (true) {delay(100);}
    }
  }
  Serial << "Increasing the thrust gracefully ...\n";
  send_udp("Increasing the thrust gracefully ...\n");
  delay(2000);
  
  grace_thrust = FLIGHT_THRUST / 2;
  thrust_vector.Fill(grace_thrust);
  update_esc_power(thrust_vector);
  
  while (grace_thrust <= FLIGHT_THRUST)
  {
    thrust_vector.Fill(grace_thrust);
    update_esc_power(thrust_vector);
    grace_thrust += 1;
    delay(200);
  }
  
  Serial << "Achieved flight at thrust = " << FLIGHT_THRUST << "...\n";
  sprintf(udp_message, "Achieved flight at thrust = %d...\n", FLIGHT_THRUST);
  send_udp(udp_message);
  delay(1000);
}

void check_flight_status()
{
  int i = 0;
  char udp_message[100];
  BLA::Matrix<4> thrust_vector;
  sprintf(udp_message, "Server listening on %s:%d, waiting for response...\n",  WiFi.localIP().toString().c_str(), SERVER_PORT);
  server.handleClient();
  
  while(!INITIATE_FLIGHT)
  {
    IS_FLIGHT_ACHIEVED = false;
    FLIGHT_THRUST = MIN_THRUST;
    thrust_vector.Fill(FLIGHT_THRUST);
    update_esc_power(thrust_vector);
    server.handleClient();
    delay(100);
    i++;
    if (i == 50)
    {
      Serial.println(udp_message);
      send_udp(udp_message);
      i = 0; 
    }
  }
}
