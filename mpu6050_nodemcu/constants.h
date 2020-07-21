#include <BasicLinearAlgebra.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

#define NTP_OFFSET   19800      // In seconds
#define NTP_INTERVAL 60 * 1000    // In milliseconds
#define SERIAL_BAUD_RATE 115200
#define MAX_DATA_RETENTION 1024
#define FRONTMA 0
#define FRONTMB 1
#define REARMA 2
#define REARMB 3

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

uint8_t REINIT_CALLED = 0;
//const char *ssid = "sharma";
//const char *password = "H0m$#@12345";
const char *ssid = "aad";
const char *password = "yoyoyoyo";
uint16_t SERVER_PORT = 80;
Ticker COMBINE_MPU_DATA_TICKER, BALANCE_DRONE_TICKER, UDP_STATS_EXPORTER;
const uint8_t COMBINE_MPU_DATA_TICKER_INTERVAL_MS = 3, BALANCE_DRONE_TICKER_INTERVAL_MS = 1, UDP_STATS_EXPORTER_INTERVAL_MS = 5;
float BALANCE_SENSITIVITY = 1.7;
float BALANCE_AMPLITUDE = 0.8;

const char REMOTE_IP[] = "192.168.43.13";
const uint16_t REMOTE_PORT = 8000;
const uint8_t MIN_PULSE = 0;
const uint8_t MAX_PULSE = 180;

boolean INITIATE_FLIGHT = false;
boolean IS_FLIGHT_ACHIEVED = false;
boolean IS_AUTO_BALANCED = false;
const double ACC_WEIGHT = 0.02;
const uint16_t ACC_SCALE_FACTOR = 16384;
const uint16_t GYRO_SCALE_FACTOR = 131;

WiFiUDP udp_client;
ESP8266WebServer server(SERVER_PORT);

//  Global matrices:

//  Gyro-accelerometer axis mapping
BLA::Matrix<3> ADX = {0,1,0}, ADY = {-1,0,0}, ADZ = {0,0,0}, MPU_ACC_OFF = {0,0,1}, DES_YPR = {0,0,0};
BLA::Matrix<3,3> ANGLE_DELTA_TRANSFORM = {0,-1,0, 1,0,0, 0,0,0};
// Changing global variables
BLA::Matrix<3> MPU_ACC_AVG, MPU_GYRO_AVG, YPR_GYRO, YPR;
BLA::Matrix<4> CURRENT_THRUST_VECTOR, DESIRED_THRUST_VECTOR, CURRENT_THRUST_RATIO, STABLE_THRUST_RATIO = {0,0,0,0};

uint32_t GYRO_START_TIME = 0, GYRO_END_TIME = 0;
uint8_t FLIGHT_THRUST = 0;
int FLIGHT_THRUST_DIFF = 0;
