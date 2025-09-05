// GRUPO 8 - SCHAJRIS DAGOTTO ALFIZ - EJERCICIO 1 (único)

//Librería de la eeprom
#include <Preferences.h>

//Librería del display             
#include <U8g2lib.h>   

//LIbrerías del sensor de temperatura             
#include <DHT.h>                    
#include <DHT_U.h>     

//Botones y en que pin están
#define PIN_BOTON1 35
#define PIN_BOTON2 34

//Led
#define PIN_LED LED_BUILTIN

// Defino estados de nuestra máquina de estados
#define PANTALLA1 1
#define ESTADO_CONFIRMACION1 2
#define PANTALLA2 3
#define SUBIR_UMBRAL 4
#define BAJAR_UMBRAL 5
#define DHTPIN 23      // pin del dht11
#define DHTTYPE DHT11  // tipo de dht (hay otros)

DHT dht(DHTPIN, DHTTYPE); // defino el DHT (el sensor necesita ser iniciado con esta función) (Más abajo la pantallita es iniciada con una función similar)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);  //defino la pantallita


//Estados de los botones
#define PULSADO LOW
#define N_PULSADO !PULSADO

//Valores usados para el delay sin bloqueo
unsigned long TiempoUltimoCambio = 0; //Variable que almacenará millis.
const long INTERVALO = 1000; //Cuanto tiempo dura el delay sin bloqueo
unsigned long TiempoAhora = millis(); //Guarda los milisegundos desde que se ejecutó el programa
unsigned long TiempoConteo = 0; //Variable para iniciar el conteo

bool cambioHecho = LOW; //cambioHecho se usa para que al apretar cualquier boton, el valor aumente 1 sola vez
bool boton1Previo = PULSADO; 

float CAMBIO_UMB = 1;   //Cuanto cambia el umbral
float umbral;

Preferences preferences; //Creo una "instancia" de la librería Preferences

int estadoActual = PANTALLA1;  // La máquina de estados inicia en PANTALLA1. Más abajo se usa esta variable para cambiar de estado.

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  preferences.begin("eeprom", false);
  umbral = preferences.getFloat("umbral", 21.00);


  // Pines configurados como entradas normales (sin pull-up porque 34 y 35 no lo soportan)
  pinMode(PIN_BOTON1, INPUT);
  pinMode(PIN_BOTON2, INPUT);

  //Led
  pinMode(PIN_LED, OUTPUT);

  dht.begin();   // inicializo el dht          (No confundir estas dos funciones de inicialización con las definiciones arriba)
  u8g2.begin();  //inicializo la pantallita

}

