#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "sharma";
const char* password = "H0m$#@12345";

void setup() 
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print("."); 
  }
  Serial.println("Connected!");
}

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
//    const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
//    DynamicJsonDocument root(bufferSize);
//    deserializeJson(root, http.getString());
//    int id = root["id"]; 
//    const char* name = root["name"]; 
//    const char* username = root["username"]; 
//    const char* email = root["email"]; 
//  
//    Serial.print("Name:");
//    Serial.println(name);
//    Serial.print("Username:");
//    Serial.println(username);
//    Serial.print("Email:");
//    Serial.println(email);
    Serial.println("Got response:");
    Serial.println(http.getString());
  }
  else
  {
    Serial.println("GET request failed!");  
  }
  http.end(); //Close connection
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
  
//  for(JsonPair& node : header_json) 
//  {
//    Serial.print("0 State: ");
//    Serial.println(*node);
//  }
//  serializeJsonPretty(header_json, Serial);

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
  
  http.end(); //Close connection
}

void loop()
{
//  setup();
  char *test_url_1 = "http://jsonplaceholder.typicode.com/users/1";
  char *test_url_2 = "https://google.com";
  char *test_url_3 = "http://google.com";
  char * google_SHA1 = "F9:D6:3C:0F:77:F9:CA:BD:8A:D9:B6:16:40:A1:62:7E:40:F7:F7:DD";
  const char *SLACK_POST_MESSAGE_URL = "https://hooks.slack.com/services/<CODE>";
  const char *slack_payload = "{\"channel\": \"@aadityasharma\",\"username\": \"test_user\",\"text\": \"test_message\",\"icon_emoji\": \":rube:\"}";
  const char *slack_headers = "{\"Content-Type\": \"application/json\"}";
   
//  http_post(SLACK_POST_MESSAGE_URL, slack_payload, slack_headers);
  Serial.println("Trying with 1");
  http_get(test_url_1);
  Serial.println("Trying with 2");
  http_get(test_url_2);
    Serial.println("Trying with 3");
  http_get(test_url_3);
  delay(60000);
}
