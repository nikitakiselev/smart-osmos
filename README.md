# Умный Осмос — прошивка ESP32

Прошивка для системы мониторинга воды: TDS-метр, два расходомера, Wi-Fi, веб-интерфейс.

---

## Аппаратура

| Компонент | Описание |
|-----------|----------|
| **ESP32** | DOIT ESP32 DEVKIT V1 или аналог |
| **TDS meter v1.0** | Датчик качества воды, аналоговый выход → пин `PIN_TDS` |
| **Два расходомера** | С импульсным выходом → пины `PIN_FLOW_IN`, `PIN_FLOW_OUT` |

---

## 1. Установка зависимостей

Выберите один способ: **Arduino IDE** или **PlatformIO**.

### Вариант A: Arduino IDE

1. Скачайте и установите [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x или 2.x).
2. Откройте **Файл → Настройки** (File → Preferences).
3. В поле **Дополнительные ссылки для менеджера плат** (Additional boards manager URLs) добавьте:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. **Инструменты → Плата → Менеджер плат** (Tools → Board → Boards Manager).
5. Найдите **esp32** от **Espressif Systems** и нажмите **Установить** (Install).
6. Дополнительные библиотеки не требуются — используются встроенные WiFi и WebServer.

### Вариант B: PlatformIO

1. Установите [PlatformIO](https://platformio.org/install):
   - как расширение для [VS Code](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide), или  
   - как [Core (CLI)](https://docs.platformio.org/en/latest/core/installation.html):  
     `pip install platformio`
2. При первой сборке (`pio run`) PlatformIO сам скачает платформу **espressif32** и нужные инструменты — интернет должен быть включён.

---

## 2. Настройка перед сборкой

### Wi-Fi (обязательно, не хранится в репозитории)

1. В папке **arduino/smart-osmos** скопируйте файл-шаблон в рабочий файл:
   - **Windows:** `copy wifi_secrets.h.example wifi_secrets.h`
   - **Linux/macOS:** `cp wifi_secrets.h.example wifi_secrets.h`
2. Откройте **arduino/smart-osmos/wifi_secrets.h** и подставьте свои `WIFI_SSID` и `WIFI_PASSWORD`.  
   Файл `wifi_secrets.h` в git не попадает — пароль остаётся только у вас.

### Пины и прочее (config.h)

В файле **arduino/smart-osmos/config.h** задайте:

| Параметр | Описание |
|----------|----------|
| `PIN_TDS` | Пин ADC для TDS-метра (рекомендуется **34** — ADC1, совместим с Wi-Fi) |
| `PIN_FLOW_IN` | Пин расходомера на дренаж (например 19) |
| `PIN_FLOW_OUT` | Пин выходного расходомера (например 16) |
| `FLOW_PULSES_PER_LITER` | Импульсов на литр по даташиту расходомера (часто 450–750) |

Остальные константы (таймауты, калибровка TDS) при необходимости тоже меняются в `config.h`.

### Удалённая телеметрия (HTTPS, опционально)

Если нужна отправка данных на ваш HTTPS-сервер:

1. В **arduino/smart-osmos** скопируйте `server_secrets.h.example` → `server_secrets.h`.
2. В **server_secrets.h** укажите: `SERVER_HOST`, `SERVER_PORT`, `SERVER_PATH`, `SERVER_API_KEY` (тот же ключ, что и на Python-сервере).
3. В **config.h** задайте `REMOTE_SEND_INTERVAL_MS` (мс между отправками, например 60000). При 0 отправка отключена.

---

## 3. Прошивка (загрузка на ESP32)

### Arduino IDE

1. Подключите ESP32 к компьютеру по USB.
2. Откройте папку **arduino/smart-osmos**:
   - **Файл → Открыть** и выберите файл `arduino/smart-osmos/smart-osmos.ino`,  
   или откройте саму папку `arduino/smart-osmos` (должен открыться скетч `smart-osmos.ino`).
3. **Инструменты → Плата** → выберите **ESP32 Arduino** → **DOIT ESP32 DEVKIT V1** (или **ESP32 Dev Module**).
4. **Инструменты → Порт** → выберите порт ESP32 (например **COM3** в Windows, **/dev/ttyUSB0** в Linux).
5. Нажмите кнопку **Загрузка** (стрелка вправо).
6. Если ошибка «Failed to connect» — переведите ESP32 в режим загрузки: зажмите кнопку **BOOT** на плате, нажмите **EN** (RST), отпустите **BOOT** после начала загрузки.

### PlatformIO (CLI)

1. Подключите ESP32 по USB.
2. В корне проекта выполните:
   ```bash
   pio run -t upload
   ```
3. Порт по умолчанию задаётся в **platformio.ini** (`upload_port = COM3`). Чтобы указать порт вручную:
   ```bash
   pio run -t upload --upload-port COM3
   ```
   (подставьте свой порт: `COM4`, `/dev/ttyUSB0` и т.д.)
4. При ошибке подключения переведите ESP32 в режим загрузки (кнопка **BOOT** + сброс **EN**), затем повторите команду.

### PlatformIO (VS Code)

1. Откройте папку проекта в VS Code с установленным расширением PlatformIO.
2. В нижней панели нажмите **Upload** (стрелка вправо) или через меню **Project → Upload**.
3. Порт можно выбрать в статус-баре (внизу) или в **platformio.ini** (`upload_port`).

---

## 4. После запуска

1. Подключите компьютер/телефон к той же Wi-Fi сети, что указана в `config.h`.
2. Откройте **Монитор порта** (Arduino IDE: Инструменты → Монитор порта; PlatformIO: кнопка Serial Monitor). Скорость: **115200** бод.
3. В логе появится строка вида:  
   `[Web] Сервер запущен`  
   `      http://192.168.x.x`
4. Откройте этот адрес в браузере — отобразятся TDS (ppm), расход вход/выход (л/мин), объём (л). Данные обновляются каждые 2 с и по кнопке **Обновить**.

---

## 5. Python-сервер (приём телеметрии и история)

Сервер принимает JSON по HTTPS, пишет в SQLite и отдаёт UI с графиками по часам/дням/неделям.

### Зависимости

```bash
cd server
pip install -r requirements.txt
```

### Запуск

1. Задайте API-ключ (должен совпадать с `SERVER_API_KEY` в прошивке):
   ```bash
   set OSMOS_API_KEY=your-secret-api-key
   ```
   (Linux/macOS: `export OSMOS_API_KEY=your-secret-api-key`)

2. Опционально — сгенерировать сертификат для HTTPS (иначе используется adhoc):
   ```bash
   python gen_cert.py
   ```
   Появятся `cert.pem` и `key.pem` в папке `server/`.

3. Запуск сервера (порт 8443 по умолчанию):
   ```bash
   python app.py
   ```
   Переменные окружения: `OSMOS_API_KEY`, `OSMOS_HOST` (0.0.0.0), `OSMOS_PORT` (8443), `OSMOS_DB` (путь к SQLite), `OSMOS_CERT`/`OSMOS_KEY` (пути к сертификату).

4. В браузере откройте `https://localhost:8443` (при самоподписанном сертификате подтвердите исключение безопасности). Вкладки «По часам», «По дням», «По неделям» — графики TDS и объёма.

5. В **server_secrets.h** на ESP32 укажите IP машины с сервером и порт (например `SERVER_HOST` = IP ПК, `SERVER_PORT` = 8443). После прошивки ESP32 будет раз в минуту (или по `REMOTE_SEND_INTERVAL_MS`) отправлять JSON на `https://<SERVER_HOST>:<PORT>/api/ingest`.

### Сервер за Caddy (reverse proxy с HTTPS)

Если перед приложением стоит **Caddy** как reverse proxy (HTTPS на Caddy, проксирование на бэкенд):

1. **Python-сервер** запускайте **без HTTPS** — Caddy принимает HTTPS и проксирует на бэкенд по HTTP. Пример:
   ```bash
   set OSMOS_PORT=8080
   set OSMOS_NO_SSL=1
   python app.py
   ```
   (Linux/macOS: `export OSMOS_NO_SSL=1`.) В Caddy укажите `reverse_proxy 127.0.0.1:8080`.

2. В **server_secrets.h** на ESP32 укажите хост и порт **Caddy** (домен или IP сервера, порт 443): `SERVER_HOST`, `SERVER_PORT` = 443, `SERVER_PATH` = `/api/ingest` (если в Caddy путь к бэкенду именно такой).

3. Если ESP32 выдаёт **«Connection reset by peer»** при подключении по HTTPS к Caddy — часто причина в том, что Caddy по умолчанию предлагает только современные шифры/протоколы, с которыми mbedTLS на ESP32 не согласуется. В **Caddyfile** для этого сайта добавьте явные `protocols` и `ciphers`, совместимые с ESP32 (TLS 1.2 и ECDHE-RSA с AES):
   ```caddyfile
   your-domain.com {
       tls {
           protocols tls1.2 tls1.3
           ciphers TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384
       }
       reverse_proxy 127.0.0.1:8080
   }
   ```
   После правок перезагрузите Caddy (`caddy reload` или перезапуск сервиса).

---

## Структура проекта

| Путь | Назначение |
|------|------------|
| **arduino/smart-osmos/** | Проект для Arduino IDE (скетч и все исходники) |
| **arduino/smart-osmos/config.h** | Пины, калибровка TDS и расходомеров |
| **arduino/smart-osmos/wifi_secrets.h** | SSID и пароль Wi-Fi (создать из `wifi_secrets.h.example`, в git не коммитить) |
| **arduino/smart-osmos/smart-osmos.ino** | Точка входа для Arduino (setup/loop) |
| **arduino/smart-osmos/tds_sensor.*** | Чтение TDS, усреднение, формула TDS meter v1.0 |
| **arduino/smart-osmos/flow_meter.*** | Расходомеры: прерывания, дебаунс, л/мин и объём |
| **arduino/smart-osmos/wifi_connector.*** | Подключение к Wi-Fi и реконнект |
| **arduino/smart-osmos/web_server.*** | HTTP-сервер, веб-страница и JSON API |
| **arduino/smart-osmos/remote_sender.*** | Отправка телеметрии по HTTPS на удалённый сервер |
| **arduino/smart-osmos/server_secrets.h** | Хост, порт, путь и API-ключ сервера (создать из `.example`, в git не коммитить) |
| **platformio.ini** | Конфигурация PlatformIO, каталог исходников и порт загрузки |
| **server/** | Python HTTPS-сервер: приём JSON, SQLite, UI с графиками по часам/дням/неделям |

---

## Расширение (NVS/EEPROM)

- Калибровку TDS (`setCalibration`/`getCalibration`) можно сохранять в NVS при изменении и подгружать в `setup`.
- Счётчики импульсов расходомеров при необходимости периодически сохранять в NVS и восстанавливать после перезагрузки (сейчас объём считается с нуля при каждом старте).
