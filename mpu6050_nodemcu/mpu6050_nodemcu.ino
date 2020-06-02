#include <Ticker.h>
#include <WiFiUdp.h>
#include <wireless_operations.h>
#include <Servo.h>
#include <Wire.h>
#include "constants.h"
#include <math.h>
#include <BasicLinearAlgebra.h>
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
#define FRONTMA 0
#define FRONTMB 1
#define REARMA 2
#define REARMB 3
#define FRONTA D0
#define FRONTB D1
#define REARA D2
#define REARB D3

//  TODO: TURN THE FILTER FUNCTION INTO A BLACK BOX, AND REDUCE THE CODE IN THE LOOP() FUNCTION
//  TODO: REDUCE GLOBAL VARIABLE COUNT AND RECTIFY THE LINTING!!
//  TODO: TRY TO REUSE VARIABLES AND REDUCE THE AMOUNT OF MEMORY USED (TO COMPENSATE FOR THE POSSIBILITY OF CODE EXPANSION)!
//  MOVE SOME OF THE GLOBAL VARS TO CONSTANTS FILE

using namespace BLA;
WiFiUDP udp_client;

// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;
uint32_t GYRO_START_TIME, GYRO_END_TIME;
const double ACC_WEIGHT = 0.02;
uint8_t FLIGHT_THRUST = 60;

BLA::Matrix<3> MPU_ACC, MPU_GYRO, MPU_ACC_AVG, MPU_GYRO_AVG, MPU_ACC_OFF, ANGLE_DELTA, GYRO_ANGLES, YPR_GYRO = {0,0,0}, YPR_ACC = {0,0,0}, YPR = {0,0,0}, DES_YPR = {0,0,0};
BLA::Matrix<4> THRUST_MATRIX;
BLA::Matrix<3> ADX = {0,1,0}, ADY = {-1,0,0}, ADZ = {0,0,0};
//int16_t Temperature;

int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;
double acc_x_avg = 0, acc_y_avg = 0, acc_z_avg = 0, gyro_x_avg = 0, gyro_y_avg = 0, gyro_z_avg = 0;

char mpu_data[150];

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
  delay(10000);

//  INIT_POW_MATRIX = {20,20,20,20};    //calculate the actual values
//  POW_MATRIX = INIT_POW_MATRIX;
  MPU6050_Init();
}

void loop()
{
//  double Ax, Ay, Az, T, Gx, Gy, Gz;

  GYRO_START_TIME = micros();
  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
  
  //divide each with their sensitivity scale factor
//  Ax = ((double)AccelX / AccelScaleFactor) - acc_x_avg;
//  Ay = ((double)AccelY / AccelScaleFactor) - acc_y_avg;
//  Az = ((double)AccelZ / AccelScaleFactor) - acc_z_avg;
//  T = (double)Temperature/340+36.53; //temperature formula
//  Gx = ((double)GyroX / GyroScaleFactor) - gyro_x_avg;
//  Gy = ((double)GyroY / GyroScaleFactor) - gyro_y_avg;
//  Gz = ((double)GyroZ / GyroScaleFactor) - gyro_z_avg;
  
  MPU_ACC = (MPU_ACC / (double) AccelScaleFactor) - MPU_ACC_AVG + MPU_ACC_OFF;
  MPU_GYRO = (MPU_GYRO / (double) GyroScaleFactor) - MPU_GYRO_AVG;

  for (int i = 0; i < MPU_ACC.GetRowCount(); i++) 
  {
    YPR_ACC(i) = atan2f((double)(MPU_ACC(i)), (double)MPU_ACC(AZ)) * (180 / PI);
  }

  GYRO_END_TIME = micros();

  ANGLE_DELTA = (MPU_GYRO * 4.0 * (double)((GYRO_END_TIME - GYRO_START_TIME) / (double) MEGA));
  YPR_GYRO += ADX * (double)ANGLE_DELTA(GX) + ADY * (double)ANGLE_DELTA(GY) + ADZ * (double)ANGLE_DELTA(GZ);

  YPR = YPR_ACC * (double)(ACC_WEIGHT) + YPR_GYRO * (double)(1 - ACC_WEIGHT);
  Serial  << "[YPR] => " << YPR << " [YPR_GYRO] => " << YPR_GYRO << " [YPR_ACC] => " << YPR_ACC << "\n";
  YPR_GYRO = YPR;
  sprintf(mpu_data, "YX: %10lf YY: %10lf YZ: %10lf AX: %10lf AY: %10lf AZ: %10lf", YPR(AX), YPR(AY), YPR(AZ), MPU_ACC(AX), MPU_ACC(AY), MPU_ACC(AZ));

  YPR = YPR - DES_YPR;

  update_thrust_vector();
  
  // udp send takes around 700 - 750 microseconds
  udp_client.beginPacket(REMOTE_IP, REMOTE_PORT);
  udp_client.write((char*)mpu_data, strlen(mpu_data));
  udp_client.endPacket();
}

