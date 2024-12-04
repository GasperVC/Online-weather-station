#include "DFRobot_BMP280.h"
#include "Wire.h"
#include <DHT.h>

typedef DFRobot_BMP280_IIC    BMP;    // ******** use abbreviations instead of full names ********

BMP bmp(&Wire, BMP::eSdoLow);
DHT dhtsensor(2, DHT11);

#define SEA_LEVEL_PRESSURE    1015.0f   // sea level pressure

int sensorFC28 = A0;
int sensorFC37 = A1;

// Declaración de las variables para almacenar los valores de los sensores
int humedadAnalogica;
int lluviaAnalogica;
float humedad;
float temperatura;
float temp;
uint32_t presion;
float alti;

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

  // Iniciar el sensor DHT11
  dhtsensor.begin();
  
  delay(100);
}

void loop() {
  // Llamar a la función para leer los sensores
  leerSensores();
  
  // Llamar a la función para imprimir los datos leídos
  imprimirDatos();

  // Esperar 4 segundos antes de la siguiente lectura
  delay(4000);
}

// Función para leer los valores de los sensores
void leerSensores() {
  // Leer el valor del sensor de humedad del suelo conectado al pin A0
  humedadAnalogica = analogRead(sensorFC28);

  // Leer el valor del sensor de lluvia conectado al pin A1
  lluviaAnalogica = analogRead(sensorFC37);

  // Leer la humedad y temperatura ambiente del sensor DHT
  humedad = dhtsensor.readHumidity();
  temperatura = dhtsensor.readTemperature();

  // Verificar si la lectura del sensor DHT es exitosa
  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println(F("Error al leer del sensor DHT"));
    return;
  }

  // Leer la temperatura y la presión del sensor BMP
  temp = bmp.getTemperature();
  presion = bmp.getPressure();
  alti = bmp.calAltitude(1013.25, presion);  // Nivel de presión al nivel del mar (en hPa)
}

// Función para imprimir los valores leídos
void imprimirDatos() {
  Serial.println();
  Serial.println("======== INICIO DE IMPRESIÓN ========");
  
  // Imprimir los valores de los sensores
  Serial.print("Humedad del suelo (A0): "); Serial.println(humedadAnalogica);
  Serial.print(F("Sensor de lluvia (A1): ")); Serial.println(lluviaAnalogica);
  Serial.print("Humedad ambiente: "); Serial.println(humedad);
  // Serial.print(F("Temperatura ambiente (%): ")); Serial.println(temperatura);
  Serial.print("Temperatura (°C): "); Serial.println(temp);
  Serial.print("Presión (hPa): "); Serial.println(presion / 100.0);  // Mostrar la presión en hPa (hPa = Pa / 100)
  Serial.print("Altitud (m): "); Serial.println(alti);  // Calcular y mostrar la altitud en metros
  Serial.println("======== FIN DE IMPRESIÓN ========");

}
