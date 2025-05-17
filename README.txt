Feuchtigkeitsüberwachung mit MKR IoT Carrier und MQTT
Dieses Projekt nutzt den Arduino MKR IoT Carrier, um die Bodenfeuchtigkeit zu überwachen und ein Relais zu steuern. Die Feuchtigkeit wird kontinuierlich gemessen, und das Relais kann automatisch oder manuell über MQTT gesteuert werden.

Funktionen
Feuchtemessung: Ein Feuchtigkeitssensor misst die Bodenfeuchtigkeit und zeigt den Wert auf dem Display des MKR IoT Carrier an.

Relaissteuerung: Das Relais kann automatisch (basierend auf der Feuchtigkeit) oder manuell über MQTT gesteuert werden.

Anzeige: Der aktuelle Feuchtigkeitswert und der Zustand des Relais werden auf dem Display angezeigt.

MQTT-Kommunikation: Feuchtigkeitswerte und Relaiszustand werden über MQTT an einen Broker gesendet. Über MQTT kann auch das Relais manuell gesteuert werden.

Komponenten
Arduino MKR WiFi 1010: Mikrocontroller

MKR IoT Carrier: Display und Relaissteuerung

Bodenfeuchtesensor: Für die Feuchtigkeitsmessung

MQTT-Broker: Zum Senden und Empfangen von Nachrichten

MQTT-Nachrichten
Empfangen
Thema: relais/steuerung

Payload:

AN: Relais einschalten

AUS: Relais ausschalten

AUTO: Automatische Steuerung des Relais basierend auf dem Feuchtigkeitswert

Senden
Thema: sensor/feuchte

Payload: JSON-Objekt mit den aktuellen Werten:

json
Kopieren
Bearbeiten
{
  "feuchte": <Feuchtigkeitswert>,
  "relais": <true/false>
}
Funktionsweise
WLAN-Verbindung:

Das System verbindet sich beim Start mit einem WLAN-Netzwerk, um MQTT-Nachrichten zu senden und zu empfangen.

Feuchtigkeitsmessung:

Der Feuchtigkeitssensor liest kontinuierlich den Feuchtigkeitswert aus und zeigt diesen auf dem Display an. Wenn der Wert unter eine festgelegte Schwelle fällt, wird das Relais aktiviert.

Relaissteuerung:

Das Relais kann entweder automatisch über den Feuchtigkeitswert gesteuert werden (wenn der Wert unter die Schwelle fällt) oder manuell über MQTT-Nachrichten.

Anzeige:

Auf dem Display werden der Feuchtigkeitswert und der Relaiszustand angezeigt.

MQTT-Übertragung:

Der Feuchtigkeitswert sowie der Zustand des Relais werden alle 1 Sekunde an den MQTT-Broker gesendet.

Setup
Hardware
Arduino MKR WiFi 1010: Mikrocontroller mit Wi-Fi-Funktionalität

MKR IoT Carrier: Display und Relaissteuerungseinheit

Bodenfeuchtesensor: Angeschlossen an Pin A6

Software
Bibliotheken:

Arduino_MKRIoTCarrier.h: Für die Steuerung des MKR IoT Carrier.

Arduino.h: Standardbibliothek für Arduino-Boards.

variant.h: Wird für bestimmte Hardwarekonfigurationen benötigt.

Tropfenanzeige.hpp: Verantwortlich für die Anzeige der Feuchtigkeitswerte und Relaiszustände.

MqttClientWrapper.hpp: Für die MQTT-Verbindung und -Kommunikation.

Konfiguration:

WLAN: Das WLAN wird mit mqtt.connectWiFi() verbunden.

MQTT: Der MQTT-Broker wird mit den entsprechenden Zugangsdaten konfiguriert.

MQTT-Verbindung
Die MQTT-Verbindung wird mit einem Broker unter der IP-Adresse 192.168.1.117 und Port 1883 hergestellt. Die Anmeldedaten für den Broker sind "DLProduktion" und "Holzbalken8214".

Anpassungen
Feuchteschwelle: Der Wert trockenSchwelle kann an die Umgebungsbedingungen angepasst werden, um die gewünschte Feuchtigkeit für das Einschalten des Relais festzulegen.

WLAN- und MQTT-Einstellungen: Stelle sicher, dass die WLAN- und MQTT-Daten korrekt in der mqtt-Konfiguration hinterlegt sind.

Codeübersicht
Setup
Im setup() wird das System initialisiert:

WLAN-Verbindung herstellen

MQTT-Verbindung aufbauen

Relais auf Startzustand setzen

Display zeigt den Status von WLAN, MQTT und Relais an

Loop
Im loop() wird der Feuchtigkeitswert des Sensors ausgelesen und das Relais gesteuert:

Feuchtigkeitswert wird gemessen.

Relais wird im automatischen oder manuellen Modus gesteuert.

Der Feuchtigkeitswert und der Relaiszustand werden als JSON-Nachricht an den MQTT-Broker gesendet.

Hinweise
Trockenwert anpassen: Die Feuchteschwelle (trockenSchwelle) sollte an die Umgebung angepasst werden. Der Sensorwert variiert je nach Feuchtigkeit des Bodens.

WLAN-Verbindung: Eine stabile WLAN-Verbindung ist erforderlich, um die MQTT-Kommunikation zu ermöglichen.