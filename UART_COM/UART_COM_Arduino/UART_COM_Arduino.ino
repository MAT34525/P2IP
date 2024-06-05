// Aneau LED
#include "Adafruit_NeoPixel.h"

#define NUM_LEDS 16
#define BRIGHTNESS 255
#define pinOutLampe 7
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, pinOutLampe, NEO_GRBW + NEO_KHZ800);

// Capteurs
#define pinInLuminosite 20
#define pinInMouvement 8
#define pinInESP 13

// Encodeur
#define pinInBoutton 2 // SW
#define pinInRotation 3 // DT
#define pinInSensRotation 4 // CLK
int positionEncodeur = 0;
int derniereRotation;
int nouvelleRotation;
bool sensRotation;


void setup()
{
  // Anneau Led Initial
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show();

  // Capteurs
  pinMode(pinInLuminosite, INPUT);
  pinMode(pinInMouvement, INPUT);
  pinMode(pinInESP, INPUT);

  // Encodeur
  pinMode(pinInBoutton, INPUT);
  pinMode(pinInRotation, INPUT);
  pinMode(pinInSensRotation, INPUT);
  derniereRotation = digitalRead(pinInRotation); 

  Serial.begin(9600);
}

void loop()
{
  LireEncodeur(); 
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

int RecieveByte(int pin)
{

  Serial.println("Début de la réception ...");

  int num = 0;

  // Reccupère chacun des bits de la valeur reçue
  for (int i=0; i< 8; i++)
  {
    // Convertis la chaine binaire en entier
    num *= 2;
    if (digitalRead(pin) == 1) num++; 

    delay(50); // Durée de la transmission d'un bit
  }

  num -= 128;

  Serial.println("Fin de la réception");
  Serial.print("Valeur reçue : ");
  Serial.println(num);

  return num;

}

bool LireBoutton()
{
  bool ret = digitalRead(pinInBoutton);

  digitalWrite(pinInBoutton, HIGH);

  Serial.print("Boutton : ");
  Serial.println(!ret);

  return !ret;
}

int LireEncodeur()
{
  nouvelleRotation = digitalRead(pinInRotation);

  if (nouvelleRotation != derniereRotation){ 

    // Sens horaire
    if (digitalRead(pinInSensRotation) != nouvelleRotation) 
    {
      positionEncodeur ++;
      sensRotation = true;

    }
    // Sens anti horaire 
    else 
    {
      sensRotation = false;
      positionEncodeur--;
    }

    Serial.print ("Rotation: ");
    Serial.println(positionEncodeur);
  }
  
  derniereRotation = nouvelleRotation;

  return positionEncodeur;
}


/* Versions précédentes

// Adapte l'intensité en fonction de la luminosité extérieure
void lightSensor() {
  int brightness = analogRead(inPin_LightSensor);
  brightness = map(brightness, 0, 1023, 0, 255);

  strip.setBrightness(brightness);

  strip.show();
}

// Adapte la couleur et l'intensité en fonction de l'heure de la journée
void hourWiFi() {
  int hour = analogRead(inPin_Hour);

  if (hour >= 10 && hour < 20) { // Journée
    strip.fill(strip.Color(255, 245, 222)); // Blanc
    strip.setBrightness(200); // Lumineux
  }

  else if (hour >= 8 && hour < 10 || hour >= 20 && hour < 23) { // Matinée et soirée
    strip.fill(strip.Color(255, 125, 0)); // Orange
    strip.setBrightness(100); // Sombre
  }
  
  else { // Nuit
      strip.clear(); // Eteint
  }
  */