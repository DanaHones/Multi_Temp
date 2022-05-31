#include <DS18B20.h>

static uint32_t msSample = 0;

const int MAXRETRY = 3;
const uint32_t msSampleTime = 10000;
const int16_t dsData = D4;
const int16_t jsonBufferSize = 256;
const int16_t dateStringSize = 15;
const int16_t timeStringSize = 15;
const int16_t sensorIdStringSize = 25;
const int BadValue = -100;

DS18B20 ds18b20(dsData);

const int nSENSORS = 2;
//retained uint8_t sensorAddresses[nSENSORS][8];
uint8_t sensorAddresses[nSENSORS][8];
String sensorHexString[nSENSORS];
char dateString[dateStringSize] = {0};
char timeString[timeStringSize] = {0};
char sensorIdString[sensorIdStringSize] = {0};
char json_buffer[jsonBufferSize];
String Running = "RUN";

void setup()
{
  Particle.variable("Running", Running);
	Particle.function("SetRunning", SetRunning);

  // Time zone for MST
  Time.zone(-6);

  if (2 == 1)
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
  }

  Serial.println("Let's get started!");
}

void loop()
{
  if (Running == "RUN")
  {
    if (millis() - msSample >= msSampleTime)
    {
      Set_DateTime();
      msSample = millis();
      for (int i = 0; i < nSENSORS; i++)
      {
        float temp = getTemp(sensorAddresses[i]);
        if (isnan(temp))
        {
          temp = BadValue;
          Serial.println("Wooo");
        }
        Create_TempZone_Json(json_buffer, jsonBufferSize, dateString, timeString, sensorHexString[i], temp);
        Serial.println(json_buffer);
      }
    }
  }
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
    //showAddress(addr, false);
    //Serial.print(" - ");
    //Serial.println(_temp);
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

void Set_DateTime() 
{
    dateString[dateStringSize] = {0};
    timeString[timeStringSize] = {0};
    
    sprintf(dateString, "%02u/%02u/%04u",
      Time.month(),
      Time.day(),
      Time.year()
      );

    sprintf(timeString, "%02u:%02u:%02u",
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

void Create_TempZone_Json(char *ptr, int size, String date, String time, String sensorId, int zoneTemp)
{
    //memset(ptr, 0, sizeof(ptr));
    
    //Serial.println(sizeof(json_buffer));
    //Serial.println(sizeof(*ptr));
    
    JSONBufferWriter writer(ptr, size);
    //JSONBufferWriter writer(json_buffer, sizeof(json_buffer));
    
    writer.beginObject();
        writer.name("date").value(date);
        writer.name("time").value(time);
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