void update_thrust_vector()
{
  uint8_t roll_deviation = abs(YPR(ROLL));
  uint8_t pitch_deviation = abs(YPR(PITCH));

// Fix roll
  if (YPR(ROLL) < 0) 
  {
    THRUST_MATRIX(FRONTMA) = THRUST_MATRIX(FRONTMB) = map(roll_deviation, 90, 0, 0, FLIGHT_THRUST);
    THRUST_MATRIX(REARMA) = THRUST_MATRIX(REARMB) = map(roll_deviation, 0, 90, FLIGHT_THRUST, 2 * FLIGHT_THRUST); 
  }
  else
  {
    THRUST_MATRIX(FRONTMA) = THRUST_MATRIX(FRONTMB) = map(roll_deviation, 0, 90, FLIGHT_THRUST, 2 * FLIGHT_THRUST);
    THRUST_MATRIX(REARMA) = THRUST_MATRIX(REARMB) = map(roll_deviation, 90, 0, 0, FLIGHT_THRUST);
  }
//  Fix Pitch
  if (YPR(ROLL) < 0) 
  {
    THRUST_MATRIX(FRONTMA) = THRUST_MATRIX(REARMA) = map(roll_deviation, 90, 0, 0, FLIGHT_THRUST);
    THRUST_MATRIX(FRONTMB) = THRUST_MATRIX(REARMB) = map(roll_deviation, 0, 90, FLIGHT_THRUST, 2 * FLIGHT_THRUST); 
  }
  else
  {
    THRUST_MATRIX(FRONTMA) = THRUST_MATRIX(REARMA) = map(roll_deviation, 0, 90, FLIGHT_THRUST, 2 * FLIGHT_THRUST);
    THRUST_MATRIX(FRONTMB) = THRUST_MATRIX(REARMB) = map(roll_deviation, 90, 0, 0, FLIGHT_THRUST);
  }
}

void update_drone_thrust()
{
  
}

void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data)
{
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

// read all 14 register
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress)
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
}

//configure MPU6050
void MPU6050_Init()
{
  Serial.println("Initialising MPU6050...");
  delay(2000);
//  delay(100);
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

  Serial.println("");
  uint8_t avg_count = 500;
  
  for (int i = 0; i < avg_count; i++)
  {
    Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
    
    MPU_ACC_AVG += (MPU_ACC / (double)(AccelScaleFactor));
    MPU_GYRO_AVG += (MPU_GYRO / (double)(GyroScaleFactor));       
  }
  MPU_ACC_AVG /= (double)avg_count;
  MPU_GYRO_AVG /= (double)avg_count;

  Serial << "Got GYRO_AVG: " << MPU_GYRO_AVG << "\nGot ACC_AVG: " << MPU_ACC_AVG << "\n";
  delay(4000);

}
