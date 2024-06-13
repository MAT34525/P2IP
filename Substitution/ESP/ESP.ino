// Bluetooth

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Anneau LED
#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 16
#define BRIGHTNESS 255
#define pinOutLampe 5       // Modifier
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, pinOutLampe, NEO_GRB + NEO_KHZ800);

// Capteurs
#define pinInLuminosite 2  // Modifier
#define pinInMouvement 4    // Modifier

// Configurationqqq
#include <EEPROM.h>
#define EEPROM_SIZE 50 // Ajouté pour l'ESP

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

byte luminositeMin = 0;
byte luminositeMax = 255;

Heure heureDebutActivite;
Heure heureFinActivite;

Configuration * Configs = new Configuration[5];

bool estNocturne;
byte configurationParDefaut;

// Parametres globaux
Heure heureActuelle;
int iter = 0;
String valor_return = "";
bool estConnecte = false;
bool estAuto = false;

// Configuration et sauvegardes ////////////////////////////////////////

// Charge la configuration depuis l'EEPROM
void Chargement_EEPROM()
{
  // Chargement de la dernière configuration
  // get(address, data) : autre que des byte
  // read(address) : byte

  Serial.println("Chargement des données de l'EEPROM...");

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
  for(int i = 0; i < 5; i++)
  {
    Configs[i] = EEPROM.get(6 + 8 * i, typeConfiguration);
  }

  // Configuration par défaut & autres
  estNocturne = EEPROM.read(4 + 2 + 8 * 5);
  configurationParDefaut = EEPROM.read(4 + 2 + 8 * 5 + 1);

  Serial.println("Données chargées !");
}

// Sauvegrade la configuration dans l'EEPROM
void Sauvegarde_EEPROM()
{
  Serial.println("Sauvegarde des données dans l'EEPROM...");

  // Attention : 100 000 écritures max, on a de la marge
  // https://docs.arduino.cc/learn/built-in-libraries/eeprom/#length
  
  // put(address, data) : autre que des bytes a stocker
  // update(address, value) : byte a stocker ssi différent de déjà stocké (sinon utiliser write)

  // Ecriture de la dernière configuration

  // Luminosité (1 byte)
  EEPROM.write(0, luminositeMax);
  EEPROM.write(1, luminositeMin);

  // Activité (2 bytes)
  EEPROM.put(2, heureDebutActivite);
  EEPROM.put(4, heureFinActivite);

  // Configurations (8 bytes)
  for(int i = 0; i < 5; i++)
  {
    EEPROM.put(6 + 8 * i, Configs[i]);
  }

  // Configuration par défaut & autres
  EEPROM.write(4 + 2 + 8 * 5, estNocturne);
  EEPROM.write(4 + 2 + 8 * 5 + 1, configurationParDefaut);

  EEPROM.commit(); // Ajouté pour l'ESP

  Serial.println("Données sauvegardées !");
}

void Sauvegarde_Brute_EEPROM(String strs[] )
{
  Serial.println("Sauvegarde brute des données dans l'EEPROM...");

  // Affiche toutes les Valeurs de l'EEPROM
  for(int i = 0; i < EEPROM_SIZE -2 ; i++)
  {
    EEPROM.write(i, strs[i].toInt());
  }

  EEPROM.commit(); // Ajouté pour l'ESP

  Dump_EEPROM();
}

// Affiche tout le contenu de l'EEPROM
void Dump_EEPROM()
{

  Serial.println("Début du dump de l'EEPROM ...");

  // Affiche toutes les Valeurs de l'EEPROM
  for(int i = 0; i < EEPROM_SIZE ; i++)
  {
    Serial.print("ADD [");
    Serial.print(i);
    Serial.print("] : ");
    Serial.println(EEPROM.read(i));
  }
}

// Charge l'EEPROM avec des données initiales
void Initialisation_EEPROM()
{
  heureDebutActivite.heure = 7;
  heureDebutActivite.minute = 20;

  heureFinActivite.heure = 21;
  heureFinActivite.minute = 30;

  luminositeMax = 255;
  luminositeMin = 0;

  estNocturne = 1;
  configurationParDefaut = 0;

  for(int i = 0; i < 5; i++)
  {
    Heure heureDebut = {10+i, 0};
    Heure heureFin = {10+i, 50};
    Configs[i] = {heureDebut, heureFin, 255, i * 30,  i * 30, i * 30 };
  }

  for(int i = 0; i < 5; i++)
  {
    Serial.print("Config");
    Serial.println(i);
    Serial.println(Configs[i].luminosite);
    Serial.println(Configs[i].rouge);
    Serial.println(Configs[i].vert);
    Serial.println(Configs[i].bleu);
  }

  Sauvegarde_EEPROM();

}

