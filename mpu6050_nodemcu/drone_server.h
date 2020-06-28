#include <BasicLinearAlgebra.h>
#ifndef __constants_h__
#define __constants_h__
#endif
#ifndef __drone_h__
#define __drone_h__
#endif

//  Function definitions
void initiate_flight();
void api_not_found();
void abort_flight();
void simulate_flight();

void send_udp(char *message)
{
  // udp send takes around 700 - 750 microseconds
  udp_client.beginPacket(REMOTE_IP, REMOTE_PORT);
  udp_client.write(message, strlen(message));
  udp_client.endPacket();
}

//  Server code:

void initiate_server()
{
  char udp_message[100];
  server.on("/initiate", initiate_flight);
  server.on("/abort", abort_flight);
  server.on("/simulate_flight", simulate_flight);
  server.onNotFound(api_not_found);

  server.begin();
//  handle_server_requests.attach(0.2, handle_client_requests);
  
  Serial << "Server initiated, listening on "; Serial.print(WiFi.localIP()); Serial << ":" << SERVER_PORT << "\n";
  sprintf(udp_message, "Server initiated, listening on %s:%d...\n",  WiFi.localIP().toString().c_str(), SERVER_PORT);
  send_udp(udp_message);
}

void initiate_flight()
{
  char *message = "Command received, initiating the flight sequence...\n";
  Serial << message;
  send_udp(message);
  INITIATE_FLIGHT = true;
  IS_FLIGHT_ACHIEVED = false;
  
  server.send(200, "text/plain", message);
}

void simulate_flight()
{
  char *message = "Command received, simulating flight = true...\n";
  Serial << message;
  send_udp(message);
  IS_FLIGHT_ACHIEVED = true;
  
  server.send(200, "text/plain", message);
}

void abort_flight()
{
  BLA::Matrix<4> thrust_vector;
  char *message = "Command received, aborting...\n";
  Serial << message;
  send_udp(message);
        
  change_flight_thrust(MIN_PULSE);
  INITIATE_FLIGHT = false;
  IS_FLIGHT_ACHIEVED = false;
  
  server.send(200, "text/plain", message);
}

void api_not_found()
{
  server.send(404, "text/plain", "Not found");
}
