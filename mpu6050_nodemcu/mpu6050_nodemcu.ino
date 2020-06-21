#include <Ticker.h>
#include <wireless_operations.h>
#include <Servo.h>
#include <Wire.h>
#include <math.h>
#include <BasicLinearAlgebra.h>
#include "constants.h"
#include "pwm.h"
#include <_mpu6050.h>
#include "drone.h"
#include "drone_server.h"


//  T = (double)Temperature/340+36.53; //temperature formula
//  TODO: TURN THE FILTER FUNCTION INTO A BLACK BOX, AND REDUCE THE CODE IN THE LOOP() FUNCTION
//  TODO: REDUCE GLOBAL VARIABLE COUNT AND RECTIFY THE LINTING!!
//  TODO: TRY TO REUSE VARIABLES AND REDUCE THE AMOUNT OF MEMORY USED (TO COMPENSATE FOR THE POSSIBILITY OF CODE EXPANSION)!
//  MOVE SOME OF THE GLOBAL VARS TO CONSTANTS FILE
//  REVISIT THE find_angles boolean IN READRAWVALUE CODE!

//  NEXT (BEFORE FLIGHT): 
//  SHIFT THE MPU LIBRARY TO A NEW FILE/FOLDER, THEN TEST AGAIN. 
//  SHIFT DRONE RELATED FUNCTIONS TO A NEW LIBRARY, THEN TEST AGAIN.
//  RETURN THE VALUES FROM ReadRaw INSTEAD OF SETTING THE GLOBAL VARIABLES
//  REMOVE ALL GLOBAL VARIALBES.
//  MOVE THE FILTER TO A TICKER FUNCTION, THEN TEST PROPERLY
//  MOVE THE SERVER TO A NEW LIBRARY.
//  REMOVE UNECESSARY VARIABLES.

//  function definitions
// uint8_t get_mapped_thrust(uint8_t reference, uint8_t value, uint8_t min_val, uint8_t max_val, boolean throttle);

using namespace BLA;

// Global variables

//Ticker handle_server_requests;

void setup() 
{
  Serial.begin(115200);
  connect_AP(ssid, password);
  MPU_ACC_AVG.Fill(0);
  MPU_GYRO_AVG.Fill(0);
  YPR_GYRO.Fill(0);
  YPR.Fill(0);
  
  mpu_init();
  calibrate_esc();
  initiate_server();
  check_flight_status();
  calibrate_flight_thrust();
  GYRO_START_TIME = micros();
}


void loop()
{
  BLA::Matrix<3> mpu_values[2];
   char mpu_data[150];
   check_flight_status();
   if (!IS_FLIGHT_ACHIEVED) 
   {
     calibrate_flight_thrust();
     GYRO_START_TIME = micros();
   }
  
   filter_and_update_thrust();
   sprintf(mpu_data, "DBG:: YX: %10lf YY: %10lf YZ: %10lf", YPR(AX), YPR(AY), YPR(AZ));

//   Serial << "YPR => " << YPR << "\n";

   send_udp(mpu_data);
}
