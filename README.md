# Измерение качества воздуха

За основу была взята эта прошивка: https://github.com/airgradienthq/arduino

Контроллер Wemos D1 Mini ESP8266.

Поддерживаемые датчики:
- PMS5003 - датчик загрязнений воздуха
- SHT30 - датчик температуры и влажности
- AM2320 - датчик температуры и влажности
- DHT22 - датчик температуры и влажности

При первом включении, если нет подключения к Wi-fi, включается веб-сервер по адресу: http://192.168.4.1. Нужно подключиться к Wi-fi сети "AIR-SENSOR-xxx" и открыть в браузере этот адрес. Далее ввести данные подключения к вашему Wi-fi роутеру и сохранить. Устройство перезагрузится, данные будут сохранены в памяти EEPROM. Для сброса данных подключения к Wi-fi сети зажать кнопку и подержать несколько секунд. Так же кнопкой можно включать и выключать экран (по умолчанию выключен).

В коде можно включить/выключить отправку данных на https://narodmon.ru/ или Shopker (https://habr.com/ru/post/535158/).
Для отправки данных на narodmon в параметре ``char apiOwnerName[20] = "username";`` указать имя своего пользователя. GPS координаты прибора можно не указывать, а указать на сайте narodmon.


