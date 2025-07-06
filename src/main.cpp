#include <Arduino_MKRIoTCarrier.h>
#include <Arduino.h>
#include <variant.h>
#include <FlashStorage.h>
#include "Tropfenanzeige.hpp"
#include "MqttClientWrapper.hpp"

MKRIoTCarrier carrier;

const int feuchteSensorPin = A6;
int fuellstandPin = A5;

FlashStorage(schwelleOben, int);
FlashStorage(schwelleUnten, int);

int feuchteSchwelleOben = 950;  // Relais EIN oberhalb dieses Wertes
int feuchteSchwelleUnten  = 870; // Relais AUS unterhalb dieses Wertes

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
bool schwellenwertMenue = false;

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
void zeigeSchwellenwerte() {
    carrier.display.fillScreen(ST77XX_BLACK);
    carrier.display.setTextSize(3);
    carrier.display.setTextColor(ST77XX_GREEN);
    carrier.display.setCursor(10, 40);
    carrier.display.print("Oben: ");
    carrier.display.println(feuchteSchwelleOben);
    carrier.display.print("Unten: ");
    carrier.display.println(feuchteSchwelleUnten);

    carrier.display.setCursor(10, 120);
    carrier.display.setTextSize(2);
    carrier.display.setTextColor(ST77XX_WHITE);
    carrier.display.println("T0:+O T1:-O T4:+U T3:-U");
    carrier.display.println("T2: Zurueck");
}
void ladeSchwellenAusFlash() {
    feuchteSchwelleOben = schwelleOben.read();
    feuchteSchwelleUnten = schwelleUnten.read();

    // Wenn leer oder ungültig, Standardwerte setzen
    if (feuchteSchwelleOben < 500 || feuchteSchwelleOben > 1023) feuchteSchwelleOben = 950;
    if (feuchteSchwelleUnten < 500 || feuchteSchwelleUnten > feuchteSchwelleOben - 10) feuchteSchwelleUnten = 870;
}

void speichereSchwellenInFlash() {
    schwelleOben.write(feuchteSchwelleOben);
    schwelleUnten.write(feuchteSchwelleUnten);
}

void setup() {
    Serial.begin(9600);
    CARRIER_CASE = true;

    carrier.begin();
    carrier.leds.begin();
    ladeSchwellenAusFlash();
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
    carrier.Buttons.update();
    float temperatur = carrier.Env.readTemperature();
    float luftfeuchte = carrier.Env.readHumidity();
    // Menü-Toggle mit Taste T2
    if (carrier.Buttons.onTouchDown(TOUCH2)) {
        schwellenwertMenue = !schwellenwertMenue;
        carrier.display.fillScreen(ST77XX_BLACK);
        if (!schwellenwertMenue) {
            speichereSchwellenInFlash();
            carrier.display.fillScreen(ST77XX_BLACK);
            // Werte vom Sensor lesen (oder gespeicherte Werte verwenden)
            int sensorWert = analogRead(feuchteSensorPin);
            int fuellstandWert = analogRead(fuellstandPin);
            bool fuellstandOK = fuellstandWert > 500;  // Schwelle anpassen je nach Realität
            anzeige.zeichne(sensorWert, relaisZustand, fuellstandOK);
            carrier.display.setCursor(10, 200);
            carrier.display.setTextSize(2);
            carrier.display.setTextColor(ST77XX_YELLOW);
            carrier.display.print("Tank ADC: ");
            carrier.display.println(fuellstandWert);
        }
        delay(500);
    }

    if (schwellenwertMenue) {
        bool geaendert = false;

        if (carrier.Buttons.onTouchDown(TOUCH0)) { // Oben ++
            feuchteSchwelleOben += 5;
            geaendert = true;
        }
        if (carrier.Buttons.onTouchDown(TOUCH1)) { // Oben --
            feuchteSchwelleOben -= 5;
            geaendert = true;
        }
        if (carrier.Buttons.onTouchDown(TOUCH4)) { // Unten ++
            feuchteSchwelleUnten += 5;
            geaendert = true;
        }
        if (carrier.Buttons.onTouchDown(TOUCH3)) { // Unten --
            feuchteSchwelleUnten -= 5;
            geaendert = true;
        }

        if (feuchteSchwelleOben <= feuchteSchwelleUnten + 10) {
            feuchteSchwelleOben = feuchteSchwelleUnten + 10;
        }

        if (geaendert) {
            zeigeSchwellenwerte();
            delay(300);
        }

        return; // Menü aktiv, nichts anderes ausführen
    }


    int sensorWert = analogRead(feuchteSensorPin);
    int feuchteProzent = anzeige.berechneProzent(sensorWert);
    bool fuellstandOK = (digitalRead(fuellstandPin) == HIGH);
    Serial.print("Feuchtewert: ");
    Serial.println(sensorWert);

    if (fuellstandOK && !relaisManuell) {
        if (!relaisZustand && sensorWert >= feuchteSchwelleOben) {
            relaisZustand = true;
        } else if (relaisZustand && sensorWert <= feuchteSchwelleUnten) {
            relaisZustand = false;
        }
    }

// Sicherheitsabschaltung bei leerem Tank
    if (!fuellstandOK) {
        relaisZustand = false;
    }

// Relais schalten
    if (relaisZustand) {
        carrier.Relay2.open();
    } else {
        carrier.Relay2.close();
    }

// Immer anzeigen
    anzeige.zeichne(sensorWert, relaisZustand, fuellstandOK);

// Visualisierung der Touch-Tasten mit LEDs
    carrier.leds.setPixelColor(0, carrier.Buttons.getTouch(TOUCH0) ? carrier.leds.Color(255, 0, 0) : 0);
    carrier.leds.setPixelColor(1, carrier.Buttons.getTouch(TOUCH1) ? carrier.leds.Color(0, 255, 0) : 0);
    carrier.leds.setPixelColor(2, carrier.Buttons.getTouch(TOUCH2) ? carrier.leds.Color(0, 0, 255) : 0);
    carrier.leds.setPixelColor(3, carrier.Buttons.getTouch(TOUCH3) ? carrier.leds.Color(255, 255, 0) : 0);
    carrier.leds.setPixelColor(4, carrier.Buttons.getTouch(TOUCH4) ? carrier.leds.Color(255, 0, 255) : 0);
    carrier.leds.show();

    // MQTT senden
    String payload = String("{\"feuchte\":") + feuchteProzent +
            ",\"relais\":" + (relaisZustand ? "true" : "false") +
            ",\"temperatur\":" + temperatur +
            ",\"luftfeuchte\":" + luftfeuchte +
            ",\"tankVoll\":" + (fuellstandOK ? "true" : "false")+ "}";

    mqtt.publish("sensor/garten", payload);

    mqtt.loop();  // nicht vergessen
    delay(1000);
}
