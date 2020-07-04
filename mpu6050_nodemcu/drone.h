#include <BasicLinearAlgebra.h>

#ifndef __constants_h__
#define __constants_h__
#endif

using namespace BLA;

//  function declarations
uint8_t get_mapped_thrust(uint8_t reference, uint8_t value, uint8_t min_val, uint8_t max_val, boolean throttle);
void update_thrust_vector();
void fix_roll (uint8_t deviation, uint8_t min_value, uint8_t max_value, boolean left_tilt, BLA::Matrix<4>* thrust_vector);
void fix_pitch (uint8_t deviation, uint8_t min_value, uint8_t max_value, boolean backward_lean, BLA::Matrix<4>* thrust_vector);
void change_drone_thrust_vector();
void recalibrate_thrust_vector(BLA::Matrix<4> *thrust_vector, uint8_t current_flight_thrust);
void calibrate_flight_thrust();
void check_flight_status();
void change_flight_thrust(uint8_t flight_thrust);
void change_auto_balancing_status(boolean is_enabled);
void change_mpu_filtering_status(boolean is_enabled);


// Changes YPR, YPR_GYRO, GYRO_START_TIME, GYRO_END_TIME
void complementary_filter()
{
  BLA::Matrix<3> mpu_values[2], ypr_acc, angle_delta, ypr_delta;

  read_mpu_values(mpu_values, MPU6050_SLAVE_ADDRESS, MPU6050_REGISTER_ACCEL_XOUT_H, true);

  for (int i = 0; i < mpu_values[0].GetRowCount(); i++) 
  {
    ypr_acc(i) = atan2f((double)(mpu_values[0](i)), (double)mpu_values[0](AZ)) * (180 / PI);
  }

  GYRO_END_TIME = micros();

  angle_delta = (mpu_values[1] * 1.0 * (double)((GYRO_END_TIME - GYRO_START_TIME) / (double) MEGA));
  YPR_GYRO += ADX * (double)angle_delta(GX) + ADY * (double)angle_delta(GY) + ADZ * (double)angle_delta(GZ);
  
  YPR = ypr_acc * (double)(ACC_WEIGHT) + YPR_GYRO * (double)(1 - ACC_WEIGHT);
  YPR_GYRO = YPR;
  GYRO_START_TIME = micros();
}


// Changes DRONE_THRUST_VECTOR
void update_thrust_vector()
{
//  long unsigned int st,en;
//  st=micros();
//  Serial.println("updating...");

  // Declarations
  BLA::Matrix<4> thrust_vector;
  BLA::Matrix<3> ypr_delta;
  uint8_t roll_deviation;
  uint8_t pitch_deviation, current_flight_thrust;
  boolean left_tilt;
  boolean backward_lean;

  change_drone_thrust_vector();
  
//  Assignments
  thrust_vector = DRONE_THRUST_VECTOR;
  ypr_delta = YPR - DES_YPR; 
  roll_deviation = abs(ypr_delta(ROLL));
  pitch_deviation = abs(ypr_delta(PITCH));
  current_flight_thrust = FLIGHT_THRUST;
  left_tilt = (ypr_delta(ROLL) < 0);
  backward_lean = (ypr_delta(PITCH) < 0);
  

  fix_roll(abs(ypr_delta(ROLL)), (uint8_t) (0.5 * current_flight_thrust), 2 * current_flight_thrust, left_tilt, &thrust_vector);
  fix_pitch(abs(ypr_delta(PITCH)), (uint8_t) (0.5 * current_flight_thrust), 2 * current_flight_thrust, backward_lean, &thrust_vector);

//  Serial << " Got thrust vector: " << thrust_vector;
//  Serial << "Roll deviation: " << roll_deviation << " pitch dev: " << pitch_deviation << "\n";
  
  recalibrate_thrust_vector(&thrust_vector, current_flight_thrust);
  
  update_esc_power(thrust_vector);
  DRONE_THRUST_VECTOR = thrust_vector;
//  Serial.println("ending...");
//  en=micros();

//  Serial.print("Took: "); Serial.println(en-st);
}

