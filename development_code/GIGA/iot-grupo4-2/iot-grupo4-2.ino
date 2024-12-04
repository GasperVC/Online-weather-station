#include "DFRobot_BMP280.h"
#include "Wire.h"

typedef DFRobot_BMP280_IIC    BMP;    // ******** use abbreviations instead of full names ********

BMP bmp(&Wire, BMP::eSdoLow);

#define SEA_LEVEL_PRESSURE    1015.0f   // sea level pressure

int sensorFC28 = A0;
int sensorFC37 = A1;

// show last sensor operate status
void printLastOperateStatus(BMP::eStatus_t eStatus)
{
  switch(eStatus) {
  case BMP::eStatusOK:    Serial.println("everything ok"); break;
  case BMP::eStatusErr:   Serial.println("unknow error"); break;
  case BMP::eStatusErrDeviceNotDetected:    Serial.println("device not detected"); break;
  case BMP::eStatusErrParameter:    Serial.println("parameter error"); break;
  default: Serial.println("unknow status"); break;
  }
}

void setup()
{
  Serial.begin(115200);
  bmp.reset();
  Serial.println("bmp read data test");
  while(bmp.begin() != BMP::eStatusOK) {
    Serial.println("bmp begin faild");
    printLastOperateStatus(bmp.lastOperateStatus);
    delay(2000);
  }
  Serial.println("bmp begin success");

  pinMode(sensorFC28, INPUT);
  pinMode(sensorFC37, INPUT);
  
  delay(100);
}

void loop()
{
  // Los valores obtenidos van desde 0 sumergido en agua, a 1023 en el aire (o en un suelo muy seco)
  int humedadAnalogica = analogRead(sensorFC28);

  // Leer el valor del sensor de lluvia conectado al pin A1
  int lluviaAnalogica = analogRead(sensorFC37);
  
  //float   temp = bmp.getTemperature();
  uint32_t    press = bmp.getPressure();
  float   alti = bmp.calAltitude(SEA_LEVEL_PRESSURE, press);
  float   presion = press/100;

  Serial.println();
  Serial.println("======== start print ========");
  Serial.print("humedad suelo (A0): "); Serial.println(humedadAnalogica);
  Serial.print(F("lluvia (A1): ")); Serial.println(lluviaAnalogica);
  //Serial.print("temperature (unit Celsius): "); Serial.println(temp);
  Serial.print("pressure (unit pa):         "); Serial.println(presion);
  Serial.print("altitude (unit meter):      "); Serial.println(alti);
  Serial.println("========  end print  ========");

  delay(2000);
}
