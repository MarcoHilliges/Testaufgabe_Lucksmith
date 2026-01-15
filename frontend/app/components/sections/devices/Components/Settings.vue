<script setup lang="ts">
import type { DeviceStatus } from "~/models/message";
import { Info } from "lucide-vue-next";

interface SettingsItem {
  key: string;
  label: string;
  description: string;
  value: string | number | null;
  valueType: "string" | "number";
}

const { $mqtt } = useNuxtApp();
const { t } = useI18n();

const props = defineProps<{
  deviceStatus: DeviceStatus;
  deviceId: string;
  deviceName: string;
}>();

const settingsItems = ref<SettingsItem[]>([
  {
    key: "deviceName",
    label: t("device.settings.deviceName"),
    description: t("device.settings.deviceNameDescription"),
    value: null,
    valueType: "string",
  },
  {
    key: "wifiUpdateInterval",
    label: t("device.settings.wifiUpdateInterval"),
    description: t("device.settings.wifiUpdateIntervalDescription"),
    value: null,
    valueType: "number",
  },
]);

const isLoadingSettings = ref(false);

onMounted(() => {
  $mqtt.on("message", (topic, message) => {
    console.log("Received MQTT message on topic:", topic);
    console.log("Message payload:", message.toString());
    const deviceIdAndName = topic.split("/")[1]; // Extrahiere die Geräte-ID aus dem Topic

    const last_dash_index = deviceIdAndName?.lastIndexOf("-");
    if (!last_dash_index || !deviceIdAndName) return;
    const deviceName = deviceIdAndName.substring(0, last_dash_index);
    const deviceId = deviceIdAndName.substring(last_dash_index + 1);

    if (deviceId !== props.deviceId) return;
    console.log("message received", JSON.parse(message.toString()));
    const messageData = JSON.parse(message.toString());
    
  });
  getSettings();
});

function getSettings() {
  const topic = `esp32/${props.deviceName}-${props.deviceId}/settings/get`;
  $mqtt.publish(topic, "");
}
</script>

<template>
  <div class="relative">
    <ul
      class="overflow-y-auto"
      :class="{
        'opacity-70': isLoadingSettings || props.deviceStatus === 'offline',
      }"
    >
      <li
        v-for="item in settingsItems"
        :key="item.key"
        class="flex items-center gap-12 py-8 border-b last:border-0 px-16 text-12"
      >
        <span class="font-semibold">{{ item.label }}</span>
        <BasicTooltip :tooltipText="item.description">
          <Info class="w-14 h-14 text-gray-500" />
        </BasicTooltip>
        <span class="ml-auto">{{ item.value }}</span>
      </li>
    </ul>
    <div
      v-if="isLoadingSettings || props.deviceStatus === 'offline'"
      class="absolute top-0 left-0 right-0 bottom-0 flex flex-col justify-center items-center"
    >
      <template v-if="isLoadingSettings">
        <BasicSpinner :size="24" />
        <span>{{ t("device.settings.loading") }}</span>
      </template>
      <template v-else>
        <span>{{ t("device.deviceIsOffline") }}</span>
      </template>
    </div>
  </div>
</template>
