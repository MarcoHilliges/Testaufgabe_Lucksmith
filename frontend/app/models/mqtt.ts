export type MqttClientState =
  | "connected"
  | "disconnected"
  | "error"
  | "reconnecting";