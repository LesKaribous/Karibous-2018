#include "Strategie.h"


void setup()
{
  pinMode(pinEquipe, INPUT_PULLUP);
  pinMode(pinStrategie, INPUT_PULLUP);
  pinMode(pinTourette, INPUT_PULLUP);
  pinMode(pinValidation, INPUT_PULLUP);
  pinMode(pinTirette,INPUT_PULLUP);

	u8g2.begin();
	// Logo des Karibous
	u8g2_splash_screen();
	Wire.begin();
	Serial.begin(9600);
	SerialPlayer.begin(9600);
	delay(2000);
	// Initialisation du MP3
	if (!myDFPlayer.begin(SerialPlayer)) statutMp3 = false;
	else statutMp3 = true;
  myDFPlayer.setTimeOut(500);
	myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  //myDFPlayer.playFolder(1, 2); //play specific mp3 in SD:/02/004.mp3; Folder Name(1~10); File Name(1~255)
  // Gestion tirette
  while (digitalRead(pinTirette))
  {
    // Menu d'avant Match
    bouttonIHM();
    u8g2_menu_avant_match();
  }
  while (!digitalRead(pinTirette))
  {
    // Menu d'avant Match
    bouttonIHM();
    u8g2_menu_avant_match();
  }
	// Lancement du Match
	timeInit = millis();
	u8g2_splash_screen_GO();
	delay (100);
}

//----------------BOUCLE----------------
void loop()
{
	//testDeplacement();
  //chateauFirst();
  Homologation();
  //testBarillet();
}

//----------------GESTION DES BOUTTONS DE L'IHM----------------
void bouttonIHM()
{
  equipe = digitalRead(pinEquipe);
  strategie = digitalRead(pinStrategie);
  tourette = digitalRead(pinTourette);
  tirette = digitalRead(pinTirette);
}

//----------------TEST DE DEPLACEMENT----------------
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

//----------------STRATEGIE D'HOMOLOGATION----------------
void Homologation()
{
  //PANNEAU DOMOTIQUE + ABEILLE + BALLES
  majScore(deposeAbeille, 1);
  majScore(deposePanneau, 1);
	turnGo(0,false,0,900);
	turnGo(0,true,90,150);
  majScore(activePanneau, 1);
	turnGo(0,false,0,-1000);
	turnGo(0,false,-45,-930);
	turnGo(0,false,-45,-230);
	turnGo(0,true,0,-140);             // Recalage bordure
  if (equipe==vert) action(BD_BAS);
  else action(BG_BAS);
	turnGo(0,false,0,300);
  majScore(activeAbeille, 1);
  if (equipe==vert) action(BD_HAUT);
  else action(BG_HAUT);
  attente(300);
	turnGo(0,true,90,-120);
  turnGo(0,false,0,1045);
  turnGo(0,true,90,280);
	// turnGo(0,false,0,-480);
	// turnGo(0,false,90,500);
	// turnGo(0,true,0,220); // recalage Tube
	// turnGo(0,false,0,-200);
	// turnGo(0,false,90,800);
  while(1);
}
void chateauFirst()
{
  //CHATEAU + ABEILLE
  majScore(deposeAbeille, 1);
  majScore(deposePanneau, 1);
	turnGo(0,false,0,150);
  turnGo(0,false,-90,350);
  turnGo(0,false,-90,220);
  action(RECUP_BALLES_SAFE);
  turnGo(0,false,0,-300);
  turnGo(0,false,-75,0);
  action(ENVOI_BALLES);
  turnGo(0,false,-15,-1000);
  turnGo(0,true,0,-50);
  turnGo(0,false,0,70);
  turnGo(0,false,-90,-500);
  turnGo(0,true,0,-40);
  if (equipe==vert) action(BD_BAS);
  else action(BG_BAS);
	turnGo(0,false,0,300);
  majScore(activeAbeille, 1);
  if (equipe==vert) action(BD_HAUT);
  else action(BG_HAUT);
  while(1);
}
void testBarillet()
{
  //action(RECUP_BALLES_COMPLET);
  action(RECUP_BALLES_SAFE);
  action(ENVOI_BALLES);
  while(1);
}

