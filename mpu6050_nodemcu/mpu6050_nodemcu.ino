#include <Ticker.h>
#include <WiFiUdp.h>
#include <wireless_operations.h>
#include <Servo.h>
#include <Wire.h>
#include "constants.h"
#include <math.h>
#include "pwm.h"
#include <ESP8266WebServer.h>

#define MEGA 1000000
#define AX 0
#define AY 1
#define AZ 2
#define EX 0
#define EY 1
#define EZ 2
#define GX 0
#define GY 1
#define GZ 2
#define ROLL 0
#define PITCH 1
#define YAW 2

//  T = (double)Temperature/340+36.53; //temperature formula
//  TODO: TURN THE FILTER FUNCTION INTO A BLACK BOX, AND REDUCE THE CODE IN THE LOOP() FUNCTION
//  TODO: REDUCE GLOBAL VARIABLE COUNT AND RECTIFY THE LINTING!!
//  TODO: TRY TO REUSE VARIABLES AND REDUCE THE AMOUNT OF MEMORY USED (TO COMPENSATE FOR THE POSSIBILITY OF CODE EXPANSION)!
//  MOVE SOME OF THE GLOBAL VARS TO CONSTANTS FILE
//  REVISIT THE find_angles boolean IN READRAWVALUE CODE!

//  function definitions
uint8_t get_mapped_thrust(uint8_t reference, uint8_t value, uint8_t min_val, uint8_t max_val, boolean throttle);

using namespace BLA;
WiFiUDP udp_client;
ESP8266WebServer server(SERVER_PORT);

// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;
uint32_t GYRO_START_TIME, GYRO_END_TIME;
const double ACC_WEIGHT = 0.08;
uint8_t FLIGHT_THRUST = 0;

BLA::Matrix<3> MPU_ACC, MPU_GYRO, MPU_ACC_AVG, MPU_GYRO_AVG, MPU_ACC_OFF, ANGLE_DELTA, GYRO_ANGLES, YPR_GYRO = {0,0,0}, YPR_ACC = {0,0,0}, YPR = {0,0,0}, DES_YPR = {0,0,0}, YPR_DELTA;
BLA::Matrix<4> THRUST_MATRIX;
BLA::Matrix<3> ADX = {0,1,0}, ADY = {-1,0,0}, ADZ = {0,0,0};
//int16_t Temperature;

int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;
double acc_x_avg = 0, acc_y_avg = 0, acc_z_avg = 0, gyro_x_avg = 0, gyro_y_avg = 0, gyro_z_avg = 0;

// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;

// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D6;
const uint8_t sda = D7;

// MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;

//Ticker handle_server_requests;

void setup() 
{
  Serial.begin(115200);
  connect_AP(ssid, password);
  MPU_ACC_OFF = {0,0,1};
//  AA /= (double)5;
//  Serial << "AA:" << AA;
  MPU_ACC.Fill(0);
  MPU_GYRO.Fill(0);
  MPU_ACC_AVG.Fill(0);
  MPU_GYRO_AVG.Fill(0);
  ANGLE_DELTA.Fill(0);
  GYRO_ANGLES.Fill(0);
  YPR_GYRO.Fill(0);
  YPR_ACC.Fill(0);
  YPR.Fill(0);
  DES_YPR.Fill(0);
  THRUST_MATRIX.Fill(FLIGHT_THRUST);
  Wire.begin(sda, scl);
  
  Serial << "MPU_ACC_AVG: " << MPU_ACC_AVG << "\nMPU_GYRO_AVG: " << MPU_GYRO_AVG << "\nMPU_ACC: " << MPU_ACC << "\nMPU_GYRO: " << MPU_GYRO;
  delay(200);
  
  MPU6050_Init();
  calibrate_esc();
  initiate_server();
  check_flight_status();
  calibrate_flight_thrust();
  
  GYRO_START_TIME = micros();
}

