#include <UnoR4WiFi_WebServer.h>
#include "index.h"
#include "LedControl.h"
#include "DFRobotDFPlayerMini.h"

// =====================================================================
// Configuración de Red y Servidor Web
// =====================================================================
const char WIFI_SSID[] = "A56 de Antonio G";
const char WIFI_PASSWORD[] = "persona5";

UnoR4WiFi_WebServer server(80);
UnoR4WiFi_WebSocket* webSocket;

unsigned long last_update = 0;
const long updateInterval = 50; // Intervalo de envío (20Hz)

// Page handlers
void handleHome(WiFiClient& client, const String& method, const String& request, const QueryParams& params, const String& jsonData) {
  server.sendResponse(client, HTML_CONTENT);
}

// WebSocket event handlers
void onWebSocketOpen(net::WebSocket& ws) {
  Serial.println("Cliente Web Conectado");
}

void onWebSocketMessage(net::WebSocket& ws, const net::WebSocket::DataType dataType, const char* message, uint16_t length) {}

void onWebSocketClose(net::WebSocket& ws, const net::WebSocket::CloseCode code, const char* reason, uint16_t length) {
  Serial.println("Cliente Web Desconectado");
}

// =====================================================================
// Hardware y Pines (Actuadores y Sensores)
// =====================================================================
const int emgPin      = A2; // sEMG (Controla LEDs, Audio y Calibración)
const int flexPin     = A1; // Sensor de Flexión

// Pines de Matrices LED MAX7219
const int DIN_PIN     = 11;
const int CLK_PIN     = 13;
const int CS_PIN      = 10;
const int NUM_DEVICES = 4;
LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, NUM_DEVICES);

// MP3 (DFPlayer Mini en Serial1)
DFRobotDFPlayerMini miMP3;
unsigned long timerMP3Cooldown = 0;
const long cooldownAudio = 800; 
int ultimoBloqueReproducido = 0;

// Fuente de caracteres (R, C, 3, 2, 1) para las Matrices
const byte FONT_R[8] = { B00000000, B01111100, B01000010, B01000010, B01111100, B01001000, B01000100, B01000010 };
const byte FONT_C[8] = { B00000000, B00111100, B01000010, B01000000, B01000000, B01000000, B01000010, B00111100 };
const byte FONT_3[8] = { B00000000, B00111100, B00000010, B00000010, B00011100, B00000010, B00000010, B00111100 };
const byte FONT_2[8] = { B00000000, B00111100, B00000010, B00000010, B00111100, B01000000, B01000000, B00111110 };
const byte FONT_1[8] = { B00000000, B00001000, B00011000, B00001000, B00001000, B00001000, B00001000, B00011110 };

// Variables de Calibración EMG
const int NUM_LEDS = 32;
float valReposo = 0; 
float valMax = 0; 

#define MUESTRAS_CAL 500
float muestrasContraccion[MUESTRAS_CAL];
int indiceMuestra = 0;
int nivelActual = 0;

void mostrarPatron(int device, const byte patron[8]) {
  lc.clearDisplay(device);
  for (int row = 0; row < 8; row++) {
    lc.setRow(device, row, patron[row]);
  }
}

