#include <ArduinoHttpClient.h>
#include <WiFi.h>

// Configuración de la red WiFi
char ssid[] = "clifford";  // Tu SSID de WiFi
char password[] = "cliff4yyyy"; // Tu contraseña de WiFi

// Configuración del servidor
const char* serverAddress = "192.168.16.24";  // Dirección IP del servidor
const int serverPort = 80;  // Puerto HTTP (por defecto 80)

void setup() {
  Serial.begin(115200);

  // Conéctate a la red Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red WiFi...");
  }

  Serial.println("Conectado a la red WiFi");
}

void loop() {
  // Datos que deseas enviar
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

  // Crea una instancia de HttpClient
  WiFiClient wifi;
  HttpClient http = HttpClient(wifi, serverAddress, serverPort);

  // Construye la URL completa
  String url = "/post";  // Sólo la parte del camino después del dominio

  // Realiza la solicitud POST
  http.post(url, "application/x-www-form-urlencoded", postData);

  // Obtiene y muestra la respuesta del servidor
  String response = http.responseBody();
  Serial.print("Respuesta del servidor: ");
  Serial.println(response);

  // Cierra la conexión
  http.stop();

  delay(5000);  // Espera 5 segundos antes de la siguiente solicitud
}