void loop()
{
  char mpu_data[150];
  check_flight_status();
  if (!IS_FLIGHT_ACHIEVED) calibrate_flight_thrust();
  
  filter_and_update_thrust();
  sprintf(mpu_data, "DBG:: YX: %10lf YY: %10lf YZ: %10lf", YPR(AX), YPR(AY), YPR(AZ));

  Serial << "YPR => " << YPR << " ACC => " << YPR_ACC << "\n";

  send_udp(mpu_data);
}

void send_udp(char *message)
{
  // udp send takes around 700 - 750 microseconds
  udp_client.beginPacket(REMOTE_IP, REMOTE_PORT);
  udp_client.write(message, strlen(message));
  udp_client.endPacket();
}

void filter_and_update_thrust()
{
  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H, true);

  for (int i = 0; i < MPU_ACC.GetRowCount(); i++) 
  {
    YPR_ACC(i) = atan2f((double)(MPU_ACC(i)), (double)MPU_ACC(AZ)) * (180 / PI);
  }

  GYRO_END_TIME = micros();

  ANGLE_DELTA = (MPU_GYRO * 4.0 * (double)((GYRO_END_TIME - GYRO_START_TIME) / (double) MEGA));
  YPR_GYRO += ADX * (double)ANGLE_DELTA(GX) + ADY * (double)ANGLE_DELTA(GY) + ADZ * (double)ANGLE_DELTA(GZ);
  
  YPR = YPR_ACC * (double)(ACC_WEIGHT) + YPR_GYRO * (double)(1 - ACC_WEIGHT);
  YPR_GYRO = YPR;
  YPR_DELTA = YPR - DES_YPR;

  update_thrust_vector();
  GYRO_START_TIME = micros();
}


void update_thrust_vector()
{
  uint8_t roll_deviation = abs(YPR_DELTA(ROLL));
  uint8_t pitch_deviation = abs(YPR_DELTA(PITCH));
  uint8_t reference_vector[4] = {FLIGHT_THRUST, FLIGHT_THRUST, FLIGHT_THRUST, FLIGHT_THRUST};
  
  fix_roll(reference_vector, abs(YPR_DELTA(ROLL)), 0, 2 * FLIGHT_THRUST);
  fix_pitch(reference_vector, abs(YPR_DELTA(PITCH)), 0, 2 * FLIGHT_THRUST);
  
  update_esc_power(THRUST_MATRIX);
  
//  Serial << "new FRONT => MA: " <<  THRUST_MATRIX(FRONTMA) << " MB: " << THRUST_MATRIX(FRONTMB) << " REAR => MA: " << THRUST_MATRIX(REARMA) << " MB: " << THRUST_MATRIX(REARMB) << "\n";
}

void fix_roll (uint8_t *reference_vector, uint8_t deviation, uint8_t min_value, uint8_t max_value)
{
  if (YPR_DELTA(ROLL) < 0)  // left tilt
  {
    THRUST_MATRIX(FRONTMA) = reference_vector[FRONTMA] = get_mapped_thrust(reference_vector[FRONTMA], deviation, min_value, max_value, true);
    THRUST_MATRIX(REARMA) = reference_vector[REARMA] = get_mapped_thrust(reference_vector[REARMA], deviation, min_value, max_value, true);
    THRUST_MATRIX(FRONTMB) = reference_vector[FRONTMB] = get_mapped_thrust(reference_vector[FRONTMB], deviation, min_value, max_value, false);
    THRUST_MATRIX(REARMB) = reference_vector[REARMB] = get_mapped_thrust(reference_vector[REARMB], deviation, min_value, max_value, false);
  }
  else    // right tilt
  {
    THRUST_MATRIX(FRONTMA) = reference_vector[FRONTMA] = get_mapped_thrust(reference_vector[FRONTMA], deviation, min_value, max_value, false);
    THRUST_MATRIX(REARMA) = reference_vector[REARMA] = get_mapped_thrust(reference_vector[REARMA], deviation, min_value, max_value, false);
    THRUST_MATRIX(FRONTMB) = reference_vector[FRONTMB] = get_mapped_thrust(reference_vector[FRONTMB], deviation, min_value, max_value, true);
    THRUST_MATRIX(REARMB) = reference_vector[REARMB] = get_mapped_thrust(reference_vector[REARMB], deviation, min_value, max_value, true);
  }
}

