<script setup lang="ts">
import { WifiZero, WifiLow, WifiHigh, Wifi } from "lucide-vue-next";
import type { ContentTab, Device, GPIOPin, GPIOPinState } from "~/models/device";
import {
  MessageTopic,
  WifiSubTopic,
} from "~/models/message";
import type { MqttClientState } from "~/models/mqtt";

const emit = defineEmits<{
  setGpioPin: [{ pin: GPIOPin; value: GPIOPinState }];
  getStatus: [];
  getWifiScan: [];
  getGpioStates: [];
}>();

const props = defineProps<{
  device: Device;
  clientState: MqttClientState;
  activeTab?: ContentTab;
}>();

const toast = useToast();
const { t } = useI18n();

// Status
const lastStatusMessage = computed(() => {
  return (
    props.device.messages.find((msg) => msg.topic === MessageTopic.STATUS)
      ?.messages[0] || null
  );
});

const deviceStatus = computed(() => {
  return props.clientState === "connected" &&
    props.device.lastSeen &&
    lastStatusMessage.value?.status
    ? lastStatusMessage.value?.status
    : "offline";
});

const statusColor = computed(() => {
  switch (deviceStatus.value) {
    case "online":
      return "bg-success";

    case "error":
      return "bg-error";

    case "offline":
      return "bg-gray-300";

    default:
      return "bg-gray-300";
  }
});

watch(
  () => deviceStatus.value,
  (newValue) => {
    console.log(newValue);
    switch (newValue) {
      case "online":
        toast.success({
          title: props.device.name,
          message: t(`common.status.${newValue}`),
        });
        break;
      case "error":
        toast.error({
          title: props.device.name,
          message: t(`common.status.${newValue}`),
        });
        break;
      case "offline":
        toast.info({
          title: props.device.name,
          message: t(`common.status.${newValue}`),
        });
    }
  },
);

// Wifi
const wifiScanMessages = computed(() => {
  return (
    props.device.messages
      .find((msg) => msg.topic === MessageTopic.WIFI)
      ?.messages.filter((m) => m.supTopic === WifiSubTopic.SCAN) || []
  );
});

// GPIO
const gpioMessages = computed(() => {
  return (
    props.device.messages.find((msg) => msg.topic === MessageTopic.GPIO)
      ?.messages || []
  );
});

// Helpers
function formatTimestamp(timestamp: number | undefined | null) {
  if (!timestamp) return "-";
  const date = new Date(timestamp);
  return date.toLocaleString();
}

watch(
  () => props.device.lastSeen,
  (newVal) => {
    if (
      props.clientState === "connected" &&
      Date.now() - (newVal || 60000) > 45000
    ) {
      emit("getStatus");
    }
  },
  { immediate: true },
);
</script>

<template>
  <BasicCard
    :device-status="deviceStatus"
    class="max-w-[400px] min-w-[300px] sm:min-w-[400px] h-[300px] flex flex-col pt-16 m-24 rounded-md card-color light-effect"
  >
    <div class="mx-16">
      <div class="flex justify-between gap-10">
        <h2>{{ props.device.name }}</h2>
        <div class="flex gap-12 items-center">
          <BasicTooltip v-if="lastStatusMessage && deviceStatus === 'online'">
            <component
              :is="
                lastStatusMessage.rssi >= -50
                  ? Wifi
                  : lastStatusMessage.rssi < -50 &&
                      lastStatusMessage.rssi >= -65
                    ? WifiHigh
                    : lastStatusMessage.rssi < -65 &&
                        lastStatusMessage.rssi >= -75
                      ? WifiLow
                      : WifiZero
              "
              class="w-16 h-16"
            />

            <template #tooltipText>
              <div class="flex gap-6">
                <span>{{ lastStatusMessage.wifi }}</span>
                |
                <span>RSSI: {{ lastStatusMessage.rssi }} dBm</span>
              </div>
            </template>
          </BasicTooltip>
        </div>
      </div>
      <div class="text-10">
        <span> #{{ props.device.id }} </span>
      </div>
    </div>

    <div class="flex-grow overflow-hidden text-12">
      <div
        v-if="props.activeTab === 'overview'"
        class="flex justify-between gap-12 h-full px-16"
      >
        <DevicesSectionsWiFiScanList
          :wiFiScanMessages="wifiScanMessages"
          :deviceStatus="deviceStatus"
          :connectedWiFi="lastStatusMessage?.wifi || ''"
          class="w-1/2"
          @getWifiScan="emit('getWifiScan')"
        />
        <DevicesSectionsGpioList
          :deviceName="props.device.name"
          :gpios="props.device.gpios"
          :gpioStateMessages="gpioMessages"
          :deviceStatus="deviceStatus"
          class="w-1/2"
          @getGpioStates="emit('getGpioStates')"
          @setGpioPin="(params) => emit('setGpioPin', params)"
        />
      </div>

      <DevicesSectionsWiFiHistory
        :wiFiScanMessages="wifiScanMessages"
        v-else-if="props.activeTab === 'wifi'"
      />

      <DevicesSectionsGpioDetailList
        v-else-if="props.activeTab === 'gpio'"
        :deviceName="props.device.name"
        :gpios="props.device.gpios"
        :gpioStateMessages="gpioMessages"
        :deviceStatus="deviceStatus"
        @getGpioStates="emit('getGpioStates')"
        @setGpioPin="(params) => emit('setGpioPin', params)"
      />

      <DevicesSectionsSettings
        v-else-if="props.activeTab === 'settings'"
        :deviceStatus="deviceStatus"
        :deviceId="props.device.id"
        :deviceName="props.device.name"
      />
    </div>
  </BasicCard>
</template>
