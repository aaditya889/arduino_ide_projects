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
void increase_sensitivity();
void increase_amplitude();
void decrease_sensitivity();
void decrease_amplitude();
void trim_left();
void trim_right();
void trim_forward();
void trim_backward();
void trim_left_rotation();
void trim_right_rotation();


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
  server.on("/inc_sensitivity", increase_sensitivity);
  server.on("/inc_amplitude", increase_amplitude);
  server.on("/dec_sensitivity", decrease_sensitivity);
  server.on("/dec_amplitude", decrease_amplitude);
  server.on("/trim_left", trim_left);
  server.on("/trim_right", trim_right);
  server.on("/trim_forward", trim_forward);
  server.on("/trim_backward", trim_backward);
  server.on("/trim_left_rotation", trim_left_rotation);
  server.on("/trim_right_rotation", trim_right_rotation);
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

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void simulate_flight()
{
  char *message = "Command received, simulating flight = true...\n";
  Serial << message;
  send_udp(message);
  IS_FLIGHT_ACHIEVED = true;

  server.sendHeader("Access-Control-Allow-Origin", "*");
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

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void increase_sensitivity()
{
  char message[50];
  BALANCE_SENSITIVITY += 0.1;
  sprintf(message, "sensitivity = %f", BALANCE_SENSITIVITY);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void increase_amplitude()
{
  char message[50];
  BALANCE_AMPLITUDE += 0.1;
  sprintf(message, "amplitude = %f", BALANCE_AMPLITUDE);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void decrease_sensitivity()
{
  char message[50];
  BALANCE_SENSITIVITY -= 0.1;
  sprintf(message, "sensitivity = %f", BALANCE_SENSITIVITY);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void decrease_amplitude()
{
  char message[50];
  BALANCE_AMPLITUDE -= 0.1;
  sprintf(message, "amplitude = %f", BALANCE_AMPLITUDE);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void trim_left()
{
  char message[80];
  BLA::Matrix<4> thrust_ratio = CURRENT_THRUST_RATIO;
  thrust_ratio(FRONTMB) += MAX_TRIM_RATIO;
  thrust_ratio(REARMB) += MAX_TRIM_RATIO;
  change_drone_thrust_ratio(thrust_ratio);
  
  sprintf(message, "thrust_ratio = [%f %f %f %f]", CURRENT_THRUST_RATIO(FRONTMA), CURRENT_THRUST_RATIO(FRONTMB), CURRENT_THRUST_RATIO(REARMA), CURRENT_THRUST_RATIO(REARMB));
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void trim_right()
{
  char message[80];
  BLA::Matrix<4> thrust_ratio = CURRENT_THRUST_RATIO;
  thrust_ratio(FRONTMA) += MAX_TRIM_RATIO;
  thrust_ratio(REARMA) += MAX_TRIM_RATIO;
  change_drone_thrust_ratio(thrust_ratio);
  
  sprintf(message, "thrust_ratio = [%f %f %f %f]", CURRENT_THRUST_RATIO(FRONTMA), CURRENT_THRUST_RATIO(FRONTMB), CURRENT_THRUST_RATIO(REARMA), CURRENT_THRUST_RATIO(REARMB));
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void trim_forward()
{
  char message[80];
  BLA::Matrix<4> thrust_ratio = CURRENT_THRUST_RATIO;
  thrust_ratio(REARMB) += MAX_TRIM_RATIO;
  thrust_ratio(REARMA) += MAX_TRIM_RATIO;
  change_drone_thrust_ratio(thrust_ratio);
  
  sprintf(message, "thrust_ratio = [%f %f %f %f]", CURRENT_THRUST_RATIO(FRONTMA), CURRENT_THRUST_RATIO(FRONTMB), CURRENT_THRUST_RATIO(REARMA), CURRENT_THRUST_RATIO(REARMB));
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void trim_backward()
{
  char message[80];
  BLA::Matrix<4> thrust_ratio = CURRENT_THRUST_RATIO;
  thrust_ratio(FRONTMA) += MAX_TRIM_RATIO;
  thrust_ratio(FRONTMB) += MAX_TRIM_RATIO;
  change_drone_thrust_ratio(thrust_ratio);
  
  sprintf(message, "thrust_ratio = [%f %f %f %f]", CURRENT_THRUST_RATIO(FRONTMA), CURRENT_THRUST_RATIO(FRONTMB), CURRENT_THRUST_RATIO(REARMA), CURRENT_THRUST_RATIO(REARMB));
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void trim_left_rotation()
{
  char message[80];
  BLA::Matrix<4> thrust_ratio = CURRENT_THRUST_RATIO;
  thrust_ratio(FRONTMB) += MAX_TRIM_RATIO;
  thrust_ratio(REARMA) += MAX_TRIM_RATIO;
  change_drone_thrust_ratio(thrust_ratio);
  
  sprintf(message, "thrust_ratio = [%f %f %f %f]", CURRENT_THRUST_RATIO(FRONTMA), CURRENT_THRUST_RATIO(FRONTMB), CURRENT_THRUST_RATIO(REARMA), CURRENT_THRUST_RATIO(REARMB));
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void trim_right_rotation()
{
  char message[80];
  BLA::Matrix<4> thrust_ratio = CURRENT_THRUST_RATIO;
  thrust_ratio(FRONTMA) += MAX_TRIM_RATIO;
  thrust_ratio(REARMB) += MAX_TRIM_RATIO;
  change_drone_thrust_ratio(thrust_ratio);
  
  sprintf(message, "thrust_ratio = [%f %f %f %f]", CURRENT_THRUST_RATIO(FRONTMA), CURRENT_THRUST_RATIO(FRONTMB), CURRENT_THRUST_RATIO(REARMA), CURRENT_THRUST_RATIO(REARMB));
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", message);
}

void api_not_found()
{
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(404, "text/plain", "Not found");
}
