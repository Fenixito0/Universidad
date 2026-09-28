/*
  Practica 7 - Control de acceso con RFID RC522 (bus SPI)
  Arduino UNO R4 WiFi + lector MFRC522 + LED + matriz LED 12x8 integrada

  - Al arrancar verifica si hay comunicacion con el lector (registro VersionReg).
  - Muestra el UID de cada tarjeta en hexadecimal: "UID: 3A F2 1C 7B".
  - Si el UID es el autorizado: "ACCESO PERMITIDO", LED encendido 2000 ms (millis()).
  - Si no: "ACCESO DENEGADO" y el LED no se enciende.
  - Extra: la matriz LED escribe ACEPTADO / RECHAZADO desplazandose sin bloquear.

  Conexion del RC522 (se alimenta con 3.3 V, NUNCA con 5 V):
    SDA(SS) -> D10   SCK -> D13   MOSI -> D11   MISO -> D12
    RST     -> D9    GND -> GND   3.3V -> 3.3V  (IRQ sin conectar)
  LED: D7 -> resistencia 470 ohm -> LED -> GND
*/

#include <SPI.h>
#include <MFRC522.h>
#include "ArduinoGraphics.h"      // debe ir antes de Arduino_LED_Matrix.h para poder escribir texto
#include "Arduino_LED_Matrix.h"

const uint8_t RC522_SS  = 10;
const uint8_t RC522_RST = 9;      // no usar PIN_RST: ese nombre ya existe en el core de la R4
const uint8_t LED_ACCESO = 7;

const unsigned long TIEMPO_LED = 2000;    // ms que el LED queda encendido
const unsigned long PASO_TEXTO = 70;      // ms entre cada desplazamiento del texto en la matriz

// UID de la tarjeta autorizada.
// PASO 1: carga el programa, acerca la tarjeta y el llavero y anota el UID que imprime el Monitor Serie.
// PASO 2: escribe aqui los bytes de la tarjeta que elijas como autorizada y vuelve a cargar el programa.
const byte UID_AUTORIZADO[] = {0x00, 0x00, 0x00, 0x00};
const byte LARGO_AUTORIZADO = sizeof(UID_AUTORIZADO);

MFRC522 lector(RC522_SS, RC522_RST);
ArduinoLEDMatrix matriz;

// LED de acceso
bool ledEncendido = false;
unsigned long inicioLed = 0;

// Texto que corre por la matriz
const char* textoMatriz = nullptr;
int posTexto = 0;          // columna donde empieza el texto (baja hasta quedar fuera por la izquierda)
int anchoTexto = 0;
unsigned long ultimoPaso = 0;
bool resultadoOk = false;  // para elegir el icono final

// Iconos 12x8 que quedan fijos cuando el texto termina de pasar (sin const: renderBitmap no acepta const)
uint8_t ICONO_OK[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,1,0},
  {0,0,0,0,0,0,0,0,0,1,0,0},
  {0,0,0,0,0,0,0,0,1,0,0,0},
  {0,0,1,0,0,0,0,1,0,0,0,0},
  {0,0,0,1,0,0,1,0,0,0,0,0},
  {0,0,0,0,1,1,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};
uint8_t ICONO_NO[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,1,0,0,0,0,1,0,0,0},
  {0,0,0,0,1,0,0,1,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,1,0,0,1,0,0,0,0},
  {0,0,0,1,0,0,0,0,1,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

void mostrarTexto(const char* texto, bool ok) {
  textoMatriz = texto;
  resultadoOk = ok;
  posTexto = 12;                          // entra por la derecha
  anchoTexto = strlen(texto) * 5;         // Font_5x7: 5 columnas por letra
  ultimoPaso = 0;                         // dibuja el primer cuadro de inmediato
}

void actualizarMatriz() {
  if (textoMatriz == nullptr) return;
  if (millis() - ultimoPaso < PASO_TEXTO) return;
  ultimoPaso = millis();

  if (posTexto < -anchoTexto) {           // ya salio por la izquierda: dejar el icono
    if (resultadoOk) matriz.renderBitmap(ICONO_OK, 8, 12);
    else             matriz.renderBitmap(ICONO_NO, 8, 12);
    textoMatriz = nullptr;
    return;
  }
  matriz.clear();
  matriz.beginDraw();
  matriz.stroke(0xFFFFFFFF);
  matriz.textFont(Font_5x7);
  matriz.text(textoMatriz, posTexto, 1);
  matriz.endDraw();
  posTexto--;
}

void imprimirUID(const MFRC522::Uid& uid) {
  Serial.print("UID:");
  for (byte i = 0; i < uid.size; i++) {
    Serial.print(' ');
    if (uid.uidByte[i] < 0x10) Serial.print('0');   // 0x0A -> "0A", siempre 2 digitos
    Serial.print(uid.uidByte[i], HEX);
  }
  Serial.println();
}

bool esAutorizado(const MFRC522::Uid& uid) {
  if (uid.size != LARGO_AUTORIZADO) return false;
  for (byte i = 0; i < uid.size; i++) {
    if (uid.uidByte[i] != UID_AUTORIZADO[i]) return false;
  }
  return true;
}

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 3000) { }  // la R4 usa USB nativo: esperar a que abra el Monitor Serie

  pinMode(LED_ACCESO, OUTPUT);
  digitalWrite(LED_ACCESO, LOW);
  matriz.begin();

  SPI.begin();
  lector.PCD_Init();
  delay(50);                              // solo en setup: el lector tarda en arrancar

  byte version = lector.PCD_ReadRegister(MFRC522::VersionReg);
  if (version == 0x00 || version == 0xFF) {
    Serial.println("ERROR: no hay comunicacion con el lector RC522.");
    Serial.println("Revisa el cableado (MISO D12, MOSI D11, SCK D13, SDA D10, RST D9) y que este en 3.3 V.");
    mostrarTexto("SIN LECTOR", false);
  } else {
    Serial.print("Lector RC522 detectado, version 0x");
    Serial.println(version, HEX);
    Serial.println("Acerca una tarjeta...");
    mostrarTexto("LISTO", true);
  }
}

void loop() {
  // 1) Apagar el LED cuando se cumplen los 2000 ms, sin detener nada mas
  if (ledEncendido && millis() - inicioLed >= TIEMPO_LED) {
    digitalWrite(LED_ACCESO, LOW);
    ledEncendido = false;
  }

  // 2) Avanzar un paso el texto de la matriz
  actualizarMatriz();

  // 3) Leer tarjetas (tambien mientras el LED esta encendido)
  if (!lector.PICC_IsNewCardPresent()) return;
  if (!lector.PICC_ReadCardSerial()) return;

  imprimirUID(lector.uid);

  if (esAutorizado(lector.uid)) {
    Serial.println("ACCESO PERMITIDO");
    digitalWrite(LED_ACCESO, HIGH);
    ledEncendido = true;
    inicioLed = millis();                 // si se vuelve a pasar la tarjeta, el conteo reinicia
    mostrarTexto("ACEPTADO", true);
  } else {
    Serial.println("ACCESO DENEGADO");
    mostrarTexto("RECHAZADO", false);     // el LED no se toca: si estaba encendido sigue su conteo
  }

  lector.PICC_HaltA();                    // la misma tarjeta no se vuelve a leer hasta retirarla
  lector.PCD_StopCrypto1();
}
