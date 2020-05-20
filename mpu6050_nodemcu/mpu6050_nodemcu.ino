#include <Ticker.h>
#include <WiFiUdp.h>
#include <wireless_operations.h>
#include <Servo.h>
#include <Wire.h>
#include "constants.h"

WiFiUDP udp_client;

char mpu_data[150];
// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;

// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D6;
const uint8_t sda = D7;

// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

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

int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;
double acc_x_avg = 0, acc_y_avg = 0, acc_z_avg = 0, gyro_x_avg = 0, gyro_y_avg = 0, gyro_z_avg = 0;

void setup() 
{
  Serial.begin(115200);
  connect_AP(ssid, password);
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
//
//  Serial.print("Ax: "); Serial.print(Ax);
//  Serial.print(" Ay: "); Serial.print(Ay);
//  Serial.print(" Az: "); Serial.print(Az);
//  Serial.print(" T: "); Serial.print(T);
//  Serial.print(" Gx: "); Serial.print(Gx);
//  Serial.print(" Gy: "); Serial.print(Gy);
//  Serial.print(" Gz: "); Serial.println(Gz);

  sprintf(mpu_data, "AX: %10lf AY: %10lf AZ: %10lf GX: %10lf GY: %10lf GZ: %10lf T: %10lf", Ax, Ay, Az, Gx, Gy, Gz, T);

//  Serial.println(mpu_data);

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
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}

//configure MPU6050
void MPU6050_Init()
{
  delay(1000);
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
    acc_x_avg += (double) AccelX / (AccelScaleFactor);
    acc_y_avg += (double) AccelY / (AccelScaleFactor);
    acc_z_avg += (double) AccelZ / (AccelScaleFactor);
    
    gyro_x_avg += (double)GyroX / (GyroScaleFactor);
    gyro_y_avg += (double)GyroY / (GyroScaleFactor);
    gyro_z_avg += (double)GyroZ / (GyroScaleFactor);       
  }
  acc_x_avg = (double) acc_x_avg / avg_count;
  acc_y_avg = (double) acc_y_avg / avg_count;
  acc_z_avg = (double) acc_z_avg / avg_count;
  gyro_x_avg = (double) gyro_x_avg / avg_count;
  gyro_y_avg = (double) gyro_y_avg / avg_count;
  gyro_z_avg = (double) gyro_z_avg / avg_count;
  
  Serial.println("Got average values:");
  Serial.println(acc_x_avg);
  Serial.println(acc_y_avg);
  Serial.println(acc_z_avg);
  Serial.println(gyro_x_avg);
  Serial.println(gyro_y_avg);
  Serial.println(gyro_z_avg);
  delay(5000);
}
