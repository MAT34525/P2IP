
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

int luminositeMin = 0;
int luminositeMax = 0;

Heure heureDebutActivite;
Heure heureFinActivite;

Configuration configuration1;
Configuration configuration2;
Configuration configuration3;
Configuration configuration4;
Configuration configuration5;

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

Encoder myEnc(pinInSensRotation, pinInRotation);

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE);
}

void loop() {

  Serial.println("Yeet");

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

void AffichageSelectionHeure(int * heure, int * minute)
{
  int selection = 0;
  int estSelectionne = 0;
  int heureSaisie = 0;
  int minuteSaisie = 0; 

  delay(100);

  while(1)
  {
    selection = SelectionMenu(myEnc.read(), 3, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);

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
        Serial.println("Saisie Heure");       

        // Selection de l'heure
        while(1)
        {
          delay(50);

          // Effacer l'ancienne valeur
          display.fillRect(56, 0, 40 , 10, WHITE);

          // Ecrire la nouvelle valeur
          display.setTextColor(BLACK);
          display.setCursor(60, 0); 
          display.println(heureSaisie);
          heureSaisie = SelectionMenu(myEnc.read(), 24, 0);

          display.display();

          if(LireBoutton())
          {
            Serial.println("Sortir Saisie Heure");
            break;
          }
        }
      }
      else if(selection == 1)
      {
        Serial.println("Saisie Minutes");

        // Selection de l'heure
        while(1)
        {
          delay(50);

          // Effacer l'ancienne valeur
          display.fillRect(56, 10,  40, 10, WHITE);

          // Ecrire la nouvelle valeur
          display.setTextColor(BLACK);
          display.setCursor(60, 10); 
          display.println(minuteSaisie);
          minuteSaisie = SelectionMenu(myEnc.read(), 60, 0);

          display.display();

          if(LireBoutton())
          {
            Serial.println("Sortir Saisie Minutes");
            break;
          }
        }
      }
      else
      {
        *heure = heureSaisie;
        *minute = minuteSaisie;
        return;
      }

    }

    display.display();

    delay(100);
  }

}

void AffichageSelectionCouleur(int * rouge, int * vert, int * bleu)
{
  int selection = 0;
  int estSelectionne = 0;

  int rougeSaisie = 0;
  int vertSaisie = 0; 
  int bleuSaisie = 0; 

   delay(100);

  while(1)
  {
    selection = SelectionMenu(myEnc.read(), 4, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);

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

    display.setCursor(2, selection * 10); 
    display.println(">");

    if(estSelectionne)
    {

      if(selection == 0)
      { 
        delay(100);

        Serial.println("Saisie Rouge");       

        // Selection de l'heure
        while(1)
        {
          delay(50);

          // Effacer l'ancienne valeur
          display.fillRect(56, 0, 40 , 10, WHITE);

          // Ecrire la nouvelle valeur
          display.setTextColor(BLACK);
          display.setCursor(60, 0); 
          display.println(rougeSaisie);
          rougeSaisie = SelectionMenu(myEnc.read(), 256, 0);

          display.display();

          if(LireBoutton())
          {
            Serial.println("Sortir Saisie Rouge");
            break;
          }
        }
      }
      else if(selection == 1)
      {
        Serial.println("Saisie Vert");

        delay(100);

        // Selection de l'heure
        while(1)
        {
          delay(50);

          // Effacer l'ancienne valeur
          display.fillRect(56, 10,  40, 10, WHITE);

          // Ecrire la nouvelle valeur
          display.setTextColor(BLACK);
          display.setCursor(60, 10); 
          display.println(vertSaisie);
          vertSaisie = SelectionMenu(myEnc.read(), 256, 0);

          display.display();

          if(LireBoutton())
          {
            Serial.println("Sortir Saisie Vert");
            break;
          }
        }
      }
      else if(selection == 2)
      {
        delay(100);

        Serial.println("Saisie Bleu");

        // Selection de l'heure
        while(1)
        {
          delay(50);

          // Effacer l'ancienne valeur
          display.fillRect(56, 20,  40, 10, WHITE);

          // Ecrire la nouvelle valeur
          display.setTextColor(BLACK);
          display.setCursor(60, 20); 
          display.println(bleuSaisie);
          bleuSaisie = SelectionMenu(myEnc.read(), 256, 0);

          display.display();

          if(LireBoutton())
          {
            Serial.println("Sortir Saisie Bleu");
            break;
          }
        }
      }
      else
      {
        Serial.println("Sortie");
        *rouge = rougeSaisie;
        *vert = vertSaisie;
        *bleu = bleuSaisie;
        return;
      }

      selection = 0;

    }

    display.display();

    delay(100);
  }

}

