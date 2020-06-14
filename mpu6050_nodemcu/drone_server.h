

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

void abort_flight()
{
  BLA::Matrix<4> thrust_vector;
  char *message = "Command received, aborting...\n";
  Serial << message;
  send_udp(message);
        
  FLIGHT_THRUST = MIN_THRUST;
  thrust_vector.Fill(FLIGHT_THRUST);
  update_esc_power(thrust_vector);
  INITIATE_FLIGHT = false;
  
  server.send(200, "text/plain", message);
}

void api_not_found()
{
  server.send(404, "text/plain", "Not found");
}
