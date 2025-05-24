#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <RTC_BQ32000.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>


//FUNCIONES
char leerPulsadores(void);
void resetearPulsadores(void);
void mostrarmenu(void);
int num_mod(int N, char pulsa);
void mostrar(void);
void envio(float corr, float tens, float pot, float lum); //ver que enviar en esta funcion

void recibirMensaje();

//DEFINICIONES
#define backlight 8
#define N_muestras 20

//VARIABLES 

int b0=0;
int i_pos=0;
int nummenu = 0;
int curs = 3;
int contador_horizontal=0;
int contador_vertical=0;
float muestras=5.0;

//variable bandera para la lectura del adc con bandera en RSI
bool bandera_rsi=0;

uint16_t espera = 100;
uint16_t timer1_counter;
uint16_t timer5_counter;

LiquidCrystal lcd(0, 1, 2, 3, 5, 6);

float Sensibilidad=0.66;
float Corriente;		//Pin: A1
float Tension;		//Pin: A0
double Potencia;		
float Irradiancia;	//Pin: A2
float Irradiancia2;	//Pin: A4

int contador_envio=0;
char ADC03;
char E='1';
char R='0';
char L='0';
char U='0';
char D='0';

//-----------------------------<<<<<  Bandera para controlar la interrupcion bluetooth  >>>>>----------------------------
char mensaje_recibido[20];
bool mensaje_completo = false;

//-----------------------------<<<<<  PARA ENVIO BLUETOOTH  >>>>>----------------------------
SoftwareSerial bluetooth(10,11); //Pin 10 en RX y pin 11 en TX



//-----------------------<<<<<  VARIABLE PARA PASAR DE FLOAT A CHAR  >>>>>----------------------------
char buffercorriente [10]; //variable char para tension
char buffertension [10]; //variable char para tension
char bufferpotencia [10];
char bufferluminancia [10];


//-----------------------<<<<<  VARIABLE PARA contar los ciclos  >>>>>----------------------------
float contador=0;


struct Menu
{
	String Item [9];
	int cantitem;
	};

Menu menu[] = {{{"Monitorizacion","I: NNNN    V: nnnn ","P: MMMM  L: mmmds ","","","","","",""},4}};
	
void setup()
{
	wdt_disable();
	noInterrupts();           // disable all interrupts
	Wire.begin();

	//configuracioon del display
	pinMode(8, OUTPUT);
	digitalWrite(8, HIGH);
	lcd.begin(20,4); //habilitad todo el display, si pones (20,2) solamente habilita la mitad del display
	lcd.home(); //Coloca el curso en el inicio del display


	//ADC
	ADMUX=ADMUX=0b11000000;	//se configura con una ref de 2,5V

	//Timer5
	TCCR5A = 0;
	TCCR5B = 0;
	timer5_counter = 3036;		// preload timer 65536-16MHz/256/1Hz
			
	TCNT5 = timer5_counter;   // preload timer
	TCCR5B |= (1 << CS52);    // 256 prescaler
	TIMSK5 |= (1 << TOIE5);   // enable timer overflow interrupt
			
	interrupts();             // enable all interrupts

	//-----------------------------<<<<<  PARA ENVIO BLUETOOTH  >>>>>----------------------------
	//Se crea la instancia para poder configurar los parametros del modulo
	bluetooth.begin(38400);
}
	
