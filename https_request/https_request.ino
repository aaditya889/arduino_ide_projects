#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

/* Set these to your desired credentials. */
const char *ssid = "sharma";  //ENTER YOUR WIFI SETTINGS
const char *password = "H0m$#@12345";

//const char fingerprint[] PROGMEM = "06 68 CF CF 1D 2C E8 30 5F DF 33 C9 33 C3 10 01 22 13 15 1E";
//Link to read data from https://jsonplaceholder.typicode.com/comments?postId=7
//Web/Server address to read/write from 
//=======================================================================
//                    Power on setup
//=======================================================================

void setup() 
{
  delay(1000);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
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
  
//  String getData, Link;
  
//  //POST Data
//  Link = "/post";

  Serial.print("requesting URL: ");
  Serial.println(host);
  /*
   POST /post HTTP/1.1
   Host: postman-echo.com
   Content-Type: application/x-www-form-urlencoded
   Content-Length: 13
  
   say=Hi&to=Mom
    
   */

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

  Serial.println("reply was:");
  Serial.println("==========");
  String line;
  while(httpsClient.available()){        
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println("==========");
  Serial.println("closing connection");
    
//  delay(2000);  //POST Data at every 2 seconds
}
//=======================================================================

void send_slack_message(String from_username, String destination, String message, String icon)
{
  const char *host = "hooks.slack.com";
  String api = "/services/T034MTGTM/B32G6C3EV/UgJsOUiHlRVqnfn5LLtksHZV";
  const char *slack_fingerprint = "C1 0D 53 49 D2 3E E5 2B A2 61 D5 9E 6F 99 0D 3D FD 8B B2 B3";
  String http_headers = "{\"Content-Type\": \"application/json\"}";
  
  String payload = "{\"channel\": \"" + destination + 
                   "\",\"username\": \"" + from_username + 
                   "\",\"text\": \""+ message + 
                   "\",\"icon_emoji\": \":" + icon + ":\"}";
  
  https_post(host, api, slack_fingerprint, payload, http_headers);
}

void loop()
{
  String from_username = "Aaditya Sharma";
  String destination = "@aadityasharma";
  String message = "hello world!";
  String icon = "rube";
  
  send_slack_message(from_username, destination, message, icon);
  
  delay(600000);
}
