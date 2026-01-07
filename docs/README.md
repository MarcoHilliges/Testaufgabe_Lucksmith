# Testaufgabe_Lucksmith

* Start Docker
- docker compose up -d

* Fehlersuche
- compose exec rabbitmq cat /etc/rabbitmq/enabled_plugin
- docker compose exec rabbitmq rabbitmq-plugins list -E