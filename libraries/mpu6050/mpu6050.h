#include <Wire.h>
#include <math.h>

// MPU6050 Slave Device Address
const uint8_t mpu_slave_address = 0x68;

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

// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D6;
const uint8_t sda = D7;


void i2c_write(uint8_t device_address, uint8_t reg_address, uint8_t data)
{
  Wire.beginTransmission(device_address);
  Wire.write(reg_address);
  Wire.write(data);
  Wire.endTransmission();
}


// read all 14 register
void read_mpu_values(uint8_t device_address, uint8_t reg_address, boolean find_angles)
{
  Wire.beginTransmission(device_address);
  Wire.write(reg_address);
  Wire.endTransmission();
  Wire.requestFrom(device_address, (uint8_t)14);
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
    MPU_GYRO = (MPU_GYRO / (double) GYRO_SCALE_FACTOR) - MPU_GYRO_AVG; 
  }
}


//configure MPU6050
void mpu_init()
{
  Serial.println("Initialising MPU6050...");
  Wire.begin(sda, scl);
  delay(200);
  i2c_write(mpu_slave_address, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  i2c_write(mpu_slave_address, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  i2c_write(mpu_slave_address, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  i2c_write(mpu_slave_address, MPU6050_REGISTER_CONFIG, 0x00);
  i2c_write(mpu_slave_address, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  i2c_write(mpu_slave_address, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  i2c_write(mpu_slave_address, MPU6050_REGISTER_FIFO_EN, 0x00);
  i2c_write(mpu_slave_address, MPU6050_REGISTER_INT_ENABLE, 0x01);
  i2c_write(mpu_slave_address, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  i2c_write(mpu_slave_address, MPU6050_REGISTER_USER_CTRL, 0x00);
  Serial.println("MPU6050 initialised!");
  
  uint16_t avg_count = 500;
  
  for (uint16_t i = 0; i < avg_count; i++)
  {
    read_mpu_values(mpu_slave_address, MPU6050_REGISTER_ACCEL_XOUT_H, false);
    MPU_ACC_AVG += (MPU_ACC / (double)(AccelScaleFactor));
    MPU_GYRO_AVG += (MPU_GYRO / (double)(GYRO_SCALE_FACTOR));       
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
    read_mpu_values(mpu_slave_address, MPU6050_REGISTER_ACCEL_XOUT_H, true);
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
