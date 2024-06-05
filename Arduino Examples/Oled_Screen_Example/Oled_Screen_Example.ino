//ARDUINO HOCAM UI(USER INTERFACE) - 
//This code  prepared semi-english and turkish. Which is why i added some explanations beside some functions or variables
//for those who does not understand turkish . If any question please let me know and ask me without any doubt .
//Arduino Hocam 
//Ali SEVİNDİK
#include <OLED_I2C.h>
extern uint8_t SmallFont[];
int i  = 10; // this is the y position of selection sign ">"
int m = 207; // brightness of oled screen as default
int page = 1; // page as default
int x = 0; int y = 0;  // randomly named variables in order to controll button pressed or not
int son = 0; int son2 = 0; int son3 = 0;  // randomly named variables in order to controll button pressed or not
int diger = 1; int diger2 = 1; int diger3 = 1; //  randomly named variables in order to controll button pressed or not
volatile boolean inv = false;
volatile boolean tus = false; volatile boolean tus3 = false;
volatile boolean inv2 = false;

OLED  myOLED(SDA, SCL, 8);
void setup() {


  // put your setup code here, to run once:
  myOLED.begin(); myOLED.clrScr();

  pinMode(2, INPUT);
  myOLED.setFont(SmallFont);

}

void loop() {

  int z = digitalRead(4);
  int y = digitalRead(3);
  int x = digitalRead(2);




  geri(); //geri means back. For turn back we need to create this function.
  if (page == 1) {

    myOLED.clrScr();
    myOLED.invertText(true);
    myOLED.print("ANA MENU", CENTER, 0); // MANI MENU
    myOLED.invertText(false);
    myOLED.print("INVERT MODE:", 10, 10); //INVERT MODE
    myOLED.print("PARLAKLIK", 10, 25); //BRIGHTNESS
    myOLED.print("MENU 3", 10, 40); //MENU3
    if (inv2 == true && page == 1)
      myOLED.print("ON", 85, 10);
    if (inv2 == false && page == 1)
      myOLED.print("OFF", 85, 10);
    myOLED.print(">", 0, i); myOLED.update();
    yon();
  }

  secim(); //SELECTION BUTTON FUNCTION

  if (page == 2) // WHEN IT IS PAGE 2

  {
    myOLED.clrScr();
    myOLED.invertText(true);
    myOLED.print("ALT MENU", CENTER, 0); //MAIN MENU
    myOLED.invertText(false);
    myOLED.print("altMENU 1", 10, 10); //SUB MENU1
    myOLED.print("altMENU 2", 10, 25);//SUB MENU2
    myOLED.print("altMENU 3", 10, 40);//SUB MENU3

    myOLED.print(">", 0, i); myOLED.update();
    yon();
  }

  invertFunc(); // INVERT FUNCTION

  if (i == 25 && page == 1 && y == 1 )  //If we select brightness mode it must open new page which is third page.
  {
    page = 3;
  }

  if (page == 3 ) //if the page is 3
  {
    myOLED.clrScr();
    myOLED.print("PARLAKLIK:", 0, i); //BRIGHTNESS : 
    myOLED.printNumI(m, 60, 25);
    myOLED.update();
    if (y == 1 && page == 3)
      parlaklikAyar();  //brightnessSetting function
  }

  myOLED.setBrightness(m);


}




void parlaklikAyar() //brightnessSetting function
{
  int y = digitalRead(3);
  if (y == 0)
    diger3 = 0;
  if (y == 1 && diger3 == 0)
  {
    son3 = 1;
    tus3 = true;   

  }
  if (son3 == 1 && tus3 == true)
  { diger3 = 1;
    myOLED.clrScr();
    myOLED.update();
    m = m + 20;
    tus3 = false; son3 = 0;

    if (m > 255)
    {
      m = 0;
    }
  }
}
void invertFunc()  
{
  int y = digitalRead(3);
  if (i == 10 && page == 1 && y == 1 && son2 == 0)
  {
    son2 = 1;
    if (inv2 == false)
    { inv2 = true;
    }
    else if (inv2 == true) {
      inv2 = false;
    }

  }
  else if (i == 10 && page == 1 && y == 0 && son2 == 1)
    son2 = 0;
  if (inv2 == true)
    myOLED.invert(true);
  if (inv2 == false)
    myOLED.invert(false);




}
void geri()  
{ int z = digitalRead(4);
  if (z == 1)
  { i = 10;

    page = page - 1;
    if (page < 1)
      page = 1;

  }
}
void secim()
{

  int y = digitalRead(3);
  if (y == 1 && i != 10 && i != 25)
  {
    page = 2;
    i = 10;
  }


}
void yon()  // down function. When button pressed selection sign ">" will move downwards.
{ int x = digitalRead(2);
  if (x == 0)
    diger = 0;
  if (x == 1 && diger == 0)
  {
    son = 1;
    tus = true;

  }
  if (son == 1 && tus == true)
  { diger = 1;
    myOLED.clrScr();
    myOLED.update();
    i = i + 15;
    tus = false; son = 0;

    if (i > 40)
    {
      i = 10;
    }
  }

}



