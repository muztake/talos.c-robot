#pragma config(Sensor, S1,     colorA,         sensorEV3_Color, modeEV3Color_Color)
#pragma config(Sensor, S2,     i2c,            sensorEV3_GenericI2C)
#pragma config(Sensor, S3,     infraR,         sensorEV3_IRSensor)
#pragma config(Sensor, S4,     colorB,         sensorEV3_Color, modeEV3Color_Color)
//S1 = Direita S4 = Esquerda
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//


// Definindo endere�os do Arduino
#define ARDUINO_ADDRESS 0x08
#define ARDUINO_PORT S2
#define KP 1
#define KI 0.0
#define KD 0.2
#define SET_POINT 65
#define OFFSET -22
#define TURN_RATE_90 70
#define TURN_TIME_90 60
#define TURN_SPEED_90 30
#define G_THRESH 2.6
#define TURN_ERRO_K 8
#define IMAGE_KP 0.2
#define IMAGE_SETPOINT 34
#define IMAGE_OFFSET 8
#define IMAGE_ERRO 10
#define COLOR_ERRO 6
#define INT_COUNT_MAX 100

bool resgate = false;
int limiarWhite[2][3];



/* ---------------------------------
||						UTILS								||
--------------------------------- */
// Limpar tela
void print(char *str){
	eraseDisplay();
	displayCenteredBigTextLine(1,"%s",str);
}

// Realiza mudan�a na magnitude
long map( long x, long in_min, long in_max, long out_min, long out_max){
	return (x - in_min) * ( out_max - out_min) / ( in_max - in_min ) + out_min ;
}

void stopUs(){
	while (1){
		motor[motorA] = 0;
		motor[motorB] = 0;
	}
}


/* ---------------------------------
||							I2C								||
--------------------------------- */
// Vari�vel que armazena a posi��o do sensor de 0 a 127
int linha;
// Vari�vel que armazena o estado especial do sensor de linha
int estado;
// Armazena o status do sensor
TI2CStatus mI2CStatus;
// Armazena a resposta
byte replyMsg[10];
// Armazena a mensagem a ser enviada
byte sendMsg[10];

/**
 * TESTE
 */
void i2c_msg(int reply_size, int message_size, byte byte1, byte byte2, byte byte3, byte byte4){
	// Pegando o status do sensor I2C
	mI2CStatus = nI2CStatus[i2c];
	// Reservando espa�o na mem�ria para a resposta
	memset(replyMsg, 0, sizeof(replyMsg));
	// Reservando espa�o no tamanho da mensagem
	message_size += 3;
	// Atribuindo o tamanho da mensagem e o endere�o
	sendMsg[0] = message_size;
	sendMsg[1] = ARDUINO_ADDRESS;
	// Atribuindo os bytes da mensagem
	sendMsg[2] = byte1;
	sendMsg[3] = byte2;
	sendMsg[4] = byte3;
	sendMsg[5] = byte4;

	// Enviando mensagem
	sendI2CMsg(i2c, &sendMsg[0], 8);
	// Esperar 30ms
	wait1Msec(30);

	// Ler resposta
	readI2CReply(i2c, &replyMsg[0], reply_size);

	// Resposta
	linha = replyMsg[0];
	estado = replyMsg[1];
	wait1Msec(35);
}
//Leitura de bolas
int read_camera(){
	i2c_msg(8,3,13,0,0,0);
	return linha;
}

/* ---------------------------------
||						MOVIMENTOS					||
--------------------------------- */
// Faz o rob� andar para frente
void walk(int value, float duration){
	displayCenteredBigTextLine(1,"WALK");
	resetMotorEncoder(motorA);
	int a = getMotorEncoder(motorA);
	if(value > 0){

		while(getMotorEncoder(motorA) + duration > a){
			motor[motorA] = -value;
			motor[motorB] = -value;
		}
	}else{
		while(getMotorEncoder(motorA) - duration < a){
			motor[motorA] = -value;
			motor[motorB] = -value;
		}
	}
}
// Faz o rob� girar em graus
void turn(float value, bool direction){
	value = map(value, 0, 180, 0, 1320);
	int a = getMotorEncoder(motorA);
	if(direction){
		while(getMotorEncoder(motorA) < a + value){
			motor[motorA] = 30;
			motor[motorB] = -30;
		}
		}	else{
		while(getMotorEncoder(motorA) + value > a ){
			motor[motorA] = -30;
			motor[motorB] = 30;
		}
	}
}

