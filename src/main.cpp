#include <Arduino.h>
#include "ServoEasing.hpp"

// Criação dos objetos da biblioteca ServoEasing
ServoEasing eyeLidServo;
ServoEasing upDownServo;
ServoEasing leftRightServo;

// Pinos (para Arduino ou outra placa)
int eyeLidPin     = 8;
int upDownPin     = 6;
int leftRightPin  = 7;

// Limites dos servos
// 1) Pálpebra
int eyeLidServoLower   = 40;   // Posição "fechada"
int eyeLidServoUpper   = 140;  // Posição "aberta"

// 2) Movimento vertical
int upDownServoLower   = 90;
int upDownServoUpper   = 50;
int upDownServoCentre  = 75;

// 3) Movimento horizontal
int leftRightServoLower  = 120;   // Menor valor
int leftRightServoUpper  = 50;  // Maior valor
int leftRightServoCentre = 90;  // Centro
// -----------------------------------------------------------------------
// Pisca (fecha e abre) a pálpebra usando easing
void blink(int closeSpeed, int openSpeed, int closeDelay, int openPosition) {
  // Fecha (eyeLidServoUpper)
  eyeLidServo.easeTo(eyeLidServoUpper, closeSpeed);
  while (ServoEasing::areInterruptsActive()) {
    delay(20);
  }

  // Espera closeDelay, se necessário
  if (closeDelay > 0) {
    delay(closeDelay);
  }

  // Abre (openPosition)
  eyeLidServo.startEaseTo(openPosition, openSpeed);
  // Aqui usamos startEaseTo(), mas se quiser bloquear, podemos aguardar:
  while (ServoEasing::areInterruptsActive()) {
    delay(20);
  }
}
// -----------------------------------------------------------------------
// Move o olho (2 servos) para posição (posLR, posUD) usando "speed" do ServoEasing
void moveEyeBall(int speed, int posLR, int posUD) {
  // Garante que a posição está dentro dos limites
  posLR = constrain(posLR, leftRightServoLower, leftRightServoUpper);
  posUD = constrain(posUD, upDownServoLower,   upDownServoUpper);

  // Inicia movimento "easing" simultâneo nos dois servos
  leftRightServo.setEaseTo(posLR, speed);
  upDownServo.setEaseTo(posUD, speed);

  // Sincroniza e inicia a interrupção (movimento simultâneo)
  setEaseToForAllServosSynchronizeAndStartInterrupt(speed);

  // Aguardar até terminar a animação
  while (ServoEasing::areInterruptsActive()) {
    delay(20);
  }
}

// -----------------------------------------------------------------------
// Função para gerar posição aleatória, com maior chance de ficar perto do "centro"
int getRndEyePos(int lowerLimit, int upperLimit, int centrePos, float centreFactor) {
  int range = (upperLimit - lowerLimit);

  float centreMin = centrePos - (range * centreFactor) / 2.0;
  float centreMax = centrePos + (range * centreFactor) / 2.0;

  // Garante que centreMin/centreMax fiquem nos limites
  centreMin = constrain((int)centreMin, lowerLimit, upperLimit);
  centreMax = constrain((int)centreMax, lowerLimit, upperLimit);

  // 2 em 3 vezes escolhe um ponto entre centreMin e centreMax
  if (random(0, 3)) {
    return random((int)centreMin, (int)centreMax + 1);
  } else {
    // 1 em 3 vezes escolhe qualquer valor entre lowerLimit e upperLimit
    return random(lowerLimit, upperLimit + 1);
  }
}


// -----------------------------------------------------------------------
// Rotina principal de animação do olho
void eyeMotion() {
  long currTime = millis();

  // Tempo de piscar
  int eyeOpenShort    = 300;   // Piscada rápida
  int eyeOpenMin      = 600;
  int eyeOpenMax      = 2000;
  int eyeOpenDuration = random(eyeOpenMin, eyeOpenMax);
  long lastBlinkTime  = currTime + eyeOpenDuration;

  // Tempo para ficar parado
  int eyeStillMin     = 400;
  int eyeStillMax     = 1200;
  int eyeStillDuration = random(eyeStillMin, eyeStillMax);
  long lastMoveTime    = currTime;

  // Loop infinito: piscar e mover o olho indefinidamente
  while (1) {
    currTime = millis();

    // Verifica se é hora de piscar
    if ((currTime - lastBlinkTime) > eyeOpenDuration) {
      // Realiza a piscada
      blink(random(800, 1000), random(900, 1000), 0, eyeLidServoLower);

      // Às vezes move o olho junto com a piscada
      if (!random(0, 2)) {
        moveEyeBall(
          random(400, 600),
          getRndEyePos(leftRightServoLower, leftRightServoUpper, leftRightServoCentre, 0.4),
          getRndEyePos(upDownServoLower, upDownServoUpper, upDownServoCentre, 0.4)
        );
        lastMoveTime = millis();
      }

      // Escolhe se a próxima piscada será rápida ou não
      if (!random(0, 5)) {
        eyeOpenDuration = random(eyeOpenShort, eyeOpenShort + 100);
      } else {
        eyeOpenDuration = random(eyeOpenMin, eyeOpenMax);
      }
      lastBlinkTime = millis();
    }

    // Verifica se é hora de mover o olho
    if ((currTime - lastMoveTime) > eyeStillDuration) {
      moveEyeBall(
        random(100, 200),
        getRndEyePos(leftRightServoLower, leftRightServoUpper, leftRightServoCentre, 1),
        getRndEyePos(upDownServoLower, upDownServoUpper, upDownServoCentre, 1)
      );
      lastMoveTime = millis();
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Se estiver no ESP32, muitas vezes é preciso alocar timers:
  // ESP32PWM::allocateTimer(0);
  // ESP32PWM::allocateTimer(1);
  // ESP32PWM::allocateTimer(2);
  // ESP32PWM::allocateTimer(3);

  // (Opcional) Ajuste da frequência de operação do servo (50 Hz)
 // eyeLidServo.setPeriodHertz(50);
  //upDownServo.setPeriodHertz(50);
  //leftRightServo.setPeriodHertz(50);
 
  // Anexa cada servo ao seu pino
  eyeLidServo.attach(eyeLidPin);
  upDownServo.attach(upDownPin);
  leftRightServo.attach(leftRightPin);
  delay(1000);
  // Posição inicial (exemplo):
  // Move pálpebra para "aberta" (upper), e olho centralizado
  // Usamos a função "easeTo(..., speed)" para demonstrar.


  eyeLidServo.easeTo(eyeLidServoUpper, 60);
  upDownServo.easeTo(upDownServoCentre,60);
  leftRightServo.easeTo(leftRightServoCentre, 60);

  // Espera o movimento terminar. (Se quiser que tudo acabe junto, use setEaseToForAllServos... etc.)
  delay(5000);

  // Move pálpebra para um valor mais baixo (fechar um pouco)
  //eyeLidServo.easeTo(eyeLidServoLower, 40);
  //delay(100);
}

void loop() {
 eyeMotion(); // Fica “preso” nessa função

}

