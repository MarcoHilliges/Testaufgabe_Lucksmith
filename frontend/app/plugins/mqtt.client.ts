import mqtt from 'mqtt'

// WICHTIG: Ersetze dies durch die IP-Adresse deines PCs, auf dem Docker läuft
const MQTT_BROKER_HOST = '192.168.178.80'; // Deine Docker-Host IP
const MQTT_BROKER_PORT = 15675;         // Port des http/web-mqtt Listeners
const MQTT_BROKER_PATH = '/ws';         // Standardpfad für RabbitMQ's MQTT over WebSockets
const MQTT_USERNAME = 'guest';          // Gemäß docker-compose.yml
const MQTT_PASSWORD = 'guest';          // Gemäß docker-compose.yml

// Eindeutige Client ID für das Dashboard
const CLIENT_ID = 'nuxt-dashboard-' + Math.random().toString(16).substring(2, 8); 

export default defineNuxtPlugin(nuxtApp => {
  // Verbindungskonfiguration für mqtt.js
  const client = mqtt.connect({
    host: MQTT_BROKER_HOST,
    port: MQTT_BROKER_PORT,
    path: MQTT_BROKER_PATH, // Wichtig für WebSockets
    username: MQTT_USERNAME,
    password: MQTT_PASSWORD,
    clientId: CLIENT_ID,
    protocol: 'ws',         // <-- WICHTIG: 'ws' für WebSockets im Browser
    reconnectPeriod: 5000,  // Alle 5 Sekunden reconnecten bei Verbindungsverlust
    keepalive: 5,          // Keepalive-Interval in Sekunden
  });

  client.on('connect', () => {
    console.log('MQTT Client connected to broker (manual plugin)!');
    // Hier könntest du einen globalen Event-Bus triggern, wenn du möchtest
    client.subscribe("esp32/TestESP/gpio/state");
    client.subscribe("esp32/TestESP/status");
    client.subscribe("esp32/TestESP/wifi/scan");
  });

  client.on('error', (error) => {
    console.error('MQTT Client connection error (manual plugin):', error);
  });

  client.on('reconnect', () => {
    console.log('MQTT Client reconnecting...');
  });

  client.on('close', () => {
    console.log('MQTT Client disconnected.');
  });

  // Stelle den MQTT-Client global zur Verfügung
  // So kannst du in jeder Vue-Komponente oder jeder Page darauf zugreifen:
  // const { $mqtt } = useNuxtApp();
  // $mqtt.subscribe(...)
  return {
    provide: {
      mqtt: client // Das Objekt wird unter $mqtt verfügbar sein
    }
  }
});