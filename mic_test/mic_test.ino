#include "constants.h"
#include <ArduinoJson.h>
#include <Ticker.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <slack_api.h>
#include <wireless_operations.h>


String current_output;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

Ticker find_average_ticker;
bool send_message = false;
const int MIC = 0; //the microphone amplifier output is connected to pin A0
int adc;
int db, pdb; //the variable that will hold the value read from the microphone each time
unsigned long long db_sum = 0;
unsigned long long factor = 0;
double current_unit_average;
String start_time, end_time;

void find_average_and_reset()
{
  current_unit_average = (double)db_sum / ((double)factor * 2);
//  current_output = /
//  Serial.println(current_output);/
  send_message = true;
  db_sum = 0;
  factor = 0;    
}


void setup() 
{
  Serial.begin(SERIAL_BAUD_RATE);
//  pinMode(3, OUTPUT);
  connect_AP(ssid, password);
  
  timeClient.begin();
  timeClient.setTimeOffset(NTP_OFFSET);
  timeClient.forceUpdate();
  timeClient.update();
  find_average_ticker.attach(time_interval_secs, find_average_and_reset);
  start_time = timeClient.getFormattedTime();
}


void __send_slack_message(String message)
{
  String from_username = "Aaditya Sharma";
  String destination = "@aadityasharma";
  String icon = "rube";
  
  send_slack_message(from_username, destination, message, icon);
}


void loop()
{ 
  pdb = db;
  adc= analogRead(MIC);
//  db = (adc+83.2073) / 11.003; //Convert ADC value to dB using Regression values
//  db = adc;
  db_sum += abs(adc - 512 - diff_constant);
  factor++;

  if(send_message)
  {
    end_time = timeClient.getFormattedTime();
    find_average_ticker.detach();
    __send_slack_message(start_time + " - " + end_time + ": " + String(current_unit_average));
    send_message = false;
    find_average_ticker.attach(time_interval_secs, find_average_and_reset);
    start_time = timeClient.getFormattedTime();
  }
}
