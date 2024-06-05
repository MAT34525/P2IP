#include <EEPROM.h>
#include <Arduino.h>

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

int iter = 0;
bool mode = !true; // True : Write / False : Read

void setup()
{
  Serial.begin(9600);

  Serial.println(); // Fin de la dernière ligne
  Serial.println("Nouvelle exécution -----------------"); // Fin de la dernière ligne


  configuration1.heureDebut.heure = 10;
  configuration1.heureDebut.minute = 0;

  configuration1.heureFin.heure = 11;
  configuration1.heureFin.minute = 0;

  configuration1.luminosite = 200;

  configuration1.rouge = 200;
  configuration1.vert = 150;
  configuration1.bleu = 0;

  Dump_EEPROM();

  Sauvegarde_EEPROM();

  Dump_EEPROM();
}

void loop()
{
}

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

void Dump_EEPROM()
{
  Serial.println("Début du dump de l'EEPROM ...");

  for(int i = 0; i < EEPROM.length(); i++)
  {
    Serial.print("ADD [");
    Serial.print(i);
    Serial.print("] : ");
    Serial.println(EEPROM.read(i));
  }
}