<script lang="ts" setup>
// enum MessageTopic {
//   status
//   wifi/scan
//   gpio/state
// }

type StatusMessage = {
  status: "online";
  uptime: number;
};

type WLANMessage = {
  ssid: string;
  rssi: number;
  encryption: number;
};

const { t } = useI18n();

const { $mqtt } = useNuxtApp();

const receivedMessages = ref<
  [
    {
      id: string;
      messages: [
        {
          topic: string;
          messages: [string];
        }
      ];
    }?
  ]
>([]);

const mqttIsConnected = computed(() => {
  return $mqtt && $mqtt.connected;
});

onMounted(() => {
  if (!$mqtt) {
    console.error("MQTT Client not injected by plugin.");
    return;
  }

  $mqtt.on("message", (topic, message) => {
    console.log("Received message:", topic, message.toString());
    const deviceId = topic.split("/")[1]; // Extrahiere die Geräte-ID aus dem Topic
    const topicType = topic.split("/")[2]; // Extrahiere den Nachrichtentyp aus dem Topic
    if (!deviceId || !topicType) return;
    let deviceEntry = receivedMessages.value.find(
      (entry) => entry?.id === deviceId
    );
    if (!deviceEntry) {
      deviceEntry = { id: deviceId, messages: [] };
      receivedMessages.value.push(deviceEntry);
      deviceEntry = receivedMessages.value.find(
        (entry) => entry?.id === deviceId
      );
    }
    const messages = deviceEntry?.messages;
    let topicEntry = messages.find((entry) => entry.topic === topicType);
    if (!topicEntry) {
      topicEntry = { topic: topicType, messages: [] };
      messages.push(topicEntry);
    }
    topicEntry?.messages.push(JSON.parse(message.toString()));
  });
});

onUnmounted(() => {
  // Optional: Clean up listeners or disconnect
  // $mqtt.end(); // Normalerweise will man den globalen Client nicht beenden, wenn eine Komponente unmountet wird.
  // Hier nur die spezifischen Komponent-Listener entfernen.
  $mqtt.removeAllListeners("message"); // Beispiel: Nur Message-Listener entfernen.
});
</script>

<template>
  <div id="start" class="mt-[92px] flex flex-col items-center">
    <span> mqttIsConnected: {{ mqttIsConnected }} </span>
    <div v-for="(message, index) in receivedMessages" :key="index">
      <pre>{{ message }}</pre>
    </div>
  </div>
</template>

<style scoped></style>
