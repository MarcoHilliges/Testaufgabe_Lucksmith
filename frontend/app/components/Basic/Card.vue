<script setup lang="ts">
import type { DeviceStatus } from "~/models/device";

const props = defineProps<{
  deviceStatus?: DeviceStatus;
}>();

const indicatorColor = {
  online: "#15D3A5",
  offline: "#D5D5E2",
  error: "#E51F2B",
};

const cardShadow = computed(() => {
  const shadow = "0 0 8px #c5c5ff";
  if (props.deviceStatus) {
    return `${shadow}, 0 -4px 6px 0px ${indicatorColor[props.deviceStatus]}`;
  }
  return shadow;
});
</script>

<template>
  <div class="rounded-md h-full card-color" :style="{ boxShadow: cardShadow }">
    <slot>Content</slot>
  </div>
</template>
