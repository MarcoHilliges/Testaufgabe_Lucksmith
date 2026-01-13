<script setup lang="ts">
import { LoaderCircle } from "lucide-vue-next";
import type { PropType } from "vue";
const props = defineProps<{
  size?: number;
}>();

const classes = computed(() => {
    const sizeClass = `w-${props.size || 12} h-${props.size || 12}`;

    return `${sizeClass} animate-spin text-gray-600`;
});
</script>

<template>
  <LoaderCircle :class="classes" />
</template>