// L� os valores e estado do QTR8-A
int read_line_sensor(){
	int byte1;
	if(resgate){
		byte1 = 13;
	}else{
		byte1 = 1;
	}
	i2c_msg(8,3,byte1,0,0,0);
	int value = linha;
	return value;
}

long coresA[3];
long coresB[3];
int read_color_sensor(){
	// Pegando cores do sensor esquerdo
	getColorRGB(colorB, coresB[0], coresB[1], coresB[2]);
	// Pegando cores do sensor direito
	getColorRGB(colorA, coresA[0], coresA[1], coresA[2]);

	for(int a = 0; a < 3; a++){
		coresA[a] = map(coresA[a], 0, 90, 0, 255);
	}
	for(int a = 0; a < 3; a++){
		coresB[a] = map(coresB[a], 0, 90, 0, 255);
	}
	// Detecta se o valor do verde passa de certo limiar
	//Cinza
	//stopUs();
	read_line_sensor();
	if ((coresA[2] > limiarWhite[0][2] / 4) && (coresB[2] > limiarWhite[1][2] / 4) && (coresA[2] < (3*limiarWhite[0][2])/5) && (coresB[2] < (3*limiarWhite[1][2])/5) && (linha < 70) && (linha > 50)){
			displayCenteredBigTextLine(1,"CINZA");
			setMotorTarget(motorA, 60, -20);
			setMotorTarget(motorB, 60, -20);
			if ((coresA[2] > limiarWhite[0][2] / 4) && (coresB[2] > limiarWhite[1][2] / 4) && (coresA[2] < (3*limiarWhite[0][2])/5) && (coresB[2] < (3*limiarWhite[1][2])/5)){
				return 3;
			}else{
				setMotorTarget(motorA, 50, 20);
				setMotorTarget(motorB, 50, 20);
			}
	}

	// Esquerda
	if ((coresA[1] >= 40) && ((sqrt(pow(coresA[0], 2) + pow(coresA[2], 2)) - (coresA[1] - 20)) <= 0)){
			print("GREEN TURN ESQUERDO");
			return 2;
	}
	// Direita
	if ((coresB[1] >= 40) && ((sqrt(pow(coresB[0], 2) + pow(coresB[2], 2)) - (coresB[1] - 20)) <= 0)){
			displayCenteredBigTextLine(1,"GREEN TURN DIREITO");
			return 1;
	}

	return 0;
}

// Calibra o threshold de branco e preto
void calibrateThresh(){
	// Calibra��o do branco
	while(!getButtonPress(buttonEnter)){
		// Pegando cores do sensor esquerdo
		long coresB[3];
		getColorRGB(colorB, coresB[0], coresB[1], coresB[2]);
		// Pegando cores do sensor direito
		long coresA[3];
		getColorRGB(colorA, coresA[0], coresA[1], coresA[2]);

		print("Calibrando");
		displayTextLine(4, "E => (R:%d, G:%d, B:%d)", coresB[0], coresB[1], coresB[2]);
		displayTextLine(7, "D => (R:%d, G:%d, B:%d)", coresA[0], coresA[1], coresA[2]);

		for(int a = 0; a < 2; a++){
			for(int b = 0; b < 3; b++){
				limiarWhite[a][b] = (a == 0 ? coresA[b] - COLOR_ERRO : coresB[b] - COLOR_ERRO);
				limiarWhite[a][b] = map(limiarWhite[a][b], 0, 90, 0, 255);
			}
		}
	}
}

task main()
{
	i2c_msg(2, 1, 13, 0, 0, 0);
	calibrateThresh();
	// Resgate Original
	while(1){
		displayBigTextLine(1, "%d", read_camera());
		linha = read_line_sensor();
		//PID(linha, 0, IMAGE_KP, IMAGE_SETPOINT);
			if ((linha <IMAGE_SETPOINT + IMAGE_ERRO) && (linha >= IMAGE_SETPOINT - IMAGE_ERRO)){
					walk(10, 20);
				}else if (linha > IMAGE_SETPOINT){
					motor[motorA] = IMAGE_OFFSET;
					motor[motorB] = -IMAGE_OFFSET;
				}else{
					motor[motorA] = -IMAGE_OFFSET;
					motor[motorB] = IMAGE_OFFSET;
				}
			}
		wait1Msec(50);

}
