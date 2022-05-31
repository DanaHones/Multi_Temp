#include <DS18B20.h>

static uint32_t msSample = 0;

const int MAXRETRY = 3;
const uint32_t msSampleTime = 10000;
const int16_t dsData = D4;
const int16_t dateStringSize = 22;
const int16_t sensorIdStringSize = 25;

DS18B20 ds18b20(dsData);

const int nSENSORS = 2;
float celsius[nSENSORS] = {NAN, NAN};
retained uint8_t sensorAddresses[nSENSORS][8];
char dateString[dateStringSize] = {0};
char sensorIdString[sensorIdStringSize] = {0};
String Running = "RUN";

void setup()
{
  Particle.variable("Running", Running);
	Particle.function("SetRunning", SetRunning);

  ds18b20.resetsearch();                 // initialise for sensor search

  for (int i = 0; i < nSENSORS; i++)
  {  // try to read the sensor addresses
    ds18b20.search(sensorAddresses[i]);  // and if available store
  }

  Serial.println("Let's get started");
}

void loop()
{
  if (Running == "RUN")
  {
    if (millis() - msSample >= msSampleTime)
    {
      msSample = millis();
      Show_DateTime(true);
      for (int i = 0; i < nSENSORS; i++)
      {
        float temp = getTemp(sensorAddresses[i]);
        if (!isnan(temp)) celsius[i] = temp;
      }
      Serial.println("");
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
    showAddress(addr, false);
    Serial.print(" - ");
    Serial.println(_temp);
  }
  else
  {
    _temp = NAN;
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

void Show_DateTime(bool lf) 
{
    dateString[dateStringSize] = {0};
    
    sprintf(dateString, "%02u/%02u/%04u - %02u:%02u:%02u:",
      Time.month(),
      Time.day(),
      Time.year(),
      Time.hour(),
      Time.minute(),
      Time.second()
      );
    
    if (lf)
    {
      Serial.println(dateString);
    }
    else
    {
      Serial.print(dateString);
    }
}

int SetRunning (String value)
{    
 
    Running = value;
    return 1;
}

/*
void publishData() {
  char szInfo[64];
  snprintf(szInfo, sizeof(szInfo), "%.1f �C, %.1f �C", celsius[0], celsius[1]);
  Particle.publish("dsTmp", szInfo, PRIVATE);
}
*/