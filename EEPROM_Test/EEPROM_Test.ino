#include <EEPROM.h>
#include <Arduino.h>

int iter = 0;
bool mode = !true; // True : Write / False : Read

void setup()
{
  Serial.begin(9600);

}

void loop()
{

  if(iter <= 0)
  {
    if(mode)
    {
      Serial.println("Test write ...");

    }
    else
    {
      Serial.println("Test read ...");

      Serial.print("Taille EEPROM : ");

      Serial.println(EEPROM.length());

      Chargement_EEPROM();
    }
    iter ++;
  }
  else
  {
    Serial.println("Yeet");
  }

  delay(1000);
}

void Chargement_EEPROM()
{
  for(int i = 0; i < EEPROM.length(); i++)
  {
    Serial.print("ADD [");
    Serial.print(i);
    Serial.print("] : ");
    Serial.println(EEPROM.read(i));
  }
  // get(address, data) : autre que des byte
  // read(address) : byte
}

void Sauvegarde_EEPROM()
{
  // https://docs.arduino.cc/learn/built-in-libraries/eeprom/#length
  
  // put(address, data) : autre que des bytes a stocker
  // update(address, value) : byte a stocker ssi différent de déjà stocké (sinon utiliser write)

}