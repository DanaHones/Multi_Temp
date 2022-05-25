// This #include statement was automatically added by the Particle IDE.
#include <HttpClient.h>

//#include "application.h"

/**
* Declaring the variables.
*/
unsigned int nextTime = 0;    // Next time to contact the server
HttpClient http;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
//  { "Content-Type", "application/json" },
//  { "Accept" , "application/json" },
{ "Accept" , "*/*"},
{ NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

void setup() {
Serial.begin(9600);
}

void loop() {
if (nextTime > millis()) {
return;
}

Serial.println();
Serial.println("Application>\tStart of Loop.");
// Request path and body can be set at runtime or at setup.
request.hostname = "192.168.1.82";
request.port = 8888;
request.path = "/api/readings/1";

// The library also supports sending a body with your request:
//request.body = "{\"key\":\"value\"}";

// Get request
http.get(request, response, headers);
Serial.print("Application>\tResponse status: ");
Serial.println(response.status);

Serial.print("Application>\tHTTP Response Body: ");
Serial.println(response.body);

nextTime = millis() + 10000;
}