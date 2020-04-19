#include "constants.h"
#include <WiFiUdp.h>
#include <wireless_operations.h>
//#include <Ticker.h>

//Ticker transmit_audio_data_ticker;
WiFiUDP udp_client;
///const uint8_t INSTRUCTION_TIME_US = 10;
const uint8_t delay_time_us = ((1.0 / (double) SAMPLE_RATE_KHZ) * 1000) - INSTRUCTION_TIME_US;
const uint16_t packet_size_in_bytes = ((SAMPLE_RATE_KHZ * 1000) * (BIT_DEPTH) * CAPTURE_TIME_IN_MS) / (8 * 1000);
const uint8_t packet_size_buffer = 100;
unsigned char audio_data[packet_size_in_bytes + packet_size_buffer];

const int MIC = 0; //the microphone amplifier output is connected to pin A0
uint16_t adc;
uint16_t audio_ptr = 0;
bool send_data;

void setup() 
{
  Serial.begin(SERIAL_BAUD_RATE);
//  pinMode(3, OUTPUT);
//  WiFi.mode(WIFI_NONE_SLEEP);
  connect_AP(ssid, password);
//  transmit_audio_data_ticker.attach((float)(CAPTURE_TIME_IN_MS / 1000.0), transmit_audio_data);
//  Serial.println("Printing variables:");
//  Serial.println(delay_time_us);
//  Serial.println(SAMPLE_RATE_KHZ);
//  Serial.println(BIT_DEPTH);
//  Serial.println(CAPTURE_TIME_IN_MS);
//  Serial.println();
//  Serial.println((float)(CAPTURE_TIME_IN_MS / 1000.0));
  send_data = false;
}


void transmit_audio_data()
{
  send_data = true;
//  Serial.println("Setting to true...");
}

void loop()
{
  audio_data[audio_ptr++] = analogRead(MIC);
  delayMicroseconds(delay_time_us);

  if (audio_ptr >= packet_size_in_bytes)
  {
//    Serial.println("sending data...");
    udp_client.beginPacket(REMOTE_IP, REMOTE_PORT);
    udp_client.write((char*)audio_data, sizeof(audio_data));
//    delay(1);
    udp_client.endPacket();
//    delay(1);
    audio_ptr = 0;
    send_data = false;
  }
  yield();
}
