#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <FastCRC.h>
#include "Actionneur.h"
#include <Servo.h>
//#include <Capteur.h>

// Adresse I2C du module de navigation
#define ADRESSE 80
// Etat des actions
#define FINI      0
#define EN_COURS  1
#define PREVU     2
// Etat de la nouvelle action demandée
#define VALIDEE     0 // Nouvelle action validée et prise en compte
#define DISPONIBLE  1 // Nouvelle action enregistrée
#define ERRONEE     2 // nouvelle action erronée. CRC nok.
// Liste des différentes actions
// Actions sur les bras pour l'abeille
#define BD_HAUT 0
#define BD_BAS  1
#define BG_HAUT 2
#define BG_BAS  3
// Actions de recuperation/envoi des balles
#define RECUP_BALLES 4
#define ENVOI_BALLES 5

Servo brasGauche;
Servo brasDroit;
Servo barriere;
Servo balise;

// Declaration des broches d'ES pour les satellites
// Broches analogiques :
int ana_1 = A6 ; // 1 - pin 20 ou A6 - PWM
int ana_2 = A7 ; // 2 - pin 21 ou A7 - PWM
int ana_3 = A8 ; // 3 - pin 22 ou A8 - PWM
int ana_4 = A9 ; // 4 - pin 23 ou A9 - PWM
int ana_5 = A0 ; // 5 - pin 14 ou A0
int ana_6 = A1 ; // 6 - pin 15 ou A1
int ana_7 = A2 ; // 7 - pin 16 ou A2
int ana_8 = A3 ; // 8 - pin 17 ou A3
// Broches numeriques : ( Utilisé par le module de moteurs pas-à-pas )
// int digi_1 = 5 ; // 1 - PWM
// int digi_2 = 4 ; // 2 - PWM
// int digi_3 = 3 ; // 3 - PWM
// int digi_4 = 2 ; // 4
// int digi_5 = 9 ; // 5 - PWM
// int digi_6 = 8 ; // 6
// int digi_7 = 7 ; // 7
// int digi_8 = 6 ; // 8 - PWM
int hautBrasGauche  = 118   ;
int basBrasGauche   = 20    ;
int hautBrasDroit   = 30    ;
int basBrasDroit    = 120   ;
int hautBarriere    = 120   ;
int basBarriere     = 40    ;
int droiteBalise    = 2400  ;
int gaucheBalise    = 900   ;
int milieuBalise    = 1500  ;
// Variables du moteur de lancé de balles
int moteurBalles = ana_8      ;
const int vitMaxBalles = 100  ;

bool baliseState = 0;
Actionneur baliseA(balise,ana_4,1000,800,2400);

// Pin IO pour le moteur pas-à-pas du barillet
int pinStep1=4, pinDir1=5, pinSleep1=3, pinReset1=2;
// Declaration du moteur barillet
AccelStepper MBarillet(AccelStepper::DRIVER,pinStep1, pinDir1);

FastCRC8 CRC8;
byte bufAction[2]={0,0}; // Buffer de reception des ordres d'action + le CRC
byte crcAction = 0; // CRC de controle des ordres d'action'
byte etatAction ;

const float VitesseMaxBarillet = 1000.0; //Ancien :
const float VitesseMinBarillet = 500.0; //Ancien :
const float AccelMin = 300.0; //Ancien :
const float AccelMax = 800.0; //Ancien :
const float AccelStop = 2000.0; //Ancien :

int indexAction = 0 ;

int16_t actionRequest ; // action

byte newAction = VALIDEE;

void setup();
void loop();
void receiveEvent(int HowMany);
void requestEvent();
void updateAction();
void selectAction();
void finMatch();
void updateBalise();
void executeAction();

void actionBras();
void actionEnvoiBalles();
