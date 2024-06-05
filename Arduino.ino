// Aneau LED
#include "Adafruit_NeoPixel.h"

#define NUM_LEDS 16
#define BRIGHTNESS 255
#define pinOutLampe 7
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, pinOutLampe, NEO_GRBW + NEO_KHZ800);

// Capteurs
#define pinInLuminosite 20
#define pinInMouvement 8

void setup()
{
  // Anneau Led Initial
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show();

  // Capteurs
  pinMode(pinInLuminosite, INPUT);
  pinMode(pinInMouvement, INPUT);

  Serial.begin(9600);
}


void loop()
{
  ConfigurerAnneau(strip.Color(255, 255, 255, 255));
  LectureLuminosite();
  LectureMouvement();

  delay(1000);

  ConfigurerAnneau(strip.Color(0, 0, 0, 0));
  LectureLuminosite();
  LectureMouvement();

  delay(1000);

}

int LectureLuminosite()
{
  int etatLumiere = analogRead(pinInLuminosite);
  Serial.print("Lumiere : ");
  Serial.println(etatLumiere);
  return etatLumiere;
}

bool LectureMouvement()
{
  bool etatMouvement = digitalRead(pinInMouvement);
  Serial.print("Mouvement : ");
  Serial.println(etatMouvement);
  return etatMouvement;
}

bool ConfigurerAnneau(uint32_t couleur)
{
  // Couleur : strip.Color(rouge, vert, bleu, luminosite)

  for (uint16_t i = 0; i < strip.numPixels(); i++) {
      
      strip.setPixelColor(i, couleur);  
      
      strip.show();
  }
}