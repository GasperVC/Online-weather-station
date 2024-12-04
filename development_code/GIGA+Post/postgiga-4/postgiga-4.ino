#include <ArduinoHttpClient.h>
#include <WiFi.h>
#include "DFRobot_BMP280.h"
#include "Wire.h"
#include <DHT.h>

// Configuración de la red WiFi
const char* ssid = "clifford";   // Tu SSID de WiFi
const char* password = "cliff4yyyy";  // Tu contraseña de WiFi

// Configuración del servidor
const char* serverAddress = "192.168.16.24";  // Dirección IP del servidor
const int serverPort = 80;  // Puerto HTTP (por defecto 80)

// Instancia de los sensores
typedef DFRobot_BMP280_IIC    BMP;  // Definir el tipo de objeto BMP
BMP bmp(&Wire, BMP::eSdoLow);
DHT dhtsensor(2, DHT11);  // Sensor DHT11 conectado al pin 2

#define SEA_LEVEL_PRESSURE    1015.0f   // presión al nivel del mar

// Sensores FC28 y FC37
int sensorFC28 = A0;  // Sensor de humedad de suelo
int sensorFC37 = A1;  // Sensor de lluvia

// Declaración de las variables para almacenar los valores de los sensores
int humedadAnalogica;
int lluviaAnalogica;
float humedad;
float temperaturaBait;
float temp;
uint32_t presion;
float alti;

// Instancia de cliente HTTP
WiFiClient wifi;
HttpClient http = HttpClient(wifi, serverAddress, serverPort);

// Función para conectar a la red WiFi
void connectToWiFi() {
  Serial.print("Conectando a la red WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado a la red WiFi");
}

// Función para enviar los datos al servidor
void sendDataToServer() {
  // Crear el cuerpo del POST con los parámetros leídos
  String postData = "soilMoisture=" + String(humedadAnalogica) +
                    "&rainSensor=" + String(lluviaAnalogica) +
                    "&humidity=" + String(humedad) +
                    "&temperature=" + String(temp) +
                    "&pressure=" + String(presion) +
                    "&altitude=" + String(alti);

  // Realiza la solicitud POST
  String url = "/post";  // Solo la parte del camino después de la dirección IP

  http.post(url, "application/x-www-form-urlencoded", postData);

  // Obtener y mostrar la respuesta del servidor
  String response = http.responseBody();
  Serial.print("Respuesta del servidor: ");
  Serial.println(response);

  // Cerrar la conexión
  http.stop();
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

// Función para imprimir los valores leídos
void imprimirDatos() {
  // Imprimir los valores de los sensores
  Serial.println();
  Serial.println("======== INICIO DE IMPRESIÓN ========");
  Serial.print("Humedad del suelo (A0): ");  Serial.println(humedadAnalogica);
  Serial.print(F("Sensor de lluvia (A1): "));  Serial.println(lluviaAnalogica);
  Serial.print("Humedad ambiente: ");  Serial.println(humedad);
  Serial.print("Temperatura (°C): ");  Serial.println(temp);

  // Mostrar la temperatura y la presión del sensor BMP
  Serial.print("Temperatura (°C): "); Serial.println(temp);
  Serial.print("Presión (hPa): "); Serial.println(presion);  // Mostrar la presión en hPa (hPa = Pa / 100)
  Serial.print("Altitud (m): "); Serial.println(alti);  // Calcular y mostrar la altitud en metros

  Serial.println("======== FIN DE IMPRESIÓN ========");
}

void setup() {
  Serial.begin(115200);  // Inicializar comunicación serial

  // Función para inicar los sensores
  bmp.reset();
  Serial.println("bmp read data test");
  bmp.begin();
  Serial.println("bmp begin success");

  pinMode(sensorFC28, INPUT);
  pinMode(sensorFC37, INPUT);

  // Iniciar el sensor DHT11
  dhtsensor.begin();

  // Función para conectar a la red WiFi
  Serial.print("Conectando a la red WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado a la red WiFi");

  delay(100);
}

void loop() {
  // Llamar a la función para leer los sensores
  leerSensores();

  // Llamar a la función para imprimir los datos leídos
  imprimirDatos();

  // Llamar a la función para enviar los datos al servidor
  sendDataToServer();

  // Esperar 4 segundos antes de la siguiente lectura
  delay(4000);
}
