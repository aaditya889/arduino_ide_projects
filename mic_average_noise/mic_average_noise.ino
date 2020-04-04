#include "constants.h"
#include <ArduinoJson.h>
#include <Ticker.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <slack_api.h>
#include <wireless_operations.h>
#include <http_request.h>

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
double current_unit_average = 0.0;
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
  Serial.setDebugOutput(true);
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


void __post_message(String payload)
{
  String http_header = "{\"Content-Type\": \"application/json\"}";
  http_post("192.168.1.17", "/consume/set_data", 8000, payload, http_header);
}


void loop()
{ 
  pdb = db;
  adc = analogRead(MIC);
//  db = (adc+83.2073) / 11.003; //Convert ADC value to dB using Regression values
//  db = adc;
  db_sum += abs(adc - 512 - diff_constant);
  factor++;
//  yield();

  if(send_message)
  {
    end_time = timeClient.getFormattedTime();
    find_average_ticker.detach();

    Serial.println("Posting the message");
//    __send_slack_message(start_time + " - " + end_time + ": " + String(current_unit_average));
    __post_message("{\"pod_name\": \"platform\", \"value\": \"" + String(current_unit_average) + "\"}");

//    Serial.println("Getting the google message");
//    http_get("http://google.com", 80);
//    Serial.println("Getting the local message");
//    http_get("192.168.1.17", 80);
    
    send_message = false;
    find_average_ticker.attach(time_interval_secs, find_average_and_reset);
    start_time = timeClient.getFormattedTime();
  }
}
