#include <Arduino.h>
#include <Servo.h>

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
int hautBrasGauche = 118 ;
int basBrasGauche = 20 ;
int hautBrasDroit = 30 ;
int basBrasDroit = 120 ;
int hautBarriere = 120 ;
int basBarriere = 40 ;


void setup() {
    // Declaration des pin servo
    brasGauche.attach(ana_1);
    brasDroit.attach(ana_2);
    barriere.attach(ana_3);
    balise.attach(ana_4);
    // Mise a zero des positions servomoteurs
    brasGauche.write(hautBrasGauche);
    brasDroit.write(hautBrasDroit);
    barriere.write(hautBarriere);
    balise.write(gaucheBalise);
}

void loop() {
    // put your main code here, to run repeatedly:
}
