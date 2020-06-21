#include <BasicLinearAlgebra.h>
using namespace BLA;

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
const uint8_t MPU6050_REGISTER_ACCEL_YOUT_H =  0x3D;
const uint8_t MPU6050_REGISTER_ACCEL_ZOUT_H =  0x3F;
const uint8_t MPU6050_REGISTER_GYRO_XOUT_H  =  0x43;
const uint8_t MPU6050_REGISTER_GYRO_YOUT_H  =  0x45;
const uint8_t MPU6050_REGISTER_GYRO_ZOUT_H  =  0x47;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;
const uint8_t MPU6050_REGISTER_WHOAMI = 0x75;

// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D6;
const uint8_t sda = D7;

extern void check_flight_status();

void i2c_write(uint8_t device_address, uint8_t reg_address, uint8_t data)
{
  Wire.beginTransmission(device_address);
  Wire.write(reg_address);
  Wire.write(data);
  Wire.endTransmission();
}


// read all 14 register
void read_mpu_values(BLA::Matrix<3> *mpu_values, uint8_t device_address, uint8_t reg_address, boolean find_angles)
{
  int16_t Temperature;
  Wire.beginTransmission(device_address);
  Wire.write(reg_address);
  Wire.endTransmission();
  Wire.requestFrom(device_address, (uint8_t)14);
  while(!Wire.available()) {}

  mpu_values[0](AX) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    // Acc X
  mpu_values[0](AY) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    // Acc Y
  mpu_values[0](AZ) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    //Acc Z
  Temperature = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    // Temp
  mpu_values[1](GX) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    //Gyro X
  mpu_values[1](GY) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    //Gyro Y
  mpu_values[1](GZ) = (int16_t)(((int16_t)Wire.read()<<8) | Wire.read());    //Gyro Z

  if (find_angles)
  {
    mpu_values[0] = (mpu_values[0] / (double) ACC_SCALE_FACTOR) - MPU_ACC_AVG + MPU_ACC_OFF;
    mpu_values[1] = (mpu_values[1] / (double) GYRO_SCALE_FACTOR) - MPU_GYRO_AVG; 
  }
}

void read_mpu_register_bytes(int8_t * mpu_values, uint8_t register_address, uint8_t n_bytes)
{
  uint8_t register_count = 0;

  Wire.beginTransmission(mpu_slave_address);
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(mpu_slave_address, (uint8_t)n_bytes);

  while (Wire.available()) mpu_values[register_count++] = (int8_t)(Wire.read());
}

void read_mpu_register_words(int16_t * mpu_values, uint8_t register_address, uint8_t n_words)
{
  uint8_t register_count = 0;

  Wire.beginTransmission(mpu_slave_address);
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(mpu_slave_address, (uint8_t)n_words * 2);

  while (Wire.available())
  {
    mpu_values[register_count] = (Wire.read() << 8);
    if (Wire.available()) mpu_values[register_count++] |= (Wire.read());
  } 
}

void read_mpu_average_data(BLA::Matrix<3> *mpu_values, uint16_t accel_avg_count, uint16_t gyro_avg_count)
{
  int16_t _mpu_values[3];
  mpu_values[0].Fill(0);
  mpu_values[1].Fill(0);

  for (uint16_t i = 0; i < accel_avg_count; i++)
  {
    read_mpu_register_words(_mpu_values, MPU6050_REGISTER_ACCEL_XOUT_H, 3);
    mpu_values[0](AX) += _mpu_values[0];
    mpu_values[0](AY) += _mpu_values[1];
    mpu_values[0](AZ) += _mpu_values[2];
  }
  mpu_values[0] /= (double)accel_avg_count;

  for (uint16_t i = 0; i < gyro_avg_count; i++)
  {
    read_mpu_register_words(_mpu_values, MPU6050_REGISTER_GYRO_XOUT_H, 3);
    mpu_values[1](GX) += _mpu_values[0];
    mpu_values[1](GY) += _mpu_values[1];
    mpu_values[1](GZ) += _mpu_values[2];
  }
  mpu_values[1] /= (double)gyro_avg_count;

  mpu_values[0] = (mpu_values[0] / (double) ACC_SCALE_FACTOR) - MPU_ACC_AVG + MPU_ACC_OFF;
  mpu_values[1] = (mpu_values[1] / (double) GYRO_SCALE_FACTOR) - MPU_GYRO_AVG; 
}


//configure MPU6050
void mpu_init()
{
  BLA::Matrix<3> mpu_values[2];
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
    read_mpu_values(mpu_values, mpu_slave_address, MPU6050_REGISTER_ACCEL_XOUT_H, false);
    MPU_ACC_AVG += (mpu_values[0] / (double)(ACC_SCALE_FACTOR));
    MPU_GYRO_AVG += (mpu_values[1] / (double)(GYRO_SCALE_FACTOR));       
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

void find_mpu_averages(BLA::Matrix<3> *mpu_avg_values, uint16_t avg_count, uint8_t delay_ms, boolean check_status)
{

  BLA::Matrix<3> mpu_values[2];
  
  for (uint16_t i = 0; i < avg_count; i++)
  {
    if (check_status) check_flight_status();
    read_mpu_values(mpu_values, mpu_slave_address, MPU6050_REGISTER_ACCEL_XOUT_H, true);
    mpu_avg_values[0] += mpu_values[0];
    mpu_avg_values[1] += mpu_values[1];
    if (delay_ms) delay(delay_ms);
  }

  mpu_avg_values[0] /= (double)avg_count;
  mpu_avg_values[1] /= (double)avg_count;
}
