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
const int nSENSORS = 16;
//retained uint8_t sensorAddresses[nSENSORS][8];
uint8_t sensorAddresses[nSENSORS][8];
String sensorHexString[nSENSORS];
byte sensorFlag[nSENSORS];

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
String API_Call = "FALSE";
String HoneyWellRead = "FALSE";

void setup()
{
  Particle.variable("Running", Running);
  Particle.variable("API_Call", API_Call);
  Particle.variable("HoneyWellRead", HoneyWellRead);
	Particle.function("SetRunning", SetRunning);
  Particle.function("SetAPI_Call", SetAPI_Call);
  Particle.function("SetHoneyWell", SetHoneyWell);

  // Time zone for MST
  Time.zone(-6);

  // REST call
  request.hostname = "10.0.0.50";
  request.port = 8888;

  Running = "RUN";
  API_Call = "FALSE";
  HoneyWellRead = "FALSE";

  if (Running == "SCAN")
  {
    // Look for new sensors
    ds18b20.resetsearch();                 // initialize for sensor search
    for (int i = 0; i < nSENSORS; i++)
    {  // try to read the sensor addresses
      ds18b20.search(sensorAddresses[i]);  // and if available store
    }
    API_Call = "FALSE";
  }
  else
  {
    Load_Sensor_Arrays();
  }

  Serial.println("Let's get started!");
  msSample = msSampleTime;
}

void loop()
{
  if (Running != "STOP")
  {
    if (millis() - msSample >= msSampleTime)
    {
      msSample = millis();
      if (Running == "RUN")
      {
        Scan_Sensors();
      }
      else
      {
        Show_Found_Sensors();
      }
    }
  }
}

void Show_Found_Sensors()
{
  for (int i = 0; i < nSENSORS; i++)
  {
    float temp = getTemp(sensorAddresses[i]);
    if (!isnan(temp))
    {
      Serial.print(i + 1);
      Serial.print(" of ");
      Serial.print(nSENSORS);
      Serial.print(": ");
      showAddress(sensorAddresses[i], false);
      Serial.print(" - ");
      Serial.println(temp);
    }
    else
    {
      temp = BadValue;
      Serial.println("Wooo");
    }
  }
}
  
