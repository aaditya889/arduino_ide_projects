//#include "constants.h"
//#include <WiFiUdp.h>
//#include <wireless_operations.h>
//#include <Ticker.h>
//
//Ticker transmit_audio_data_ticker;
//WiFiUDP udp_client;
//const uint8_t delay_time_us = ((1.0 / (double) SAMPLE_RATE_KHZ) * 1000) - INSTRUCTION_TIME_US;
//const uint16_t packet_size_in_bytes = ((SAMPLE_RATE_KHZ * 1000) * (BIT_DEPTH) * CAPTURE_TIME_IN_MS) / (8 * 1000);
//const uint8_t packet_size_buffer = 100;
//unsigned char audio_data[packet_size_in_bytes + packet_size_buffer];
//
//const int MIC = 0;
//uint16_t adc;
//uint16_t audio_ptr = 0;
//bool send_data;
//
//void setup() 
//{
//  Serial.begin(SERIAL_BAUD_RATE);
//  connect_AP(ssid, password);
//  transmit_audio_data_ticker.attach((float)(CAPTURE_TIME_IN_MS / 1000.0), transmit_audio_data);
//  send_data = false;
//  Serial.println(packet_size_in_bytes);
//}
//
//
//void transmit_audio_data()
//{
//  send_data = true;
//}
//
//void loop()
//{
////  for (int i = 0; i < packet_size_in_bytes; i++)
////  {
//  audio_data[audio_ptr++] = analogRead(MIC) / 4;
////  delayMicroseconds(delay_time_us);  
////  }
////  yield();
//  if (send_data)
//  {
//    Serial.println(audio_ptr);
////    udp_client.beginPacket(REMOTE_IP, REMOTE_PORT);
////    delay(1);
////    udp_client.write((char*)audio_data, sizeof(audio_data));
////  //  delay(1);
////    udp_client.endPacket();
////    delay(1);
//    audio_ptr = 0;
//    send_data = false;
//  }
//}













//#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "constants.h"
#include <WiFiUdp.h>
#include <wireless_operations.h>

Ticker timer;
const int MIC = 0;

volatile uint16_t n_interrupts;

// ISR to Fire when Timer is triggered
void ICACHE_RAM_ATTR onTime() {
//  Serial.println(n_interrupts);
//  Serial.println("Started");
  analogRead(MIC);
//  Serial.println("Stopped");
  n_interrupts++;
  
}

void setup()
{
  Serial.begin(115200);
  timer1_attachInterrupt(onTime); // Add ISR Function
  timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
  connect_AP(ssid, password);
  /* Dividers:
    TIM_DIV1 = 0,   //80MHz (80 ticks/us - 104857.588 us max)
    TIM_DIV16 = 1,  //5MHz (5 ticks/us - 1677721.4 us max)
    TIM_DIV256 = 3  //312.5Khz (1 tick = 3.2us - 26843542.4 us max)
  Reloads:
    TIM_SINGLE  0 //on interrupt routine you need to write a new value to start the timer again
    TIM_LOOP  1 //on interrupt the counter will start with the same value again
  */
  
  timer1_write(200000); // value / 80 ticks per us from TIM_DIV1 = T us 
}
uint16_t t1, t2;
void loop()
{
  t1= millis();
  while (true)
  {
    t2 = millis();
    if (t2 - t1 >= 1000)
    {
      Serial.println(n_interrupts);
      t1 = millis();
      n_interrupts = 0;
    }
    yield();
  }
}