void AffichageSelectionLuminosite(int * luminosite)
{
  int selection = 0;
  int estSelectionne = 0;

  int luminositeSaisie = 0;

   delay(100);

  while(1)
  {
    selection = SelectionMenu(myEnc.read(), 2, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);

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
        Serial.println("Saisie Luminosite");       

        // Selection de l'heure
        while(1)
        {
          delay(50);

          // Effacer l'ancienne valeur
          display.fillRect(86, 0, 40 , 10, WHITE);

          // Ecrire la nouvelle valeur
          display.setTextColor(BLACK);
          display.setCursor(90, 0); 
          display.println(luminositeSaisie);
          luminositeSaisie = SelectionMenu(myEnc.read(), 256, 0);

          display.display();

          if(LireBoutton())
          {
            Serial.println("Sortir Saisie Luminosite");
            break;
          }
        }
      }
      else
      {
        *luminosite = luminositeSaisie;
        return;
      }

    }

    display.display();

    delay(100);
  }

}

void AffichageSelectionActivite()
{
  int selection = 0;
  int estSelectionne = 0;
  
  int heureDebut = 0;
  int minuteDebut = 0;
  int heureFin = 0;
  int minuteFin = 0;

  delay(100);

  while(1)
  {
    selection = SelectionMenu(myEnc.read(), 3, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);

    /////////: Heure Debut
    display.setCursor(20, 0); 
    display.println("Debut");

    display.setCursor(60, 0); 
    display.println(heureDebut);

    display.setCursor(70, 0); 
    display.println(":");

    display.setCursor(80, 0); 
    display.println(minuteDebut);

    //////// Heure Fin
    display.setCursor(20, 10); 
    display.println("Fin");

    display.setCursor(60, 10); 
    display.println(heureFin);

    display.setCursor(70, 10); 
    display.println(":");

    display.setCursor(80, 10); 
    display.println(minuteFin);

    //////// Sortie
    display.setCursor(20, 20); 
    display.println("Sortie");

    display.setCursor(2, selection * 10); 
    display.println(">");

    if(estSelectionne)
    {

      if(selection == 0)
      { 
        Serial.println("Saisie Debut");

        AffichageSelectionHeure(&heureDebut, &minuteDebut);
        
      }
      else if(selection == 1)
      {
        Serial.println("Saisie Fin");

        AffichageSelectionHeure(&heureFin, &minuteFin);
      }
      else
      {
        return;
      }

    }

    display.display();

    delay(100);
  }
}

void AffichageSelectionIntensite()
{
  int selection = 0;
  int estSelectionne = 0;
  
  int intensiteMax = 0;
  int intensiteMin = 0;

  delay(100);

  while(1)
  {
    selection = SelectionMenu(myEnc.read(), 3, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);

    /////////: Lum Max
    display.setCursor(20, 0); 
    display.println("Max");

    display.setCursor(60, 0); 
    display.println(intensiteMax);

    //////// Lum Min
    display.setCursor(20, 10); 
    display.println("Min");

    display.setCursor(60, 10); 
    display.println(intensiteMin);

    //////// Sortie
    display.setCursor(20, 20); 
    display.println("Sortie");

    display.setCursor(2, selection * 10); 
    display.println(">");

    if(estSelectionne)
    {

      if(selection == 0)
      { 
        Serial.println("Saisie Debut");

        AffichageSelectionLuminosite(&intensiteMax);
        
      }
      else if(selection == 1)
      {
        Serial.println("Saisie Fin");

        AffichageSelectionLuminosite(&intensiteMin);

        // On a impérativement min <= max donc régualarisation si nécessaire
        if(intensiteMin > intensiteMax)
        {
          intensiteMin = intensiteMax;
        }
      }
      else
      {
        return;
      }

    }

    display.display();

    delay(100);
  }

}