void loop() {
  // put your main code here, to run repeatedly:
  TiempoAhora = millis(); //Milisegundos desde que se ejecutó el código
  float temperatura = dht.readTemperature();

  if (temperatura > umbral) {
    digitalWrite (PIN_LED, HIGH);
  }
  else {
    digitalWrite (PIN_LED, LOW);
  }

  //Lectura de botones
  bool lecturaBoton1 = digitalRead(PIN_BOTON1);
  bool lecturaBoton2 = digitalRead(PIN_BOTON2);  

  
  switch (estadoActual) { //Empieza en el estado que estadoActual tiene como valor

    case PANTALLA1: //Pantalla 1, muestra hora establecida y temperatura
      if (TiempoAhora - TiempoUltimoCambio >= INTERVALO) { //Delay sin bloqueo
        TiempoUltimoCambio = millis(); //actualizo el tiempo

        char bufferTemperatura[9]; //Reserva espacios en el búfer para la variable de temperatura
        sprintf(bufferTemperatura, "%.2f", temperatura); //Guarda la variable temperatura en los espacios reservados para bufferTemperatura
        char bufferUmbral[9]; //Reserva espacios en el búfer para la variable del umbral
        sprintf(bufferUmbral, "%.2f", umbral); //Guarda la variable umbral en los espacios reservados para bufferUmbral
        u8g2.clearBuffer(); //Borra lo almacenado en el búfer anteriormente
        
        u8g2.setFont(u8g2_font_helvB10_tf); //La fuente del texto del oled
        u8g2.drawStr(10, 15, "Temperatura"); //Imprime el string "Temperatura" en las coordenadas.
        u8g2.drawStr(10, 30, bufferTemperatura); //Imprime la variable "bufferTemperatura" en las coordenadas.        
        u8g2.drawStr(10, 45, "Umbral"); //Imprime el string "Temperatura" en las coordenadas.
        u8g2.drawStr(10, 60, bufferUmbral); //Imprime la variable "bufferTemperatura" en las coordenadas.
        u8g2.sendBuffer(); //Envía los datos guardados en el búfer a la pantalla
      }

      if (lecturaBoton1 == PULSADO) { 
        estadoActual = ESTADO_CONFIRMACION1;
      }
    break;

    case ESTADO_CONFIRMACION1:

      if (TiempoConteo == 0){
        TiempoConteo = TiempoAhora;
      }

      if (TiempoAhora >= TiempoConteo + 5000){
        TiempoConteo = 0;
        estadoActual = PANTALLA2;
      }

      if (lecturaBoton1 == N_PULSADO){
        TiempoConteo = 0;
        estadoActual = PANTALLA1;
      }

    break;

    case PANTALLA2:
      if (TiempoAhora - TiempoUltimoCambio >= INTERVALO){  //delay sin bloqueo
        TiempoUltimoCambio = millis();  // importante actualizar el tiempo

        char bufferUmbral[9]; //Reserva espacios en el búfer para la variable del umbral
        sprintf(bufferUmbral, "%.2f", umbral); //Guarda la variable umbral en los espacios reservados para bufferUmbral

        u8g2.clearBuffer(); //Véase línea 81 a 87
        u8g2.setFont(u8g2_font_helvB10_tf);
        u8g2.drawStr(5, 25, "Temperatura umbral:");
        u8g2.drawStr(5, 45, bufferUmbral);
        u8g2.sendBuffer();
      }
      if (lecturaBoton2 == PULSADO) { //Si el boton 2 es pulsado, se sale de pantalla 2 y se entra a un estado de confirmación (Línea 126)
        cambioHecho = HIGH;
        estadoActual = BAJAR_UMBRAL;
      }

      if (lecturaBoton1 == PULSADO && boton1Previo == N_PULSADO) { //Si solo el boton 1 es pulsado, entra al estado de subir hora y cambioHecho devuelve True
        cambioHecho = HIGH;
        estadoActual = SUBIR_UMBRAL;
      }
    
      boton1Previo = lecturaBoton1;
    break;

    case BAJAR_UMBRAL:

      if (TiempoConteo == 0){
        TiempoConteo = TiempoAhora;
      }

      if (TiempoAhora >= TiempoConteo + 5000){
        preferences.putFloat("umbral", umbral);
        estadoActual = PANTALLA1;
        TiempoConteo = 0;
      }

      if (lecturaBoton2 == N_PULSADO){
        if (cambioHecho == HIGH) {
          umbral = umbral - CAMBIO_UMB;
          TiempoConteo = 0;
          Serial.println ("umbral cambiado");
          estadoActual = PANTALLA2;
          cambioHecho = LOW;
        }
      }

    break;

    case SUBIR_UMBRAL:

      if (lecturaBoton1 == N_PULSADO) { //Si se deja de presionar el boton 1 sin que se presione el boton 2

        if (cambioHecho == HIGH) { //Si cambio hecho sigue siendo HIGH, el umbral aumenta 1, y se imprime en el monitor serial
          umbral = umbral + CAMBIO_UMB;
          Serial.println ("umbral cambiado");
          cambioHecho = LOW;
        }
        estadoActual = PANTALLA2;
      }
    break;

  }







}
