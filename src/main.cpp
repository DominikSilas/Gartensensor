#include <Arduino_MKRIoTCarrier.h>
#include <Arduino.h>
#include <variant.h>
#include <FlashStorage.h>
#include "Tropfenanzeige.hpp"
#include "MqttClientWrapper.hpp"

MKRIoTCarrier carrier;

const int feuchteSensorPin = A6;
const int fuellstandPin = 6;  // digitaler Eingang

FlashStorage(schwelleOben, int);
FlashStorage(schwelleUnten, int);

int feuchteSchwelleOben = 950;
int feuchteSchwelleUnten = 870;

FeuchtigkeitsAnzeige anzeige(carrier);
MqttClientWrapper mqtt(
        "DLProduktion",
        "Holzbalken8214",
        "192.168.1.117",
        "mqtt_user",
        "Garten8235",
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

void ladeSchwellenAusFlash() {
    feuchteSchwelleOben = schwelleOben.read();
    feuchteSchwelleUnten = schwelleUnten.read();

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

    carrier.display.setRotation(0);
    carrier.display.fillScreen(ST77XX_BLACK);
    carrier.display.setTextSize(3);
    carrier.display.setTextColor(ST77XX_RED);
    carrier.display.setCursor(0, 90);
    carrier.display.println("System Start");

    pinMode(fuellstandPin, INPUT);

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

    if (mqtt.setupMQTT()) {
        carrier.display.println("");
    } else {
        carrier.display.println("FEHLER");
        delay(3000);
    }

    mqtt.setCallback(mqttNachricht);

    carrier.display.setCursor(10, 140);
    carrier.display.println("Lade...");
    delay(2000);
    carrier.display.fillScreen(ST77XX_BLACK);

    anzeige.initAnzeige();
    carrier.Relay1.close(); // Startzustand
}
void zeigeSchwellenAufDisplay(int sensorWert, bool relais, bool tankVoll) {
    carrier.display.fillScreen(ST77XX_BLACK);
    anzeige.zeichne(sensorWert, relais, tankVoll);

    carrier.display.setTextSize(2);
    carrier.display.setTextColor(ST77XX_WHITE);
    carrier.display.setCursor(120, 50);
    carrier.display.print("O: ");
    carrier.display.print(feuchteSchwelleOben);
    carrier.display.setCursor(10, 50);
    carrier.display.print("U: ");
    carrier.display.print(feuchteSchwelleUnten);
}
void loop() {
    carrier.Buttons.update();

    float temperatur = carrier.Env.readTemperature();
    float luftfeuchte = carrier.Env.readHumidity();
    int sensorWert = analogRead(feuchteSensorPin);
    int feuchteProzent = anzeige.berechneProzent(sensorWert);
    bool fuellstandOK = (digitalRead(fuellstandPin) == HIGH);

    bool geaendert = false;

    if (carrier.Buttons.onTouchDown(TOUCH0)) { feuchteSchwelleOben += 5; geaendert = true; }
    if (carrier.Buttons.onTouchDown(TOUCH1)) { feuchteSchwelleOben -= 5; geaendert = true; }
    if (carrier.Buttons.onTouchDown(TOUCH4)) { feuchteSchwelleUnten += 5; geaendert = true; }
    if (carrier.Buttons.onTouchDown(TOUCH3)) { feuchteSchwelleUnten -= 5; geaendert = true; }

    if (feuchteSchwelleOben <= feuchteSchwelleUnten + 10) {
        feuchteSchwelleOben = feuchteSchwelleUnten + 10;
        geaendert = true;
    }

    if (geaendert) {
        speichereSchwellenInFlash();
        zeigeSchwellenAufDisplay(sensorWert, relaisZustand, fuellstandOK);
        delay(300);
        return;
    }

    if (fuellstandOK && !relaisManuell) {
        if (!relaisZustand && sensorWert >= feuchteSchwelleOben) {
            relaisZustand = true;
        } else if (relaisZustand && sensorWert <= feuchteSchwelleUnten) {
            relaisZustand = false;
        }
    }

    if (!fuellstandOK) {
        relaisZustand = false;
    }

    if (relaisZustand) {
        carrier.Relay2.open();
    } else {
        carrier.Relay2.close();
    }

    anzeige.zeichne(sensorWert, relaisZustand, fuellstandOK);

    String payload = String("{\"feuchte\":") + feuchteProzent +
                     ",\"relais\":" + (relaisZustand ? "true" : "false") +
                     ",\"temperatur\":" + temperatur +
                     ",\"luftfeuchte\":" + luftfeuchte +
                     ",\"tankVoll\":" + (fuellstandOK ? "true" : "false") + "}";

    mqtt.publish("sensor/garten", payload);
    mqtt.loop();
    delay(1000);
}
