<script lang="ts" setup>
const { t } = useI18n();

useHead({
  title: t("common.appTitle"),
});
</script>

<template>
  <div class="h-full flex flex-col gap-48">
    <TopNav show-nav/>
    <Devices />
  </div>
</template>
