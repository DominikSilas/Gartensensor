//
// Created by domin on 12.05.2025.
//

#ifndef CODE_MQTTCLIENTWRAPPER_HPP
#define CODE_MQTTCLIENTWRAPPER_HPP

#include <Arduino_MKRIoTCarrier.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <functional>

class MqttClientWrapper {
private:
    MKRIoTCarrier* carrier;
    const char* ssid;
    const char* password;
    const char* broker;
    const char* mqttUser;
    const char* mqttPassword;
    int port;
    WiFiClient wifiClient;
    PubSubClient mqttClient;

    std::function<void(String, String)> onMessageCallback;

    static void callbackWrapper(char* topic, byte* payload, unsigned int length) {
        if (instance && instance->onMessageCallback) {
            String t = topic;
            String p;
            for (unsigned int i = 0; i < length; i++) {
                p += (char)payload[i];
            }
            instance->onMessageCallback(t, p);
        }
    }

    static MqttClientWrapper* instance;

    unsigned long lastReconnectAttempt = 0;

public:
    MqttClientWrapper(const char* ssid, const char* password, const char* broker, const char* mqttUser, const char* mqttPassword, MKRIoTCarrier* carrier, int port = 1883)
            : ssid(ssid), password(password), broker(broker), mqttUser(mqttUser), mqttPassword(mqttPassword),
              port(port), mqttClient(wifiClient), carrier(carrier) {
        instance = this;
    }

    bool isWiFiConnected() {
        return WiFi.status() == WL_CONNECTED;
    }

    bool isMqttConnected() {
        return mqttClient.connected();
    }

    bool connectWiFi() {
        WiFi.begin(ssid, password);
        int versuche = 0;
        while (WiFi.status() != WL_CONNECTED && versuche < 10) {
            delay(500);
            Serial.print(".");
            versuche++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WLAN verbunden!");
            return true;
        } else {
            Serial.println("WLAN-Verbindung fehlgeschlagen!");
            return false;
        }
    }

    bool setupMQTT() {
        mqttClient.setServer(broker, port);
        mqttClient.setCallback(callbackWrapper);

        int versuche = 0;

        while (!mqttClient.connected() && versuche < 2) {
            Serial.print("Verbinde mit MQTT...");

            if (carrier) {
                carrier->display.fillRect(10, 90, 240, 30, ST77XX_BLACK);
                carrier->display.setCursor(10, 90);
                carrier->display.print("Verbinde...");
            }

            if (mqttClient.connect("gartensensor", mqttUser, mqttPassword)) {
                Serial.println(" verbunden.");
                mqttClient.subscribe("relais/steuerung");

                if (carrier) {
                    carrier->display.fillRect(10, 90, 240, 30, ST77XX_BLACK);
                    carrier->display.setCursor(10, 90);
                    carrier->display.println("MQTT OK");
                }

                return true;
            } else {
                Serial.print(" Fehler, rc=");
                Serial.println(mqttClient.state());

                if (carrier) {
                    carrier->display.fillRect(10, 90, 240, 30, ST77XX_BLACK);
                    carrier->display.setCursor(10, 120);
                    carrier->display.print("Fehler: ");
                    carrier->display.println(mqttClient.state());
                }

                delay(2000);
                versuche++;
            }
        }

        return false;
    }

    void publish(const char* topic, const String& payload) {
        mqttClient.publish(topic, payload.c_str());
    }

    void setCallback(std::function<void(String, String)> callback) {
        onMessageCallback = callback;
    }

    void loop() {
        if (!isWiFiConnected() || !isMqttConnected()) {
            unsigned long now = millis();
            if (now - lastReconnectAttempt > 300000) { // alle 5 Minuten
                lastReconnectAttempt = now;
                Serial.println("Versuche WLAN und MQTT neu zu verbinden...");
                if (connectWiFi()) {
                    setupMQTT();
                }
            }
        } else {
            mqttClient.loop();
        }
    }
};


#endif //CODE_MQTTCLIENTWRAPPER_HPP
