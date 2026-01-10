<script lang="ts" setup>
const { t } = useI18n();

const { $mqtt } = useNuxtApp();

const receivedMessages = ref([]);

const mqttIsConnected = computed(() => {
  return $mqtt && $mqtt.connected;
});

onMounted(() => {
  if (!$mqtt) {
    console.error("MQTT Client not injected by plugin.");
    return;
  }

  console.log($mqtt);

  $mqtt.on("message", (topic, message) => {
    console.log("Received message:", topic, message.toString());
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
  </div>
</template>

<style scoped></style>
