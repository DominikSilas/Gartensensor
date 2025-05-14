#include <Arduino_MKRIoTCarrier.h>
#include <Arduino.h>
#include <variant.h>
#include "Tropfenanzeige.hpp"
#include "MqttClientWrapper.hpp"

MKRIoTCarrier carrier;

const int feuchteSensorPin = A6;
const int trockenSchwelle = 900; // Passe diesen Wert an deine Umgebung an
FeuchtigkeitsAnzeige anzeige(carrier);
MqttClientWrapper mqtt("DLProduktion", "Holzbalken8214", "192.168.1.117", 1883);

bool relaisManuell = false;
bool relaisZustand = false;

void mqttNachricht(String topic, String payload) {
    if (topic == "relais/steuerung") {
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
    carrier.display.setCursor(10, 70);
    carrier.display.print("MQTT: ");
    if (mqtt.setupMQTT()) {
        carrier.display.println("OK");
    } else {
        carrier.display.println("FEHLER");
        delay(3000);
    }

    mqtt.setCallback(mqttNachricht);

    carrier.display.setCursor(10, 100);
    carrier.display.println("Lade...");
    delay(1000);
    carrier.display.fillScreen(ST77XX_BLACK);

    anzeige.initAnzeige();
    carrier.Relay1.open(); // Startzustand
}

void loop() {
    int sensorWert = analogRead(feuchteSensorPin);
    Serial.print("Feuchtewert: ");
    Serial.println(sensorWert);

    if (!relaisManuell) {
        relaisZustand = (sensorWert >= trockenSchwelle);
    }

    if (relaisZustand) {
        carrier.Relay1.close();
    } else {
        carrier.Relay1.open();
    }

    // Immer anzeigen – auch bei manuellem Modus
    anzeige.zeichne(sensorWert, relaisZustand);

    // MQTT senden
    String payload = String("{\"feuchte\":") + sensorWert + ",\"relais\":" + (relaisZustand ? "true" : "false") + "}";
    mqtt.publish("sensor/feuchte", payload);

    mqtt.loop();  // nicht vergessen
    delay(1000);
}
