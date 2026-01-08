#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ----------------------------------------
// 1. WLAN-Einstellungen
// ----------------------------------------
const char* ssid = "DEIN_WLAN_SSID";
const char* password = "DEIN_WLAN_PASSWORT";

// ----------------------------------------
// 2. MQTT-Einstellungen
// ----------------------------------------
// !!! WICHTIG: Ersetze dies durch die IP-Adresse deines PCs, auf dem Docker läuft !!!
const char* mqtt_broker = "192.168.1.100";
const int mqtt_port = 1883;
const char* mqtt_user = "guest";          // Gemäß docker-compose.yml
const char* mqtt_pass = "guest";          // Gemäß docker-compose.yml

// Eindeutige ID für diesen ESP32 (muss für jedes Gerät eindeutig sein!)
// Wird im MQTT-Topic verwendet.
const String deviceId = "TestESP"; // Z.B. esp32-1, esp32-buero, etc.

// MQTT Topics (verwende String-Objekte für einfache Konkatenation)
String topic_status_pub = "esp32/" + deviceId + "/status"; // Für Heartbeat/Online-Status
String topic_wifi_scan_pub = "esp32/" + deviceId + "/wifi/scan"; // Für WiFi-Scan-Ergebnisse
String topic_gpio_state_pub = "esp32/" + deviceId + "/gpio/state"; // Für GPIO-Zustände
String topic_gpio_set_sub = "esp32/" + deviceId + "/gpio/set"; // Zum Empfangen von GPIO-Befehlen

WiFiClient espClient; // Erstellt einen TCP-Client für die WLAN-Verbindung
PubSubClient client(espClient); // Erstellt den MQTT-Client mit dem TCP-Client

// ----------------------------------------
// 3. GPIO-Steuerung
// ----------------------------------------
// Definiere die Pins, die du steuern möchtest (mindestens 4)
// Beispiele für häufig verwendete GPIOs: 2, 4, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27
// Vermeide GPIOs 0, 1, 3, 5, 6-11 (Flash-Pins), 12 (Boot-Pin), 14 (Boot-Pin), 15 (Boot-Pin) für einfache Verwendung
#define LED_PIN 2     // Beispiel-LED
#define RELAY_PIN_1 4
#define RELAY_PIN_2 16
#define SENSOR_GPIO 17 // Beispiel: Dieser Pin meldet nur seinen Zustand, wird nicht gesteuert

// Aktuelle Zustände der steuerbaren Pins (für Status-Reporting)
int gpio_states[4] = {LOW, LOW, LOW, LOW}; // Entspricht der Reihenfolge der Pins im Array unten
int control_pins[] = {LED_PIN, RELAY_PIN_1, RELAY_PIN_2, SENSOR_GPIO}; // Die zu steuernden/überwachenden Pins
const int NUM_PINS = sizeof(control_pins) / sizeof(control_pins[0]);


// ----------------------------------------
// 4. Timer für periodische Aufgaben
// ----------------------------------------
long lastHeartbeatTime = 0;
const long heartbeatInterval = 30000; // 30 Sekunden

long lastWifiScanTime = 0;
const long wifiScanInterval = 60000; // 60 Sekunden

// ----------------------------------------
// MQTT Callback-Funktion (für eingehende Nachrichten)
// ----------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Nachricht empfangen auf Topic: [");
  Serial.print(topic);
  Serial.print("] Payload: ");
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Wenn der Befehl für GPIO-Steuerung empfangen wird
  if (String(topic) == topic_gpio_set_sub) {
    // Annahme: Message-Format ist "PIN_NUMMER:STATE" z.B. "2:ON" oder "4:OFF"
    int colonIndex = message.indexOf(':');
    if (colonIndex != -1) {
      String pinStr = message.substring(0, colonIndex);
      String stateStr = message.substring(colonIndex + 1);

      int pinNum = pinStr.toInt();
      int newState = (stateStr == "ON" || stateStr == "1") ? HIGH : LOW;

      // Finde den Pin im Array und aktualisiere seinen Zustand
      for (int i = 0; i < NUM_PINS; i++) {
        if (control_pins[i] == pinNum) {
          digitalWrite(pinNum, newState);
          gpio_states[i] = newState; // Zustand im Array aktualisieren
          Serial.print("GPIO ");
          Serial.print(pinNum);
          Serial.print(" auf ");
          Serial.println(newState == HIGH ? "HIGH" : "LOW");

          // Statusänderung zurückmelden
          // Sende den Zustand aller Pins, oder nur des geänderten
          reportGpioStates(); // Sende alle Pins als JSON
          break;
        }
      }
    }
  }
}

// ----------------------------------------
// WLAN-Verbindungslogik
// ----------------------------------------
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WLAN verbunden!");
  Serial.print("IP Adresse: ");
  Serial.println(WiFi.localIP());
}

