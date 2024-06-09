#include <WiFi.h>
#include "time.h"


const char* ssid = "ALCT";
const char* password = "azertyuiop";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600 * 2;
const int   daylightOffset_sec = 0;

const int outPinTime = 5;
const int inPinTime = 2; 

int seconde = 0;
int heure = 0;
int minute = 0;

int compteur;

void setup() 
{
  Serial.begin(9600);

  // Configuration WIFI

  WiFi.begin(ssid, password);

  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");

  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Port de sortie de l'heure
  pinMode(outPinTime, OUTPUT);
  pinMode(inPinTime, INPUT);

  // Dès qu'on a accès à l'heure, on effectue une unique requête pour définir le temps

  struct tm currentTime = getTime();

  heure = currentTime.tm_hour;
  minute = currentTime.tm_min;
  seconde  = currentTime.tm_sec;

  Serial.print("Heure d'initialisation : ");
  Serial.print(heure);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(seconde);

  compteur = 0;
}

void loop() {

  // Envoie l'heure si une demande est effectuée
  if(digitalRead(inPinTime) == 1)
  {
    Serial.println("Demande Reçue !");

    TransferByte(heure, outPinTime);

    TransferByte(minute, outPinTime);

    Serial.print("Heure transmise : ");
    Serial.print(heure);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(seconde);

  }
  // Met à jour l'heure (on se sert de l'ESP comme d'une horloge)
  else
  {
    compteur = compteur + 1;

    delay(1000);

    seconde ++;
    if (seconde > 60)
    {
      seconde = 0;
      minute ++;
    }

    if (minute > 60)
    {
      minute = 0;
      heure ++;
    }

    if (heure > 23)
    {
      heure = 0;
    }

    if(compteur > 60 * 5) // Actualiser toutes les 5 minutes
    {

      struct tm currentTime = getTime();

      heure = currentTime.tm_hour;
      minute = currentTime.tm_min;
      seconde  = currentTime.tm_sec;

      Serial.print("Heure actualisée : ");
      Serial.print(heure);
      Serial.print(":");
      Serial.print(minute);
      Serial.print(":");
      Serial.println(seconde);

      compteur  = 0 ;
    }
  
  }
}

struct tm getTime() 
{

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }

  return timeinfo;
}

void TransferByte(int num, int pin)
{
  // Nécessite la configuration du pin en OUTPUT

  char binary[9] = {0}; // Message en binaire

  num += 128; // On ajoute 128 pour que la longueur du message en charactère soit de 8
  itoa(num,binary,2); // Convertir le nombre en binaire

  // Transmission bit par bit
  for(int i = 0; i < 8; i++){

    if (binary[i] == '1')
    {
      digitalWrite(pin,HIGH);
    }
    else
    {
      digitalWrite(pin,LOW);
    }

    delay(50); // Durée de la transmission

  }

  digitalWrite(pin, LOW); // Fin de la transmission

}