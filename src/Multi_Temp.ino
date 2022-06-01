#include <HttpClient.h>
#include <DS18B20.h>

static uint32_t msSample = 0;

const int MAXRETRY = 3;
const uint32_t msSampleTime = 10000;
const int16_t dsData = D4;
const int16_t jsonBufferSize = 256;
const int16_t isoDateTimeStringSize = 25;
const int16_t sensorIdStringSize = 25;
const int BadValue = -100;

DS18B20 ds18b20(dsData);
const int nSENSORS = 1;
//retained uint8_t sensorAddresses[nSENSORS][8];
uint8_t sensorAddresses[nSENSORS][8];
String sensorHexString[nSENSORS];

HttpClient http;
// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] =
{
  { "Content-Type", "application/json" },
  { "Accept" , "application/json" },
{ "Accept" , "*/*"},
{ NULL, NULL } // NOTE: Always terminate headers will NULL
};
http_request_t request;
http_response_t response;

char isoDateTimeString[isoDateTimeStringSize] = {0};
char sensorIdString[sensorIdStringSize] = {0};
char json_buffer[jsonBufferSize];
String Running = "RUN";
String HoneyWellRead = "FALSE";

void setup()
{
  Particle.variable("Running", Running);
  Particle.variable("HoneyWellRead", HoneyWellRead);
	Particle.function("SetRunning", SetRunning);
  Particle.function("SetHoneyWell", SetHoneyWell);

  // Time zone for MST
  Time.zone(-6);

  // REST call
  request.hostname = "10.0.0.50";
  request.port = 8888;

  if (1 == 1)
  {
    // Look for new sensors
    ds18b20.resetsearch();                 // initialize for sensor search
    for (int i = 0; i < nSENSORS; i++)
    {  // try to read the sensor addresses
      ds18b20.search(sensorAddresses[i]);  // and if available store
    }
  }
  else
  {
    // Sensor 1
    sensorHexString[0] = "28:DE:29:95:F0:01:3C:F6";
    sensorAddresses[0][0] = 0x28; sensorAddresses[0][1] = 0xDE; sensorAddresses[0][2] = 0x29; sensorAddresses[0][3] = 0x95; 
    sensorAddresses[0][4] = 0xF0; sensorAddresses[0][5] = 0x01; sensorAddresses[0][6] = 0x3C; sensorAddresses[0][7] = 0xF6; 

    // Sensor 2
    sensorHexString[1] = "28:5F:9F:95:F0:01:3C:F8";
    sensorAddresses[1][0] = 0x28; sensorAddresses[1][1] = 0x5F; sensorAddresses[1][2] = 0x9F; sensorAddresses[1][3] = 0x95; 
    sensorAddresses[1][4] = 0xF0; sensorAddresses[1][5] = 0x01; sensorAddresses[1][6] = 0x3C; sensorAddresses[1][7] = 0xF8; 

    // Sensor 3
    sensorHexString[2] = "28:79:6A:96:F0:01:3C:00";
    sensorAddresses[2][0] = 0x28; sensorAddresses[2][1] = 0x79; sensorAddresses[2][2] = 0x6A; sensorAddresses[2][3] = 0x96; 
    sensorAddresses[2][4] = 0xF0; sensorAddresses[2][5] = 0x01; sensorAddresses[2][6] = 0x3C; sensorAddresses[2][7] = 0x00; 

    // Sensor 4
    sensorHexString[3] = "28:6C:E1:95:F0:01:3C:83";
    sensorAddresses[3][0] = 0x28; sensorAddresses[3][1] = 0x6C; sensorAddresses[3][2] = 0xE1; sensorAddresses[3][3] = 0x95; 
    sensorAddresses[3][4] = 0xF0; sensorAddresses[3][5] = 0x01; sensorAddresses[3][6] = 0x3C; sensorAddresses[3][7] = 0x83; 

    // Sensor 5
    sensorHexString[4] = "28:04:ED:49:F6:17:3C:44";
    sensorAddresses[4][0] = 0x28; sensorAddresses[4][1] = 0x04; sensorAddresses[4][2] = 0xED; sensorAddresses[4][3] = 0x49; 
    sensorAddresses[4][4] = 0xF6; sensorAddresses[4][5] = 0x17; sensorAddresses[4][6] = 0x3C; sensorAddresses[4][7] = 0x44; 

    // Sensor 6
    sensorHexString[5] = "28:96:A6:48:F6:F8:3C:E0";
    sensorAddresses[5][0] = 0x28; sensorAddresses[5][1] = 0x96; sensorAddresses[5][2] = 0xA6; sensorAddresses[5][3] = 0x48; 
    sensorAddresses[5][4] = 0xF6; sensorAddresses[5][5] = 0xF8; sensorAddresses[5][6] = 0x3C; sensorAddresses[5][7] = 0xE0; 

    // Sensor 7
    sensorHexString[6] = "28:C3:B8:48:F6:B4:3C:2D";
    sensorAddresses[6][0] = 0x28; sensorAddresses[6][1] = 0xC3; sensorAddresses[6][2] = 0xB8; sensorAddresses[6][3] = 0x48; 
    sensorAddresses[6][4] = 0xF6; sensorAddresses[6][5] = 0xB4; sensorAddresses[6][6] = 0x3C; sensorAddresses[6][7] = 0x2D; 

    // Sensor 8
    sensorHexString[7] = "28:4A:41:48:F6:7F:3C:93";
    sensorAddresses[7][0] = 0x28; sensorAddresses[7][1] = 0x4A; sensorAddresses[7][2] = 0x41; sensorAddresses[7][3] = 0x48; 
    sensorAddresses[7][4] = 0xF6; sensorAddresses[7][5] = 0x7F; sensorAddresses[7][6] = 0x3C; sensorAddresses[7][7] = 0x93; 

    // Sensor 9
    sensorHexString[8] = "28:66:63:48:F6:27:3C:22";
    sensorAddresses[8][0] = 0x28; sensorAddresses[8][1] = 0x66; sensorAddresses[8][2] = 0x63; sensorAddresses[8][3] = 0x48; 
    sensorAddresses[8][4] = 0xF6; sensorAddresses[8][5] = 0x27; sensorAddresses[8][6] = 0x3C; sensorAddresses[8][7] = 0x22; 

    // Sensor 10
    sensorHexString[9] = "28:B1:FE:48:F6:71:3C:E5";
    sensorAddresses[9][0] = 0x28; sensorAddresses[9][1] = 0xB1; sensorAddresses[9][2] = 0xFE; sensorAddresses[9][3] = 0x48; 
    sensorAddresses[9][4] = 0xF6; sensorAddresses[9][5] = 0x71; sensorAddresses[9][6] = 0x3C; sensorAddresses[9][7] = 0xE5; 

    // Sensor 11
    sensorHexString[10] = "28:70:92:48:F6:1E:3C:3F";
    sensorAddresses[10][0] = 0x28; sensorAddresses[10][1] = 0x70; sensorAddresses[10][2] = 0x92; sensorAddresses[10][3] = 0x48; 
    sensorAddresses[10][4] = 0xF6; sensorAddresses[10][5] = 0x1E; sensorAddresses[10][6] = 0x3C; sensorAddresses[10][7] = 0x3F; 

    // Sensor 12
    sensorHexString[11] = "28:D9:6A:48:F6:D9:3C:B7";
    sensorAddresses[11][0] = 0x28; sensorAddresses[11][1] = 0xD9; sensorAddresses[11][2] = 0x6A; sensorAddresses[11][3] = 0x48; 
    sensorAddresses[11][4] = 0xF6; sensorAddresses[11][5] = 0xD9; sensorAddresses[11][6] = 0x3C; sensorAddresses[11][7] = 0xB7;     
  }

  Serial.println("Let's get started!");
  msSample = msSampleTime;
}

