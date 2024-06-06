
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>

#define OLED_WIDTH 128 // OLED display width,  in pixels
#define OLED_HEIGHT 32 // OLED display height, in pixels

// Encodeur
#define pinInBoutton 2 // SW
#define pinInRotation 3 // DT
#define pinInSensRotation 4 // CLK

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

// Configuration menus
int delaiRafraichissement = 100;
int delaiPression = 200;

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

Encoder myEnc(pinInSensRotation, pinInRotation);

void setup() {
  Serial.begin(9600);

  Configs[0] = configuration1;
  Configs[1] = configuration2; 
  Configs[2] = configuration3; 
  Configs[3] = configuration4; 
  Configs[4] = configuration5; 

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  display.display();

  display.setTextColor(WHITE);
}

void loop() {

  if(LireBoutton())
  {
    delay(100);

    MenuPrincipal();
    
    display.clearDisplay();
    display.display();
  }
  else
  {
    // Mode automatique
  }

  delay(100);
}

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
        // Sauvegarde EEPROM
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

bool LireBoutton()
{
  bool ret = digitalRead(pinInBoutton);

  digitalWrite(pinInBoutton, HIGH);

  return !ret;
}