//----------------DEMANDE L'ETAT DU DEPLACEMENT----------------
int askNavigation()
{
  int etatNavigation ;
  Wire.requestFrom(carteDeplacement, 1);
  char reponseNavigation = Wire.read();
  if (reponseNavigation=='N') etatNavigation = RECU ;
  else if (reponseNavigation=='O') etatNavigation = TERMINEE ;
  else etatNavigation = ERRONEE ;
	return etatNavigation;
}

//----------------ENVOI UNE COMMANDE DE DEPLACEMENT ABSOLU----------------
void sendNavigation(byte fonction, int X, int Y, int rot)
{
	// Stockage des valeurs à envoyer dans le buffer
  bufNavAbsolu[0]=fonction;
	bufNavAbsolu[1]=rot >> 8;
	bufNavAbsolu[2]=rot & 255;
	bufNavAbsolu[3]=X >> 8;
	bufNavAbsolu[4]=X & 255;
	bufNavAbsolu[5]=Y >> 8;
	bufNavAbsolu[6]=Y & 255;

	// Calcul du CRC
	crcNavRelatif = CRC8.smbus(bufNavAbsolu, sizeof(bufNavAbsolu));
	//Serial.println(crcNavRelatif);

	// Envoi des données
	Wire.beginTransmission(carteDeplacement);
	for(int i=0;i<=6;i++)
	{
		Wire.write(bufNavRelatif[i]);
	}
	Wire.write(crcNavRelatif);
	Wire.endTransmission();

}

//----------------ENVOI UNE COMMANDE DE DEPLACEMENT RELATIF----------------
void sendNavigation(byte fonction, int rot, int dist)
{
	if ( equipe == vert ) rot = -rot ;
	// Stockage des valeurs à envoyer dans le buffer
	bufNavRelatif[0]=fonction;
	bufNavRelatif[1]=rot >> 8;
	bufNavRelatif[2]=rot & 255;
	bufNavRelatif[3]=dist >> 8;
	bufNavRelatif[4]=dist & 255;
	// Calcul du CRC
	crcNavRelatif = CRC8.smbus(bufNavRelatif, sizeof(bufNavRelatif));
	//Serial.println(crcNavRelatif);
	// Envoi des données
	Wire.beginTransmission(carteDeplacement);
	for(int i=0;i<=4;i++)
	{
		Wire.write(bufNavRelatif[i]);
	}
	Wire.write(crcNavRelatif);
	Wire.endTransmission();
}

//----------------ENVOI UNE COMMANDE A LA PARTIE ACTION------------
void sendAction(byte actionRequested)
{
	// Stockage des valeurs à envoyer dans le buffer
	bufAction[0] = actionRequested;
	// Calcul du CRC
	crcAction = CRC8.smbus(bufAction, sizeof(bufAction));
	//Serial.println(crcAction);
	// Envoi des données
	Wire.beginTransmission(carteActionneur);
	Wire.write(bufAction[0])  ;
	Wire.write(crcAction)  ;
	Wire.endTransmission() ;
}

//----------------PROCEDURE DE MAJ DU SCORE----------------
void majScore(int points, int multiplicateur)
{
	score = score + (points*multiplicateur);
}

//----------------MISE A JOUR DU TEMPS DE MATCH----------------
void majTemps()
{
  tempsRestant = ( tempsMatch - (millis() - timeInit) ) / 1000;
  if ( tempsRestant <= 0 )
  {
    finMatch();
  }
}

//----------------PROCEDURE D'ATTENTE----------------
void attente(int temps)
{
	int initTemps = millis();
	while( (millis()-initTemps) <= temps)
	{
		majTemps();
		u8g2_menu_pendant_match();
	}
}

//----------------DEMANDE L'ETAT DU DEPLACEMENT----------------
int askAction()
{
  int etatAction ;
  Wire.requestFrom(carteActionneur, 1);
  char reponseAction = Wire.read();
  if (reponseAction=='N') etatAction = RECU ;
  else if (reponseAction=='O') etatAction = TERMINEE ;
  else if (reponseAction=='E') etatAction = ERRONEE ;
  else etatAction = BIZARRE ;
	return etatAction;
}