void loop()
{
  if (Running == "RUN")
  {
    if (millis() - msSample >= msSampleTime)
    {
      Set_ISODateTime();
      msSample = millis();
      for (int i = 0; i < nSENSORS; i++)
      {
        float temp = getTemp(sensorAddresses[i]);
        if (isnan(temp))
        {
          temp = BadValue;
          Serial.println("Wooo");
        }
        //Create_TempZone_Json(json_buffer, jsonBufferSize, isoDateTimeString, sensorHexString[i], temp);
        //Serial.println(json_buffer);
        //MakeTheHttpCall();
      }
    }
  }
}

void MakeTheHttpCall()
{
  // The library also supports sending a body with your request:
  //request.body = "{\"key\":\"value\"}";
  // Get request
  ////http.get(request, response, headers);

  if (HoneyWellRead == "TRUE")
  {
    request.path = "/api/readings?honeywellRead=true";
  }
  else
  {
    request.path = "/api/readings?honeywellRead=false";
  }

  request.body = json_buffer;

  Serial.println("Starting call");
  //Serial.println(request.hostname);
  //Serial.println(request.port);
  //Serial.println(request.path);
  //Serial.println(request.body);

  http.put(request, response, headers);
  Serial.print("Application>\tResponse status: ");
  Serial.println(response.status);
        
  Serial.print("Application>\tHTTP Response Body: ");
  Serial.println(response.body);
}

