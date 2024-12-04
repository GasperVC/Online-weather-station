#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

//POST en CMD: curl -X POST -d "soilMoisture=500&rainSensor=300&humidity=45&temperature=25&pressure=1013&altitude=100" http://192.168.16.24/post

// Credenciales Wi-Fi
const char* ssid = "clifford";
const char* password = "cliff4yyyy";

// Variables de sensores
int soilMoisture = 0; // Sensor de humedad de tierra
int rainSensor = 0;   // Sensor de lluvia
int humidity = 0;     // Humedad atmosférica
int temperature = 0;  // Temperatura
int pressure = 0;     // Presión atmosférica
int altitude = 0;     // Altitud

// Pines de los LEDs RGB (agrupados por sensor)
const int rgbPins[4][3] = {
  {16, 17, 18}, // soilMoisture: {Red, Green, Blue}
  {19, 21, 22}, // rainSensor: {Red, Green, Blue}
  {23, 25, 26}, // humidity: {Red, Green, Blue}
  {27, 32, 33}  // temperature: {Red, Green, Blue}
};

// Variables de sensores
int sensorValues[4] = {0, 0, 0, 0}; // {soilMoisture, rainSensor, humidity, temperature}

// Crear servidor web y WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Función para controlar el color de un LED RGB
void setRGBAnalog(int sensorIndex, int redValue, int greenValue, int blueValue) {
  analogWrite(rgbPins[sensorIndex][0], redValue);
  analogWrite(rgbPins[sensorIndex][1], greenValue);
  analogWrite(rgbPins[sensorIndex][2], blueValue);
}

// Actualizar LEDs RGB según los valores de los sensores
void updateRGB() {
  // LED para soilMoisture (Humedad del suelo)
  if (soilMoisture == 0) {
    setRGBAnalog(0, 0, 255, 0); // Verde (Muy húmedo, peligroso)
  } else if (soilMoisture < 200) {
    setRGBAnalog(0, 144, 238, 144); // Verde claro (Húmedo, crítico)
  } else if (soilMoisture < 400) {
    setRGBAnalog(0, 255, 255, 0); // Amarillo (Moderado)
  } else if (soilMoisture < 600) {
    setRGBAnalog(0, 255, 165, 0); // Naranja (Moderado)
  } else if (soilMoisture < 800) {
    setRGBAnalog(0, 255, 69, 0); // Rojo anaranjado (Seco, ideal)
  } else {
    setRGBAnalog(0, 255, 0, 0); // Rojo (Extremadamente seco)
  }

  // LED para rainSensor (Sensor de lluvia)
  if (rainSensor == 0) {
    setRGBAnalog(1, 0, 255, 0); // Verde (Tormenta extrema, crítico)
  } else if (rainSensor < 200) {
    setRGBAnalog(1, 255, 255, 0); // Amarillo (Tormenta fuerte, riesgo)
  } else if (rainSensor < 400) {
    setRGBAnalog(1, 255, 165, 0); // Naranja (Lluvia intensa)
  } else if (rainSensor < 600) {
    setRGBAnalog(1, 255, 69, 0); // Rojo anaranjado (Lluvia moderada, alerta)
  } else if (rainSensor < 800) {
    setRGBAnalog(1, 255, 0, 0); // Rojo (Lluvia ligera, no crítica)
  } else {
    setRGBAnalog(1, 139, 0, 0); // Rojo oscuro (Sin lluvia, ideal)
  }

  // LED para humidity (Humedad ambiente)
  if (humidity < 30) {
    setRGBAnalog(2, 255, 0, 0); // Rojo (Muy baja humedad, crítico)
  } else if (humidity < 40) {
    setRGBAnalog(2, 255, 69, 0); // Rojo anaranjado (Baja humedad, riesgo)
  } else if (humidity < 45) {
    setRGBAnalog(2, 255, 140, 0); // Naranja (Humedad baja, tolerable)
  } else if (humidity < 50) {
    setRGBAnalog(2, 144, 238, 144); // Verde claro (Humedad ideal, óptima)
  } else if (humidity < 55) {
    setRGBAnalog(2, 0, 128, 0); // Verde medio (Buena, dentro de rango)
  } else if (humidity < 60) {
    setRGBAnalog(2, 255, 255, 0); // Amarillo (Humedad moderada)
  } else if (humidity < 70) {
    setRGBAnalog(2, 255, 165, 0); // Amarillo anaranjado (Alta humedad, alerta)
  } else {
    setRGBAnalog(2, 255, 0, 0); // Rojo (Humedad muy alta, peligroso)
  }

  // LED para temperature (Temperatura ambiente)
  if (temperature < 15) {
    setRGBAnalog(3, 0, 0, 255); // Azul (Muy frío, riesgo)
  } else if (temperature < 20) {
    setRGBAnalog(3, 0, 128, 255); // Azul claro (Frío, tolerable)
  } else if (temperature < 22) {
    setRGBAnalog(3, 0, 255, 255); // Cian (Temperatura baja, cómoda)
  } else if (temperature < 25) {
    setRGBAnalog(3, 0, 255, 0); // Verde (Temperatura óptima)
  } else if (temperature < 28) {
    setRGBAnalog(3, 144, 238, 144); // Verde claro (Temperatura cálida, ideal)
  } else if (temperature < 30) {
    setRGBAnalog(3, 255, 255, 0); // Amarillo (Cálido, no crítico)
  } else if (temperature < 35) {
    setRGBAnalog(3, 255, 165, 0); // Naranja (Cálido, alerta)
  } else if (temperature < 40) {
    setRGBAnalog(3, 255, 0, 0); // Rojo (Calor, peligro)
  } else {
    setRGBAnalog(3, 139, 0, 0); // Rojo oscuro (Temperatura extrema, crítico)
  }
}

