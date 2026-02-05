/**
 * @file remote_sender.h
 * @brief Периодическая отправка телеметрии (JSON) на удалённый HTTPS-сервер.
 */

#ifndef REMOTE_SENDER_H
#define REMOTE_SENDER_H

#include <Arduino.h>

class RemoteSender {
public:
    RemoteSender();

    /** Инициализация (вызвать в setup после WiFi) */
    void begin();

    /**
     * Неблокирующее обновление: при наступлении интервала — отправка JSON по HTTPS.
     * Вызывать в loop при подключённом Wi-Fi.
     */
    void update();

    /** Включена ли отправка (если нет server_secrets.h или не настроено — false) */
    bool isEnabled() const { return _enabled; }

private:
    void sendPayload();

    bool _enabled;
    unsigned long _lastSendMs;
    bool _busy;
};

extern RemoteSender remoteSender;

#endif // REMOTE_SENDER_H
