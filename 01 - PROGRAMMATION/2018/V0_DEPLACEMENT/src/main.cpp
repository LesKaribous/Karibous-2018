#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <FastCRC.h>
//#include <Capteur.h>

//Adresse I2C du module de navigation
#define ADRESSE 60
//Etat des déplacements
#define FINI 0
#define EN_COURS 1
#define PREVU 2
//Etat de la nouvelle position demandée
#define VALIDEE 0 // Nouvelle position validée et prise en compte
#define DISPONIBLE 1 // Nouvelle position enregistrée
#define ERRONEE 2 // nouvelle position erronée. CRC nok.


int pinStep1=4;
int pinDir1=5;
int pinSleep1=3;
int pinReset1=2;

int pinStep2=8;
int pinDir2=9;
int pinSleep2=7;
int pinReset2=6;

AccelStepper MGauche(AccelStepper::DRIVER,pinStep1, pinDir1);
AccelStepper MDroit(AccelStepper::DRIVER,pinStep2, pinDir2);

FastCRC8 CRC8;
byte bufNavRelatif[6]={0,0,0,0,0,0}; // Buffer de reception des ordres de navigation relatifs + le CRC
byte crcNavRelatif = 0; // CRC de controle des ordres de navigation relatifs

byte fonction ;
int16_t relativeRequest[2] ; // rotation, distance

byte newPos = VALIDEE;
bool PRESENCE_ARRIERE = 0, PRESENCE_AVANT = 0;
int ADVERSAIRE_ARRIERE = 22;
int ADVERSAIRE_AVANT = 23;

double AskX, AskRot, TempGauche, TempDroit, NewX, NewRot ;

int sensorTime = 2000;
int avantTimeInit = 0;
int arriereTimeInit = 0;

bool optionAdversaire = false;
bool optionRecalage = false;
bool optionRalentit = false;

char etatRotation, etatAvance;

const float FacteurX= 2.17; //Ancien : 154.8
const float FacteurDroit = 8.0; //Ancien : 154.8
const float FacteurGauche = 8.0; //Ancien : 154.8
const float FacteurRot = 6.17; //Ancien : 19.64

const float VitesseMaxDroite = 4500.0; //Ancien : 8000
const float VitesseMaxGauche = 4500.0; //Ancien : 8000
const float VitesseMinDroite = 2000.0; //Ancien : 5000
const float VitesseMinGauche = 2000.0; //Ancien : 5000
const float AccelRot = 2000.0; //Ancien : 2000
const float AccelMin = 1000.0; //Ancien : 2000
const float AccelMax = 2000.0; //Ancien : 5000
const float AccelStop = 4000.0; //Ancien : 8000

byte BORDURE = 0 ;
// AV_DROIT , AV_GAUCHE , AR_DROIT , AR_GAUCHE
//int PIN_BORDURE[4] = {20,17,16,21};

void receiveEvent(int HowMany);
void requestEvent();
void adversaire();
void updatePos();
void bordure();
void turnGo();
void recalage();
void FIN_MATCH();