// =====================================================================
// SETUP
// =====================================================================
void setup() {
  Serial.begin(9600);
  delay(1000);
  analogReadResolution(14); // 14 bits (0 a 16383)

  // --- DFPlayer (Audio) ---
  Serial1.begin(9600);
  delay(500);
  if (!miMP3.begin(Serial1)) {
    Serial.println(F("Error MP3. Revisa cableado RX/TX."));
  } else {
    Serial.println(F("MP3 OK"));
    miMP3.volume(28); 
  }

  // --- Matrices LED ---
  for (int i = 0; i < NUM_DEVICES; i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 2);
    lc.clearDisplay(i);
  }

  // --- INICIALIZAR RED Y MOSTRAR IP PRIMERO EN EL MONITOR SERIAL ---
  Serial.println(F("\n========================================"));
  Serial.println(F("Iniciando Servidor Web y Conectando WiFi..."));
  
  server.addRoute("/", handleHome);
  server.begin(WIFI_SSID, WIFI_PASSWORD);

  webSocket = server.enableWebSocket(81);
  if (webSocket != nullptr) {
    webSocket->onOpen(onWebSocketOpen);
    webSocket->onMessage(onWebSocketMessage);
    webSocket->onClose(onWebSocketClose);
  }

  delay(2000); // Pausa breve para asegurar asignación DHCP

  Serial.println(F("----------------------------------------"));
  Serial.print(F("IP Local asignada: http://"));
  Serial.println(WiFi.localIP());
  Serial.println(F("========================================\n"));

  // -------------------------------------------------------------------
  // ETAPA 1: REPOSO (Calibración base en A2)
  // -------------------------------------------------------------------
  Serial.println(F("--- REPOSO: Mantenga el músculo relajado ---"));
  mostrarPatron(0, FONT_R);
  valReposo = 0;

  for (int seg = 3; seg >= 1; seg--) {
    mostrarPatron(1, seg == 3 ? FONT_3 : seg == 2 ? FONT_2 : FONT_1);
    unsigned long t0 = millis();
    while (millis() - t0 < 1000) {
      float lecturaEMG = (float)analogRead(emgPin);
      server.handleClient();
      server.handleWebSocket();
      if (lecturaEMG > valReposo) valReposo = lecturaEMG;
      delayMicroseconds(500);
    }
  }
  valReposo *= 1.05f; 

  // -------------------------------------------------------------------
  // ETAPA 2: CONTRACCIÓN MÁXIMA (Calibración pico en A2)
  // -------------------------------------------------------------------
  Serial.println(F("--- CONTRACCIÓN: ¡Apriete fuerte y mantenga! ---"));
  mostrarPatron(0, FONT_C); 
  indiceMuestra = 0;
  valMax = 0;

  for (int seg = 3; seg >= 1; seg--) {
    mostrarPatron(1, seg == 3 ? FONT_3 : seg == 2 ? FONT_2 : FONT_1);
    unsigned long t0 = millis();
    while (millis() - t0 < 1000) {
      float lecturaEMG = (float)analogRead(emgPin);
      server.handleClient();
      server.handleWebSocket();
      if (indiceMuestra < MUESTRAS_CAL) {
        muestrasContraccion[indiceMuestra++] = lecturaEMG;
      }
      delayMicroseconds(500);
    }
  }

  // Ordenar y promediar picos altos
  for (int i = 0; i < indiceMuestra - 1; i++) {
    for (int j = i + 1; j < indiceMuestra; j++) {
      if (muestrasContraccion[j] > muestrasContraccion[i]) {
        float tmp = muestrasContraccion[i];
        muestrasContraccion[i] = muestrasContraccion[j];
        muestrasContraccion[j] = tmp;
      }
    }
  }

  int topN = max(1, indiceMuestra / 10);
  float suma = 0;
  for (int i = 0; i < topN; i++) suma += muestrasContraccion[i];
  valMax = suma / topN;

  if (valMax <= valReposo) valMax = valReposo + 500.0f; 

  for (int i = 0; i < NUM_DEVICES; i++) lc.clearDisplay(i);
  
  Serial.print(F("Calibración Lista -> Reposo: ")); Serial.print(valReposo);
  Serial.print(F(" | Máximo: ")); Serial.println(valMax);
}

// =====================================================================
// LOOP PRINCIPAL
// =====================================================================
void loop() {
  server.handleClient();
  server.handleWebSocket();

  unsigned long millisActual = millis();

  // Lecturas de sensores (Flex en A1, sEMG en A2)
  int flexValue  = analogRead(flexPin);
  float x        = (float)analogRead(emgPin);
  float y        = (x <= valReposo) ? 0.0f : x;

  String semgStr = String((int)x);      // sEMG primero (arr[0] para la web)
  String flexStr = String(flexValue);   // Flex segundo (arr[1] para la web)

  // Mapeo EMG a barra LED
  int nivelDeseado = 0;
  if (y > 0) {
    float rango = valMax - valReposo;
    nivelDeseado = (int)(((y - valReposo) / rango) * NUM_LEDS + 0.5f);
    nivelDeseado = constrain(nivelDeseado, 0, NUM_LEDS);
  }
  nivelDeseado = (nivelDeseado / 4) * 4;

  // Animación LED
  if (nivelDeseado > nivelActual) {
    for (int i = nivelActual; i < nivelDeseado; i++) {
      lc.setColumn(i / 8, 7 - (i % 8), B11111111);
    }
    nivelActual = nivelDeseado;
  } 
  else if (nivelDeseado < nivelActual) {
    for (int i = nivelActual - 1; i >= nivelDeseado; i--) {
      lc.setColumn(i / 8, 7 - (i % 8), B00000000);
    }
    nivelActual = nivelDeseado;
  }

  // Audio por bloques
  int bloqueActual = nivelActual / 8;
  if (bloqueActual > ultimoBloqueReproducido && bloqueActual > 0) {
    if (millisActual - timerMP3Cooldown >= cooldownAudio) {
      miMP3.playMp3Folder(bloqueActual);
      timerMP3Cooldown = millisActual;
      ultimoBloqueReproducido = bloqueActual;
    }
  } 
  else if (bloqueActual < ultimoBloqueReproducido) {
    ultimoBloqueReproducido = bloqueActual; 
  }

  // Envío sincronizado (Orden corregido: sEMG primero, Flex segundo para coincidir con index.h)
  if (millisActual - last_update > updateInterval) {
    last_update = millisActual;

    // TO SERIAL PLOTTER
    Serial.print(semgStr);
    Serial.print(" ");           
    Serial.println(flexStr);

    // TO WEB PLOTTER
    if (webSocket != nullptr) {
      webSocket->broadcastTXT(semgStr);
      webSocket->broadcastTXT(" ");
      webSocket->broadcastTXT(flexStr);
      webSocket->broadcastTXT("\r\n");
    }
  }

  delayMicroseconds(500); 
}