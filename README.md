# Измерение качества воздуха

За основу была взята эта прошивка: https://github.com/airgradienthq/arduino  
Для визуального подключения к сети Wi-fi используется библиотека https://github.com/tzapu/WiFiManager

Контроллер Wemos D1 Mini ESP8266.

Поддерживаемые датчики:
- PMS5003 - датчик загрязнений воздуха
- BME280 - датчик атм. давления, влажности и темературы
- DS18b20 - датчик температуры
- SHT30 - датчик температуры и влажности
- AM2320 - датчик температуры и влажности
- DHT22 - датчик температуры и влажности

При первом включении, если нет подключения к Wi-fi, включается веб-сервер по адресу: http://192.168.4.1. Нужно подключиться к Wi-fi сети "AIR-SENSOR-xxx" и открыть в браузере этот адрес. Далее ввести данные подключения к вашему Wi-fi роутеру и сохранить. Устройство перезагрузится, данные будут сохранены в памяти EEPROM. Для сброса данных подключения к Wi-fi сети зажать кнопку и подержать несколько секунд. Так же кнопкой можно включать и выключать экран (по умолчанию выключен).

Поддерживается режим глубокого сна (deepSleep). Прибор включается за 30 секунд перед отправкой данных на сервер, после отправки снова засыпает. Это помогает экономить заряд аккумуляторной батареи.

В коде можно включить/выключить отправку данных на https://narodmon.ru/ или Shopker (https://habr.com/ru/post/535158/).
Для отправки данных на narodmon в параметре ``char apiOwnerName[20] = "username";`` указать имя своего пользователя. GPS координаты прибора можно не указывать, а указать на сайте narodmon.

## Подключение

**PMS5003**  
Питание - 5В  
TX - D5  
RX - D6  

**BME280**  
Питание - 3.3В  
SCL - D1  
SDA - D2  

**DS18b20**  
Питание - 5В  
Данные - D4 - подтянуть к питанию через резистор 4.7кОм  

Кнопка Wemos shield - D3  

Программы:  
Arduino IDE https://www.arduino.cc/en/software  
Как добавить поддержку платы Wemos D1 mini в Arduino IDE: https://gist.github.com/carljdp/e6a3f5a11edea63c2c14312b534f4e53#file-d1-mini-esp8266-md

![Фото #1](https://github.com/andchir/air-sensor/blob/main/photo/001.jpg?raw=true "Фото #1")
![Фото #1](https://github.com/andchir/air-sensor/blob/main/photo/005.jpg?raw=true "Фото #5")
![Фото #2](https://github.com/andchir/air-sensor/blob/main/photo/002.jpg?raw=true "Фото #2")
![Фото #3](https://github.com/andchir/air-sensor/blob/main/photo/003.jpg?raw=true "Фото #3")
![Фото #4](https://github.com/andchir/air-sensor/blob/main/photo/004.jpg?raw=true "Фото #4")
