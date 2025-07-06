//
// Created by domin on 03.05.2025.
//

#ifndef CODE_TROPFENANZEIGE_HPP
#define CODE_TROPFENANZEIGE_HPP

#include <Arduino_MKRIoTCarrier.h>
#include <Arduino.h>
#include <variant.h>

using namespace std;

class FeuchtigkeitsAnzeige {
public:
    FeuchtigkeitsAnzeige(MKRIoTCarrier &carrierRef)
    : carrier(carrierRef) {}

    void initAnzeige() {

    }
    int berechneProzent(int rohwert) {
        int maxWert = 1000;
        int minWert = 800;
        rohwert = constrain(rohwert, minWert, maxWert);
        return map(rohwert, minWert, maxWert, 100, 0);
    }


    void zeichne(int feuchtigkeit, bool relaisAn, bool tankVoll) {
        // Wertebereich begrenzen
        int maxWert = 1000;
        int minWert = 800;
        int x = 120;
        int y = 140;
        int radius = 40;
        int tropfenhoehe = 70;
        feuchtigkeit = constrain(feuchtigkeit, minWert, maxWert);
        long prozent = berechneProzent(feuchtigkeit);
        int fuellHoehe = map(prozent, 0, 100,0, tropfenhoehe + radius);


        if (letzteProzent != prozent) {
        // Nur Füllung & Textbereiche löschen

        carrier.display.fillRect(x - radius, y - tropfenhoehe, radius * 2, tropfenhoehe + radius, ST77XX_BLACK);
        carrier.display.fillRect(70, 10, 160, 20, ST77XX_BLACK);
        carrier.display.fillRect(50, 140, 160, 20, ST77XX_BLACK);


            // Füllung zeichnen
            uint16_t farbe = (prozent < 40) ? ST77XX_ORANGE : ST77XX_BLUE;

            int startY = y + radius; // Unterkante Kreis
            int endY = y - tropfenhoehe; // Spitze Tropfen

            int zeichenStart = startY - fuellHoehe;

            for (int yPos = startY; yPos >= zeichenStart; yPos--) {
                if (yPos < endY) continue; // über die Tropfenspitze hinaus

                int breite = 0;

                if (yPos < y) {
                    // Dreieck (oben)
                    breite = map(y - yPos, 0, tropfenhoehe, radius, 0);
                } else {
                    // Kreis (unten)
                    int dy = abs(yPos - y);
                    if (dy <= radius) {
                        breite = sqrt(sq(radius) - sq(dy));
                    } else {
                        continue;
                    }
                }

                carrier.display.drawFastHLine(x - breite, yPos, breite * 2, farbe);
            }

            // Feuchte-Text aktualisieren
            carrier.display.setTextSize(2);
            carrier.display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            carrier.display.setCursor(60, 10);
            carrier.display.print("Feuchte: ");
            carrier.display.print(prozent);
            carrier.display.print("%");
            carrier.display.setCursor(60,50);
            carrier.display.print("Wert: ");
            carrier.display.print(feuchtigkeit);

            letzteProzent = prozent;
        }

        // Statusanzeige Wasser EIN/AUS (immer aktualisieren)
        carrier.display.setTextSize(2);
        carrier.display.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        carrier.display.setCursor(60, 200);
        carrier.display.print(relaisAn ? "Wasser EIN " : "Wasser AUS ");
        carrier.display.setCursor(60, 180);
        carrier.display.print(tankVoll ? "Tank VOLL " : "Tank LEER");
    }


private:
    MKRIoTCarrier &carrier;
    int letzteProzent = -1;

    void zeichneRahmen() {
        int x = 120;
        int y = 140;
        int radius = 40;
        int tropfenhoehe = 70;

        // Tropfen-Rahmen (statisch)
        carrier.display.drawCircle(x, y, radius, ST77XX_WHITE);
        carrier.display.drawTriangle(x - radius, y, x + radius, y, x, y - tropfenhoehe, ST77XX_WHITE);
    }
};

#endif //CODE_TROPFENANZEIGE_HPP
