#include <BasicLinearAlgebra.h>
#include <Ticker.h>
#include <WiFiUdp.h>
#include <wireless_operations.h>
#include <Servo.h>
#include <ESP8266WebServer.h>

using namespace BLA;

void setup() 
{
  Serial.begin(115200);
  connect_AP("sharma", "H0m$#@12345");
  delay(3000);
}

void test_fun(BLA::Matrix<3>* mat_test)
{ 
  mat_test[0] = {22.1, 23.0, 24.5};
  mat_test[1] = {10.1, 11.0, 12.5};

}
void loop()
{
  BLA::Matrix<3> vals[2];
  test_fun(vals);
  Serial << "vals 0: " << vals[0] << "vals 1: " << vals[1] << "\n";

  delay(500000);
  
}