// ----------------------------------------
// MQTT Reconnect-Logik
// ----------------------------------------
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Versuche MQTT-Verbindung...");
    if (client.connect(deviceId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("verbunden");
      // Nach erfolgreicher Verbindung:
      client.publish(topic_status_pub.c_str(), "online", true); // "online" mit retained flag
      client.subscribe(topic_gpio_set_sub.c_str()); // Steuer-Topic abonnieren

      // Initialen GPIO-Status senden
      reportGpioStates();

    } else {
      Serial.print("fehlgeschlagen, rc=");
      Serial.print(client.state());
      Serial.println(" -> Erneuter Versuch in 5 Sekunden");
      delay(5000);
    }
  }
}

// ----------------------------------------
// Funktionen für Pflicht-Features
// ----------------------------------------

// Heartbeat / Online-Status senden
void sendHeartbeat() {
  Serial.println("Sende Heartbeat: online");
  // Optional: JSON mit Uptime, Speicher etc.
  String payload = "{\"status\":\"online\",\"uptime\":" + String(millis()/1000) + "}";
  client.publish(topic_status_pub.c_str(), payload.c_str(), true); // Retained, falls Dashboard später startet
}

// WiFi-Scan durchführen und Ergebnisse senden
void performWifiScan() {
  Serial.println("Suche WLAN-Netzwerke...");
  int n = WiFi.scanNetworks();

  if (n <= 0) { // Fehler oder keine Netzwerke gefunden
    Serial.print("Keine Netzwerke gefunden oder Fehler beim Scan: ");
    Serial.println(n);
    if (client.connected()) {
      client.publish(topic_wifi_scan_pub.c_str(), "{\"networks\":[]}"); // Leeres JSON-Array senden
    } else {
      Serial.println("MQTT Client ist NICHT verbunden, WiFi Scan nicht gesendet.");
    }
    
  } else { // Netzwerke gefunden
    // 2048 Bytes sind eine gute, sichere Obergrenze für normale Nachrichten.
    DynamicJsonDocument doc(2048); // Versuche 2048 Bytes als Kapazität

    JsonObject root = doc.to<JsonObject>();
    JsonArray networks = root.createNestedArray("networks");

    for (int i = 0; i < n; ++i) {
      JsonObject network = networks.createNestedObject();
      network["ssid"] = WiFi.SSID(i);
      network["rssi"] = WiFi.RSSI(i);
      network["encryption"] = WiFi.encryptionType(i); 
    }

    // Erstelle den JSON-String
    String json_output;
    serializeJson(doc, json_output);

    Serial.print("Heap nach JSON-Erstellung: "); Serial.println(ESP.getFreeHeap());
    Serial.print("Größe der JSON-Nachricht: "); Serial.print(json_output.length()); 
    Serial.println(" Bytes");

    if (client.connected()) {
      Serial.println("MQTT Client ist verbunden, sende WiFi Scan.");
      client.publish(topic_wifi_scan_pub.c_str(), json_output.c_str());
    } else {
      Serial.println("MQTT Client ist NICHT verbunden, WiFi Scan nicht gesendet.");
    }
  }

  WiFi.scanDelete(); // Scan-Ergebnisse löschen, um Speicher freizugeben
}

// Aktuellen GPIO-Status aller Pins senden
void reportGpioStates() {
  String json = "{\"gpio_states\":{";
  for (int i = 0; i < NUM_PINS; i++) {
    json += "\"" + String(control_pins[i]) + "\":" + (gpio_states[i] == HIGH ? "1" : "0");
    if (i < NUM_PINS - 1) {
      json += ",";
    }
  }
  json += "}}";
  Serial.print("Sende GPIO-Zustände: ");
  Serial.println(json);
  client.publish(topic_gpio_state_pub.c_str(), json.c_str());
}

// ----------------------------------------
// SETUP-Funktion
// ----------------------------------------
void setup() {
  Serial.begin(115200); // Serielle Ausgabe starten für Debugging

  // GPIO Pins als OUTPUT konfigurieren und initialen Zustand setzen
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(control_pins[i], OUTPUT); // Für SENSOR_GPIO würde man INPUT_PULLUP nutzen
    digitalWrite(control_pins[i], LOW);
    gpio_states[i] = LOW; // Initialzustand speichern
  }

  setup_wifi(); // WLAN verbinden

  client.setServer(mqtt_broker, mqtt_port); // MQTT-Broker einstellen
  client.setCallback(callback); // Callback-Funktion für eingehende Nachrichten registrieren

  client.setBufferSize(2048); // Erhöhe den Puffer auf 2048 Bytes (oder mehr, falls nötig)
}

// ----------------------------------------
// LOOP-Funktion (wird wiederholt ausgeführt)
// ----------------------------------------
void loop() {
  // Sicherstellen, dass MQTT verbunden ist
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop(); // Muss regelmäßig aufgerufen werden, um Nachrichten zu empfangen und Keep-Alive zu senden

  unsigned long currentMillis = millis();

  // Heartbeat senden
  if (currentMillis - lastWifiScanTime < wifiScanInterval && currentMillis - lastHeartbeatTime >= heartbeatInterval) {
    lastHeartbeatTime = currentMillis;
    sendHeartbeat();
  }

    // WiFi-Scan durchführen
  if (currentMillis - lastWifiScanTime >= wifiScanInterval) {
    lastWifiScanTime = currentMillis;
    performWifiScan();
  }

  // Kleine Verzögerung, um den ESP32 nicht zu überlasten
  delay(1);
}