void fix_pitch (uint8_t *reference_vector, uint8_t deviation, uint8_t min_value, uint8_t max_value)
{
  if (YPR_DELTA(PITCH) < 0)  // backward lean
  {
    THRUST_MATRIX(FRONTMA) = reference_vector[FRONTMA] = get_mapped_thrust(reference_vector[FRONTMA], deviation, min_value, max_value, false);
    THRUST_MATRIX(FRONTMB) = reference_vector[FRONTMB] = get_mapped_thrust(reference_vector[FRONTMB], deviation, min_value, max_value, false);
    THRUST_MATRIX(REARMA) = reference_vector[REARMA] = get_mapped_thrust(reference_vector[REARMA], deviation, min_value, max_value, true);
    THRUST_MATRIX(REARMB) = reference_vector[REARMB] = get_mapped_thrust(reference_vector[REARMB], deviation, min_value, max_value, true);
  }
  else    // forward lean
  {
    THRUST_MATRIX(FRONTMA) = reference_vector[FRONTMA] = get_mapped_thrust(reference_vector[FRONTMA], deviation, min_value, max_value, true);
    THRUST_MATRIX(FRONTMB) = reference_vector[FRONTMB] = get_mapped_thrust(reference_vector[FRONTMB], deviation, min_value, max_value, true);
    THRUST_MATRIX(REARMA) = reference_vector[REARMA] = get_mapped_thrust(reference_vector[REARMA], deviation, min_value, max_value, false);
    THRUST_MATRIX(REARMB) = reference_vector[REARMB] = get_mapped_thrust(reference_vector[REARMB], deviation, min_value, max_value, false);
  }
}


uint8_t get_mapped_thrust(uint8_t reference, uint8_t value, uint8_t min_val, uint8_t max_val, boolean throttle) 
{
  return (throttle ? map(value, 0, 90, reference, max_val) : map(value, 90, 0, min_val, reference));
}

void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data)
{
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

// read all 14 register
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress, boolean find_angles)
{
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  while(!Wire.available()) {}
  AccelX = MPU_ACC(AX) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    // Acc X
  AccelY = MPU_ACC(AY) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    // Acc Y
  AccelZ = MPU_ACC(AZ) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    //Acc Z
  Temperature = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    // Temp
  GyroX = MPU_GYRO(GX) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    //Gyro X
  GyroY = MPU_GYRO(GY) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    //Gyro Y
  GyroZ = MPU_GYRO(GZ) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    //Gyro Z

  if (find_angles)
  {
    MPU_ACC = (MPU_ACC / (double) AccelScaleFactor) - MPU_ACC_AVG + MPU_ACC_OFF;
    MPU_GYRO = (MPU_GYRO / (double) GyroScaleFactor) - MPU_GYRO_AVG; 
  }
}

//configure MPU6050
void MPU6050_Init()
{
  Serial.println("Initialising MPU6050...");
  delay(200);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
  Serial.println("MPU6050 initialised!");
  
  uint16_t avg_count = 500;
  
  for (uint16_t i = 0; i < avg_count; i++)
  {
    Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H, false);
    MPU_ACC_AVG += (MPU_ACC / (double)(AccelScaleFactor));
    MPU_GYRO_AVG += (MPU_GYRO / (double)(GyroScaleFactor));       
  }
  MPU_ACC_AVG /= (double)avg_count;
  MPU_GYRO_AVG /= (double)avg_count;

  Serial << "Got GYRO_AVG: " << MPU_GYRO_AVG << "\nGot ACC_AVG: " << MPU_ACC_AVG << "\n";
  
  if (MPU_GYRO_AVG(GZ) > 2.7)   // TODO: Try to fix this without resetting!
  {
    Serial << "Got incorrect average values, resetting the module...\n";
    ESP.restart(); 
  }
  delay(400);
}

