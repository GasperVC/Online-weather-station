#include <WiFi.h>
#include <HTTPClient.h>

// Configuración de WiFi
const char* ssid = "clifford";            // Tu SSID de WiFi
const char* password = "cliff4yyyy";    // Tu contraseña de WiFi

// Definición de los parámetros a enviar
int soilMoisture = 500;
int rainSensor = 300;
int humidity = 45;
int temperature = 25;
int pressure = 1013;
int altitude = 100;

void setup() {
  // Iniciar la comunicación serial
  Serial.begin(115200);

  // Conectar al WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado al WiFi!");

  // Realizar el POST
  sendPostRequest();
}

void loop() {
  // Realizar el POST cada 10 segundos
  delay(10000);
  sendPostRequest();
}

void sendPostRequest() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Definir la URL del servidor
    http.begin("http://192.168.16.24/post");

    // Establecer los encabezados del POST (si es necesario)
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Crear el cuerpo del POST con los parámetros
    String postData = "soilMoisture=" + String(soilMoisture) +
                      "&rainSensor=" + String(rainSensor) +
                      "&humidity=" + String(humidity) +
                      "&temperature=" + String(temperature) +
                      "&pressure=" + String(pressure) +
                      "&altitude=" + String(altitude);

    // Enviar la solicitud POST
    int httpResponseCode = http.POST(postData);

    // Verificar la respuesta del servidor
    if (httpResponseCode > 0) {
      Serial.print("Respuesta del servidor: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error en la solicitud POST: ");
      Serial.println(httpResponseCode);
    }

    // Cerrar la conexión HTTP
    http.end();
  } else {
    Serial.println("Error en la conexión WiFi");
  }
}
