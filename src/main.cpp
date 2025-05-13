#include <Arduino_MKRIoTCarrier.h>
#include <Arduino.h>
#include <variant.h>
#include "Tropfenanzeige.hpp"
#include "MqttClientWrapper.hpp"

MKRIoTCarrier carrier;

const int feuchteSensorPin = A6;
const int trockenSchwelle = 900; // Passe diesen Wert an deine Umgebung an
FeuchtigkeitsAnzeige anzeige(carrier);
MqttClientWrapper mqtt("DLProduktion", "Holzbalken8214", "192.168.1.117");

bool relaisManuell = false;
bool relaisZustand = false;

//void mqttNachricht(String topic, String payload) {
 //   if (topic == "relais/steuerung") {
 //       if (payload == "AN") {
  //          relaisZustand = true;
  //          relaisManuell = true;
    //    } else if (payload == "AUS") {
      //      relaisZustand = false;
        //    relaisManuell = true;
//        } else if (payload == "AUTO") {
//            relaisManuell = false;
//        }
//    }
//}

void setup() {
    Serial.begin(9600);
    carrier.display.fillScreen(ST77XX_BLUE);
    delay(1000);
//    mqtt.connectWiFi();
//    mqtt.setupMQTT();
//    mqtt.setCallback(mqttNachricht);

    CARRIER_CASE = true;

    carrier.begin();
    carrier.display.begin();

    anzeige.initAnzeige();

        carrier.Relay1.open(); // Startzustand
        carrier.display.setRotation(0);         // Teste andere Werte bei Bedarf
        carrier.display.fillScreen(ST77XX_BLACK);
        carrier.display.setTextSize(3);
        carrier.display.setTextColor(ST77XX_RED); // Knallige Farbe
        carrier.display.setCursor(0, 90);         // Sichere Position
        carrier.display.println("System Start");

    delay(2000);
        carrier.display.fillScreen(ST77XX_BLACK);
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

    // 🔄 Immer anzeigen – auch bei manuellem Modus
    anzeige.zeichne(sensorWert, relaisZustand);

    // MQTT senden
//    String payload = String("{\"feuchte\":") + sensorWert + ",\"relais\":" + (relaisZustand ? "true" : "false") + "}";
//    mqtt.publish("sensor/feuchte", payload);

//    mqtt.loop();  // nicht vergessen
//    delay(1000);
}
