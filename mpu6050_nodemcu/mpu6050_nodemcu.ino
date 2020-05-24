#include <Ticker.h>
#include <WiFiUdp.h>
#include <wireless_operations.h>
#include <Servo.h>
#include <Wire.h>
#include "constants.h"
#include <BasicLinearAlgebra.h>
#define AX 0
#define AY 1
#define AZ 2
#define GX 0
#define GY 1
#define GZ 2

using namespace BLA;
WiFiUDP udp_client;

// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

BLA::Matrix<3> MPU_ACC, MPU_GYRO, MPU_ACC_AVG, MPU_GYRO_AVG;
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
  BLA::Matrix<5> AA = {1,2,3,4,5};
  AA /= (double)5;
  Serial << "AA:" << AA;
  MPU_ACC.Fill(0);
  MPU_GYRO.Fill(0);
  MPU_ACC_AVG.Fill(0);
  MPU_GYRO_AVG.Fill(0);
  Wire.begin(sda, scl);
  MPU6050_Init();
}

void loop()
{
  double Ax, Ay, Az, T, Gx, Gy, Gz;
  
  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
  
  //divide each with their sensitivity scale factor
  Ax = ((double)AccelX / AccelScaleFactor) - acc_x_avg;
  Ay = ((double)AccelY / AccelScaleFactor) - acc_y_avg;
  Az = ((double)AccelZ / AccelScaleFactor) - acc_z_avg;
  T = (double)Temperature/340+36.53; //temperature formula
  Gx = ((double)GyroX / GyroScaleFactor) - gyro_x_avg;
  Gy = ((double)GyroY / GyroScaleFactor) - gyro_y_avg;
  Gz = ((double)GyroZ / GyroScaleFactor) - gyro_z_avg;

  MPU_ACC = (MPU_ACC / (double) AccelScaleFactor) - MPU_ACC_AVG;
  MPU_GYRO = (MPU_GYRO / (double) GyroScaleFactor) - MPU_GYRO_AVG;

  sprintf(mpu_data, "AX: %10lf AY: %10lf AZ: %10lf GX: %10lf GY: %10lf GZ: %10lf", MPU_ACC(AX), MPU_ACC(AY), MPU_ACC(AZ), MPU_GYRO(GX), MPU_GYRO(GY), MPU_GYRO(GZ));

  Serial.println(mpu_data);

  // udp send takes around 700 - 750 microseconds
  udp_client.beginPacket(REMOTE_IP, REMOTE_PORT);
  udp_client.write((char*)mpu_data, strlen(mpu_data));
  udp_client.endPacket();
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
  delay(150);
  Serial.println("Initialising MPU6050...");
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
  uint8_t avg_count = 100;
  
  for (int i = 0; i < avg_count; i++)
  {
    Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
    
    MPU_ACC_AVG += (MPU_ACC / (double)(AccelScaleFactor));
    MPU_GYRO_AVG += (MPU_GYRO / (double)(GyroScaleFactor));       
  }
  MPU_ACC_AVG /= (double)avg_count;
  MPU_GYRO_AVG /= (double)avg_count;
}
