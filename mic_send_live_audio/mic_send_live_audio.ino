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
char *audio_data, audio_l, audio_h, *audio_ptr;
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
  audio_data = (char *)malloc((MAX_DATA_RETENTION) * 2.5);
  
}

void loop()
{
//    audio_data = (audio_data << 10) | adc;
    
    for(uint16_t i = 0; i < MAX_DATA_RETENTION; i+=2)
    {
      adc = analogRead(MIC);
      audio_data[i+1] = adc & 0xff;
      audio_data[i] = (adc >> 8) & 0xff;
//      Serial.print(adc);
//      Serial.print(" ");
    }
//    Serial.println(" ");
//    Serial.println(audio_data);
//    Serial.println("Sending data");
    udp_client.beginPacket(REMOTE_IP, REMOTE_PORT);
    delay(1);
//    Serial.println(sizeof(audio_data));
    udp_client.write(audio_data);
    delay(1);
    udp_client.endPacket();
//    Serial.println(udp_client.endPacket());
    delay(1);
//    delay(60000);
}