#include "Action.h"


void setup()
{
  pinMode(pinReset1, OUTPUT);
	pinMode(pinSleep1, OUTPUT);

	digitalWrite(pinReset1, HIGH);
  // Desactiver le controleur au demarrage pour ne pas faire chauffer le moteur
	digitalWrite(pinSleep1, LOW) ;

	Serial.begin(9600) ;
	Wire.begin(ADRESSE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

  MBarillet.setMaxSpeed(VitesseMaxBarillet);
  MBarillet.setAcceleration(AccelMax);
  // Declaration de la pin moteur des balles
  pinMode(moteurBalles,OUTPUT);
  // Declaration des pin servo
  brasGauche.attach(ana_5)  ;
  brasDroit.attach(ana_6)   ;
  barriere.attach(ana_7)    ;
  selecteur.attach(ana_8)   ;
  // Mise a zero des positions servomoteurs
  brasGauche.write(hautBrasGauche)      ;
  brasDroit.write(hautBrasDroit)        ;
  barriere.write(hautBarriere)          ;
  selecteur.write(positionSelecteur[0]);

}

void loop()
{
  updateAction();
  //updateBalise();
  executeAction();
	if (actionRequest == 255) finMatch();
}

void updateAction()
{
	if (newAction==DISPONIBLE)
	{
		etatAction = PREVU ;
	}
	newAction = VALIDEE;
}

void executeAction()
{
  if (etatAction==PREVU)
  {
    // L'action passe en cours d'execution
    etatAction=EN_COURS;
    // On réinitialise l'index d'action
    indexAction = 0;
  }
  if (etatAction==EN_COURS)
  {
    switch (actionRequest)
    {
      case BD_HAUT:
        actionBras();
      break;
      case BD_BAS:
        actionBras();
      break;
      case BG_HAUT:
        actionBras();
      break;
      case BG_BAS:
        actionBras();
      break;
      case RECUP_BALLES_COMPLET:
        actionRecuperationComplet();
      break;
      case RECUP_BALLES_SAFE:
        actionRecuperationSafe();
      break;
      case ENVOI_BALLES:
        actionEnvoiBalles();
      break;
      default:
      // statements
      break;
    }
  }
}

void actionBras()
{
  switch(indexAction)
  {
    case 0:
      if      (actionRequest==BD_HAUT)  brasDroit.write(hautBrasDroit)  ;
      else if (actionRequest==BD_BAS)   brasDroit.write(basBrasDroit)   ;
      else if (actionRequest==BG_HAUT)  brasGauche.write(hautBrasGauche);
      else if (actionRequest==BG_BAS)   brasGauche.write(basBrasGauche) ;
      indexAction++;
    break;
    case 1:
      // Attente
      indexAction++;
    break;
    case 2:
      // FIN de l'action
      etatAction=FINI;
      indexAction++;
    break;
  }
}

void actionEnvoiBalles()
{
  switch(indexAction)
  {
    case 0:
      // Positionner le selecteur
      selecteur.write(positionSelecteur[0]);
      indexAction++;
    break;
    case 1:
      // Attente que le moteur soit à une certaine vitesse
      if (!accelerationMoteur()) indexAction++;
    break;
    case 2:
      // lever la barriere
      barriere.write(hautBarriere);
      indexAction++;
    break;
    case 3:
      // Attendre l'envoi de la balle
      if (!attente(500)) indexAction++;
    break;
    case 4:
      // selectionner la balle suivante
      nbrBalles++;
      barriere.write(basBarriere);
      digitalWrite(pinSleep1, HIGH);
      MBarillet.move(sequenceBarilletEnvoi[0]);
      indexAction++;
    break;
    case 5:
      // Attendre la fin du mouvement du barillet
      if(!MBarillet.run())
      {
        if (nbrBalles<=4) indexAction=2;
        else indexAction++ ;
        digitalWrite(pinSleep1, LOW);
      }
    break;
    case 6:
      // Fin de l'actions
      analogWrite(moteurBalles,0);
      etatAction=FINI;
      indexAction++;
    break;
  }
}

void actionRecuperationComplet()
{
  switch(indexAction)
  {
    case 0:
      //
      barriere.write(basBarriere);
      selecteur.write(positionSelecteur[0]);
      digitalWrite(pinSleep1, HIGH);
      MBarillet.move(sequenceBarilletComplet[0]);
      indexAction++;
    break;
    case 1:
      // Attendre la fin du mouvement du barillet
      if(!MBarillet.run())
      {
        indexAction++;
        digitalWrite(pinSleep1, LOW);
      }
    break;
    case 2:
      //
      digitalWrite(pinSleep1, HIGH);
      MBarillet.move(sequenceBarilletComplet[1]);
      indexAction++;
    break;
    case 3:
      // Attendre la fin du mouvement du barillet
      if(!MBarillet.run())
      {
        indexAction++;
        digitalWrite(pinSleep1, LOW);
      }
    break;
    case 4:
      //
      selecteur.write(positionSelecteur[1]);
      digitalWrite(pinSleep1, HIGH);
      MBarillet.move(sequenceBarilletComplet[2]);
      indexAction++;
    break;
    case 5:
      // Attendre la fin du mouvement du barillet
      if(!MBarillet.run())
      {
        indexAction++;
        digitalWrite(pinSleep1, LOW);
      }
    break;
    case 6:
      selecteur.write(positionSelecteur[2]);
      digitalWrite(pinSleep1, HIGH);
      MBarillet.move(sequenceBarilletComplet[3]);
      indexAction++;
    break;
    case 7:
      // Attendre la fin du mouvement du barillet
      if(!MBarillet.run())
      {
        indexAction++;
        digitalWrite(pinSleep1, LOW);
      }
    break;
    case 8:
      //
      indexAction++;
    break;
    case 9:
      // Fin de l'actions
      etatAction=FINI;
      indexAction++;
    break;
  }
}

void actionRecuperationSafe()
{
  switch(indexAction)
  {
    case 0:
      //
      barriere.write(basBarriere);
      selecteur.write(positionSelecteur[0]);
      digitalWrite(pinSleep1, HIGH);
      MBarillet.move(sequenceBarilletSafe[0]);
      indexAction++;
    break;
    case 1:
      // Attendre la fin du mouvement du barillet
      if(!MBarillet.run())
      {
        indexAction++;
        digitalWrite(pinSleep1, LOW);
      }
    break;
    case 2:
      //
      if (!attente(1000)) indexAction++;
    break;
    case 3:
      //
      selecteur.write(positionSelecteur[1]);
      digitalWrite(pinSleep1, HIGH);
      MBarillet.move(sequenceBarilletSafe[1]);
      indexAction++;
    break;
    case 4:
      // Attendre la fin du mouvement du barillet
      if(!MBarillet.run())
      {
        indexAction++;
        digitalWrite(pinSleep1, LOW);
      }
    break;
    case 5:
      //
      if (!attente(1000)) indexAction++;
    break;
    case 6:
      //
      digitalWrite(pinSleep1, HIGH);
      MBarillet.move(sequenceBarilletSafe[2]);
      indexAction++;
    break;
    case 7:
      // Attendre la fin du mouvement du barillet
      if(!MBarillet.run())
      {
        indexAction++;
        digitalWrite(pinSleep1, LOW);
      }
    break;
    case 8:
      //
      if (!attente(1000)) indexAction++;
    break;
    case 9:
      selecteur.write(positionSelecteur[0]);
      indexAction++;
    break;
    case 10:
      // Fin de l'actions
      etatAction=FINI;
      indexAction++;
    break;
  }
}

void receiveEvent(int howMany)
{
	if(howMany == 2)
	{
		// Si un déplacement relatif est demandé
		// On receptionne les données
		for (int i=0;i<=1;i++)
		{
			bufAction[i]=Wire.read();
		}
	}
	// On calcul le CRC
	crcAction = CRC8.smbus(bufAction, sizeof(bufAction)-1); //On enleve le CRC
	//Serial.println(crcAction);
	// On regarde si le CRC calculé correspond à celui envoyé
	if (crcAction==bufAction[1])
	{
		// CRC ok
		// On traite les données
		actionRequest = bufAction[0];
		// On indique qu'une nouvelle action est disponible
		newAction = DISPONIBLE;
	}
	else
	{
		// CRC nok - la donnée est erronée
		// On indique que la prochaine action est erronée pour en renvoi eventuel
		newAction = ERRONEE;
	}

}

void updateBalise()
{
	baliseA.update();
  if (baliseState == 0)
  {
    if (baliseA.readPosition() == gaucheBalise)
    {
		baliseA.setDestination(droiteBalise, 1000);
    }
    else
    {
		baliseA.setDestination(gaucheBalise, 1000);
    }
    baliseState = 1 ;
  }
  if (baliseState == 1 && !baliseA.update())
  {
      baliseState = 0 ;
  }
}

bool attente(int temps)
{
  if (!indexTemps)
  {
    initTemps = millis();
    indexTemps = true   ;
  }
	else if( (millis()-initTemps) >= temps)
  {
    indexTemps = false;
  }
  return(indexTemps);
}

bool accelerationMoteur()
{
  if (indexAccMoteur <= vitMaxBalles)
  {
    indexAccMoteur++ ;
    analogWrite(moteurBalles,indexAccMoteur);
    delay(50);
    return true;
  }
  else
  {
    indexAccMoteur = vitMinBalles ;
    return false;
  }

}

//Fin de match
void finMatch()
{
   	while(1)
   	{
		    // Couper tous les servomoteurs
   	}
}

void requestEvent()
{

	if ( etatAction == FINI && newAction == VALIDEE)
  {
    // Action terminée
		Wire.write("O");
		//Serial.println('O');
	}
	else if (newAction == ERRONEE)
	{
    // Commande non validé
		Wire.write("E");
		//Serial.println('N');
	}
	else
	{
    // Action non terminée
		Wire.write("N");
		//Serial.println('N');
	}
}
