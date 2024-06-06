#include "Arduino.h"
#include "string.h"

#ifndef Menus_h
#define Menus_h

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

class Menus
{
  private :

    

  public :

    Menus() : test(0) {};
    int test = 0;
};


#endif