#include <Arduino.h>
#include <Wire.h>

// Points pour chaque action
#define recuperateur 10 // points pour chaque récupérateur au moins vidé d’une balle par l’équipe à qui il appartient
#define chateau 5 //points pour chaque balle de la bonne couleur dans le château d’eau.
#define epuration 10 // points par balle de la couleur adverse dans la station d’épuration
#define deposePanneau 5 //points pour la dépose du panneau devant le château d’eau
#define activePanneau 25 //points pour un panneau alimenté (interrupteur fermé) à la fin du match
#define deposeAbeille 5 //points pour la dépose de l’abeille sur la ruche
#define activeAbeille 50 //points pour une fleur butinée (ballon éclaté)
// Adressage I2C pour les cartes esclaves
#define carteDeplacement 60
#define carteActionneur 80
// Couleur Equipe
#define vert 1
#define orange 0
// Autres
#define tempsMatch 99000

bool equipe = vert;
byte optionNavigation = 0;
int score = 0;
double timeInit=0;

void sendNavigation(byte fonction, int X, int Y, int rot);
void sendNavigation(byte fonction, int rot, int dist);
void majScore(int points, int multiplicateur);
void turnGo(bool recalage,bool ralentit,int turn, int go);
void attente(int temps);
void testDeplacement();
void Homologation();
void recalageInit();
void finMatch();
bool askNavigation();
int majTemps();

void setup()
{
	Wire.begin();
	Serial.begin(9600);

	// while(!digitalRead(pinTirette))
	// {
	// 	//Attente insertion tirette
	// }
  //
	// while(digitalRead(pinTirette))
	// {
	// 	//attente relache tirette
	// 	askIHM();
	// 	delay(100);
	// 	//attente(100);
	// }
	timeInit = millis();
  delay(1000);
}

void loop()
{
	//testDeplacement();
  Homologation();
}

void testDeplacement()
{
	sendNavigation(1, 0, 300);
	attente(2000);
	while(askNavigation())
	{
		attente(100);
		Serial.println(askNavigation());
	}
	sendNavigation(1, 0,-300);
	attente(2000);
	while(askNavigation())
	{
		attente(100);
		Serial.println(askNavigation());
	}
}

bool askNavigation()
{
  bool etatNavigation = true;
  char reponseNavigation ;
	Wire.requestFrom(carteDeplacement, 1);
  reponseNavigation = Wire.read();
  if (reponseNavigation=='N') etatNavigation = true ;
  else if (reponseNavigation=='O') etatNavigation = false ;
  Serial.println(reponseNavigation);

	return etatNavigation;
}

void majScore(int points, int multiplicateur)
{
	score = score + (points*multiplicateur);
}

//----------------MISE A JOUR DU TEMPS DE MATCH----------------
int majTemps()
{
  int tempsRestant = ( tempsMatch - (millis() - timeInit) ) / 1000;
  if ( tempsRestant <= 0 )
  {
    finMatch();
  }
  return tempsRestant;
}

void attente(int temps)
{
	int initTemps = millis();
	while( (millis()-initTemps) <= temps)
	{
		// Faire des choses dans la procedure d'attente
		majTemps();
	}
}

void turnGo(bool recalage,bool ralentit,int turn, int go)
{
	bitWrite(optionNavigation,0,equipe);
	bitWrite(optionNavigation,1,recalage);
	bitWrite(optionNavigation,2,ralentit);
	sendNavigation(optionNavigation, turn, go);
	attente(600);
	while(askNavigation())
	{
		attente(100);
		//Serial.println(askNavigation());
	}
}

void Homologation()
{
	turnGo(0,false,0,900);
	turnGo(0,true,90,150);
	turnGo(0,false,0,-1000);
	turnGo(0,false,-45,-980);
	turnGo(0,false,-45,-250);
	turnGo(0,true,0,-100);
	turnGo(0,false,0,300);
	// turnGo(0,false,-45,750);
  // turnGo(0,false,135,650); // Pousser les cube
	// turnGo(0,false,0,-480);
	// turnGo(0,false,90,500);
	// turnGo(0,true,0,220); // recalage Tube
	// turnGo(0,false,0,-200);
	// turnGo(0,false,90,800);

  while(1);
}

void sendNavigation(byte fonction, int X, int Y, int rot)
{
	Wire.beginTransmission(carteDeplacement);
	Wire.write(fonction);
	Wire.write(X >> 8);
	Wire.write(X & 255);
	Wire.write(Y >> 8);
	Wire.write(Y & 255);
	Wire.write(rot >> 8);
	Wire.write(rot & 255);
	Wire.endTransmission();
}

void sendNavigation(byte fonction, int rot, int dist)
{
	if ( equipe == vert ) rot = -rot ;
	Wire.beginTransmission(carteDeplacement);
	Wire.write(fonction);
	Wire.write(rot >> 8);
	Wire.write(rot & 255);
	Wire.write(dist >> 8);
	Wire.write(dist & 255);
	Wire.endTransmission();
}

void finMatch()
{
	// Stopper les moteurs
	sendNavigation(255, 0, 0);
	// Boucle infinie
	while(1)
	{
		// Stopper les moteurs
		sendNavigation(255, 0, 0);
	}
}
