// Bluetooth

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Configuration
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
byte luminositeMax = 0;

Heure heureDebutActivite;
Heure heureFinActivite;

Configuration * Configs = new Configuration[5];

bool estNocturne;
byte configurationParDefaut;

Heure heureActuelle;

String valor_return = "";

// Configuration et sauvegardes //////////////////////////////////////////////////////

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
  for(int i = 0; i < EEPROM_SIZE-2 ; i++)
  {
    EEPROM.write(i, strs[i].toInt());
  }
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
  heureDebutActivite.heure = 0;
  heureDebutActivite.minute = 0;

  heureFinActivite.heure = 0;
  heureFinActivite.minute = 0;

  luminositeMax = 0;
  luminositeMin = 0;

  estNocturne = 1;
  configurationParDefaut = 0;

  for(int i = 0; i < 5; i++)
  {
    Heure heureDebut = {10+i, 0};
    Heure heureFin = {10+i, 50};
    Configs[i] = {heureDebut, heureFin, i * 10, i * 10,  i * 10, i * 10 };
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

class MyCallbacks : public BLECharacteristicCallbacks {

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

          data += String(heureDebutActivite.heure) + "/";
          data += String(heureDebutActivite.minute) + "/";
          data += String(heureFinActivite.heure) + "/";
          data += String(heureFinActivite.minute) + "/";
          data += String(luminositeMax) + "/";
          data += String(luminositeMin) + "/";

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
        }
        else if (value.startsWith("LEAVE"))
        {
          Serial.println("LEAVE Atteint");
          ESP.restart();
        }

        value = "";        
      };
    }

};

void setup() {
  Serial.begin(9600);

  // Configuration
  EEPROM.begin(EEPROM_SIZE); 
  Chargement_EEPROM();

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

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
}

