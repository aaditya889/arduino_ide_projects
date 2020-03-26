#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>


void https_post(String host, String api, const char *fingerprint, String http_payload, String http_headers) 
{
  WiFiClientSecure httpsClient;
  const int httpsPort = 443;
  
  Serial.print("Host: ");
  Serial.print(host);
  Serial.println(api);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  // delay(1000);
  
  Serial.print("HTTPS Connecting...");
  int retry = 0; //retry counter
  while((!httpsClient.connect(host, httpsPort)) && (retry < 30))
  {
      delay(100);
      Serial.print(".");
      retry++;
  }
  if(retry == 30) {
    Serial.println("Connection failed");
    return;
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
}

