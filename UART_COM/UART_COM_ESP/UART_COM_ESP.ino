#include <WiFi.h>
#include "time.h"

const char* ssid = "ALCT";
const char* password = "azertyuiop";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600 * 1;
const int   daylightOffset_sec = 3600 * 0;

const int outPinTime = 5;

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
}

void loop() {

  struct tm currentTime = getTime();
  int hour = currentTime.tm_hour;

  Serial.print("Heure actuelle : ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(currentTime.tm_min);
  Serial.print(":");
  Serial.println(currentTime.tm_sec);

  TransferByte(hour, outPinTime);

  delay(2000);
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
  Serial.println("Début de la transmission ...");

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

  Serial.println("Fin de la transmission ...");
  Serial.print("Valeur transmise : ");
  Serial.println(num - 128);

  digitalWrite(pin, LOW); // Fin de la transmission

}