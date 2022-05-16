#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

void http_get(const char* host, uint16_t http_port) 
{
  Serial.print("HOST: ");
  Serial.println(host);

  HTTPClient http;
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed YO!!!!!");
    return;
  }
  else{
    Serial.println("YOYOYOYOYO");
  }

  String url = "http://" + (String)host;
  Serial.print("URL: ");
  Serial.println(url);

  http.begin(url);
  
  int httpCode = http.GET();

  if (httpCode > 0) 
  {
    Serial.println("Got response:");
    Serial.println(http.getString());
  }
  else
  {
    Serial.println("GET request failed!");  
    Serial.println(httpCode);
  }
  http.end();
}


void http_post(const char *host, String api, uint16_t port, String http_payload, String http_headers)
{

  WiFiClient http_client;
  
  Serial.println("Host: " + (String)host + " API: "+ api + " Port: " + port);

  Serial.print("HTTP Connecting...");
  int retry = 0;
  while((!http_client.connect(host, port)) && (retry < 30))
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


  HTTPClient http; //Object of class HTTPClient
  DynamicJsonDocument header_json(http_headers.length() + 20);
  DynamicJsonDocument payload_json(http_payload.length() + 200);

  auto error_payload = deserializeJson(payload_json, http_payload);
  auto error_headers = deserializeJson(header_json, http_headers);
  
  if(error_payload || error_headers)
  {
    Serial.print("deserializeJson() failed with code ");
    Serial.print("Payload - ");
    Serial.println(error_payload.c_str());
    Serial.println("Headers - ");
    Serial.println(error_headers.c_str());
    return;
  }

  String content_type = header_json["Content-Type"];
  String url = "http://" + (String)host + ":" + (String)port + api;
  Serial.println("Sending request to: " + url);

  http.begin(url);
  http.addHeader("Content-Type", content_type);

  Serial.println("Sending the POST request...");
  int http_code = http.POST(http_payload);

  if(http_code > 0) 
  {
    Serial.println("POST request succeeded, dumping respnose...");
    Serial.println(http.getString());
  }
  else 
  {
    Serial.println("POST request failed!");
    Serial.println(http_code);
    Serial.println(http.writeToStream(&Serial));
  }

  
  http.end();
}
