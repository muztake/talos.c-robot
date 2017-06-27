#pragma config(Sensor, S1,     colorA,         sensorEV3_Color, modeEV3Color_Color)
#pragma config(Sensor, S2,     i2c,            sensorEV3_GenericI2C)
#pragma config(Sensor, S3,     infraR,         sensorEV3_IRSensor)
#pragma config(Sensor, S4,     colorB,         sensorEV3_Color, modeEV3Color_Color)
/** CONFIGURA��O DOS SENSORES
 * S1 => Sensor de Cor Direito
 * S2 => Sensor do I2C
 * S3 => Sensor Infravermelho
 * S4 => Sensor de Cor Esquerdo
 */

// CONSTANTES GLOBAIS
#define ARDUINO_ADDRESS 0x08 // Endere�o do Arduino
#define ARDUINO_PORT S2 // Port do sensor do Arduino
#define KP 0.85 // Constante Proporcional do PID
#define KI 0.0 // Constante de Integral do PID
#define KD 0.2 // Constante Derivativa do PID
#define SET_POINT 65 // Ponto intermedi�rio das leituras do sensor QTR8
#define SET_POINT_INFRA 2 //Ponto que o rob� tenta se estabilizar nas leituras do sensor infravermelho
#define OFFSET -18 // Offset de Velocidade do PID
#define TURN_TIME_90 60 // Tempo da virada na curva de 90�
#define TURN_SPEED_90 30 // Velocidade da virada na curva de 90�
#define TURN_ERRO_K 8 // Erro permitido na virada da curva de 90�
#define IMAGE_KP 0.2 // Constante proporcional da busca no resgate
#define TIMER_ESPERA 1000000 // Tempo que o rob� esperar� parado depois de encontrar alguma bola e perde-la
#define IMAGE_SETPOINT 60 // Ponto intermedi�rio da busca no resgate
#define IMAGE_ERRO 10 // Erro toler�vel de alinhamento em rela��o � bolinha
#define IMAGE_OFFSET 4 // Velocidade de aproxima��o no resgate
#define COLOR_ERRO 6 // Erro permitido da cor durante a calibra��o
#define INT_COUNT_MAX 20 // M�ximo de itera��es da saida de estado
#define SMOOTH_K 0.2 // Costante do Exponential Smoothing, usado no girosc�pio
#define A_MOTOR_OFFSET 1 // Ajuste do offset no motor A
#define B_MOTOR_OFFSET 1 // Ajuste do offset no motor B
#define LF_MSG 1 // Mensagem I2C para ficar no modo de seguir linha
#define DISTANCE 30 //Dist�ncia de entrada
#define ENTRADA_ERRO 10
#define UPVEL 30
#define DWVEL -30
#define SETPOINTIR 1
#define KPIR 1

long timer = 0;
bool resgate = false; // Armazena o estado do resgate
bool corrigido = false; // Armazena o estado da corre��o
bool offRoad = false; // Armazena o estado do rob� em rela��o ao terreno
bool obst = false // Armazena se o obstaculo j� foi superado
int limiarWhite[2][3]; // Armazena os limiares da cor branca
int linha; // Armazena posi��o do sensor QTR8-A | Lim: 0 - 127
int estado; // Armazena estado do sensor | {1, 2, 3, 4}
int ultra1; // Armazena o valor do primeiro sensor ultrasonico
int ultra2; // Armazena o valor do segundo sensor ultrasonico
int gyro = 1; // Armazena a angula��o do rob�
int gyroV[2];
int resgateCount = 0; // Armazena o contador para entrar no resgate
int garantiaRampa = 0; //Garante que a entrada na rampa n�o � apenas uma flutua��o
long coresA[3]; // Armazena as cores do sensor de cor direito
long coresB[3]; // Armazena as cores do sensor de cor esquerdo
TI2CStatus mI2CStatus; // Armazena o status do sensor
byte replyMsg[10]; // Armazena a resposta do I2C
byte sendMsg[10]; // Armazena a mensagem a ser enviada

// INCLUINDO BIBLIOTECAS
#include "utils.c"
#include "comms.c"
#include "movement.c"
#include "sensors.c"
#include "heuristica.c"

// RESGATE
void resgateMode(void){
	walk(TURN_SPEED_90, TURN_TIME_90*10);
	//turn(25, 0);
	int distanceInf = getIRDistance(infraR);
	for(int a = 0; a < 1000; a++){
 		motor[motorA] = - ((distanceInf	- 25) * KP * 4);
 		motor[motorB] = - ((distanceInf	- 25) * KP * 4);
 		distanceInf = getIRDistance(infraR);
  }
  setSpeed(0, 0);
  turning(false);
  for(int a = 0; a < 1000; a++){
 		motor[motorA] = - ((distanceInf	- 25) * KP * 4);
 		motor[motorB] = - ((distanceInf	- 25) * KP * 4);
 		distanceInf = getIRDistance(infraR);
  }
  setSpeed(0, 0);
	//walk(TURN_SPEED_90, TURN_TIME_90*20);
	i2c_msg(2, 8, 13, 0, 0, 0, 30);
	displayCenteredBigTextLine(1, "RESGATE");
	displayCenteredBigTextLine(5, "%d | %d", estado, linha);
	//entry();
	closeG();
	cDown();
	parseUP();
	while(1){
		while(!searchBall()){
			i2c_msg(8, 8, 13, 0, 0, 0, 300);
			displayCenteredBigTextLine(1, "RESGATE");
			displayCenteredBigTextLine(5, "%d | %d", estado, linha);
			searchBall();
		}
		back();
		while(!searchRecipe()){
			i2c_msg(8, 8, 13, 0, 0, 0, 300);
			displayCenteredBigTextLine(1, "RECEPT");
			displayCenteredBigTextLine(5, "%d", replyMsg[5]);
			searchRecipe();
		}
		back();
	}
	//regate done
	stopUs();

}

// ESCOPO PRINCIPAL
task main
{
	// Manda mensagem para o Arduino sair do modo de resgate
	i2c_msg(2, 8, 1, 0, 0, 0, 30);
	while(0)
		setSpeed(-20, -20);
	while(0){
		i2c_msg(8, 8, 13, 0, 0, 0, 50);
		searchBall();
		displayCenteredBigTextLine(1, "ULTRA");
		displayCenteredBigTextLine(5, "%d | %d", replyMsg[3], replyMsg[4]);
		displayCenteredBigTextLine(10, "%d | %d", linha, estado);
		//stopUs();

	}
	// Calibra o limiar de branco
	calibrateThresh();
	// Loop principal
	while(1){
			// Caso esteja na rampa
			if(checkRampa()){
				garantiaRampa++;
				continue;
			}
			if(resgateCount > 0){
				if (estado != 4)
					walk(TURN_SPEED_90, TURN_TIME_90/2);
				read_line_sensor(1);
				if(estado == 3)
					resgateMode();
					//resgateCount = 0;
			}
			// Executa a fun��o de seguir linhas
			lineFollowing();
			if(garantiaRampa > 100){

			resgateCount++;
				displayCenteredBigTextLine(5, "RAMPA: %d | %d", garantiaRampa, resgateCount);
			}
	}
}