void Scan_Sensors()
{
  Set_ISODateTime();
  //Serial.println("In Scan Sensors");
  for (int i = 0; i < nSENSORS; i++)
  {
    if (sensorFlag[i] == 1)
    {
      float temp = getTemp(sensorAddresses[i]);
      if (isnan(temp))
      {
        temp = BadValue;
        Serial.println("Wooo");
      }
      
      Create_TempZone_Json(json_buffer, jsonBufferSize, isoDateTimeString, sensorHexString[i], temp);
      Serial.println(json_buffer);

      if (API_Call == "TRUE")
      {
        MakeTheHttpCall();
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

int SetAPI_Call(String value)
{
  API_Call = value;
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

void Load_Sensor_Arrays()
{
   // Sensor 1
    sensorFlag[0] = 0;
    sensorHexString[0] = "28:DE:29:95:F0:01:3C:F6";
    sensorAddresses[0][0] = 0x28; sensorAddresses[0][1] = 0xDE; sensorAddresses[0][2] = 0x29; sensorAddresses[0][3] = 0x95; 
    sensorAddresses[0][4] = 0xF0; sensorAddresses[0][5] = 0x01; sensorAddresses[0][6] = 0x3C; sensorAddresses[0][7] = 0xF6; 

    // Sensor 2
    sensorFlag[1] = 0;
    sensorHexString[1] = "28:5F:9F:95:F0:01:3C:F8";
    sensorAddresses[1][0] = 0x28; sensorAddresses[1][1] = 0x5F; sensorAddresses[1][2] = 0x9F; sensorAddresses[1][3] = 0x95; 
    sensorAddresses[1][4] = 0xF0; sensorAddresses[1][5] = 0x01; sensorAddresses[1][6] = 0x3C; sensorAddresses[1][7] = 0xF8; 

    // Sensor 3
    sensorFlag[2] = 0;
    sensorHexString[2] = "28:79:6A:96:F0:01:3C:00";
    sensorAddresses[2][0] = 0x28; sensorAddresses[2][1] = 0x79; sensorAddresses[2][2] = 0x6A; sensorAddresses[2][3] = 0x96; 
    sensorAddresses[2][4] = 0xF0; sensorAddresses[2][5] = 0x01; sensorAddresses[2][6] = 0x3C; sensorAddresses[2][7] = 0x00; 

    // Sensor 4
    sensorFlag[3] = 0;
    sensorHexString[3] = "28:6C:E1:95:F0:01:3C:83";
    sensorAddresses[3][0] = 0x28; sensorAddresses[3][1] = 0x6C; sensorAddresses[3][2] = 0xE1; sensorAddresses[3][3] = 0x95; 
    sensorAddresses[3][4] = 0xF0; sensorAddresses[3][5] = 0x01; sensorAddresses[3][6] = 0x3C; sensorAddresses[3][7] = 0x83; 

    // Sensor 5
    sensorFlag[4] = 0;
    sensorHexString[4] = "28:04:ED:49:F6:17:3C:44";
    sensorAddresses[4][0] = 0x28; sensorAddresses[4][1] = 0x04; sensorAddresses[4][2] = 0xED; sensorAddresses[4][3] = 0x49; 
    sensorAddresses[4][4] = 0xF6; sensorAddresses[4][5] = 0x17; sensorAddresses[4][6] = 0x3C; sensorAddresses[4][7] = 0x44; 

    // Sensor 6
    sensorFlag[5] = 1;
    sensorHexString[5] = "28:96:A6:48:F6:F8:3C:E0";
    sensorAddresses[5][0] = 0x28; sensorAddresses[5][1] = 0x96; sensorAddresses[5][2] = 0xA6; sensorAddresses[5][3] = 0x48; 
    sensorAddresses[5][4] = 0xF6; sensorAddresses[5][5] = 0xF8; sensorAddresses[5][6] = 0x3C; sensorAddresses[5][7] = 0xE0; 

    // Sensor 7
    sensorFlag[6] = 0;
    sensorHexString[6] = "28:C3:B8:48:F6:B4:3C:2D";
    sensorAddresses[6][0] = 0x28; sensorAddresses[6][1] = 0xC3; sensorAddresses[6][2] = 0xB8; sensorAddresses[6][3] = 0x48; 
    sensorAddresses[6][4] = 0xF6; sensorAddresses[6][5] = 0xB4; sensorAddresses[6][6] = 0x3C; sensorAddresses[6][7] = 0x2D; 

    // Sensor 8
    sensorFlag[7] = 0;
    sensorHexString[7] = "28:4A:41:48:F6:7F:3C:93";
    sensorAddresses[7][0] = 0x28; sensorAddresses[7][1] = 0x4A; sensorAddresses[7][2] = 0x41; sensorAddresses[7][3] = 0x48; 
    sensorAddresses[7][4] = 0xF6; sensorAddresses[7][5] = 0x7F; sensorAddresses[7][6] = 0x3C; sensorAddresses[7][7] = 0x93; 

    // Sensor 9
    sensorFlag[8] = 0;
    sensorHexString[8] = "28:66:63:48:F6:27:3C:22";
    sensorAddresses[8][0] = 0x28; sensorAddresses[8][1] = 0x66; sensorAddresses[8][2] = 0x63; sensorAddresses[8][3] = 0x48; 
    sensorAddresses[8][4] = 0xF6; sensorAddresses[8][5] = 0x27; sensorAddresses[8][6] = 0x3C; sensorAddresses[8][7] = 0x22; 

    // Sensor 10
    sensorFlag[9] = 0;
    sensorHexString[9] = "28:B1:FE:48:F6:71:3C:E5";
    sensorAddresses[9][0] = 0x28; sensorAddresses[9][1] = 0xB1; sensorAddresses[9][2] = 0xFE; sensorAddresses[9][3] = 0x48; 
    sensorAddresses[9][4] = 0xF6; sensorAddresses[9][5] = 0x71; sensorAddresses[9][6] = 0x3C; sensorAddresses[9][7] = 0xE5; 

    // Sensor 11
    sensorFlag[10] = 0;
    sensorHexString[10] = "28:70:92:48:F6:1E:3C:3F";
    sensorAddresses[10][0] = 0x28; sensorAddresses[10][1] = 0x70; sensorAddresses[10][2] = 0x92; sensorAddresses[10][3] = 0x48; 
    sensorAddresses[10][4] = 0xF6; sensorAddresses[10][5] = 0x1E; sensorAddresses[10][6] = 0x3C; sensorAddresses[10][7] = 0x3F; 

    // Sensor 12
    sensorFlag[11] = 1;
    sensorHexString[11] = "28:D9:6A:48:F6:D9:3C:B7";
    sensorAddresses[11][0] = 0x28; sensorAddresses[11][1] = 0xD9; sensorAddresses[11][2] = 0x6A; sensorAddresses[11][3] = 0x48; 
    sensorAddresses[11][4] = 0xF6; sensorAddresses[11][5] = 0xD9; sensorAddresses[11][6] = 0x3C; sensorAddresses[11][7] = 0xB7;     

    // Sensor 13
    sensorFlag[12] = 0;
    sensorHexString[12] = "28:7D:6B:96:F0:01:3C:11";
    sensorAddresses[12][0] = 0x28; sensorAddresses[12][1] = 0x7D; sensorAddresses[12][2] = 0x6B; sensorAddresses[12][3] = 0x96; 
    sensorAddresses[12][4] = 0xF0; sensorAddresses[12][5] = 0x01; sensorAddresses[12][6] = 0x3C; sensorAddresses[12][7] = 0x11;     

    // Sensor 14
    sensorFlag[13] = 0;
    sensorHexString[13] = "28:11:91:48:F6:B9:3C:05";
    sensorAddresses[13][0] = 0x28; sensorAddresses[13][1] = 0x11; sensorAddresses[13][2] = 0x91; sensorAddresses[13][3] = 0x48; 
    sensorAddresses[13][4] = 0xF6; sensorAddresses[13][5] = 0xB9; sensorAddresses[13][6] = 0x3C; sensorAddresses[13][7] = 0x05;     

    // Sensor 15
    sensorFlag[14] = 0;
    sensorHexString[14] = "28:B8:E6:48:F6:C4:3C:CC";
    sensorAddresses[14][0] = 0x28; sensorAddresses[14][1] = 0xB8; sensorAddresses[14][2] = 0xE6; sensorAddresses[14][3] = 0x48; 
    sensorAddresses[14][4] = 0xF6; sensorAddresses[14][5] = 0xC4; sensorAddresses[14][6] = 0x3C; sensorAddresses[14][7] = 0xCC;     

    // Sensor 16
    sensorFlag[15] = 0;
    sensorHexString[15] = "28:70:C7:48:F6:37:3C:0B";
    sensorAddresses[15][0] = 0x28; sensorAddresses[15][1] = 0x70; sensorAddresses[15][2] = 0xC7; sensorAddresses[15][3] = 0x48; 
    sensorAddresses[15][4] = 0xF6; sensorAddresses[15][5] = 0x37; sensorAddresses[15][6] = 0x3C; sensorAddresses[15][7] = 0x0B;     
}

/*
void publishData() {
  char szInfo[64];
  snprintf(szInfo, sizeof(szInfo), "%.1f �C, %.1f �C", celsius[0], celsius[1]);
  Particle.publish("dsTmp", szInfo, PRIVATE);
}
*/