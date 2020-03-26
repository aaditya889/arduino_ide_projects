#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <slack_api.h>
#include <wireless_operations.h>
//#include<string>  /


#define NTP_OFFSET   19800      // In seconds
#define NTP_INTERVAL 60 * 1000    // In milliseconds
//#define NTP_ADDRESS  "ntp-b.nist.gov"
///ntp-b.nist.gov

const char *ssid = "sharma";
const char *password = "H0m$#@12345";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

Ticker find_average_ticker;
const int MIC = 0; //the microphone amplifier output is connected to pin A0
int adc;
int db, pdb; //the variable that will hold the value read from the microphone each time
unsigned long long db_sum = 0;
unsigned long long factor = 0;
double unit_average;
//unsigned int *second_averages; 
const unsigned short diff_constant = 40;
const short time_interval_secs = 10;

void find_average_and_reset()
{
  
//  timeClient.forceUpdate();/
  unit_average = (double)db_sum / ((double)factor * 2);
//  Serial.println((unsigned long)db_sum);
//  Serial.println((unsigned long)factor);
  String output = timeClient.getFormattedTime() + ": " + String(unit_average);

  __send_slack_message("hello!");
//  Serial.println(unit_average);/
//  Serial.println("");/
//  Serial.println("");/
  db_sum = 0;
  factor = 0;    
}


void setup() 
{
  Serial.begin(115200);
  pinMode(3, OUTPUT);
  connect_AP(ssid, password);
  
  timeClient.begin();
  timeClient.setTimeOffset(NTP_OFFSET);
  timeClient.forceUpdate();
  timeClient.update();
  
  find_average_ticker.attach(time_interval_secs, find_average_and_reset);
}


void loop()
{ 
  pdb = db;
  adc= analogRead(MIC);
//  db = (adc+83.2073) / 11.003; //Convert ADC value to dB using Regression values
//  db = adc;
  db_sum += abs(adc - 512 - diff_constant);
  factor++;   
}

void __send_slack_message(String message)
{
  String from_username = "Aaditya Sharma";
  String destination = "@aadityasharma";
  String icon = "rube";
  
  send_slack_message(from_username, destination, message, icon);
  
//  delay(600000);/
}