// Peut etre retrourner un Strcut de configuration
void AffichageSelectionConfiguration(int nbConfiguration)
{
  
  int selection = 0;
  int estSelectionne = 0;

  int heureDebut = 0;
  int minuteDebut = 0;
  int heureFin = 0;
  int minuteFin = 0;

  int intensite = 0;

  int rougeSaisie = 0;
  int vertSaisie = 0;
  int bleuSaisie = 0; 

  while(1)
  {
    delay(100);

    selection = SelectionMenu(myEnc.read(), 5, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);

    // Affichage premiere section
    if(selection < 3)
    {
      /////////: Heure Debut
      display.setCursor(20, 0); 
      display.println("Debut");

      display.setCursor(60, 0); 
      display.println(heureDebut);

      display.setCursor(70, 0); 
      display.println(":");

      display.setCursor(80, 0); 
      display.println(minuteDebut);

      //////// Heure Fin
      display.setCursor(20, 10); 
      display.println("Fin");

      display.setCursor(60, 10); 
      display.println(heureFin);

      display.setCursor(70, 10); 
      display.println(":");

      display.setCursor(80, 10); 
      display.println(minuteFin);

      /////////: Lum Max
      display.setCursor(20, 20); 
      display.println("Luminosite");

      display.setCursor(60, 20); 
      display.println(intensite);

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
      display.println(rougeSaisie);

      display.setCursor(65, 0); 
      display.println(",");

      display.setCursor(70, 0); 
      display.println(vertSaisie);

      display.setCursor(85, 0); 
      display.println(",");

      display.setCursor(90, 0); 
      display.println(bleuSaisie);

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
        switch(nbConfiguration)
        {
          case 1:

            Serial.println("Configuration 1 mise à jour");
            configuration1.heureDebut.heure = heureDebut;
            configuration1.heureDebut.minute = minuteDebut;

            configuration1.heureFin.heure = heureDebut;
            configuration1.heureFin.minute = minuteDebut;

            configuration1.luminosite = intensite;

            configuration1.rouge = rougeSaisie;
            configuration1.vert = vertSaisie;
            configuration1.bleu = bleuSaisie;

            break;

          case 2:
            Serial.println("Configuration 2 mise à jour");
            configuration2.heureDebut.heure = heureDebut;
            configuration2.heureDebut.minute = minuteDebut;
            configuration2.heureFin.heure = heureDebut;
            configuration2.heureFin.minute = minuteDebut;
            configuration2.luminosite = intensite;
            configuration2.rouge = rougeSaisie;
            configuration2.vert = vertSaisie;
            configuration2.bleu = bleuSaisie;

            break;

          case 3:
            Serial.println("Configuration 3 mise à jour");
            configuration3.heureDebut.heure = heureDebut;
            configuration3.heureDebut.minute = minuteDebut;
            configuration3.heureFin.heure = heureDebut;
            configuration3.heureFin.minute = minuteDebut;
            configuration3.luminosite = intensite;
            configuration3.rouge = rougeSaisie;
            configuration3.vert = vertSaisie;
            configuration3.bleu = bleuSaisie;

            break;

          case 4:
            Serial.println("Configuration 4 mise à jour");
            configuration4.heureDebut.heure = heureDebut;
            configuration4.heureDebut.minute = minuteDebut;
            configuration4.heureFin.heure = heureDebut;
            configuration4.heureFin.minute = minuteDebut;
            configuration4.luminosite = intensite;
            configuration4.rouge = rougeSaisie;
            configuration4.vert = vertSaisie;
            configuration4.bleu = bleuSaisie;

            break;

          case 5:
            Serial.println("Configuration 5 mise à jour");
            configuration5.heureDebut.heure = heureDebut;
            configuration5.heureDebut.minute = minuteDebut;
            configuration5.heureFin.heure = heureDebut;
            configuration5.heureFin.minute = minuteDebut;
            configuration5.luminosite = intensite;
            configuration5.rouge = rougeSaisie;
            configuration5.vert = vertSaisie;
            configuration5.bleu = bleuSaisie;

            break;
        }
        return;
      }

      selection = 0;

    }

    display.display();

    delay(100);
  }
}

void MenuPrincipal()
{
  Serial.println("Entree dans le menu principal");
  
  int selection = 0;
  int estSelectionne = 0;

  while(1)
  {
    selection = SelectionMenu(myEnc.read(), 8, 0);
    estSelectionne = LireBoutton();

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);

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
    else
    {
      display.setCursor(20, 0); 
      display.println("Configuration 5");

      display.setCursor(20, 10); 
      display.println("Sortie");

      display.setCursor(2, (selection - 6) * 10); 
      display.println(">");
    }

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

        Serial.println("Sortie du menu principal");

        break;
      }
    }

    display.display();

    delay(100);
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