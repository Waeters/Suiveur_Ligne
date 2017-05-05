/*
 * Code de démo suiveur de ligne (line-following) pour le Robot Zumo de Polulo
 *
 * Ce code suivra une ligne noire sur un fond blan et utilise un 
 * algorithme de type PID.  Il fonctionne correctement sur des 
 * circuits avec des courbes ayant un rayon de 15 cm.
 * L'algorithme à été testé sur Zumo avec des moteurs 30:1 HP et
 * 75:1 HP.  Pourrait demander des modifications pour fonctionner
 * sur d'autres circuits ou avec d'autres moteurs.
 *
 * http://www.pololu.com/catalog/product/2506
 * http://www.pololu.com
 * http://forum.pololu.com
 *
 * Zumo également disponible chez MC Hobby (le traducteur du tutoriel)
 *
 * http://shop.mchobby.be/product.php?id_product=448
 */

#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>


ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
int lastError = 0;

// Ceci est la vitesse de rotation maximale des moteurs.
// (400 permet au moteur d'aller a vitesse max; diminuer la valeur pour imposer une vitesse limite)
const int MAX_SPEED = 400;


void setup()
{
  // Jouer une petite chanson d'acceuil
  buzzer.play(">g32>>c32");

  // Initialise le réseau de senseur infrarouge
  reflectanceSensors.init();

  // Attendre que le bouton utilisateur soit pressé et relâché
  button.waitForButton();

  // Allumer la LED et pour indiquer que l'on est en mode calibration
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  // Attendre 1 seconde puis démarrer la calibration automatique
  // du senseur en faisant des rotations sur place pour faire passer
  // le senseur au dessus de la ligne
  delay(1000);
  int i;
  for(i = 0; i < 80; i++)
  {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-200, 200);
    else
      motors.setSpeeds(200, -200);
    reflectanceSensors.calibrate();

    // Puisque notre compteur va jusque 80, le délais total sera de
    // 80*20 = 1600 ms.
    delay(20);
  }
  motors.setSpeeds(0,0);

  // Eteindre la LED pour indiquer que nous avons terminé la calibration
  digitalWrite(13, LOW);
  buzzer.play(">g32>>c32");

  // Attendre que le bouton utilisateur soit pressé et relâché
  button.waitForButton();

  // Jouer la musique et attendre qu'elle soit finie pour 
  // commencer le pilotage.
  buzzer.play("L16 cdegreg4");
  while(buzzer.isPlaying());
}

void loop()
{
  unsigned int sensors[6];

  // Obtenir la position de la ligne.  Notez qu'il FAUT fournit le senseur "sensors"
  // en argument à la fonction readLine(), même si nous ne sommes intéressé
  // par les lectures individuelles des différents senseurs.
  int position = reflectanceSensors.readLine(sensors);

  // L'erreur ("error") est la distance par rapport au centre de la ligne, qui 
  // correspond à la position 2500.
  int error = position - 2500;

  // Calculer la différence de vitesse (speedDifference) entre les moteurs 
  // en utilisant un les termes proportionnels et dérivés du régulateur PID.
  // (Le terme intégral n'est généralement pas très utile dans le 
  // suivit de ligne).
  // Nous utiliserons 1/4 pour la constante proportionnelle et 6 pour la 
  // constante dérivée 6, qui devrait fonctionner correctement avec de 
  // nombreux choix de Zumo.
  // Vous aurez probablement besoin d'ajuster ces constantes par
  // essai/erreur pour votre zumo et/ou le circuit.
  int speedDifference = error / 4 + 6 * (error - lastError);

  lastError = error;

  // Calculer la vitesse de chaque moteur. Le signe de la différence (speedDifference)
  // determine si le moteur tourne à gauche ou a droite.
  int m1Speed = MAX_SPEED + speedDifference;
  int m2Speed = MAX_SPEED - speedDifference;

  // Nous allons contraindre la vitesse des moteurs entre 0 et MAX_SPEED.
  // D'une façon générale, un des moteurs est toujours à MAX_SPEED
  // et l'autre sera à MAX_SPEED-|speedDifference| si elle est positif,
  // sinon il sera en vitesse stationnaire. Pour certaines applications, 
  // vous pourriez désirer une vitesse négative, ce qui permettrai de
  // tourner à l'envers.
  if (m1Speed < 0)
    m1Speed = 0;
  if (m2Speed < 0)
    m2Speed = 0;
  if (m1Speed > MAX_SPEED)
    m1Speed = MAX_SPEED;
  if (m2Speed > MAX_SPEED)
    m2Speed = MAX_SPEED;

  motors.setSpeeds(m1Speed, m2Speed);
}
