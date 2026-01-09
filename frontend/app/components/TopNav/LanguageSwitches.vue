<script setup>
const { locale, locales, setLocale } = useI18n();

const availableLocales = computed(() => {
  return locales.value.filter((i) => i.code !== locale.value);
});
</script>

<template>
  <div class="flex justify-center h-full">
    <button
      v-for="language in locales"
      :key="language.code"
      @click="setLocale(language.code)"
      class="nav-icon-button !p-0 text-12 w-26 h-26"
      :class="{
        'is-active': locale === language.code,
        selectable: locale !== language.code,
      }"
    >
      {{ language.code.toUpperCase() }}
    </button>
  </div>
</template>
