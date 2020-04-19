#include "constants.h"
#include <WiFiUdp.h>
#include <wireless_operations.h>
//
//struct uint10_t {
//    uint16_t value : 10;
//    uint16_t _     : 6;
//}current_value;

WiFiUDP udp_client;
char replyPacket[] = "Hi there! Got the message :-)";
unsigned char audio_data[(MAX_DATA_RETENTION) * 2], audio_l, audio_h, *audio_ptr;
const int MIC = 0; //the microphone amplifier output is connected to pin A0
uint16_t adc;
//byte dat = 0;
char *dat = "HELLO";

void setup() 
{
  Serial.begin(SERIAL_BAUD_RATE);
//  pinMode(3, OUTPUT);
  connect_AP(ssid, password);
//  audio_data = malloc((MAX_DATA_RETENTION * 10 / 8) + 1);
//  audio_data = (unsigned char *)malloc((MAX_DATA_RETENTION) * 2.5);
//  #if ARDUINO_VERSION <= 106
//  Serial.println("THIS IS TRUE!");
//  #endif
}

void loop()
{
//    audio_data = (audio_data << 10) | adc;

//    Serial.println("original:");
    for(uint16_t i = 0; i < MAX_DATA_RETENTION; i++)
    {
      adc = analogRead(MIC);
//      audio_data[i+1] = adc & 0xff;
//      audio_data[i] = (adc >> 8) & 0xff;
//      delayMicroseconds(5);
      audio_data[i] = adc;
//      Serial.print(adc);
//      Serial.print(" ");
    }
//    Serial.println(" ");
//    Serial.println("audio_data:");
//    for (uint16_t i = 0; i < MAX_DATA_RETENTION; i+=1)
//    {
//      Serial.print((uint16_t)audio_data[i]);
//      Serial.print(" ");
//    }
//    Serial.println("Sending data");  
    
    udp_client.beginPacket(REMOTE_IP, REMOTE_PORT);
//    delay(1);
    delay(0.001);
//    Serial.println(sizeof(audio_data));
    udp_client.write((char*)audio_data, sizeof(audio_data));
//    delay(1);
    udp_client.endPacket();
//    Serial.pr intln(udp_client.endPacket());
//    delay(0.01);
    delay(1);
//    delay(600000);
}
