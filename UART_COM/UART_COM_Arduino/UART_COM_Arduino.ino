// Anneau LED
#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 16
#define BRIGHTNESS 40
#define pinOutLampe 7
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, pinOutLampe, NEO_GRB + NEO_KHZ800);

// Capteurs
#define pinInLuminosite 20
#define pinInMouvement 8
#define pinInESP 13
#define pinOutESP 12

// Encodeur
#include <Encoder.h>

#define pinInBoutton 2 // SW    (Bouton)
#define pinInRotation 3 // DT
#define pinInSensRotation 4 // CLK
int positionEncodeur = 0;
int derniereRotation;
int nouvelleRotation;
bool sensRotation;

Encoder myEnc(pinInSensRotation, pinInRotation);

// Ecran OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH 128 // OLED display width,  in pixels
#define OLED_HEIGHT 32 // OLED display height, in pixels

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

int delaiRafraichissement = 100;
int delaiPression = 70;

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

byte luminositeMin = 0;
byte luminositeMax = 0;

Heure heureDebutActivite;
Heure heureFinActivite;

Configuration * Configs = new Configuration[5];

bool estNocturne;
byte configurationParDefaut;

// Parametres globaux
Heure heureActuelle;
int iter = 0;

void setup()
{
    // Console
  Serial.begin(9600);

  // Anneau Led Initial
  strip.setBrightness(255);
  strip.begin();
  strip.clear();
  strip.show();

  // Capteurs
  pinMode(pinInLuminosite, INPUT);
  pinMode(pinInMouvement, INPUT);
  pinMode(pinInESP, INPUT);
  pinMode(pinOutESP, OUTPUT);

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

  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Reçoit l'heure au début du projet
  SynchronisationESP();

  Serial.print("Heure d'initialisation : ");
  Serial.print(heureActuelle.heure);
  Serial.print(" : ");
  Serial.println(heureActuelle.minute);

  Serial.println("Début de l'exécution ...");

}

void loop()
{
  delay(delaiPression);

  if(LireBoutton())
  {
    while(LireBoutton())
    {

      display.setCursor(20, 10); 
      display.println("Chargement ...");

      display.display();
      // On attend que l'utilisateur finisse l'appui du bouton avant de passer au menu 
    }

    // Mise en pause, accès au menu
    MenuPrincipal();
    
    display.clearDisplay();
    display.display();

    RecevoirHeure();
  }
  else
  {
    // Mode automatique
    
    // On récuppère les différentes données

    bool mouvement = digitalRead(pinInMouvement);
    int luminosite = analogRead(pinInLuminosite);
    if(iter >= 500) // Heure actualisée au bout d'un certain temps
    {
      // On actualise l'heure
      iter = 0;
      RecevoirHeure();
    }

    // Valeurs retournées par la décision
    uint32_t couleur;
    int intensite;

    Decision(mouvement, luminosite, &couleur, &intensite ); // L'heure est une variable globale

    // Utiliser une méthode pour lisser la transition
    TransitionVersCouleur(couleur, intensite, 2000);

    strip.setBrightness(intensite);

    iter++;
  }

}

