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

char json_buffer[256];
JSONBufferWriter writer(json_buffer, sizeof(json_buffer));

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

writer.beginObject();
    writer.name("zoneid").value(1);
    writer.name("zonetemp").value(178);
writer.endObject();
writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
Serial.println("First Json");
Serial.println(writer.buffer());

writer.beginObject();
    writer.name("zoneid").value(2);
    writer.name("zonetemp").value(68);
writer.endObject();
writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
Serial.println("Second Json");
Serial.println(writer.buffer());


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