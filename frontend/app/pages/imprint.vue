<script setup lang="ts">
const { t } = useI18n();

useHead({
  title: t("common.imprint"),
});
</script>

<template>
  <div class="flex flex-col items-center pt-[100px]">
    <TopNav />
    <h1 class="mb-24">
      {{ t("common.imprint") }}
    </h1>

    Marco Hilliges <br />
    Clara-Zetkin-Ring 55 <br />
    15232 Frankfurt (Oder)<br />

    <br />
    {{ t("common.phone") }}: +49 174 1847047 <br />
    <br />
    E-Mail: marco.hilliges@gmx.de
  </div>
</template>
