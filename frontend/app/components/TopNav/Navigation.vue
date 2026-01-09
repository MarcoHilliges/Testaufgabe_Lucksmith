<script lang="ts" setup>
const { t } = useI18n();

const navLinks = [
  {
    id: "#start",
    text: t("common.start"),
  }
];
</script>

<template>
  <ul class="flex font-bold text-12 sm:text-16">
    <li
      v-for="link in navLinks"
      :key="link.id"
      class="border-r px-8 last:border-none"
    >
      <NuxtLink :to="link.id" class="nav-link">{{ link.text }}</NuxtLink>
    </li>
  </ul>
</template>

<style scoped>
.nav-link {
  @apply hover:text-primary-hover active:text-primary-active dark:hover:text-primary-lightHover dark:active:text-primary-lightActive;
}
</style>
