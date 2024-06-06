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
#include <Encoder.h>

#define pinInBoutton 2 // SW    (Bouton)
#define pinInRotation 3 // DT
#define pinInSensRotation 4 // CLK
int positionEncodeur = 0;
int derniereRotation;
int nouvelleRotation;
bool sensRotation;

// Ecran OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH 128 // OLED display width,  in pixels
#define OLED_HEIGHT 32 // OLED display height, in pixels

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

// Configuration
#include <EEPROM.h>

struct Heure
{
  byte heure;
  byte minute;
};

struct Configuration
{
  Heure heureDebut;
  Heure heureFin;
  byte luminosite;
  byte rouge;
  byte vert;
  byte bleu;
};

int luminositeMin = 0;
int luminositeMax = 0;

Heure heureDebutActivite;
Heure heureFinActivite;

Configuration configuration1;
Configuration configuration2;
Configuration configuration3;
Configuration configuration4;
Configuration configuration5;

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

  // On charge les configurations depuis l'EEPROM
  Chargement_EEPROM();

  // Initialisation de l'écran OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  display.display();

  display.setTextColor(WHITE);

  // Console
  Serial.begin(9600);
}

void loop()
{
}

// Lis la valeur analogique de la luminosité sur un port analogique
int LireLuminosite()
{
  int etatLumiere = analogRead(pinInLuminosite);
  Serial.print("Lumiere : ");
  Serial.println(etatLumiere);
  return etatLumiere;
}

// Lis l'état du capteur de mouvement sur un port digital
bool LireMouvement()
{
  bool etatMouvement = digitalRead(pinInMouvement);
  Serial.print("Mouvement : ");
  Serial.println(etatMouvement);
  return etatMouvement;
}

// Modifie la configuration de l'anneau LED avec un port digital
bool ConfigurerAnneau(uint32_t couleur)
{
  // Couleur : strip.Color(rouge, vert, bleu, luminosite)

  for (uint16_t i = 0; i < strip.numPixels(); i++) {
      
      strip.setPixelColor(i, couleur);  
      
      strip.show();
  }
}

// Lis l'état d'un bouton avec un port digital
bool LireBoutton()
{
  bool ret = digitalRead(pinInBoutton);

  digitalWrite(pinInBoutton, HIGH);

  Serial.print("Boutton : ");
  Serial.println(!ret);

  return !ret;
}

// Ecoute un port en particulier et lis puis transcrit une série de 8 bits émis sur ce port 
// après un front montant
int RecevoirByte(int pin)
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

// Configuration et sauvegardes //////////////////////////////////////////////////////

// Charge la configuration depuis l'EEPROM
void Chargement_EEPROM()
{
  // Chargement de la dernière configuration
  // get(address, data) : autre que des byte
  // read(address) : byte

  // Structures de données
  Heure typeHeure;
  Configuration typeConfiguration;

  // Luminosité (1 byte)
  luminositeMax = EEPROM.read(0);
  luminositeMin = EEPROM.read(1);

  // Activité (2 bytes)
  heureDebutActivite = EEPROM.get(2, typeHeure);
  heureFinActivite = EEPROM.get(4, typeHeure);

  // Configurations (8 bytes)
  configuration1 = EEPROM.get(6, typeConfiguration);
  configuration2 = EEPROM.get(14, typeConfiguration);
  configuration3 = EEPROM.get(22, typeConfiguration);
  configuration4 = EEPROM.get(30, typeConfiguration);
  configuration5 = EEPROM.get(38, typeConfiguration);
}

// Sauvegrade la configuration dans l'EEPROM
void Sauvegarde_EEPROM()
{
  // Attention : 100 000 écritures max, on a de la marge
  // https://docs.arduino.cc/learn/built-in-libraries/eeprom/#length
  
  // put(address, data) : autre que des bytes a stocker
  // update(address, value) : byte a stocker ssi différent de déjà stocké (sinon utiliser write)

  // Ecriture de la dernière configuration

  // Luminosité (1 byte)
  EEPROM.update(0, luminositeMax);
  EEPROM.update(1, luminositeMin);

  // Activité (2 bytes)
  EEPROM.put(2, heureDebutActivite);
  EEPROM.put(4, heureFinActivite);

  // Configurations (8 bytes)
  EEPROM.put(6, configuration1);

  EEPROM.put(14, configuration2);
  EEPROM.put(22, configuration3);
  EEPROM.put(30, configuration4);
  EEPROM.put(38, configuration5);

}

// Affiche tout le contenu de l'EEPROM
void Dump_EEPROM()
{

  Serial.println("Début du dump de l'EEPROM ...");

  // Affiche toutes les Valeurs de l'EEPROM
  for(int i = 0; i < EEPROM.length(); i++)
  {
    Serial.print("ADD [");
    Serial.print(i);
    Serial.print("] : ");
    Serial.println(EEPROM.read(i));
  }
}