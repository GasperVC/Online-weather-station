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
float temperaturaBait;
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
  temperaturaBait = dhtsensor.readTemperature();

  // Verificar si la lectura del sensor DHT es exitosa
  if (isnan(humedad) || isnan(temperaturaBait)) {
    Serial.println(F("Error al leer del sensor DHT"));
    return;
  }

  // Leer la temperatura y la presión del sensor BMP
  temp = bmp.getTemperature();
  presion = bmp.getPressure();
  alti = bmp.calAltitude(SEA_LEVEL_PRESSURE, presion);  // Nivel de presión al nivel del mar (en hPa)
  presion /= 100.0;
}

// Función para categorizar la humedad del suelo
String categorizarHumedad(int valor) {
  if (valor < 200) {
    return "Muy Húmedo";
  } else if (valor >= 200 && valor < 400) {
    return "Húmedo";
  } else if (valor >= 400 && valor < 600) {
    return "Moderado";
  } else if (valor >= 600 && valor < 800) {
    return "Seco";
  } else {
    return "Muy Seco";
  }
}

// Función para categorizar el sensor de lluvia
String categorizarLluvia(int valor) {
  if (valor < 200) {
    return "Sin lluvia";
  } else if (valor >= 200 && valor < 400) {
    return "Lluvia ligera";
  } else if (valor >= 400 && valor < 600) {
    return "Lluvia moderada";
  } else {
    return "Lluvia intensa";
  }
}

// Función para categorizar la humedad ambiente
String categorizarHumedadAmbiente(float valor) {
  if (valor < 30) {
    return "Baja";
  } else if (valor >= 30 && valor < 60) {
    return "Normal";
  } else {
    return "Alta";
  }
}

// Función para categorizar la temperatura
String categorizarTemperatura(float valor) {
  if (valor < 10) {
    return "Muy Frío";
  } else if (valor >= 10 && valor < 20) {
    return "Frío";
  } else if (valor >= 20 && valor < 30) {
    return "Temperatura Ideal";
  } else if (valor >= 30 && valor < 40) {
    return "Calor";
  } else {
    return "Muy Caliente";
  }
}

// Función para imprimir los valores leídos
void imprimirDatos() {
  // Imprimir los valores de los sensores
  Serial.println();
  Serial.println("======== INICIO DE IMPRESIÓN ========");

  // Categorizar y mostrar el valor de la humedad del suelo
  String humedadCategoria = categorizarHumedad(humedadAnalogica);
  Serial.print("Humedad del suelo (A0): "); 
  Serial.print(humedadAnalogica);
  Serial.print(" - Categoria: "); Serial.println(humedadCategoria);

  // Categorizar y mostrar el valor de lluvia
  String lluviaCategoria = categorizarLluvia(lluviaAnalogica);
  Serial.print(F("Sensor de lluvia (A1): ")); 
  Serial.print(lluviaAnalogica);
  Serial.print(" - Categoria: "); Serial.println(lluviaCategoria);

  // Categorizar y mostrar la humedad ambiente
  String humedadAmbienteCategoria = categorizarHumedadAmbiente(humedad);
  Serial.print("Humedad ambiente: "); 
  Serial.print(humedad);
  Serial.print(" - Categoria: "); Serial.println(humedadAmbienteCategoria);

  // Categorizar y mostrar la temperatura ambiente
  String temperaturaCategoria = categorizarTemperatura(temp);
  Serial.print("Temperatura (°C): "); 
  Serial.print(temp);
  Serial.print(" - Categoria: "); Serial.println(temperaturaCategoria);

  // Mostrar la temperatura y la presión del sensor BMP
  Serial.print("Temperatura (°C): "); Serial.println(temp);
  Serial.print("Presión (hPa): "); Serial.println(presion);  // Mostrar la presión en hPa (hPa = Pa / 100)
  Serial.print("Altitud (m): "); Serial.println(alti);  // Calcular y mostrar la altitud en metros

  Serial.println("======== FIN DE IMPRESIÓN ========");
}