void Decision(bool &mvt, int &lum, uint32_t *couleur, int *intensite)
{

  // Priorité aux configurations
  for(int nConfig = 0; nConfig < 5; nConfig ++)
  {
    if(estDansIntervalle(Configs[nConfig].heureDebut, Configs[nConfig].heureFin))
    {
      Serial.print("Configuration ");
      Serial.print(nConfig+1);
      Serial.println(" utilisée !");

      *couleur = strip.Color(Configs[nConfig].rouge, Configs[nConfig].vert, Configs[nConfig].bleu);
      *intensite = (luminositeMax < Configs[nConfig].luminosite)? luminositeMax : Configs[nConfig].luminosite;
      return;
    }
  }

  // COnfiguration par défaut
  if(estDansIntervalle(heureDebutActivite, heureFinActivite))
  {
    if(lum < 200)
    {
      if(configurationParDefaut != 0)
      {
        *couleur = strip.Color(Configs[configurationParDefaut-1].rouge,Configs[configurationParDefaut-1].vert, Configs[configurationParDefaut-1].bleu);
        *intensite = Configs[configurationParDefaut-1].luminosite;
      }
      else
      {
        *couleur = strip.Color(255, 255, 255);
        *intensite = luminositeMax;
      }

    }
    else // Trop lumineux, éteint
    {
      *couleur = strip.Color(0, 0, 0);
      *intensite = 0;
    }
  }
  else
  {
    if(estNocturne && mvt) // Lumiere rouge
    {
      *couleur = strip.Color(255, 0, 0);
      *intensite = (luminositeMin != 0)? luminositeMin : luminositeMax / 4;
    }
    else // Eteint
    {
      *couleur = strip.Color(0, 0, 0);
      *intensite = 0;
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
void TransitionVersCouleur(uint32_t nouvelleCouleur, int nouvelleLuminosite, int tempsDeTransition) 
{
  // Décomposer la nouvelle couleur en composants R, G et B
  uint8_t nouveauR = (nouvelleCouleur >> 16) & 0xFF;
  uint8_t nouveauG = (nouvelleCouleur >> 8) & 0xFF;
  uint8_t nouveauB = nouvelleCouleur & 0xFF;
  
  // Obtenir la couleur actuelle de la première LED (en supposant que toutes les LED ont la même couleur)
  uint32_t couleurActuelle = strip.getPixelColor(0);
  uint8_t actuelR = (couleurActuelle >> 16) & 0xFF;
  uint8_t actuelG = (couleurActuelle >> 8) & 0xFF;
  uint8_t actuelB = couleurActuelle & 0xFF;
  
  // Obtenir la luminosité actuelle
  int luminositeActuelle = strip.getBrightness();

  // Calculer les étapes nécessaires à la transition
  int etapes = tempsDeTransition / 10;
  float etapeR = (nouveauR - actuelR) / (float)etapes;
  float etapeG = (nouveauG - actuelG) / (float)etapes;
  float etapeB = (nouveauB - actuelB) / (float)etapes;
  float etapeLuminosite = (nouvelleLuminosite - luminositeActuelle) / (float)etapes;

  for (int i = 0; i <= etapes; i++) {
    uint8_t r = actuelR + (etapeR * i);
    uint8_t g = actuelG + (etapeG * i);
    uint8_t b = actuelB + (etapeB * i);
    int luminosite = luminositeActuelle + (etapeLuminosite * i);

    // Ajuster les valeurs RGB en fonction de la luminosité
    r = r * luminosite / 255;
    g = g * luminosite / 255;
    b = b * luminosite / 255;
    
    uint32_t couleur = strip.Color(r, g, b);
    
    for (int j = 0; j < NUM_LEDS; j++) {
      strip.setPixelColor(j, couleur);
    }
    strip.show();
    delay(10); // Délai pour créer l'effet de transition
  }
}

// Lis l'état d'un bouton avec un port digital
bool LireBoutton()
{
  bool ret = digitalRead(pinInBoutton);

  digitalWrite(pinInBoutton, HIGH);
  return !ret;
}

// Ecoute un port en particulier et lis puis transcrit une série de 8 bits émis sur ce port 
// après un front montant
int RecevoirByte(int pin)
{
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

  return num;
}

void RecevoirHeure()
{
  digitalWrite(pinOutESP, HIGH);

  Serial.println("Envoi de la demande");

  while(digitalRead(pinInESP) != 1)
  {
    delay(10);
    if(digitalRead(pinInESP) == 1)
    {
      heureActuelle.heure = RecevoirByte(pinInESP);
      heureActuelle.minute = RecevoirByte(pinInESP);
    }
  }

  digitalWrite(pinOutESP, LOW);

  Serial.print("Heure reçue : ");
  Serial.println(heureActuelle.heure);
  Serial.print("Minute reçue : ");
  Serial.println(heureActuelle.minute);
}

bool SynchronisationESP()
{
  bool connecte = false;

  int i = 0;
  int moveLed = 0;

  while(!connecte)
  {

    digitalWrite(pinOutESP, HIGH);

    Serial.println("Envoi de la demande");

    while(digitalRead(pinInESP) != 1)
    {
      // Bouget la led toutes les 20 demandes
      if(i > 20)
      {
        i = 0;
        moveLed++;
      }

      strip.clear();
      strip.setPixelColor(moveLed%NUM_LEDS, strip.Color(255, 0, 0));
      strip.show();

      delay(10);

      if(digitalRead(pinInESP) == 1)
      {
        heureActuelle.heure = RecevoirByte(pinInESP);
        heureActuelle.minute = RecevoirByte(pinInESP);
      }
      i++;
    }

    digitalWrite(pinOutESP, LOW);

    Serial.print("Heure reçue : ");
    Serial.println(heureActuelle.heure);
    Serial.print("Minute reçue : ");
    Serial.println(heureActuelle.minute);

    connecte = (
        heureActuelle.heure < 24 && 
        heureActuelle.heure >= 0 && 
        heureActuelle.minute < 60 &&
        heureActuelle.minute >= 0);
  }

  strip.clear();
  strip.show();
}

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
  Serial.println("Sauvegrade des données dans l'EEPROM...");

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
  for(int i = 0; i < 5; i++)
  {
    EEPROM.put(6 + 8 * i, Configs[i]);
  }

  // Configuration par défaut & autres
  EEPROM.update(4 + 2 + 8 * 5, estNocturne);
  EEPROM.update(4 + 2 + 8 * 5 + 1, configurationParDefaut);

  Serial.println("Données sauvegardées !");
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

// Affichage OLED ///////////////////////////////////////////////////////////////

byte AffichageSelectionAvecSurlignage(String nom, int min, int max, int ligne, int colonneSelection, int valeurInitiale)
{
  byte valeurSelectionnee = valeurInitiale;

  Serial.println("> [+] Debut de selection " + nom);     
  display.setTextColor(BLACK);

  // Selection de l'heure
  while(1)
  {
    delay(delaiPression);

    // Surlignage en blanc de la zone de saisie
    display.fillRect(colonneSelection - 4, ligne * 10, 40 , 10, WHITE);

    // Ecrire la nouvelle valeur
    display.setCursor(colonneSelection, ligne * 10); 
    display.println(valeurSelectionnee);
    valeurSelectionnee = SelectionMenu(myEnc.read(), max, min);    

    if(LireBoutton())
    {
      Serial.println("> [-] Fin de selection " + nom);
      display.setTextColor(WHITE); // On remet la couleur du texte en blanc avant de quitter
      return valeurSelectionnee;
    }

    display.display();
  }
}

void AffichageSelectionHeure(byte * heure, byte * minute)
{
  delay(300);

  Serial.println("[+] Selection Heure");

  int selection = 0;
  int estSelectionne = 0;
  
  byte heureSaisie = *heure;
  byte minuteSaisie = *minute; 

  while(1)
  {
    delay(delaiRafraichissement);

    selection = SelectionMenu(myEnc.read(), 3, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    display.setCursor(20, 0); 
    display.println("Heure");

    display.setCursor(60, 0); 
    display.println(heureSaisie);

    display.setCursor(20, 10); 
    display.println("Minute");

    display.setCursor(60, 10); 
    display.println(minuteSaisie);

    display.setCursor(20, 20); 
    display.println("Sortie");

    display.setCursor(2, selection * 10); 
    display.println(">");

    if(estSelectionne)
    {
      if(selection == 0)
      { 
        heureSaisie = AffichageSelectionAvecSurlignage("Heure", 0, 24, selection, 60, heureSaisie);
      }
      else if(selection == 1)
      {
        minuteSaisie = AffichageSelectionAvecSurlignage("Minute", 0, 60, selection, 60, minuteSaisie);
      }
      else
      {
        Serial.println("[-] Selection Heure");
        *heure = heureSaisie;
        *minute = minuteSaisie;
        break;
      }
    }

    display.display();
  }
}

void AffichageSelectionCouleur(byte * rouge, byte * vert, byte * bleu)
{
  delay(300);

  Serial.println("[+] Selection Couleur");

  int selection = 0;
  int estSelectionne = 0;

  byte rougeSaisie = *rouge;
  byte vertSaisie = *vert; 
  byte bleuSaisie = *bleu; 

  while(1)
  {
    delay(delaiRafraichissement);

    selection = SelectionMenu(myEnc.read(), 4, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    // Affichage premiere section
    if(selection < 3)
    {
      display.setCursor(20, 0); 
      display.println("Rouge");

      display.setCursor(60, 0); 
      display.println(rougeSaisie);

      display.setCursor(20, 10); 
      display.println("Vert");

      display.setCursor(60, 10); 
      display.println(vertSaisie);

      display.setCursor(20, 20); 
      display.println("Bleu");

      display.setCursor(60, 20); 
      display.println(bleuSaisie);

      display.setCursor(2, selection * 10); 
      display.println(">");
    }
    // Affichage deuxieme section
    else
    {
      display.setCursor(20, 0); 
      display.println("Sortie");

      display.setCursor(2, (selection - 3) * 10); 
      display.println(">");
    }

    if(estSelectionne)
    {
      if(selection == 0)
      {  
        rougeSaisie = AffichageSelectionAvecSurlignage("Rouge", 0, 256, selection, 60, rougeSaisie);
      }
      else if(selection == 1)
      {
        vertSaisie = AffichageSelectionAvecSurlignage("Vert", 0, 256, selection, 60, vertSaisie);
      }
      else if(selection == 2)
      {
        bleuSaisie = AffichageSelectionAvecSurlignage("Bleu", 0, 256, selection, 60, bleuSaisie);
      }
      else
      {
        Serial.println("[-] Selection Couleur");
        *rouge = rougeSaisie;
        *vert = vertSaisie;
        *bleu = bleuSaisie;
        break;
      }
    }

    display.display();
  }

}

void AffichageSelectionLuminosite(byte * luminosite)
{
  delay(300);

  Serial.println("[+] Selection Luminosite");

  int selection = 0;
  int estSelectionne = 0;

  byte luminositeSaisie = *luminosite;

  while(1)
  {
    delay(delaiRafraichissement);

    selection = SelectionMenu(myEnc.read(), 2, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    display.setCursor(20, 0); 
    display.println("Luminosite");

    display.setCursor(90, 0); 
    display.println(luminositeSaisie);

    display.setCursor(20, 10); 
    display.println("Sortie");

    display.setCursor(2, selection * 10); 
    display.println(">");

    if(estSelectionne)
    {

      if(selection == 0)
      { 
        luminositeSaisie = AffichageSelectionAvecSurlignage("Luminosité", 0, 256, selection, 90, luminositeSaisie);
      }
      else
      {
        Serial.println("[-] Selection Luminosite");
        *luminosite = luminositeSaisie;
        break;
      }
    }

    display.display();
  }

}

void AffichageSelectionActivite()
{
  Serial.println("[+] Menu Activite");

  int selection = 0;
  int estSelectionne = 0;

  while(1)
  {
    delay(delaiRafraichissement);

    selection = SelectionMenu(myEnc.read(), 3, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    /////////: Heure Debut
    display.setCursor(20, 0); 
    display.println("Debut");

    display.setCursor(60, 0); 
    display.println(heureDebutActivite.heure);

    display.setCursor(70, 0); 
    display.println(":");

    display.setCursor(80, 0); 
    display.println(heureDebutActivite.minute);

    //////// Heure Fin
    display.setCursor(20, 10); 
    display.println("Fin");

    display.setCursor(60, 10); 
    display.println(heureFinActivite.heure);

    display.setCursor(70, 10); 
    display.println(":");

    display.setCursor(80, 10); 
    display.println(heureFinActivite.minute);

    //////// Sortie
    display.setCursor(20, 20); 
    display.println("Sortie");

    display.setCursor(2, selection * 10); 
    display.println(">");

    if(estSelectionne)
    {
      if(selection == 0)
      { 
        AffichageSelectionHeure(&heureDebutActivite.heure, &heureDebutActivite.minute);
      }
      else if(selection == 1)
      {
        AffichageSelectionHeure(&heureFinActivite.heure, &heureFinActivite.minute);
      }
      else
      {
        Serial.println("[-] Menu Activite");
        break;
      }

    }

    display.display();
  }
}

void AffichageSelectionIntensite()
{
  Serial.println("[+] Menu Intensite");
  int selection = 0;
  int estSelectionne = 0;

  while(1)
  {
    delay(delaiRafraichissement);

    selection = SelectionMenu(myEnc.read(), 3, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    //////// Lum Max
    display.setCursor(20, 0); 
    display.println("Max");

    display.setCursor(60, 0); 
    display.println(luminositeMax);

    //////// Lum Min
    display.setCursor(20, 10); 
    display.println("Min");

    display.setCursor(60, 10); 
    display.println(luminositeMin);

    //////// Sortie
    display.setCursor(20, 20); 
    display.println("Sortie");

    display.setCursor(2, selection * 10); 
    display.println(">");

    if(estSelectionne)
    {
      if(selection == 0)
      { 
        AffichageSelectionLuminosite(&luminositeMax);
      }
      else if(selection == 1)
      {
        AffichageSelectionLuminosite(&luminositeMin);

        // On a impérativement min <= max donc régualarisation si nécessaire
        if(luminositeMin > luminositeMax)
        {
          luminositeMin = luminositeMax;
        }
      }
      else
      {
        Serial.println("[-] Menu Intensite");
        break;
      }
    }

    display.display();
  }

}

void AffichageSelectionConfiguration(int nbConfiguration)
{
  Serial.print("[+] Menu Configuration ");
  Serial.println(nbConfiguration + 1);

  int selection = 0;
  int estSelectionne = 0;

  while(1)
  {
    delay(delaiRafraichissement);

    selection = SelectionMenu(myEnc.read(), 5, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    // Affichage premiere section
    if(selection < 3)
    {
      /////////: Heure Debut
      display.setCursor(20, 0); 
      display.println("Debut");

      display.setCursor(60, 0); 
      display.println(Configs[nbConfiguration].heureDebut.heure);

      display.setCursor(70, 0); 
      display.println(":");

      display.setCursor(80, 0); 
      display.println(Configs[nbConfiguration].heureDebut.minute);

      //////// Heure Fin
      display.setCursor(20, 10); 
      display.println("Fin");

      display.setCursor(60, 10); 
      display.println(Configs[nbConfiguration].heureFin.heure);

      display.setCursor(70, 10); 
      display.println(":");

      display.setCursor(80, 10); 
      display.println(Configs[nbConfiguration].heureFin.minute);

      ///////// Luminosite
      display.setCursor(20, 20); 
      display.println("Luminosite");

      display.setCursor(90, 20); 
      display.println(Configs[nbConfiguration].luminosite);

      display.setCursor(2, selection * 10); 
      display.println(">");
    }

    // Affichage deuxieme section
    else
    {
      /////// Couleur
      display.setCursor(20, 0); 
      display.println("Col.");

      display.setCursor(45, 0); 
      display.println("(");

      display.setCursor(50, 0); 
      display.println(Configs[nbConfiguration].rouge);

      display.setCursor(65, 0); 
      display.println(",");

      display.setCursor(70, 0); 
      display.println(Configs[nbConfiguration].vert);

      display.setCursor(85, 0); 
      display.println(",");

      display.setCursor(90, 0); 
      display.println(Configs[nbConfiguration].bleu);

      display.setCursor(105, 0); 
      display.println(")");

      /// Sortie
      display.setCursor(20, 10); 
      display.println("Sortie");

      display.setCursor(2, (selection-3) * 10); 
      display.println(">");
    }

    if(estSelectionne)
    {
      if(selection == 0)
      { 
        AffichageSelectionHeure(&Configs[nbConfiguration].heureDebut.heure, &Configs[nbConfiguration].heureDebut.minute);
      }
      else if(selection == 1)
      {
        AffichageSelectionHeure(&Configs[nbConfiguration].heureFin.heure, &Configs[nbConfiguration].heureFin.minute);
      }
      else if(selection == 2)
      {
        AffichageSelectionLuminosite(&Configs[nbConfiguration].luminosite);
      }
      else if(selection == 3)
      {
        AffichageSelectionCouleur(&Configs[nbConfiguration].rouge, &Configs[nbConfiguration].vert, &Configs[nbConfiguration].bleu);
        
      }
      else
      {
        Serial.print("[+] Menu Configuration ");
        Serial.println(nbConfiguration + 1);
        break;
      }

      selection = 0;
    }

    display.display();
  }
}

void MenuSelectionConfiguration()
{
  Serial.println("[+] Menu Selection Configuration");
  
  int selection = 0;
  int estSelectionne = 0;

  while(1)
  {
    delay(delaiRafraichissement);

    selection = SelectionMenu(myEnc.read(), 6, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    // Affichage premiere section
    if(selection < 3)
    {
      display.setCursor(20, 0); 
      display.println("Configuration 1");

      display.setCursor(20, 10); 
      display.println("Configuration 2");

      display.setCursor(20, 20); 
      display.println("Configuration 3");

      display.setCursor(2, selection * 10); 
      display.println(">");
    }
    // Affichage deuxieme section
    else
    {
      display.setCursor(20, 0); 
      display.println("Configuration 4");

      display.setCursor(20, 10); 
      display.println("Configuration 5");

      display.setCursor(20, 20); 
      display.println("Sortie");

      display.setCursor(2, (selection - 3) * 10); 
      display.println(">");
    }

    // Acces aux sous menus
    if(estSelectionne)
    {
      // Attention le numéro des configurations commencent à partir de 0 !!
      if(selection == 0)
      {
        AffichageSelectionConfiguration(0);
      }
      else if(selection == 1)
      {
        AffichageSelectionConfiguration(1);
      }
      else if(selection == 2)
      {
        AffichageSelectionConfiguration(2);
      }
      else if(selection == 3)
      {
        AffichageSelectionConfiguration(3);
      }
      else if(selection == 4)
      {
        AffichageSelectionConfiguration(4);
      }
      else
      {
        Serial.println("[-] Menu Selection Configuration");
        break;
      }
    }

    display.display();
  }
}

void MenuActualisationHeure()
{
  Serial.println("[+] Menu Actualisation Heure");
  
  int selection = 1;
  int estSelectionne = 0;

  while(1)
  {
    delay(delaiRafraichissement);

    selection = SelectionMenu(myEnc.read(), 3, 1);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    display.setCursor(34, 0); 
    display.println(heureActuelle.heure);

    display.setCursor(64, 0); 
    display.println(":");

    display.setCursor(94, 0); 
    display.println(heureActuelle.minute);

    display.setCursor(20, 10); 
    display.println("Actualiser");

    display.setCursor(20, 20); 
    display.println("Sortie");

    display.setCursor(2, selection * 10); 
    display.println(">");

    // Acces aux sous menus
    if(estSelectionne)
    {
      if (selection == 0)
      {
        // Affichage Heure
      }
      else if (selection == 1)
      {
        SynchronisationESP();
      }
      else
      {
        Serial.println("[-] Menu Actualisation Heure");
        break;
      }
    }

    display.display();
  }
}

void MenuPrincipal()
{
  Serial.println("[+] Menu Principal");
  
  int selection = 0;
  int estSelectionne = 0;

  while(1)
  {
    delay(delaiRafraichissement);

    selection = SelectionMenu(myEnc.read(), 6, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    // Affichage premiere section
    if(selection < 3)
    {
      display.setCursor(20, 0); 
      display.println("Activite");

      display.setCursor(20, 10); 
      display.println("Luminosite");

      display.setCursor(20, 20); 
      display.println("Configurations");

      display.setCursor(2, selection * 10); 
      display.println(">");
    }
    // Affichage deuxieme section
    else
    {
      display.setCursor(20, 0); 
      display.println("Defaut");

      display.setCursor(20, 10); 
      display.println("Synchronisation");

      display.setCursor(20, 20); 
      display.println("Sortie");

      display.setCursor(2, (selection - 3) * 10); 
      display.println(">");
    }

    // Acces aux sous menus
    if(estSelectionne)
    {
      if(selection == 0)
      {
        AffichageSelectionActivite();
      }
      else if (selection == 1)
      {
        AffichageSelectionIntensite();
      }
      // Attention le numéro des configurations commencent à partir de 0 !!
      else if(selection == 2)
      {
        MenuSelectionConfiguration();
      }
      else if(selection == 3)
      {
        AffichageSelectionDefaut();
      }
      else if(selection == 4)
      {
        MenuActualisationHeure();
      }
      else
      {
        Sauvegarde_EEPROM();
        Serial.println("[-] Menu Principal");
        break;
      }
    }

    display.display();
  }
}

void AffichageSelectionDefaut()
{
  delay(300);

  Serial.println("[+] Selection Defaut");

  int selection = 0;
  int estSelectionne = 0;

  while(1)
  {
    delay(delaiRafraichissement);

    selection = SelectionMenu(myEnc.read(), 3, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    display.setCursor(20, 0); 
    display.println("Nocturne");

    display.setCursor(80, 0); 
    display.println(estNocturne);

    display.setCursor(20, 10); 
    display.println("Defaut");

    display.setCursor(80, 10); 
    display.println(configurationParDefaut);

    display.setCursor(20, 20); 
    display.println("Sortie");

    display.setCursor(2, selection * 10); 
    display.println(">");

    if(estSelectionne)
    {
      if(selection == 0)
      { 
        estNocturne = AffichageSelectionAvecSurlignage("Nocturne", 0, 2, selection, 80, estNocturne);
      }
      else if(selection == 1)
      {
        configurationParDefaut = AffichageSelectionAvecSurlignage("Defaut", 0, 6, selection, 80, configurationParDefaut);
      }
      else
      {
        Serial.println("[-] Selection Defaut");
        break;
      }
    }

    display.display();
  }
}

int SelectionMenu(int val, int taille, int min)
{
  int val2 = 0;
  int menu_index = min;

  if(val != val2)
  {
    // La valeur à été modifiée (+ ou -) on met a jour l'index
    menu_index -= (val - val2)/4;
    val2 = val;
  }

  // Gere les différences modulos positifs / négatifs (fait en sorte que les valeurs bouclent)
  if(menu_index < min)
  {
    if(menu_index%taille == min)
    {
      menu_index = min;
    }
    else
    {
      menu_index = taille + menu_index%taille;
    }
  }
  else
  {
    menu_index = menu_index%taille;
  }

  return menu_index;
}

/*
  Configuration rapide dans l'EEPROM
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
*/