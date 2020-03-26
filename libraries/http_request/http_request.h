#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>


void http_get(const char* url) 
{
  if (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println("Network failure! Returning...");
    return;
  }
     
  HTTPClient http; //Object of class HTTPClient
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
  }
  http.end();
}


void http_post(const char *url, const char *http_payload, const char *http_headers)
{
  HTTPClient http; //Object of class HTTPClient
  DynamicJsonDocument header_json(strlen(http_headers) + 20);
  DynamicJsonDocument payload_json(strlen(http_payload) + 200);

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

  const char* content_type = header_json["Content-Type"];
  
  http.begin(url);
  http.addHeader("Content-Type", content_type);

  Serial.println("Printing the URL:");
  Serial.println(url);
  Serial.println("Printing the content-type");
  Serial.println((const char*)header_json["Content-Type"]);
  Serial.println("Printing the payload...");
  serializeJsonPretty(payload_json, Serial);
  Serial.println("Raw Payload:");
  Serial.println(http_payload);

  Serial.println("Sending the POST request...");
  int http_code = http.POST(http_payload);

  if(http_code > 0) 
  {
    Serial.println("POST request succeeded, dumping respnose...");
    Serial.println(http.getString());
  }
  else Serial.println("POST request failed!");
  
  http.end();
}
