import messages from "./locales.setup";

export default defineI18nConfig(() => {
  return {
    legacy: false,
    messages,
  };
});
