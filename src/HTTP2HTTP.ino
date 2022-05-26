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
//JSONBufferWriter writer(json_buffer, sizeof(json_buffer));

String Running = "STOP";

void setup() {
    Serial.begin(9600);
    
    Particle.variable("Running", Running);
	Particle.function("SetRunning", SetRunning);
    
    //memset(json_buffer, 0, sizeof(json_buffer));
    
    Running = "RUN";
}

void loop() {
    if (Running == "RUN")
    {
        if (nextTime > millis()) {
        return;
        }
    
        Serial.println();
        Serial.println("Application>\tStart of Loop.");
        // Request path and body can be set at runtime or at setup.
    
        //memset(json_buffer, 0, sizeof(json_buffer));
        //json_buffer[0] = 0;
        //writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
        Create_TempZone_Json(json_buffer, sizeof(json_buffer), 1, 78);
        
        Serial.println("First Json");
        Serial.println(json_buffer);
        
        Create_TempZone_Json(json_buffer, sizeof(json_buffer), 2, 68);
        Serial.println("Second Json");
        Serial.println(json_buffer);
    
        request.hostname = "192.168.1.82";
        request.port = 8888;
        request.path = "/api/readings/1";
        
        // The library also supports sending a body with your request:
        //request.body = "{\"key\":\"value\"}";
        
        // Get request
        ////http.get(request, response, headers);
        Serial.print("Application>\tResponse status: ");
        Serial.println(response.status);
        
        Serial.print("Application>\tHTTP Response Body: ");
        Serial.println(response.body);
        
        nextTime = millis() + 5000;
    }
}

//void toggleLed(const char *event, const char *data) {
int SetRunning (String value) {    
    //leds.setColorHSB(0, 0.0, 1.0, 0.5);
    
    Running = value;
    
    return 1;
}

void Create_TempZone_Json(char *ptr, int size, int zoneID, int zoneTemp)
{
    //memset(ptr, 0, sizeof(ptr));
    
    //Serial.println(sizeof(json_buffer));
    //Serial.println(sizeof(*ptr));
    
    JSONBufferWriter writer(ptr, size);
    //JSONBufferWriter writer(json_buffer, sizeof(json_buffer));
    
    writer.beginObject();
        writer.name("zoneid").value(zoneID);
        writer.name("zonetemp").value(zoneTemp);
    writer.endObject();
    //writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    //Serial.println(json_buffer);
}