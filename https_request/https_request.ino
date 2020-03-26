#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <wireless_operations.h>

const char *ssid = "sharma";  //ENTER YOUR WIFI SETTINGS
const char *password = "H0m$#@12345";

void setup() 
{
  delay(1000);
  Serial.begin(115200);
  connect_AP(ssid, password);
}


void https_post(String host, String api, const char *fingerprint, String http_payload, String http_headers) 
{
  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient
  const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80
  
  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);
  
  Serial.print("HTTPS Connecting");
  int r=0; //retry counter
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }
  if(r==30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected to web");
  }

  Serial.print("requesting URL: ");
  Serial.println(host);

  httpsClient.print(String("POST ") + api + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json"+ "\r\n" +
               "Content-Length: " + http_payload.length() + "\r\n\r\n" +
               http_payload + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
                  
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  Serial.println("Server response:");
  Serial.println(".............................................................");
  String line;
  while(httpsClient.available()){        
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println(".............................................................");
  Serial.println("Closing connection...");
    
//  delay(2000);  //POST Data at every 2 seconds
}
//=======================================================================

void send_slack_message(String from_username, String destination, String message, String icon)
{
  const char *host = "hooks.slack.com";
  String api = "/services/";
  const char *slack_fingerprint = "C1 0D 53 49 D2 3E E5 2B A2 61 D5 9E 6F 99 0D 3D FD 8B B2 B3";
  String http_headers = "{\"Content-Type\": \"application/json\"}";
  
  String payload = "{\"channel\": \"" + destination + 
                   "\",\"username\": \"" + from_username + 
                   "\",\"text\": \""+ message + 
                   "\",\"icon_emoji\": \":" + icon + ":\"}";
  
  https_post(host, api, slack_fingerprint, payload, http_headers);
}

void test()
{
  String from_username = "Aaditya Sharma";
  String destination = "@aadityasharma";
  String message = "hello world!";
  String icon = "rube";
  
  send_slack_message(from_username, destination, message, icon);
  
  delay(600000);
}