// Función para notificar cambios a los clientes WebSocket
void notifyClients() {
  String message = "{\"soilMoisture\":" + String(soilMoisture) + "," +
                   "\"rainSensor\":" + String(rainSensor) + "," +
                   "\"humidity\":" + String(humidity) + "," +
                   "\"temperature\":" + String(temperature) + "," +
                   "\"pressure\":" + String(pressure) + "," +
                   "\"altitude\":" + String(altitude) + "}";
  ws.textAll(message);

  // Actualizar LEDs RGB
  updateRGB();
}

// Función para manejar mensajes `POST`
void handlePostRequest(AsyncWebServerRequest *request) {
  // Verificar parámetros de sensores
  if (request->hasParam("soilMoisture", true)) {
    soilMoisture = request->getParam("soilMoisture", true)->value().toInt();
  }
  if (request->hasParam("rainSensor", true)) {
    rainSensor = request->getParam("rainSensor", true)->value().toInt();
  }
  if (request->hasParam("humidity", true)) {
    humidity = request->getParam("humidity", true)->value().toInt();
  }
  if (request->hasParam("temperature", true)) {
    temperature = request->getParam("temperature", true)->value().toInt();
  }
  if (request->hasParam("pressure", true)) {
    pressure = request->getParam("pressure", true)->value().toInt();
  }
  if (request->hasParam("altitude", true)) {
    altitude = request->getParam("altitude", true)->value().toInt();
  }

  // Notificar a los clientes WebSocket
  notifyClients();

  // Responder al cliente
  request->send(200, "application/json", "{\"message\":\"Datos actualizados correctamente\"}");
}

void setup() {
  Serial.begin(115200);

  // Configuración de los pines de los LEDs RGB
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      pinMode(rgbPins[i][j], OUTPUT);
    }
  }

  // Conexión Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Configuración del WebSocket
  ws.onEvent([](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type,
  void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("WebSocket client #%u connected\n", client->id());
      notifyClients();
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->opcode == WS_TEXT) {
        String message = "";
        for (size_t i = 0; i < len; i++) {
          message += (char)data[i];
        }
        Serial.println("Received WebSocket Message: " + message);
      }
    }
  });
  server.addHandler(&ws);

  // Página web con dashboard
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <title>ESP32 Dashboard - Estación Meteorológica</title>
  <style>
    /* Estilos globales */
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #f4f7fa;
      color: #333;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      text-align: center;
      flex-direction: column;
    }

    /* Título principal */
    h1 {
      font-size: 28px;
      margin: 20px 0;
      color: #3c6e71;
      text-transform: uppercase;
      letter-spacing: 1px;
    }

    /* Contenedor para los sensores */
    .sensor-container {
      display: flex;
      flex-direction: column;
      gap: 15px;
      width: 90%;
      max-width: 400px;
      padding: 10px;
    }

    /* Sección de cada sensor */
    .sensor-section {
      background-color: #fff;
      border: 1px solid #ccc;
      border-radius: 8px;
      padding: 15px;
      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
    }

    .sensor-title {
      font-size: 16px;
      font-weight: bold;
      color: #3c6e71;
      margin-bottom: 5px;
    }

    .sensor-value {
      font-size: 22px;
      font-weight: bold;
      margin: 5px 0;
    }

    .sensor-description {
      font-size: 12px;
      color: #7b7b7b;
    }

    /* Colores específicos para cada sensor */
    .sensor-soil-moisture .sensor-value {
      color: #6a994e;
    }

    .sensor-rain-sensor .sensor-value {
      color: #0077b6;
    }

    .sensor-humidity .sensor-value {
      color: #9b59b6;
    }

    .sensor-temperature .sensor-value {
      color: #e67e22;
    }

    .sensor-pressure .sensor-value {
      color: #34495e;
    }

    .sensor-altitude .sensor-value {
      color: #2ecc71;
    }

    /* Estilos para pantallas pequeñas */
    @media (max-width: 600px) {
      h1 {
        font-size: 24px;
      }

      .sensor-container {
        width: 100%;
        padding: 5px;
      }

      .sensor-title {
        font-size: 14px;
      }

      .sensor-value {
        font-size: 20px;
      }

      .sensor-description {
        font-size: 11px;
      }
    }
  </style>