// BLUETOOTH ////////////////////////////////////////////////////////////////
class MyCallbacks : public BLECharacteristicCallbacks 
{

    void onWrite(BLECharacteristic *pCharacteristic) {

      String value = pCharacteristic->getValue();
      
      if (value.length() > 0) {

        valor_return = valor_return + value.substring(0, value.length() - 1 );
      }

      if(valor_return.indexOf("#") != -1)
      { 
        value = valor_return.substring(0, valor_return.length() - 1);
        valor_return = "";

        Serial.println(" Value : " + value);
        Serial.println(" Valor return : " + value); 

        if(value.startsWith("START"))
        {
          Serial.println("START Atteint");

          estConnecte = true;
          estAuto = false;

          // on récuppère l'heure intiale passé en parametre

          if(value.length() > 6)
          {
            String tronc = value.substring(value.indexOf(" ", 0)+1, value.length());

            heureActuelle.heure = tronc.substring(0, tronc.indexOf(' ')).toInt();

            tronc = tronc.substring(tronc.indexOf(' ') + 1, tronc.length());

            heureActuelle.minute = tronc.substring(tronc.indexOf(' ') + 1, tronc.length()).toInt();

            Serial.print("Heure saisie : ");
            Serial.print(heureActuelle.heure);
            Serial.print(" : ");
            Serial.println(heureActuelle.minute);
          }

          // On envoie les données intiales
          String data = "";

          data += String(luminositeMax) + "/";
          data += String(luminositeMin) + "/";

          data += String(heureDebutActivite.heure) + "/";
          data += String(heureDebutActivite.minute) + "/";
          data += String(heureFinActivite.heure) + "/";
          data += String(heureFinActivite.minute) + "/";

          for(int i = 0; i < 5; i++)
          {
            data += String(Configs[i].heureDebut.heure)  + "/";
            data += String(Configs[i].heureDebut.minute) + "/";
            data += String(Configs[i].heureFin.heure) + "/";
            data += String(Configs[i].heureFin.minute) + "/";
            data += String(Configs[i].luminosite) + "/";
            data += String(Configs[i].rouge) + "/";
            data += String(Configs[i].vert) + "/";
            data += String(Configs[i].bleu) + "/";
          }

          data += String(estNocturne) + "/";
          data += String(configurationParDefaut);

          Serial.println(data.c_str());

          pCharacteristic->setValue(data.c_str());
        }
        else if( value.startsWith("GET_HOUR"))
        {
          Serial.println("GET_HOUR Atteint");

          String data = "RESP ";

          data += String(heureActuelle.heure) + " " + String(heureActuelle.minute);

          Serial.println(data.c_str());

          pCharacteristic->setValue(data.c_str());

        }
        else if( value.startsWith("SYNC_HOUR"))
        {
          if(value.length() > 6)
          {
            String tronc = value.substring(value.indexOf(" ", 0)+1, value.length());

            heureActuelle.heure = tronc.substring(0, tronc.indexOf(' ')).toInt();

            tronc = tronc.substring(tronc.indexOf(' ') + 1, tronc.length());

            heureActuelle.minute = tronc.substring(tronc.indexOf(' ') + 1, tronc.length()).toInt();

            Serial.print("Heure saisie : ");
            Serial.print(heureActuelle.heure);
            Serial.print(" : ");
            Serial.println(heureActuelle.minute);
          }

          Serial.println("SYNC_HOUR Atteint");

          String data = "RESP ";

          data += String(heureActuelle.heure) + " " + String(heureActuelle.minute);

          Serial.println(data.c_str());

          pCharacteristic->setValue(data.c_str());
          
        }
        else if( value.startsWith("SAVE"))
        {
          Serial.println("SAVE Atteint");
          
          String  str = value.substring(4, value.length());
          String strs[48];
          int StringCount = 0;

          // Split the string into substrings
          while (str.length() > 0)
          {
            int index = str.indexOf('/');
            if (index == -1) // No space found
            {
              strs[StringCount++] = str;
              break;
            }
            else
            {
              strs[StringCount++] = str.substring(0, index);
              str = str.substring(index+1);
            }
          }

          Sauvegarde_Brute_EEPROM(strs);
          Chargement_EEPROM();

          delay(400);

          for(int i = 0; i < NUM_LEDS; i ++)
          {
            strip.clear();
            strip.setBrightness(255);
            strip.setPixelColor(i, strip.Color(0,0, 255));
            strip.show();

            delay(50);
          }

          delay(400);

          PulseGreen();
        }
        else if (value.startsWith("AUTO"))
        {
          Serial.println("AUTO Atteint");
          estAuto = true;
          estConnecte = false;
        }
        else if (value.startsWith("LEAVE"))
        {
          estConnecte=false;
          estAuto = false;
          Serial.println("LEAVE Atteint");
          ESP.restart();
        }

        value = "";        
      };
    }

};

