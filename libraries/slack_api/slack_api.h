#include <https_request.h>

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