</head>
<body>

  <h1>ESP32 Sensor Dashboard</h1>

  <div class="sensor-container">
    
    <!-- Sección de Humedad de Tierra -->
    <div class="sensor-section sensor-soil-moisture">
      <div class="sensor-title">Humedad de Tierra</div>
      <div class="sensor-value" id="soil-moisture">-</div>
      <div class="sensor-description" id="soil-moisture-desc">-</div>
    </div>

    <!-- Sección de Sensor de Lluvia -->
    <div class="sensor-section sensor-rain-sensor">
      <div class="sensor-title">Sensor de Lluvia</div>
      <div class="sensor-value" id="rain-sensor">-</div>
      <div class="sensor-description" id="rain-sensor-desc">-</div>
    </div>

    <!-- Sección de Humedad Atmosférica -->
    <div class="sensor-section sensor-humidity">
      <div class="sensor-title">Humedad Atmosférica</div>
      <div class="sensor-value" id="humidity">-</div>
      <div class="sensor-description" id="humidity-desc">-</div>
    </div>

    <!-- Sección de Temperatura -->
    <div class="sensor-section sensor-temperature">
      <div class="sensor-title">Temperatura</div>
      <div class="sensor-value" id="temperature">-</div>
      <div class="sensor-description" id="temperature-desc">-</div>
    </div>

    <!-- Sección de Presión Atmosférica -->
    <div class="sensor-section sensor-pressure">
      <div class="sensor-title">Presión Atmosférica</div>
      <div class="sensor-value" id="pressure">-</div>
    </div>

    <!-- Sección de Altitud -->
    <div class="sensor-section sensor-altitude">
      <div class="sensor-title">Altitud</div>
      <div class="sensor-value" id="altitude">-</div>
    </div>

  </div>

  <script>
    // Conexión WebSocket
    const ws = new WebSocket(`ws://${window.location.hostname}/ws`);

    ws.onmessage = (event) => {
      console.log(event.data);  // Verificar los datos que llegan
      const data = JSON.parse(event.data);  // Recibe el mensaje en formato JSON
    
      // Actualizar los valores de los sensores en la página
      document.getElementById("soil-moisture").innerText = data.soilMoisture;
      document.getElementById("soil-moisture-desc").innerText =
        data.soilMoisture === 0 ? "Muy húmedo (máxima humedad)" :
        data.soilMoisture < 200 ? "Húmedo (alta humedad)" :
        data.soilMoisture < 400 ? "Moderado" :
        data.soilMoisture < 600 ? "Seco" :
        data.soilMoisture < 800 ? "Muy seco" : "Extremadamente seco (máxima sequedad)";
    
      // Hacer lo mismo para los otros sensores...
      document.getElementById("rain-sensor").innerText = data.rainSensor;
      document.getElementById("rain-sensor-desc").innerText =
        data.rainSensor === 0 ? "Tormenta extrema" :
        data.rainSensor < 200 ? "Lluvia intensa" :
        data.rainSensor < 400 ? "Lluvia moderada" :
        data.rainSensor < 600 ? "Lluvia ligera" : "Sin lluvia";
    
      document.getElementById("humidity").innerText = data.humidity;
      document.getElementById("humidity-desc").innerText =
        data.humidity < 30 ? "Muy baja" :
        data.humidity < 40 ? "Baja" :
        data.humidity < 45 ? "Normal baja" :
        data.humidity < 50 ? "Ideal" :
        data.humidity < 55 ? "Normal" :
        data.humidity < 60 ? "Alta" : "Muy alta";
    
      document.getElementById("temperature").innerText = data.temperature;
      document.getElementById("temperature-desc").innerText =
        data.temperature < 10 ? "Muy fría" :
        data.temperature < 20 ? "Fría" :
        data.temperature < 30 ? "Ideal" :
        data.temperature < 40 ? "Cálida" : "Extremadamente cálida";
    
      document.getElementById("pressure").innerText = data.pressure;
      document.getElementById("altitude").innerText = data.altitude;
    };
  </script>

</body>
</html>

)rawliteral");
  });

  // Ruta para manejar `POST`
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {
    handlePostRequest(request);
  });

  server.begin();
}

void loop() {
  ws.cleanupClients();
}