void InitConnection()
{
  Serial.println("Connection Bluetooth Initialisée !");
  // Bluetooth
  BLEDevice::init("MyESP32");

  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  heureActuelle.heure = 10;
  heureActuelle.minute = 25;

  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();

  pAdvertising->start();
}

// UTILITAIRE //////////////////////////////////////////////////////////////
void Decision(bool &mvt, int &lum, int * rouge, int * vert, int * bleu, int *intensite)
{
  // Priorité aux configurations
  for(int nConfig = 0; nConfig < 5; nConfig ++)
  {
    if(estDansIntervalle(Configs[nConfig].heureDebut, Configs[nConfig].heureFin))
    {
      Serial.print("Configuration ");
      Serial.print(nConfig+1);
      Serial.println(" utilisée !");

      *rouge = Configs[nConfig].rouge;
      *vert =  Configs[nConfig].vert;
      *bleu = Configs[nConfig].bleu;
      *intensite = (luminositeMax < Configs[nConfig].luminosite)? luminositeMax : Configs[nConfig].luminosite;
      return;
    }
  }

  // COnfiguration par défaut
  if(estDansIntervalle(heureDebutActivite, heureFinActivite))
  {
    Serial.println("Decision dans activité");
    if(lum < 200)
    {
      Serial.println("Configuration par défaut : " + String(configurationParDefaut));

      if(configurationParDefaut != 0)
      {
        Serial.println("Decision dans configuration custom");
        *rouge = Configs[configurationParDefaut-1].rouge;
        *vert = Configs[configurationParDefaut-1].vert;
        *bleu = Configs[configurationParDefaut-1].bleu;
        *intensite = Configs[configurationParDefaut-1].luminosite;
      }
      else
      {
        
        Serial.println("Decision OH PUTAIN MES YEUXXXXXXXXXXXXXXXXXXXXX");
        *rouge = 255;
        *vert = 255;
        *bleu = 255;
        *intensite = 255;
      }

    }
    else // Trop lumineux, éteint
    {
      Serial.println("Decision eteint");
      *rouge= 0;
      *vert = 0;
      *bleu = 0;
      *intensite = 0;
    }
  }
  else
  {
    Serial.println("Decision hors activité");

    if(estNocturne && mvt) // Lumiere rouge
    {
      Serial.println("Decision nocturne");
      *rouge = 255;
      *vert = 0;
      *bleu = 0;
      *intensite = luminositeMin + 20;
    }
    else // Eteint
    {
      Serial.println("Decision eteint");
      *rouge = 0;
      *vert = 0;
      *bleu = 0;
      *intensite = 0;
    }
  }

  // On adapte la luminosite en fonction des préférences globales
  if(*intensite != 0)
  {
    if(*intensite < luminositeMin)
    {
      *intensite = luminositeMin;
    }
    else if (*intensite > luminositeMax)
    {
      *intensite = luminositeMax;
    }

  }
}