BLA::Matrix<3> find_mpu_averages(uint16_t avg_count, uint8_t delay_ms, boolean is_acc)
{

  BLA::Matrix<3> mpu_acc_avg, mpu_gyro_avg;
  mpu_acc_avg.Fill(0);
  mpu_gyro_avg.Fill(0);
  
  for (uint16_t i = 0; i < avg_count; i++)
  {
    check_flight_status();
    Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H, true);
//    Serial << "AVG:: ACC: " << MPU_ACC << " GYRO: " << MPU_GYRO << "\n";
    mpu_acc_avg += MPU_ACC;
    mpu_gyro_avg += MPU_GYRO;
    delay(delay_ms);
  }

  mpu_acc_avg /= avg_count;
  mpu_gyro_avg /= avg_count;
  
  if (is_acc) return mpu_acc_avg;
  return mpu_gyro_avg;
}

void calibrate_flight_thrust()
{
  Serial << "Calibrating thrust...\n";
  char udp_message[150];
  uint8_t min_gyro_delta = 15, delta_thrust;
  double gyro_z;
  BLA::Matrix<4> thrust_vector;

  FLIGHT_THRUST = 5;
  
  while (!IS_FLIGHT_ACHIEVED)
  {
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
    
    gyro_z = (double)find_mpu_averages(200, 10, false)(GZ);
    Serial.print("GOT GYRO Z = "); Serial.println(gyro_z);
    gyro_z = (double)abs(gyro_z);
    sprintf(udp_message, "Current gyro Z delta = %lf, min needed for flight = %d...\n", gyro_z, min_gyro_delta);
    send_udp(udp_message);
    
    if (gyro_z >= min_gyro_delta) IS_FLIGHT_ACHIEVED = true;
    else FLIGHT_THRUST = (FLIGHT_THRUST + 3) % MAX_THRUST;

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
  FLIGHT_THRUST += 1;
  thrust_vector.Fill(FLIGHT_THRUST);
  update_esc_power(thrust_vector);
  Serial << "Achieved flight at thrust = " << FLIGHT_THRUST << "...\n";
  sprintf(udp_message, "Achieved flight at thrust = %d...\n", FLIGHT_THRUST);
  send_udp(udp_message);
  delay(1000);
}

void check_flight_status()
{
  int i = 0;
  char udp_message[100];
  sprintf(udp_message, "Server listening on %s:%d, waiting for response...\n",  WiFi.localIP().toString().c_str(), SERVER_PORT);
  server.handleClient();
  
  while(!INITIATE_FLIGHT)
  {
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

//  Server code:

void initiate_server()
{
  char udp_message[100];
  server.on("/initiate", initiate_flight);
  server.on("/abort", abort_flight);
  server.onNotFound(api_not_found);

  server.begin();
//  handle_server_requests.attach(0.2, handle_client_requests);
  
  Serial << "Server initiated, listening on "; Serial.print(WiFi.localIP()); Serial << ":" << SERVER_PORT << "\n";
  sprintf(udp_message, "Server initiated, listening on %s:%d...\n",  WiFi.localIP().toString().c_str(), SERVER_PORT);
  send_udp(udp_message);
}

void initiate_flight()
{
  char *message = "Command received, initiating the flight sequence...\n";
  Serial << message;
  send_udp(message);
  INITIATE_FLIGHT = true;
  IS_FLIGHT_ACHIEVED = false;
  
  server.send(200, "text/plain", message);
}

void abort_flight()
{
  BLA::Matrix<4> thrust_vector;
  char *message = "Command received, aborting...\n";
  Serial << message;
  send_udp(message);
        
  FLIGHT_THRUST = MIN_THRUST;
  thrust_vector.Fill(FLIGHT_THRUST);
  update_esc_power(thrust_vector);
  INITIATE_FLIGHT = false;
  
  server.send(200, "text/plain", message);
}

void api_not_found()
{
  server.send(404, "text/plain", "Not found");
}

void handle_client_requests()
{

}
