#include <ArduinoHttpClient.h>
#include <WiFi.h>

// Configuración de la red WiFi
const char* ssid = "clifford";   // Tu SSID de WiFi
const char* password = "cliff4yyyy";  // Tu contraseña de WiFi

// Configuración del servidor
const char* serverAddress = "192.168.16.24";  // Dirección IP del servidor
const int serverPort = 80;  // Puerto HTTP (por defecto 80)

WiFiClient wifi;  // Instancia de WiFiClient
HttpClient http = HttpClient(wifi, serverAddress, serverPort);  // Instancia de HttpClient

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
  // Datos a enviar
  int soilMoisture = 500;
  int rainSensor = 300;
  int humidity = 45;
  int temperature = 25;
  int pressure = 1013;
  int altitude = 100;

  // Crear el cuerpo del POST con los parámetros
  String postData = "soilMoisture=" + String(soilMoisture) +
                    "&rainSensor=" + String(rainSensor) +
                    "&humidity=" + String(humidity) +
                    "&temperature=" + String(temperature) +
                    "&pressure=" + String(pressure) +
                    "&altitude=" + String(altitude);

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

// Configuración inicial
void setup() {
  Serial.begin(115200);  // Inicializar comunicación serial
  connectToWiFi();  // Llamar a la función para conectar a WiFi
}

// Ciclo principal
void loop() {
  sendDataToServer();  // Llamar a la función para enviar datos al servidor
  delay(5000);  // Espera 5 segundos antes de la siguiente solicitud
}