bool estDansIntervalle(Heure &heureDebut, Heure &heureFin) 
{
    // Convertir les heures en minutes pour une comparaison facile
    int debutEnMinutes = heureDebut.heure * 60 +heureDebut.minute;
    int finEnMinutes = heureFin.heure * 60 + heureFin.minute;
    int actuelleEnMinutes = heureActuelle.heure * 60 + heureActuelle.minute;

    if (debutEnMinutes <= finEnMinutes) {
        // L'intervalle ne chevauche pas minuit
        return (actuelleEnMinutes >= debutEnMinutes) && (actuelleEnMinutes <= finEnMinutes);
    } else {
        // L'intervalle chevauche minuit
        return (actuelleEnMinutes >= debutEnMinutes) || (actuelleEnMinutes <= finEnMinutes);
    }
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

// Transitionne l'anneau vers les valeurs passées en paramètres
void TransitionVersCouleur(int rouge, int  vert, int bleu, int lum, int tempsDeTransition) 
{
  
  // Obtenir la couleur actuelle de la première LED (en supposant que toutes les LED ont la même couleur)
  uint32_t couleurActuelle = strip.getPixelColor(0);
  uint8_t actuelR = (couleurActuelle >> 16) & 0xFF;
  uint8_t actuelG = (couleurActuelle >> 8) & 0xFF;
  uint8_t actuelB = couleurActuelle & 0xFF;
  
  // Obtenir la luminosité actuelle
  int luminositeActuelle = strip.getBrightness();

  // Calculer les étapes nécessaires à la transition
  int etapes = tempsDeTransition / 10;
  float etapeR = (rouge - actuelR) / (float)etapes;
  float etapeG = (vert - actuelG) / (float)etapes;
  float etapeB = (bleu - actuelB) / (float)etapes;
  float etapeLuminosite = (lum - luminositeActuelle) / (float)etapes;

  for (int i = 0; i <= etapes; i++) {
    uint8_t r = actuelR + (etapeR * i);
    uint8_t g = actuelG + (etapeG * i);
    uint8_t b = actuelB + (etapeB * i);
    int luminosite = luminositeActuelle + (etapeLuminosite * i);

    // Ajuster les valeurs RGB en fonction de la luminosité
    r = r * luminosite / 255;
    g = g * luminosite / 255;
    b = b * luminosite / 255;
    
    for (int j = 0; j < NUM_LEDS; j++) {
      strip.setPixelColor(j, strip.Color(r, g, b));
    };

    strip.setBrightness(luminosite);

    strip.show();
    delay(10); // Délai pour créer l'effet de transition
  }
}

// PROGRAMME PRINCIPAL //////////////////////////////////////////////////

void setup()
{
  // Console
  Serial.begin(9600);

  // Anneau Led Initial
  pinMode(pinOutLampe, OUTPUT);
  strip.setBrightness(0);
  strip.begin();
  strip.fill(strip.Color(0,0,0));
  strip.show();

  // Capteurs
  pinMode(pinInLuminosite, INPUT);
  pinMode(pinInMouvement, INPUT);

  /////////////////////////////////////////////// SYNCHRO HEURE

  // Se fait à la suite de la connection bluetooth

  heureActuelle.heure = 0;
  heureActuelle.minute = 0;

  Serial.println("Début de l'exécution ...");
  
  // Configuration

  EEPROM.begin(EEPROM_SIZE); 

  Chargement_EEPROM();

  // Bluetooth
  InitConnection();

  estConnecte = false;
}

void loop()
{
  

  delay(1000);

  if(!estConnecte && !estAuto)
  {
    iter = 0;

    while(!estConnecte)
    {
      strip.clear();
      strip.setBrightness(255);
      strip.setPixelColor(iter%NUM_LEDS, strip.Color(255, 0, 0));
      strip.show();
      delay(50);

      iter++;
    }

    PulseGreen();

    iter = 0;
  }

  if(estAuto && iter >= 20)
  {
    // Reinitialiser le broadcast bluetooth toutes les minutes
    Serial.println("AUTO re-init bluetooth broadcast");
    InitConnection();
  }

  // Mode automatique
  
  // On récuppère les différentes données

  bool mouvement = digitalRead(pinInMouvement);
  int luminosite = analogRead(pinInLuminosite);

  Serial.println("Mouvement : " + String(mouvement));
  Serial.println("Luminosité : " + String(luminosite));

  // Valeurs retournées par la décision
  int rouge;
  int vert;
  int bleu;
  int intensite;

  Decision(mouvement, luminosite, &rouge, &vert, &bleu, &intensite ); // L'heure est une variable globale

  // Serial.println("Decision couleur : " + String(rouge) + " , " + String(vert) + " , " + String(bleu) );
  // Serial.println("Decision luminosite : " + String(intensite));

  // Utiliser une méthode pour lisser la transition
  TransitionVersCouleur(rouge, vert, bleu, intensite, 2000);

  iter ++;

  if(iter >= 20 ) // 20 défini par la valeur atteinte par iter au bout d'une minute d'exécution
  {
    Serial.print("Heure Actuelle : ");
    Serial.print(heureActuelle.heure);
    Serial.print(" : ");
    Serial.println(heureActuelle.minute);

    heureActuelle.minute ++;
    iter = 0;
  }

  if(heureActuelle.minute >= 60)
  {
    heureActuelle.minute = 0;
    heureActuelle.heure ++;

    if(heureActuelle.heure >= 23)
    {
      heureActuelle.heure = 0;
    }
  }
}

void PulseGreen()
{
  strip.clear();

  for(int i = 0; i < 25; i++)
  {
    strip.fill(strip.Color(0, 255, 0));
    strip.setBrightness(i * 10);
    strip.show();

    delay(50);
  }

  delay(50);

  strip.clear();

  for(int j = 0; j < 25; j++)
  {
    strip.fill(strip.Color(0, 255, 0));
    strip.setBrightness(255 - j * 10);
    strip.show();
    
    delay(50);
  }
}
