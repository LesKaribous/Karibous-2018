#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <FastCRC.h>
#include <DFRobotDFPlayerMini.h>

// Points pour chaque action
#define recuperateur 10 // points pour chaque récupérateur au moins vidé d’une balle par l’équipe à qui il appartient
#define chateau 5 //points pour chaque balle de la bonne couleur dans le château d’eau.
#define epuration 10 // points par balle de la couleur adverse dans la station d’épuration
#define deposePanneau 5 //points pour la dépose du panneau devant le château d’eau
#define activePanneau 25 //points pour un panneau alimenté (interrupteur fermé) à la fin du match
#define deposeAbeille 5 //points pour la dépose de l’abeille sur la ruche
#define activeAbeille 50 //points pour une fleur butinée (ballon éclaté)
#define nonForfait 10 //points bonus sont attribués à toutes les équipes qui ne sont pas « forfait »
// Adressage I2C pour les cartes esclaves
#define carteDeplacement 60
#define carteActionneur 80
// Couleur Equipe
#define vert 1
#define orange 0
// Autres
#define tempsMatch 99000
#define SerialPlayer Serial1
//Etat de la position demandée
#define TERMINEE 0  // Position validée et terminée
#define RECU 1      // Position reçu
#define ERRONEE 2   // Position erronée. CRC nok.
#define BIZARRE 3   // Reponse étrange à ne pas prendre en compte
// Etat bouttons IHM
#define fuck 1
#define noFuck 0
#define strategie1 0
#define strategie2 1
#define nok 1
#define ok 0
// Liste des différentes actions
// Actions sur les bras pour l'abeille
#define BD_HAUT 0
#define BD_BAS  1
#define BG_HAUT 2
#define BG_BAS  3
// Actions de recuperation/envoi des balles
#define RECUP_BALLES_COMPLET 4
#define RECUP_BALLES_SAFE 5
#define ENVOI_BALLES 6

// Logo Karibous
#define LOGO_KARIBOUS_width 128
#define LOGO_KARIBOUS_height 33
static unsigned char LOGO_KARIBOUS_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xfc, 0xf9, 0x03,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x1c, 0xfc, 0xfb, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x1c, 0xfc, 0xfb, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x04, 0x0b, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x1c, 0xfc, 0xf9, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x1c, 0x0c, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xfc, 0x39, 0x03,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xfc, 0xf9, 0xf9, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xfc, 0x01, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0xc0, 0x01,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xe0, 0xc3, 0x1f,
   0xe0, 0xff, 0xc7, 0xf3, 0xff, 0x81, 0xff, 0x87, 0x0f, 0x38, 0xfc, 0x3f,
   0xfc, 0xf0, 0xc1, 0xff, 0xe1, 0xff, 0xcf, 0xf3, 0xff, 0xc3, 0xff, 0x8f,
   0x0f, 0x38, 0xfe, 0x3f, 0xfc, 0xf8, 0xc0, 0xff, 0xe3, 0xff, 0xdf, 0xf3,
   0xff, 0xe7, 0xff, 0x9f, 0x0f, 0x38, 0xfe, 0x3f, 0xfc, 0xf8, 0xc0, 0xff,
   0xe7, 0xff, 0xdf, 0xf3, 0xff, 0xe7, 0xff, 0x9f, 0x0f, 0x38, 0xff, 0x1f,
   0xfc, 0x7c, 0xc0, 0xff, 0xe7, 0xff, 0xdf, 0xf3, 0xff, 0xe7, 0xff, 0x9f,
   0x0f, 0x38, 0xff, 0x1f, 0xfc, 0x3e, 0xc0, 0xff, 0xe7, 0xff, 0xdf, 0xf3,
   0xff, 0xe7, 0xff, 0x9f, 0x0f, 0x38, 0xff, 0x1f, 0xfc, 0x1f, 0x00, 0xf0,
   0x67, 0x00, 0xde, 0xf3, 0x00, 0xe7, 0x01, 0x9e, 0x0f, 0x38, 0x0f, 0x00,
   0xfc, 0x0f, 0x00, 0x00, 0x67, 0x00, 0xde, 0xf3, 0x00, 0xe3, 0x01, 0x9e,
   0x0f, 0x38, 0x0f, 0x00, 0xfc, 0x0f, 0xe0, 0xff, 0xe7, 0xff, 0xdf, 0xf3,
   0xff, 0xe1, 0x01, 0x9e, 0x0f, 0x38, 0xfe, 0x1f, 0xfc, 0x1f, 0xf0, 0xff,
   0xe7, 0xff, 0xcf, 0xf3, 0xff, 0xe3, 0x01, 0x9e, 0x0f, 0x3c, 0xfc, 0x3f,
   0xfc, 0x1f, 0x70, 0xc0, 0xe7, 0xff, 0xc7, 0x71, 0x80, 0xe7, 0x01, 0x8e,
   0x0f, 0x3c, 0x00, 0x3f, 0xfc, 0x3f, 0x70, 0x00, 0xe7, 0xf9, 0xc1, 0x71,
   0x80, 0xe7, 0x01, 0x8e, 0x0f, 0x1c, 0x00, 0x38, 0x7c, 0x7e, 0xf0, 0x01,
   0xe7, 0xf1, 0xc0, 0xf1, 0xff, 0xe7, 0x01, 0x8e, 0xff, 0x1f, 0x1e, 0x38,
   0x3c, 0x7c, 0xf0, 0xff, 0xe7, 0xf1, 0xc0, 0xf1, 0xff, 0xe7, 0xff, 0x0f,
   0xff, 0x1f, 0xfe, 0x3f, 0x3c, 0xfc, 0xf0, 0xff, 0xe7, 0xe1, 0xc1, 0xf1,
   0xff, 0xc3, 0xff, 0x0f, 0xff, 0x1f, 0xfe, 0x3f, 0x3c, 0xf8, 0xe1, 0xff,
   0xe7, 0xe1, 0xc3, 0xf1, 0xff, 0x81, 0xff, 0x07, 0xfe, 0x0f, 0xfe, 0x1f,
   0x38, 0xf0, 0x03, 0x7f, 0xe7, 0xc1, 0xc7, 0xf1, 0xff, 0x00, 0xf8, 0x03,
   0xf8, 0x07, 0xf8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Declaration des broches d'ES pour les satellites
