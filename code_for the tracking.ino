#include <ESP32Servo.h>
#include <TimeLib.h>
#include "esp_sleep.h"

#define capteur1Pin 32
#define capteur2Pin 33
#define capteur3Pin 34

#define SERVO_PIN_1 12
#define SERVO_PIN_2 13

Servo servo1;
Servo servo2;

// Heures de démarrage et d'arrêt en fonction des saisons
int startHour, stopHour;

// Tableaux pour stocker les valeurs des capteurs sur les 30 dernières minutes
int capteur1Values[30];
int capteur2Values[30];
int capteur3Values[30];
int index = 0;


void lireCapteurs(int &capteur1, int &capteur2, int &capteur3) {
  // Lecture des valeurs des capteurs
  capteur1 = analogRead(capteur1Pin);
  capteur2 = analogRead(capteur2Pin);
  capteur3 = analogRead(capteur3Pin);
}

// Mise à jour des valeurs des capteurs toutes les minutes
void mettreAJourValeursCapteurs() {
  capteur1Values[index] = analogRead(capteur1Pin);
  capteur2Values[index] = analogRead(capteur2Pin);
  capteur3Values[index] = analogRead(capteur3Pin);
  index = (index + 1) % 30;
}

// Fonction pour calculer la moyenne des valeurs des capteurs sur les 30 dernières minutes
void calculerMoyenneCapteurs(int &capteur1Moyenne, int &capteur2Moyenne, int &capteur3Moyenne) {
  // Initialisation des sommes
  long sum1 = 0;
  long sum2 = 0;
  long sum3 = 0;
  
  // Calcule de la somme des valeurs des capteurs
  for (int i = 0; i < 30; i++) {
    sum1 += capteur1Values[i];
    sum2 += capteur2Values[i];
    sum3 += capteur3Values[i];
  }
  
  // Calcule de la moyenne des valeurs des capteurs
  capteur1Moyenne = sum1 / 30;
  capteur2Moyenne = sum2 / 30;
  capteur3Moyenne = sum3 / 30;
}

void setup() {
  servo1.setPeriodHertz(50); 
  servo1.attach(SERVO_PIN_1, 500, 2400); 

  servo2.setPeriodHertz(50); 
  servo2.attach(SERVO_PIN_2, 500, 2400); 

  
  Serial.begin(115200);

}

void loop() {
  // Recherche de l'heure actuelle
  int currentHour = hour();

  // Détermination des heures de démarrage et d'arrêt en fonction des saisons
  int season = getSeason(month());
  if (season == 1) { // Été
    startHour = 9;
    stopHour = 20;
  } else if (season == 2) { // Hiver
    startHour = 8;
    stopHour = 17;
  } else { // Automne et Printemps
    startHour = 8;
    stopHour = 18;
  }
    lireCapteurs(capteur1Value, capteur2Value, capteur3Value);

  // CONDITIONS
 if (currentHour >= startHour && currentHour < stopHour) {
    //servo 1 controle l'inclinaison et servo2 controle la rotation 
    if (capteur1Value > capteur2Value && capteur1Value > capteur3Value) { //cas improbable car non paramétré pour ça, le moteur est ici mal placé
      while (capteur1Value > capteur2Value && capteur1Value > capteur3Value) {
        servo2.write(-1);
        lireCapteurs(capteur1Value, capteur2Value, capteur3Value);
      }
    } else if (capteur1Value > capteur2Value && capteur3Value > capteur2Value) { // Incliner les plaques 
      while (capteur1Value > capteur2Value && capteur3Value > capteur2Value) {
        servo1.write(1);
        lireCapteurs(capteur1Value, capteur2Value, capteur3Value);
      }
    } else if (capteur1Value > capteur2Value && capteur2Value > capteur3Value) { //Cas en fin de journée
      while (capteur1Value > capteur2Value && capteur2Value > capteur3Value) {
        servo1.write(-1);
        lireCapteurs(capteur1Value, capteur2Value, capteur3Value);
      }
    } else if (capteur3Value > capteur2Value && capteur2Value > capteur1Value) { //Cas en hiver/Automne où le soleil est vraiment bas à l'horizon 
      while (capteur3Value > capteur2Value && capteur2Value > capteur1Value) {
        servo2.write(1);
        lireCapteurs(capteur1Value, capteur2Value, capteur3Value);
      }
    } else if (capteur3Value > capteur2Value && capteur3Value > capteur1Value) { //Cas en hiver où le soleil est plus incliné, en début de journée
      while (capteur3Value > capteur2Value && capteur3Value > capteur1Value) {
        servo2.write(1);
        lireCapteurs(capteur1Value, capteur2Value, capteur3Value);
      }
    } else if (capteur2Value > capteur3Value && capteur2Value >= capteur1Value) { //Cas en hiver où le soleil est plus incliné, en fin de journée
      while (capteur2Value > capteur3Value && capteur2Value >= capteur1Value) {
        servo2.write(1);
        lireCapteurs(capteur1Value, capteur2Value, capteur3Value);
      }
    }
    else if (capteur1Value == capteur2Value && capteur2Value == capteur3Value) {
    // Mettre l'ESP32 en veille
    esp_sleep_enable_timer_wakeup(30 * 60 * 1000000); // 30 minutes en microsecondes
    esp_deep_sleep_start();
  }
    
  } else { //Pas la bonne plage horaire
    // Mettre les moteurs en position initiale
    servo1.write(0);
    servo2.write(0);
  }
}

// Fonction pour déterminer la saison en fonction du mois
int getSeason(int month) {
  if (month >= 6 && month <= 8) // Été : Juin à Août
    return 1;
  else if (month >= 12 || month <= 2) // Hiver : Décembre à Février
    return 2;
  else // Automne et Printemps : Septembre à Novembre et Mars à Mai
    return 3;
}
