#define NTP_OFFSET   19800      // In seconds
#define NTP_INTERVAL 60 * 1000    // In milliseconds
#define SERIAL_BAUD_RATE 115200
#define MAX_DATA_RETENTION 1024
#define FRONTMA 0
#define FRONTMB 1
#define REARMA 2
#define REARMB 3

const char *ssid = "sharma";
const char *password = "H0m$#@12345";
uint16_t SERVER_PORT = 80;

const unsigned short diff_constant = 40;
const short time_interval_secs = 10;
const char REMOTE_IP[] = "192.168.1.15";
const uint16_t REMOTE_PORT = 8000;
const uint8_t MIN_THRUST = 0;
const uint8_t MAX_THRUST = 180;

const uint8_t INSTRUCTION_TIME_US = 15;
// DEFINE AUDIO QUALITY
const uint8_t BIT_DEPTH = 8;      //immutable for now!
const uint8_t SAMPLE_RATE_KHZ = 16;
const short CAPTURE_TIME_IN_MS = 20;
boolean INITIATE_FLIGHT = false;
float HANDLE_REQUEST_TIME = 0.2;