//----------------ENVOI UNE COMMANDE D'ACTION----------------
void action(byte action)
{
  int reponseAction ;
	sendAction(action);
	attente(400);
  reponseAction = askAction();
  // Tant que l'action n'est pas terminée
	while(reponseAction!=TERMINEE)
	{
		attente(400);
    if ((reponseAction==ERRONEE))
    {
      sendAction(action); // l'action est erronée, on renvois la donnée
      nbrBadCRC++;
    }
    reponseAction = askAction();
		//Serial.println(askAction());
	}
}

//----------------ENVOI UNE COMMANDE TURN GO----------------
void turnGo(bool recalage,bool ralentit,int turn, int go)
{
  int reponseNavigation ;
	bitWrite(optionNavigation,0,false); // false -> la detection adverse est active
	bitWrite(optionNavigation,1,recalage);
	bitWrite(optionNavigation,2,ralentit);
	sendNavigation(optionNavigation, turn, go);
	attente(400);
  reponseNavigation = askNavigation();
	while(reponseNavigation!=TERMINEE)
	{
		attente(400);
    if (reponseNavigation==ERRONEE)
    {
      sendNavigation(optionNavigation, turn, go);
      nbrBadCRC++;
    }
    reponseNavigation = askNavigation();
		//Serial.println(askNavigation());
	}
}

//----------------PROCEDURE DE FIN DE MATCH----------------
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

//----------------GESTION DES PAGES LCD-------------------
void u8g2_prepare() {
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void u8g2_splash_screen() {
  u8g2.clearBuffer();
  u8g2.drawXBMP( 0, 15, LOGO_KARIBOUS_width, LOGO_KARIBOUS_height, LOGO_KARIBOUS_bits);
  u8g2.sendBuffer();
}

void u8g2_menu_pendant_match() {
	u8g2.clearBuffer();
	u8g2_prepare();
  u8g2.setFont(u8g2_font_inr42_mn);
  u8g2.setCursor(8, 9);
  u8g2.print(score);
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr( 0, 0, "Score:");
  u8g2.drawStr( 70, 0, "Temps:    sec");
  u8g2.setCursor(95, 0);
  u8g2.print(tempsRestant);
  u8g2.drawStr( 105, 57, "points");
  u8g2.drawStr( 0, 57, "CRC:");
  u8g2.setCursor(20, 57);
  u8g2.print(nbrBadCRC);
	u8g2.sendBuffer();
}

void u8g2_menu_avant_match() {
  u8g2.clearBuffer();
  u8g2_prepare();
    u8g2.setFont(u8g2_font_4x6_tf);
    // Etat mp3 :
    u8g2.drawStr( 0, 0, "Etat MP3:");
    if ( statutMp3 )
    {
      u8g2.drawStr( 40, 0, "OK");
    }
    else
    {
      u8g2.drawStr( 40, 0, "Echec");
    }
    // Etat equipe :
    u8g2.drawStr( 0, 10, "Equipe :");
    if ( equipe == vert )
    {
      u8g2.drawStr( 40, 10, "VERT");
    }
    else
    {
      u8g2.drawStr( 40, 10, "ORANGE");
    }
    // Etat strategie :
    u8g2.drawStr( 0, 20, "Strategie :");
    if ( strategie == strategie1 )
    {
      u8g2.drawStr( 50, 20, "NORMAL");
    }
    else
    {
      u8g2.drawStr( 50, 20, "ATTENTION");
    }
    // Etat tourette :
    u8g2.drawStr( 0, 30, "tourette :");
    if ( tourette == noFuck )
    {
      u8g2.drawStr( 40, 30, "POLIE");
      if(tourettePrec)
      {
        myDFPlayer.playFolder(1, 4);
        tourettePrec = false ;
      }
    }
    else
    {
      u8g2.drawStr( 40, 30, "PUTE PUTE PUTE PUTE PUTE");
      if(!tourettePrec)
      {
        myDFPlayer.playFolder(1, 2);
        tourettePrec = true;
      }
    }
    // Etat tirette :
    u8g2.drawStr( 0, 40, "Etat tirette :");
    if ( tirette == nok )
    {
      u8g2.drawStr( 60, 40, "Attente tirette");
    }
    else
    {
      u8g2.drawStr( 60, 40, "Tirette OK");
    }
  u8g2.sendBuffer();
}

void u8g2_splash_screen_GO() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso58_tr);
  u8g2.drawStr( 18, 2, "GO!");
  u8g2.sendBuffer();
}