void setup()
{
	pinMode(pinReset1, OUTPUT);
	pinMode(pinSleep1, OUTPUT);
	pinMode(pinReset2, OUTPUT);
	pinMode(pinSleep2, OUTPUT);

	digitalWrite(pinReset1, HIGH);
	digitalWrite(pinSleep1, HIGH);
	digitalWrite(pinReset2, HIGH);
	digitalWrite(pinSleep2, HIGH);

	Serial.begin(9600);
	Wire.begin(ADRESSE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

	MGauche.setMaxSpeed(VitesseMaxGauche);
	MGauche.setAcceleration(AccelMax);

	MDroit.setMaxSpeed(VitesseMaxDroite);
	MDroit.setAcceleration(AccelMax);

	// pinMode(ADVERSAIRE_ARRIERE, INPUT_PULLUP);
	// pinMode(ADVERSAIRE_AVANT, INPUT_PULLUP);
	//
	// for(int i = 0;i<4;i++)
	// {
	// 	pinMode(PIN_BORDURE[i], INPUT_PULLUP);
	// }
}

void loop()
{
	MGauche.run();
	MDroit.run();

	updatePos();
	adversaire();
	turnGo();
	bordure();

	if (fonction == 255) FIN_MATCH();

}

void updatePos()
{
	if (newPos==DISPONIBLE)
	{
		etatRotation = PREVU ;
		etatAvance = PREVU ;
		// Traitement de l'information
		NewX=relativeRequest[1]*FacteurX;
		NewRot=relativeRequest[0]*FacteurRot;
	}
	newPos = VALIDEE;
}

void turnGo()
{
  if ((PRESENCE_AVANT && NewX>=0 && etatAvance == EN_COURS) || (PRESENCE_ARRIERE && NewX<0 && etatAvance == EN_COURS) )
  {
     TempGauche = MGauche.distanceToGo();
     TempDroit = MDroit.distanceToGo();

     MGauche.setAcceleration(AccelStop);
     MDroit.setAcceleration(AccelStop);

     MGauche.move(0); // Stop as fast as possible: sets new target
     MDroit.move(0); // Stop as fast as possible: sets new target

     TempGauche = TempGauche + MGauche.distanceToGo();
     TempDroit = TempDroit + MDroit.distanceToGo();

     MGauche.run();
     MDroit.run();

     // Attendre que l'adversaire soit partit
     for(unsigned long i=0;i<=80000;i++) //Attendre XXXX iterations avant de recommencer
     {
       adversaire();
       if ((PRESENCE_AVANT &&  NewX>=0 ) || (PRESENCE_ARRIERE && NewX<0))
       {
          i=0;              //RAZ de l'iteration si toujours un obstacle
       }
       //delayMicroseconds(10);
       MGauche.run();
       MDroit.run();
     }

     MGauche.setAcceleration(AccelMax);
     MDroit.setAcceleration(AccelMax);

     MGauche.move(TempGauche);
     MDroit.move(TempDroit);
  }
  else
  {
    if (etatRotation == PREVU)
    {
      MGauche.setAcceleration(AccelRot);
      MDroit.setAcceleration(AccelRot);
      MDroit.move(NewRot*FacteurRot);
      MGauche.move(NewRot*FacteurRot);
      etatRotation = EN_COURS ;
    }
    if (MDroit.distanceToGo() == 0 && MGauche.distanceToGo() == 0 && etatRotation == EN_COURS)
    {
      etatRotation = FINI ;
      etatAvance = PREVU ;
    }
    if (etatAvance == PREVU && etatRotation == FINI)
    {
  		if (optionRalentit)
  		{
  			MGauche.setMaxSpeed(VitesseMinGauche);
  	        MDroit.setMaxSpeed(VitesseMinDroite);
  	        MGauche.setAcceleration(AccelMin);
  	        MDroit.setAcceleration(AccelMin);
  		}
  		else
  		{
  			MGauche.setMaxSpeed(VitesseMaxGauche);
  	        MDroit.setMaxSpeed(VitesseMaxDroite);
  	        MGauche.setAcceleration(AccelMax);
  	        MDroit.setAcceleration(AccelMax);
  		}

        MDroit.move(NewX*FacteurDroit);
        MGauche.move(-(NewX*FacteurGauche));
        etatAvance = EN_COURS ;
    }
    if (etatAvance == EN_COURS && optionRecalage)
    {
      MGauche.setMaxSpeed(VitesseMinGauche);
      //MGauche.setAcceleration(AccelMax);
      MDroit.setMaxSpeed(VitesseMinDroite);
      //MDroit.setAcceleration(AccelMax);
	  MGauche.setAcceleration(AccelMin);
	  MDroit.setAcceleration(AccelMin);
      // Si on est à la fin du mouvement
      recalage();
    }
    if (MDroit.distanceToGo() == 0 && MGauche.distanceToGo() == 0 && etatAvance == EN_COURS)
    {
      etatAvance = FINI ;
    }
  }
}

void recalage()
{
	// A MODIFIER !!!!!!!!
	// AV_DROIT , AV_GAUCHE , AR_DROIT , AR_GAUCHE
   if ( (bitRead(BORDURE,0) && NewX > 0 ) || ( bitRead(BORDURE,3) && NewX < 0))
  {
    MDroit.setAcceleration(AccelStop);
    MDroit.move(0); // Stop as fast as possible: sets new target
    MDroit.setCurrentPosition(0);
    while (MDroit.distanceToGo() != 0 )
    {
      MDroit.run();
    }
    MDroit.setMaxSpeed(VitesseMaxDroite);
    MDroit.setAcceleration(AccelMax);
  }
   if ( (bitRead(BORDURE,1) && NewX > 0) || (bitRead(BORDURE,2) && NewX < 0))
  {
    MGauche.setAcceleration(AccelStop);
    MGauche.move(0); // Stop as fast as possible: sets new target
    MGauche.setCurrentPosition(0);
    while (MGauche.distanceToGo() != 0 )
    {
      MGauche.run();
    }
    MGauche.setMaxSpeed(VitesseMaxGauche);
    MGauche.setAcceleration(AccelMax);
  }
}

void bordure()
{
	// for(int i = 0;i<4;i++)
	// {
	// 	bitWrite(BORDURE,i,digitalRead(PIN_BORDURE[i]));
	// }
	// //Serial.println(BORDURE,BIN);
	// //delay(200);

}

void adversaire()
{
	// // Si la detection adverse est activée
	// if (!optionAdversaire)
	// {
	// 	// Adversaire Avant
	// 	if (digitalReadFast(ADVERSAIRE_AVANT))
	// 	{
	// 		if (!PRESENCE_AVANT)
	// 		{
	// 			PRESENCE_AVANT = true ;
	// 		}
	// 		avantTimeInit = millis();
	// 	}
	// 	else
	// 	{
	// 		if((millis()-avantTimeInit)>=sensorTime)
	// 		{
	// 			PRESENCE_AVANT = false;
	// 		}
	// 	}
	// 	// Adversaire Arriere
	// 	if (digitalReadFast(ADVERSAIRE_ARRIERE))
	// 	{
	// 		if (!PRESENCE_ARRIERE)
	// 		{
	// 			PRESENCE_ARRIERE = true ;
	// 		}
	// 		arriereTimeInit = millis();
	// 	}
	// 	else
	// 	{
	// 		if((millis()-arriereTimeInit)>=sensorTime)
	// 		{
	// 			PRESENCE_ARRIERE = false;
	// 		}
	// 	}
	// }
	// else
	// {
	// 	PRESENCE_ARRIERE = false;
	// 	PRESENCE_AVANT = false;
	// }
	//
	// // UNIQUEMENT EN DEBUG !!!!!!!!!!!
	// /*
	// if (PRESENCE_AVANT || PRESENCE_ARRIERE)
	// {
	// 	Serial.print(PRESENCE_AVANT);
	// 	Serial.print(" - ");
	// 	Serial.println(PRESENCE_ARRIERE);
	// 	delay(200);
	// }
	// */

}

void receiveEvent(int howMany)
{
	if(howMany == 6)
	{
		// Si un déplacement relatif est demandé
		// On receptionne les données
		for (int i=0;i<=5;i++)
		{
			bufNavRelatif[i]=Wire.read();
		}
	}
	// On calcul le CRC
	crcNavRelatif = CRC8.smbus(bufNavRelatif, sizeof(bufNavRelatif)-1); //On enleve le CRC
	//Serial.println(crcNavRelatif);
	// On regarde si le CRC calculé correspond à celui envoyé
	if (crcNavRelatif==bufNavRelatif[5])
	{
		// CRC ok
		// On traite les données
		fonction = bufNavRelatif[0];
		relativeRequest[0]= bufNavRelatif[1] << 8 | bufNavRelatif[2];
		relativeRequest[1]= bufNavRelatif[3] << 8 | bufNavRelatif[4];
		optionAdversaire = bitRead(fonction, 0);
		optionRecalage = bitRead(fonction, 1);
		optionRalentit = bitRead(fonction,2);
		// On indique qu'une nouvelle position est disponible
		newPos = DISPONIBLE;
	}
	else
	{
		// CRC nok - la donnée est erronée
		// On indique que la prochaine position est erronée pour en renvois eventuel
		newPos = ERRONEE;
	}

}

//Fin de match
void FIN_MATCH()
{
	MGauche.setSpeed(VitesseMaxGauche);
	MDroit.setSpeed(VitesseMaxDroite);
	MGauche.setAcceleration(AccelStop);
	MDroit.setAcceleration(AccelStop);
 	MGauche.move(0);    //Commande de deplacement Relatif
   	MDroit.move(0);      //Commande de deplacement Relatif

   	while(1)
   	{
		MGauche.stop();
		MDroit.stop();
    	MGauche.run();
    	MDroit.run();
   	}
}

void requestEvent()
{

	if ( etatAvance == FINI && etatRotation == FINI && newPos == VALIDEE)
  {
    // Mouvement terminé
		Wire.write("O");
		//Serial.println('O');
	}
	else if (newPos == ERRONEE)
	{
    // Commande non validé
		Wire.write("E");
		//Serial.println('N');
	}
	else
	{
    // Mouvement non terminé
		Wire.write("N");
		//Serial.println('N');
	}
}