double getTemp(uint8_t addr[8]) {
  double _temp;
  int   i = 0;

  do
  {
    _temp = ds18b20.getTemperature(addr);
  } while (!ds18b20.crcCheck() && MAXRETRY > i++);

  if (i < MAXRETRY)
  {
    _temp = ds18b20.convertToFahrenheit(_temp);
    //Serial.print(addr);
    showAddress(addr, false);
    Serial.print(" - ");
    Serial.println(_temp);
  }
  else
  {
    _temp = BadValue;
    Serial.println("Invalid reading");
  }

  return _temp;
}

void showAddress(uint8_t addr[8], bool lf)
{
  sensorIdString[sensorIdStringSize] = {'0'};
  sprintf(sensorIdString, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",addr[0],addr[1],addr[2],addr[3],addr[4],addr[5], addr[6], addr[7]);

  if (lf)
  {
    Serial.println(sensorIdString);
  }
  else
  {
    Serial.print(sensorIdString);
  }
}

void Set_ISODateTime()
{
  // 2019-01-06T17:16:40.000
  isoDateTimeString[isoDateTimeStringSize] = {0};
    
  sprintf(isoDateTimeString, "%04u-%02u-%02uT%02u:%02u:%02u",
    Time.year(),
    Time.month(),
    Time.day(),
    Time.hour(),
    Time.minute(),
    Time.second()
    );
}

int SetRunning (String value)
{    
    Running = value;
    return 1;
}

int SetHoneyWell (String value)
{    
    HoneyWellRead = value;
    return 1;
}

void Create_TempZone_Json(char *ptr, int size, String isoDateTime, String sensorId, int zoneTemp)
{
    //memset(ptr, 0, sizeof(ptr));
    
    //Serial.println(sizeof(json_buffer));
    //Serial.println(sizeof(*ptr));
    
    JSONBufferWriter writer(ptr, size);
    //JSONBufferWriter writer(json_buffer, sizeof(json_buffer));
    
    writer.beginObject();
        writer.name("time").value(isoDateTime);
        writer.name("sensorid").value(sensorId);
        writer.name("zonetemp").value(zoneTemp);
    writer.endObject();
    //writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    //Serial.println(json_buffer);
}

/*
void publishData() {
  char szInfo[64];
  snprintf(szInfo, sizeof(szInfo), "%.1f �C, %.1f �C", celsius[0], celsius[1]);
  Particle.publish("dsTmp", szInfo, PRIVATE);
}
*/