// Broches numeriques :
int pinEquipe = 5 ;     // 1 - PWM
int pinStrategie = 4 ;  // 2 - PWM
int pinTourette = 3 ;   // 3 - PWM
int pinValidation = 2 ; // 4
int pinTirette = 9 ;    // 5 - PWM
int digi_6 = 8 ;        // 6
int digi_7 = 7 ;        // 7
int digi_8 = 6;         // 8 - PWM
// Broches analogiques : ( Non utilsié ici )
// int ana_1 = A6 ; // 1 - pin 20 ou A6 - PWM
// int ana_2 = A7 ; // 2 - pin 21 ou A7 - PWM
// int ana_3 = A8 ; // 3 - pin 22 ou A8 - PWM
// int ana_4 = A9 ; // 4 - pin 23 ou A9 - PWM
// int ana_5 = A0 ; // 5 - pin 14 ou A0
// int ana_6 = A1 ; // 6 - pin 15 ou A1
// int ana_7 = A2 ; // 7 - pin 16 ou A2
// int ana_8 = A3 ; // 8 - pin 17 ou A3


bool equipe = vert, strategie = strategie1, tourette = noFuck, tirette = nok;
byte optionNavigation = 0;
int score = 0;
double timeInit=0;
bool statutMp3 = false;
bool tourettePrec = noFuck;
double tempsRestant = tempsMatch;
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0,13,11,12,U8X8_PIN_NONE);
DFRobotDFPlayerMini myDFPlayer;

int nbrBadCRC = 0   ; // Nombre de CRC érronés
int nbrBizarre = 0  ; // Nombre de réponses bizarres

FastCRC8 CRC8;
byte bufNavRelatif[5]={0,0,0,0,0}; // Buffer d'envoi des ordres de navigation relatifs
byte crcNavRelatif = 0; // CRC de controle pour les ordres de navigation relatifs

byte bufNavAbsolu[6]={0,0,0,0,0,0}; // Buffer d'envoi des ordres de navigation absolus
byte crcNavAbsolu = 0 ; // CRC de controle pour les ordres de navigation absolus

byte bufAction[1] = {0} ; // Buffer d'envoi des ordres de navigation absolus
byte crcAction = 0      ; // CRC de controle pour les ordres de navigation absolus


void draw();
// Gestion Bouton IHM
void bouttonIHM();

void setup();

//BOUCLE----------------
void loop();

//GESTION DES BOUTTONS DE L'IHM----------------
void bouttonIHM();

//TEST DE DEPLACEMENT----------------
void testDeplacement();

//STRATEGIE D'HOMOLOGATION----------------
void Homologation();
void chateauFirst();
void testBarillet();

//DEMANDE L'ETAT DU DEPLACEMENT----------------
int askNavigation();

//DEMANDE L'ETAT DE L'ACTION----------------
int askAction();

//ENVOI UNE COMMANDE DE DEPLACEMENT ABSOLU----------------
void sendNavigation(byte fonction, int X, int Y, int rot);

//ENVOI UNE COMMANDE DE DEPLACEMENT RELATIF----------------
void sendNavigation(byte fonction, int rot, int dist);

//ENVOI UNE COMMANDE A LA PARTIE ACTION------------
void sendAction(byte actionRequested);

//PROCEDURE DE MAJ DU SCORE----------------
void majScore(int points, int multiplicateur);

//MISE A JOUR DU TEMPS DE MATCH----------------
void majTemps();

//PROCEDURE D'ATTENTE----------------
void attente(int temps);

//ENVOI UNE COMMANDE TURN GO----------------
void turnGo(bool recalage,bool ralentit,int turn, int go);

//ENVOI UNE ACTION----------------
void action(byte action);

//PROCEDURE DE FIN DE MATCH----------------
void finMatch();

//GESTION DES PAGES LCD-------------------
void u8g2_prepare();

void u8g2_splash_screen();

void u8g2_menu_pendant_match();

void u8g2_menu_avant_match();

void u8g2_splash_screen_GO();
