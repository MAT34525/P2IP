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

int delaiRafraichissement = 200;
int delaiPression = 50;

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

Configuration configuration1;
Configuration configuration2;
Configuration configuration3;
Configuration configuration4;
Configuration configuration5;

Configuration *  Configs = new Configuration[5];

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

  Configs[0] = configuration1;
  Configs[1] = configuration2; 
  Configs[2] = configuration3; 
  Configs[3] = configuration4; 
  Configs[4] = configuration5; 

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
    ConfigurerAnneau(couleur);

    strip.setBrightness(intensite);

    iter++;

  }
}

void Decision(bool &mvt, int &lum, uint32_t *couleur, int *intensite)
{
  // Autres parametres
  bool nocturne = true;

  // Priorité aux configurations
  for(int nConfig = 0; nConfig < 5; nConfig ++)
  {
    if(estDansIntervalle(Configs[nConfig].heureDebut, Configs[nConfig].heureFin))
    {
      *couleur = strip.Color(Configs[nConfig].rouge, Configs[nConfig].vert, Configs[nConfig].bleu);
      *intensite = (luminositeMax < Configs[nConfig].luminosite)? luminositeMax : Configs[nConfig].luminosite;
      return;
    }
  }

  // COnfiguration par défaut
  if(estDansIntervalle(heureDebutActivite, heureFinActivite))
  {
    

    if(lum < 300)
    {
      *couleur = strip.Color(255, 255, 255);
      *intensite = luminositeMax;
    }
    else if (lum < 700)
    {
      *couleur = strip.Color(255, 255, 255);

      int step = (luminositeMax - (700 - 300)) /1024; // A verifier, transition de 0 a 255 selon échelon

      *intensite = round(luminositeMax * step) ;
    }
    else // Trop lumineux, éteint
    {
      *couleur = strip.Color(0, 0, 0);
      *intensite = 0;
    }

  }
  else
  {
    if(nocturne && mvt) // Lumiere rouge
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

// Modifie la configuration de l'anneau LED avec un port digital
bool ConfigurerAnneau(uint32_t couleur)
{
  // Couleur : strip.Color(rouge, vert, bleu, luminosite)

  for (uint16_t i = 0; i < strip.numPixels(); i++) {
      
      strip.setPixelColor(i, couleur);  
  }

  strip.show();
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

  strip.clear();
  for (uint16_t i = 0; i < strip.numPixels(); i++) 
  {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }

  strip.show();

  while(!connecte)
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

// Affichage OLED ///////////////////////////////////////////////////////////////

byte AffichageSelectionAvecSurlignage(String nom, int min, int max, int ligne, int colonneSelection)
{
  byte valeurSelectionnee = min;

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
  Serial.println("[+] Selection Heure");

  int selection = 0;
  int estSelectionne = 0;
  
  byte heureSaisie = 0;
  byte minuteSaisie = 0; 

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
        heureSaisie = AffichageSelectionAvecSurlignage("Heure", 0, 24, selection, 60);
      }
      else if(selection == 1)
      {
        minuteSaisie = AffichageSelectionAvecSurlignage("Minute", 0, 60, selection, 60);
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
  Serial.println("[+] Selection Couleur");

  int selection = 0;
  int estSelectionne = 0;

  byte rougeSaisie = 0;
  byte vertSaisie = 0; 
  byte bleuSaisie = 0; 

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
        rougeSaisie = AffichageSelectionAvecSurlignage("Rouge", 0, 256, selection, 60);
      }
      else if(selection == 1)
      {
        vertSaisie = AffichageSelectionAvecSurlignage("Vert", 0, 256, selection, 60);
      }
      else if(selection == 2)
      {
        bleuSaisie = AffichageSelectionAvecSurlignage("Bleu", 0, 256, selection, 60);
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
  Serial.println("[+] Selection Luminosite");

  int selection = 0;
  int estSelectionne = 0;

  byte luminositeSaisie = 0;

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
        luminositeSaisie = AffichageSelectionAvecSurlignage("Luminosité", 0, 256, selection, 90);
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
  Serial.println(nbConfiguration);

  int selection = 0;
  int estSelectionne = 0;

  byte heureDebut = 0;
  byte minuteDebut = 0;
  byte heureFin = 0;
  byte minuteFin = 0;

  byte intensite = 0;

  byte rougeSaisie = 0;
  byte vertSaisie = 0;
  byte bleuSaisie = 0; 

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
        AffichageSelectionHeure(&heureDebut, &minuteDebut);
      }
      else if(selection == 1)
      {
        AffichageSelectionHeure(&heureFin, &minuteFin);
      }
      else if(selection == 2)
      {
        AffichageSelectionLuminosite(&intensite);
      }
      else if(selection == 3)
      {
        AffichageSelectionCouleur(&rougeSaisie, &vertSaisie, &bleuSaisie);
        
      }
      else
      {
        // Mise a jour de la configuration associée
        Configs[nbConfiguration].heureDebut.heure = heureDebut;
        Configs[nbConfiguration].heureDebut.minute = minuteDebut;
        Configs[nbConfiguration].heureFin.heure = heureDebut;
        Configs[nbConfiguration].heureFin.minute = minuteDebut;
        Configs[nbConfiguration].luminosite = intensite;
        Configs[nbConfiguration].rouge = rougeSaisie;
        Configs[nbConfiguration].vert = vertSaisie;
        Configs[nbConfiguration].bleu = bleuSaisie;

        Serial.print("[+] Menu Configuration ");
        Serial.println(nbConfiguration);
        break;
      }

      selection = 0;
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

    selection = SelectionMenu(myEnc.read(), 8, 0);
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
      display.println("Configuration 1");

      display.setCursor(2, selection * 10); 
      display.println(">");
    }
    // Affichage deuxieme section
    else if(selection < 6 && selection >= 3)
    {
      display.setCursor(20, 0); 
      display.println("Configuration 2");

      display.setCursor(20, 10); 
      display.println("Configuration 3");

      display.setCursor(20, 20); 
      display.println("Configuration 4");

      display.setCursor(2, (selection - 3) * 10); 
      display.println(">");
    }
    // Affichage troisieme section
    else
    {
      display.setCursor(20, 0); 
      display.println("Configuration 5");

      display.setCursor(20, 10); 
      display.println("Sortie");

      display.setCursor(2, (selection - 6) * 10); 
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
      else if(selection == 2)
      {
        AffichageSelectionConfiguration(1);
      }
      else if(selection == 3)
      {
        AffichageSelectionConfiguration(2);
      }
      else if(selection == 4)
      {
        AffichageSelectionConfiguration(3);
      }
      else if(selection == 5)
      {
        AffichageSelectionConfiguration(4);
      }
      else if(selection == 6)
      {
        AffichageSelectionConfiguration(5);
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