void fix_roll (uint8_t deviation, uint8_t min_value, uint8_t max_value, boolean left_tilt, BLA::Matrix<4>* thrust_vector)
{
  if (left_tilt)  // left tilt
  {
//    (*thrust_vector)(FRONTMA) = get_mapped_thrust((*thrust_vector)(FRONTMA), deviation, min_value, max_value, true);
//    (*thrust_vector)(REARMA) = get_mapped_thrust((*thrust_vector)(REARMA), deviation, min_value, max_value, true);
    (*thrust_vector)(FRONTMB) = get_mapped_thrust((*thrust_vector)(FRONTMB), deviation, min_value, max_value, false);
    (*thrust_vector)(REARMB) = get_mapped_thrust((*thrust_vector)(REARMB), deviation, min_value, max_value, false);
  }
  else    // right tilt
  {
    (*thrust_vector)(FRONTMA) = get_mapped_thrust((*thrust_vector)(FRONTMA), deviation, min_value, max_value, false);
    (*thrust_vector)(REARMA) = get_mapped_thrust((*thrust_vector)(REARMA), deviation, min_value, max_value, false);
//    (*thrust_vector)(FRONTMB) = get_mapped_thrust((*thrust_vector)(FRONTMB), deviation, min_value, max_value, true);
//    (*thrust_vector)(REARMB) = get_mapped_thrust((*thrust_vector)(REARMB), deviation, min_value, max_value, true);
  }
}

void fix_pitch (uint8_t deviation, uint8_t min_value, uint8_t max_value, boolean backward_lean, BLA::Matrix<4>* thrust_vector)
{
  if (backward_lean)  // backward lean
  {
    (*thrust_vector)(FRONTMA) = get_mapped_thrust((*thrust_vector)(FRONTMA), deviation, min_value, max_value, false);
    (*thrust_vector)(FRONTMB) = get_mapped_thrust((*thrust_vector)(FRONTMB), deviation, min_value, max_value, false);
//    (*thrust_vector)(REARMA) = get_mapped_thrust((*thrust_vector)(REARMA), deviation, min_value, max_value, true);
//    (*thrust_vector)(REARMB) = get_mapped_thrust((*thrust_vector)(REARMB), deviation, min_value, max_value, true);
  }
  else    // forward lean
  {
//    (*thrust_vector)(FRONTMA) = get_mapped_thrust((*thrust_vector)(FRONTMA), deviation, min_value, max_value, true);
//    (*thrust_vector)(FRONTMB) = get_mapped_thrust((*thrust_vector)(FRONTMB), deviation, min_value, max_value, true);
    (*thrust_vector)(REARMA) = get_mapped_thrust((*thrust_vector)(REARMA), deviation, min_value, max_value, false);
    (*thrust_vector)(REARMB) = get_mapped_thrust((*thrust_vector)(REARMB), deviation, min_value, max_value, false);
  }
}


void change_drone_thrust_vector()
{
  if(FLIGHT_THRUST_DIFF == 0) return;

  for (uint8_t i = 0; i < DRONE_THRUST_VECTOR.GetRowCount(); i++)
  {
    int value = DRONE_THRUST_VECTOR(i) + FLIGHT_THRUST_DIFF;
    DRONE_THRUST_VECTOR(i) = (value <= 0) ? 0 : value;
  }
  FLIGHT_THRUST += FLIGHT_THRUST_DIFF;
  FLIGHT_THRUST_DIFF = 0;
}


void recalibrate_thrust_vector(BLA::Matrix<4> *thrust_vector, uint8_t current_flight_thrust)
{
  double diff = MAX_PULSE;
  for(uint8_t i = 0; i < (*thrust_vector).GetRowCount(); i++) diff = (double)min(diff, ((double)current_flight_thrust - (*thrust_vector)(i)));
//  Serial.print("GOT DIFF: "); Serial.println(diff);
  (*thrust_vector) += (BLA::Matrix<4>) {diff, diff, diff, diff};
}


uint8_t get_mapped_thrust(uint8_t reference, uint8_t value, uint8_t min_val, uint8_t max_val, boolean throttle) 
{
  return (throttle ? map(value, 0, 180, reference, max_val) : map(value, 180, 0, min_val, reference));
}