void loop()
{

if(bandera_rsi==1){
	bandera_rsi=0;
	TCNT5 = timer5_counter;
	
	Tension= 0;
	Corriente= 0;
	Irradiancia= 0;
	Potencia = Corriente*Tension;

	contador_envio++;
	for(int i=0; i<muestras;i++)
	{	Tension += analogRead(0)*0.4858;
		Corriente+= (analogRead(1)-512)*0.076037778;
		Irradiancia += analogRead(2)*0.0048828;
	}
	Corriente /= muestras;
	Tension /= muestras;
	Irradiancia /= muestras;
	Potencia= Corriente*Tension;
	Irradiancia2=Irradiancia*321;

	if (Corriente<=0)
	{
		Corriente=0;
		Potencia=0;
	}
	
	}

	if(contador_vertical==1){
		switch(contador_horizontal){
			case 0:
				lcd.clear();
             	lcd.home();
				
				lcd.print(F("CORRIENTE (A)"));
              	lcd.setCursor(0, 1);
				menu[0].Item[1] = String("I: " + String(Corriente,2));
				lcd.setCursor(0,1);
				lcd.print(menu[nummenu].Item[1]);

              	lcd.setCursor(0,3);
              	lcd.print(F("<-Irr      Tension->"));
				delay(200);
				break;
			case 1:
				lcd.clear();
             	lcd.home();
				
				lcd.print(F("TENSION (V)"));
              	lcd.setCursor(0, 1);
             	menu[0].Item[1] = String(" " + String(Tension,2));
				lcd.setCursor(0,1);
				lcd.print(menu[nummenu].Item[1]);

              	lcd.setCursor(0,3);
              	lcd.print(F("<-Corriente    Pot->"));
				delay(200);
				break;
			case 2:
				lcd.clear();
             	lcd.home();
				
				lcd.print(F("POTENCIA (W)"));
              	lcd.setCursor(0, 1);
             	menu[0].Item[1] = String(" " + String(Potencia,2));
				lcd.setCursor(0,1);
				lcd.print(menu[nummenu].Item[1]);

              	lcd.setCursor(0,3);
              	lcd.print(F("<-Tension      Irr->"));
				delay(200);
				break;
			case 3:
				lcd.clear();
             	lcd.home();
				
				lcd.print(F("IRRADIANCIA (W/m2)"));
              	lcd.setCursor(0, 1);
             	menu[0].Item[1] = String(" " + String(Irradiancia2,2));
				lcd.setCursor(0,1);
				lcd.print(menu[nummenu].Item[1]);

              	lcd.setCursor(0,3);
              	lcd.print(F("<-Pot    Corriente->"));
				delay(200);
				break;
		}
	}
	else{
		menu[0].Item[1] = String("I: " + String(Corriente,2) + "A V: " + String(Tension,1) + "V");
		menu[0].Item[2] = String("P: " + String(Potencia,0) + "W I: " + String(Irradiancia2,0) + "W/m2");
//		menu[0].Item[3] = String("I: " + String(Irradiancia2,2) + "W/m2");
		delay(100);
	}

  	if (E == '1')
	    {     E='0';
              espera=0;
              nummenu = 0;
              curs = 3;
              digitalWrite(backlight, HIGH);
              lcd.clear();
              lcd.home();
              lcd.print(F("Proyecto Final         "));
              lcd.setCursor(0, 1);
              lcd.print(F("            2024"));
              lcd.setCursor(0, 2);
              lcd.print(F("  Bejarano, Daniel    "));
              lcd.setCursor(0,3);
              lcd.print(F("  Burgos, Ezequiel  "));
             
			resetearPulsadores();
            delay(1000);
 		}
   
    ADC03=leerPulsadores();//lee el analog read 3 y determina que pulsador esta siendo pulsado
	switch (ADC03)
	{
		case 'N': if (espera == 100) 
           			{digitalWrite(backlight, LOW);
					espera = 0;}
       			 else {mostrarmenu();
            			espera++;}
				break;

		case 'E':digitalWrite(backlight, HIGH);
				break;
		case 'U': 
					contador_vertical=1;
				break;
		case 'D':
					contador_vertical=0;
				break;
		case 'R': //mostrar irradiancia
				if(contador_horizontal<3){
					contador_horizontal++;
				}
				else contador_horizontal=0;
				break;
		case 'L': //Mostrar Corriente
				if(contador_horizontal>0){
					contador_horizontal--;
				}
				else contador_horizontal=3;
				break;
	}
	

  if(contador_envio==10){ 
	envio(Corriente,Tension,Potencia,Irradiancia);
	contador_envio=0;
  }
}


char leerPulsadores(void)
{
	int Lectura = analogRead(3);
	delay(100);
	if (Lectura > 900)
		return 'N'; //Valor entregado si no se pulso nada
	else if (Lectura > 700)
			{E='1'; //si se pulsa el E, se activa la bandera (que es un char)
			return 'E';}
	else if (Lectura > 500)
			{U='1';
			return 'U';}
	else if (Lectura > 300)
			{D='1';
			return 'D';}
  else if (Lectura > 100)
			{L='1';
			return 'L';}
	else
		{R='1';
		return 'R';}
}

void resetearPulsadores(void)
{	char E='0';
	char R='0';
	char L='0';
	char U='0';
	char D='0';
}

void mostrarmenu(void)
{	
	lcd.clear();
	lcd.home();

	for (int i = 0; i < 4; i++)
	{
		lcd.setCursor(0,i);
		lcd.print(menu[nummenu].Item[i]);
	}

}
void envio(float corr, float tens, float pot, float lum){
		corr=Corriente*100;
		tens=Tension;
		lum=Irradiancia*100;

//=================================================================================================
		//concatenamos el flotante de corriente en un string
			
		dtostrf(corr, 4, 0, buffercorriente);
		dtostrf(tens, 3, 0, buffertension);
		dtostrf(lum, 4, 0, bufferluminancia);

//cargo en el buffer 
		bluetooth.print(buffertension);
		bluetooth.print(buffercorriente);
		bluetooth.print(bufferluminancia);
		bluetooth.print('\n');

}
ISR(TIMER5_OVF_vect)        //1 seg interrupt service routine
{	
	bandera_rsi=1;
}