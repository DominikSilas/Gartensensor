#include <Arduino_MKRIoTCarrier.h>
#include <Arduino.h>
#include <variant.h>
#include "Tropfenanzeige.hpp"
#include "MqttClientWrapper.hpp"

MKRIoTCarrier carrier;

const int feuchteSensorPin = A6;
const int feuchteSchwelleOben = 950;  // Relais EIN oberhalb dieses Wertes
const int feuchteSchwelleUnten  = 870; // Relais AUS unterhalb dieses Wertes
FeuchtigkeitsAnzeige anzeige(carrier);
MqttClientWrapper mqtt(
        "DLProduktion",              // WLAN-SSID
        "Holzbalken8214",            // WLAN-Passwort
        "192.168.1.117",             // MQTT-Broker-IP
        "mqtt_user",                 // MQTT-Username
        "Garten8235",                // MQTT-Passwort
        &carrier,
        1883
);

bool relaisManuell = false;
bool relaisZustand = false;

void mqttNachricht(String topic, String payload) {
    if (topic == "sensor/garten") {
        if (payload == "AN") {
            relaisZustand = true;
            relaisManuell = true;
        } else if (payload == "AUS") {
            relaisZustand = false;
            relaisManuell = true;
        } else if (payload == "AUTO") {
            relaisManuell = false;
        }
    }
}

void setup() {
    Serial.begin(9600);
    CARRIER_CASE = true;

    carrier.begin();
    carrier.display.begin();

    carrier.display.setRotation(0);         // Teste andere Werte bei Bedarf
    carrier.display.fillScreen(ST77XX_BLACK);
    carrier.display.setTextSize(3);
    carrier.display.setTextColor(ST77XX_RED); // Knallige Farbe
    carrier.display.setCursor(0, 90);         // Sichere Position
    carrier.display.println("System Start");

    delay(2000);
    carrier.display.fillScreen(ST77XX_BLACK);
    carrier.display.setCursor(10, 40);
    carrier.display.print("WLAN: ");
    if (mqtt.connectWiFi()) {
        carrier.display.println("OK");
    } else {
        carrier.display.println("FEHLER");
       delay(3000);
    }

    // MQTT verbinden + Anzeige
    if (mqtt.setupMQTT()) {
        carrier.display.println("");
    } else {
        carrier.display.println("FEHLER");
        delay(3000);
    }

    mqtt.setCallback(mqttNachricht);

    carrier.display.setCursor(10, 120);
    carrier.display.println("Lade...");
    delay(2000);
    carrier.display.fillScreen(ST77XX_BLACK);

    anzeige.initAnzeige();
    carrier.Relay1.open(); // Startzustand
}

void loop() {
    int sensorWert = analogRead(feuchteSensorPin);
    int feuchteProzent = anzeige.berechneProzent(sensorWert);
    Serial.print("Feuchtewert: ");
    Serial.println(sensorWert);

    if (!relaisManuell) {
        if (!relaisZustand && sensorWert >= feuchteSchwelleOben) {
            relaisZustand = true;
        } else if (relaisZustand && sensorWert <= feuchteSchwelleUnten) {
            relaisZustand = false;
        }
    }
    if (relaisZustand) {
        carrier.Relay2.open();
    } else {
        carrier.Relay2.close();
    }

    // Immer anzeigen – auch bei manuellem Modus
    anzeige.zeichne(sensorWert, relaisZustand);

    // MQTT senden
    String payload = String("{\"feuchte\":") + feuchteProzent + ",\"relais\":" + (relaisZustand ? "true" : "false") + "}";
    mqtt.publish("sensor/garten", payload);

    mqtt.loop();  // nicht vergessen
    delay(1000);
}