void calibrate_flight_thrust()
{
  Serial << "Calibrating thrust...\n";
  char udp_message[150];
  uint8_t min_gyro_delta = 5, delta_thrust, grace_thrust = 0, flight_thrust;
  double gyro_z;
  BLA::Matrix<4> thrust_vector;
  BLA::Matrix<3> mpu_values[2];

  flight_thrust = 2;
  
  while (!IS_FLIGHT_ACHIEVED)
  {
    flight_thrust = (FLIGHT_THRUST + 3) % MAX_PULSE;  
    check_flight_status();
    change_flight_thrust(flight_thrust);
    
    Serial << "Trying to achieve flight at thrust = " << flight_thrust << "...\n";
    sprintf(udp_message, "Trying to achieve flight at thrust = %d...\n", flight_thrust);
    send_udp(udp_message);

    delta_thrust = flight_thrust / 3;
    // thrust_vector.Fill(flight_thrust);
//    thrust_vector(FRONTMA) = flight_thrust + delta_thrust;
//    thrust_vector(FRONTMB) = flight_thrust - delta_thrust;
//    thrust_vector(REARMA) = flight_thrust - delta_thrust;
//    thrust_vector(REARMB) = flight_thrust + delta_thrust;
    
    find_mpu_averages(mpu_values, 200, 10, true);   // This is also the delay for this loop
    gyro_z = mpu_values[1](GZ);
    Serial.print("GOT GYRO Z = "); Serial.println(gyro_z);
    gyro_z = (double)abs(gyro_z);
    sprintf(udp_message, "Current gyro Z delta = %lf, min needed for flight = %d...\n", gyro_z, min_gyro_delta);
    send_udp(udp_message);
    
//    if (gyro_z >= min_gyro_delta) IS_FLIGHT_ACHIEVED = true;

    if (flight_thrust >= MAX_PULSE / 2) 
    {
      Serial << "Unable to achieve flight, something might be wrong. Aborting...\n";
      send_udp("Unable to achieve flight, something might be wrong. Aborting...\n");
      
      change_flight_thrust(MIN_PULSE);
      while (true) {delay(100);}
    }
  }
  // Serial << "Increasing the thrust gracefully ...\n";
  // send_udp("Increasing the thrust gracefully ...\n");
  // delay(2000);
  
  // grace_thrust = flight_thrust / 2;
  // change_flight_thrust(grace_thrust);
  
  // while (grace_thrust <= flight_thrust)
  // {
  //   change_flight_thrust(grace_thrust);
  //   grace_thrust += 1;
  //   delay(200);
  // }
  
  Serial << "Achieved flight at thrust = " << flight_thrust << "...\n";
  sprintf(udp_message, "Achieved flight at thrust = %d...\n", flight_thrust);
  send_udp(udp_message);
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
    change_flight_thrust(MIN_PULSE);
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


void export_drone_stats()
{
  char mpu_data[150];
  BLA::Matrix<4> thrust_vector = DRONE_THRUST_VECTOR;
  BLA::Matrix<3> ypr = YPR;
  sprintf(mpu_data, "DBG:: YX: %10lf YY: %10lf YZ: %10lf DTFA: %5lf DTFB: %5lf DTRA: %5lf DTRB: %5lf", ypr(AX), ypr(AY), ypr(AZ), thrust_vector(FRONTMA), thrust_vector(FRONTMB), thrust_vector(REARMA), thrust_vector(REARMB));
  send_udp(mpu_data);
}


void change_flight_thrust(uint8_t flight_thrust)
{
  FLIGHT_THRUST_DIFF = flight_thrust - FLIGHT_THRUST;
}


void change_auto_balancing_status(boolean is_enabled)
{
  if (is_enabled) BALANCE_DRONE_TICKER.attach_ms(BALANCE_DRONE_TICKER_INTERVAL_MS, update_thrust_vector);
  else BALANCE_DRONE_TICKER.detach();
}


void change_mpu_filtering_status(boolean is_enabled)
{
  if (is_enabled) COMBINE_MPU_DATA_TICKER.attach_ms(COMBINE_MPU_DATA_TICKER_INTERVAL_MS, complementary_filter);
  else COMBINE_MPU_DATA_TICKER.detach();
}


void change_export_stats_status(boolean is_enabled)
{
  if (is_enabled) UDP_STATS_EXPORTER.attach_ms(UDP_STATS_EXPORTER_INTERVAL_MS, export_drone_stats);
  else UDP_STATS_EXPORTER.detach(); 
}