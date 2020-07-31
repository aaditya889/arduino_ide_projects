#include <BasicLinearAlgebra.h>

#ifndef __constants_h__
#define __constants_h__
#endif
#ifndef __pwm_h__
#define __pwm_h__
#endif
#ifndef __drone_h__
#define __drone_h__
#endif
#ifndef __drone_server_h__
#define __drone__server_h__
#endif

//  Function declarations
uint8_t tilt_throttle_up(BLA::Matrix<4> thrust_ratio, uint8_t tilt_direction);
uint8_t tilt_throttle_down(BLA::Matrix<4> thrust_ratio, uint8_t flight_thrust);

void find_com_thrust_ratio()
{
  char udp_message[150];
  BLA::Matrix<4> thrust_ratio;
  uint8_t flight_thrust;
  float perpendicular_relation, parallel_relation, max_ratio = 0;

  check_flight_status();
  change_auto_balance_status(false);

  Serial << "Finding the centre of mass of the frame...\n";
  sprintf(udp_message, "Finding the centre of mass of the frame...\n");
  send_udp(udp_message);

  // right tilt
  thrust_ratio(FRONTMB) = thrust_ratio(REARMB) = 0;
  thrust_ratio(FRONTMA) = thrust_ratio(REARMA) = 1;
  flight_thrust = tilt_throttle_up(thrust_ratio, ROLL);
  parallel_relation = (float)flight_thrust;
  tilt_throttle_down(thrust_ratio, flight_thrust);
  // left tilt
  thrust_ratio(FRONTMA) = thrust_ratio(REARMA) = 0;
  thrust_ratio(FRONTMB) = thrust_ratio(REARMB) = 1;
  flight_thrust = tilt_throttle_up(thrust_ratio, ROLL);
  parallel_relation /= (float)flight_thrust;
  tilt_throttle_down(thrust_ratio, flight_thrust);
  // backward lean
  thrust_ratio(REARMA) = thrust_ratio(REARMB) = 0;
  thrust_ratio(FRONTMA) = thrust_ratio(FRONTMB) = 1;
  flight_thrust = (float) tilt_throttle_up(thrust_ratio, PITCH);
  perpendicular_relation = (float)flight_thrust;
  tilt_throttle_down(thrust_ratio, flight_thrust);
  // forward lean
  thrust_ratio(FRONTMA) = thrust_ratio(FRONTMB) = 0;
  thrust_ratio(REARMA) = thrust_ratio(REARMB) = 1;
  flight_thrust = (float) tilt_throttle_up(thrust_ratio, PITCH);
  perpendicular_relation /= (float)flight_thrust;
  tilt_throttle_down(thrust_ratio, flight_thrust);
  
  thrust_ratio = {((float)parallel_relation * (float)perpendicular_relation), 1, (float)perpendicular_relation, (float)parallel_relation};

  for (uint8_t i = 0; i < thrust_ratio.GetRowCount(); i++) max_ratio = max(max_ratio, thrust_ratio(i));
  for (uint8_t i = 0; i < thrust_ratio.GetRowCount(); i++) thrust_ratio(i) /= max_ratio;
  
  STABLE_THRUST_RATIO = thrust_ratio;
  change_drone_thrust_ratio(thrust_ratio);
  
  Serial << "Found the center of mass with ratios: " << STABLE_THRUST_RATIO << "\n";
  sprintf(udp_message, "Got ratio: [%f %f %f %f]\n", STABLE_THRUST_RATIO(0), STABLE_THRUST_RATIO(1), STABLE_THRUST_RATIO(2), STABLE_THRUST_RATIO(3));
  send_udp(udp_message);
}

uint8_t tilt_throttle_up(BLA::Matrix<4> thrust_ratio, uint8_t tilt_direction) 
{
  uint8_t flight_thrust = 50, max_deviation = 2;
  char udp_message[150];

  Serial << "Throttling up with ratio: " << thrust_ratio << " ...\n";
  sprintf(udp_message, "Throttling up...\n");
  send_udp(udp_message);

  change_drone_thrust_ratio(thrust_ratio);
  
  while(true && INITIATE_FLIGHT && flight_thrust <= MAX_PULSE / 2)
  {
    check_flight_status();

    Serial << "Flight thrust = " << flight_thrust << " ...\n";
    sprintf(udp_message, "Flight thrust = %d", flight_thrust);
    send_udp(udp_message);

    change_flight_thrust(flight_thrust);
    delay(500);
    if (abs(YPR(tilt_direction)) >= max_deviation) return flight_thrust;
    flight_thrust += 1;
  }
  
  change_flight_thrust(MIN_PULSE);
  Serial << "Unable to find the tilt ratio...\n";
  sprintf(udp_message, "Unable to find the tilt ratio...\n");
  send_udp(udp_message);
  
  check_flight_status();
  while (true) delay(1000);
}

uint8_t tilt_throttle_down(BLA::Matrix<4> thrust_ratio, uint8_t flight_thrust) 
{
  char udp_message[150];

  Serial << "Throttling down with ratio: " << thrust_ratio << " ...\n";
  sprintf(udp_message, "Throttling down...\n");
  send_udp(udp_message);
  change_drone_thrust_ratio(thrust_ratio);
  
  while(flight_thrust != 0 && INITIATE_FLIGHT)
  {
    check_flight_status();

    Serial << "Flight thrust = " << flight_thrust << " ...\n";
    sprintf(udp_message, "Flight thrust = %d", flight_thrust);
    send_udp(udp_message);

    change_flight_thrust(flight_thrust);
    delay(10);
    flight_thrust -= 1;
  }
  check_flight_status();
}
