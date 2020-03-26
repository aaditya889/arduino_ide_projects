#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>

Ticker find_average_ticker;
const int MIC = 0; //the microphone amplifier output is connected to pin A0
int adc;
int db, pdb; //the variable that will hold the value read from the microphone each time
unsigned long long db_sum = 0;
unsigned long long factor = 0;
double unit_average;
//unsigned int *second_averages; 


void find_average_and_reset()
{
  unit_average = (double)db_sum / (double)factor;
  Serial.println((unsigned long)db_sum);
  Serial.println((unsigned long)factor);
  Serial.println(unit_average);
  Serial.println("");
  Serial.println("");
  db_sum = 0;
  factor = 0;    
}


void setup() 
{
  Serial.begin(115200); //sets the baud rate at 9600 so we can check the values the microphone is obtaining on the Serial Monitor
  pinMode(3, OUTPUT);
  find_average_ticker.attach(10, find_average_and_reset);
}


void loop()
{ 
  pdb = db;
  adc= analogRead(MIC); 
//  db = (adc+83.2073) / 11.003; //Convert ADC value to dB using Regression values
//  db = adc;
  db_sum += adc;
  factor++;
  
    
}
