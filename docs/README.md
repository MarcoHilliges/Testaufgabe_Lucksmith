# Testaufgabe_Lucksmith

* Start Docker
- docker compose up -d

* Stop Docker
- docker compose down -v 
- docker-compose down -v --rmi all

* Fehlersuche
- compose exec rabbitmq cat /etc/rabbitmq/enabled_plugin
- docker compose exec rabbitmq rabbitmq-plugins list -E