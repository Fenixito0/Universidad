/*
  Control de acceso RFID con RC522 por SPI
  Placa: Arduino UNO R4 WiFi

  LED verde (pin 7)  -> acceso concedido
  LED rojo  (pin 6)  -> acceso denegado
  Ambos se apagan solos a los 2000 ms, con millis().
  La matriz muestra ACEPTADO o RECHAZADO con scroll no bloqueante.
  El programa nunca se detiene: sigue leyendo tarjetas mientras
  un LED esta encendido y mientras el texto se desplaza.
*/

#include <SPI.h>
#include <MFRC522.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

const uint8_t PIN_SS  = 10;
const uint8_t PIN_RST = 9;

MFRC522 lector(PIN_SS, PIN_RST);
ArduinoLEDMatrix matriz;

// ---- Tarjetas autorizadas ----
const byte AUTORIZADOS[][4] = {
  { 0x99, 0xEB, 0x7B, 0x63 },   // llavero
  { 0xAA, 0x25, 0x60, 0xE4 }    // tarjeta
};
const byte NUM_AUTORIZADOS = sizeof(AUTORIZADOS) / 4;

// ---- LEDs con temporizador propio ----
const uint32_t T_LED = 2000;

struct Led {
  uint8_t  pin;
  bool     prendido;
  uint32_t t0;
};

Led verde = { 7, false, 0 };
Led rojo  = { 6, false, 0 };

// ---- Scroll no bloqueante en la matriz ----
char     mensaje[16] = "";
int      xTexto = 12;
uint32_t tScroll = 0;
const uint32_t PASO_SCROLL = 60;
bool     animando = false;

// ---------------------------------------------------

void prender(Led &l, uint32_t ahora) {
  digitalWrite(l.pin, HIGH);
  l.prendido = true;
  l.t0 = ahora;
}

void actualizar(Led &l, uint32_t ahora) {
  if (!l.prendido) return;
  if (ahora - l.t0 < T_LED) return;

  digitalWrite(l.pin, LOW);
  l.prendido = false;
}

void mostrarEnMatriz(const char* txt) {
  strncpy(mensaje, txt, sizeof(mensaje) - 1);
  mensaje[sizeof(mensaje) - 1] = '\0';
  xTexto   = 12;
  tScroll  = 0;
  animando = true;
}

void animarMatriz(uint32_t ahora) {
  if (!animando) return;
  if (ahora - tScroll < PASO_SCROLL) return;
  tScroll = ahora;

  matriz.beginDraw();
  matriz.clear();
  matriz.stroke(0xFFFFFFFF);
  matriz.textFont(Font_4x6);
  matriz.beginText(xTexto, 1, 0xFFFFFF);
  matriz.print(mensaje);
  matriz.endText();              // sin SCROLL_LEFT: no bloquea
  matriz.endDraw();

  xTexto--;

  if (xTexto < -(int)(strlen(mensaje) * 5)) {
    animando = false;
    matriz.beginDraw();
    matriz.clear();
    matriz.endDraw();
  }
}

bool esAutorizado() {
  if (lector.uid.size != 4) return false;

  for (byte t = 0; t < NUM_AUTORIZADOS; t++) {
    bool coincide = true;

    for (byte i = 0; i < 4; i++) {
      if (lector.uid.uidByte[i] != AUTORIZADOS[t][i]) {
        coincide = false;
        break;
      }
    }

    if (coincide) return true;
  }
  return false;
}

void imprimirUID() {
  Serial.print("UID:");
  for (byte i = 0; i < lector.uid.size; i++) {
    Serial.print(lector.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(lector.uid.uidByte[i], HEX);
  }
  Serial.println();
}

void leerTarjeta(uint32_t ahora) {
  if (!lector.PICC_IsNewCardPresent()) return;
  if (!lector.PICC_ReadCardSerial())   return;

  imprimirUID();

  if (esAutorizado()) {
    Serial.println("ACCESO CONCEDIDO");
    prender(verde, ahora);
    mostrarEnMatriz("ACEPTADO");
  } else {
    Serial.println("ACCESO DENEGADO");
    prender(rojo, ahora);
    mostrarEnMatriz("RECHAZADO");
  }

  Serial.println();
  lector.PICC_HaltA();
  lector.PCD_StopCrypto1();
}

// ---------------------------------------------------

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 3000);

  pinMode(verde.pin, OUTPUT);
  pinMode(rojo.pin,  OUTPUT);
  digitalWrite(verde.pin, LOW);
  digitalWrite(rojo.pin,  LOW);

  matriz.begin();

  SPI.begin();
  lector.PCD_Init();
  delay(50);                     // unico delay, solo en el arranque

  byte version = lector.PCD_ReadRegister(MFRC522::VersionReg);

  if (version == 0x00 || version == 0xFF) {
    Serial.println("ERROR: no hay comunicacion con el RC522");
    Serial.println("Revisa el cableado SPI y que este a 3.3 V");
  } else {
    Serial.print("RC522 detectado. Version del firmware: 0x");
    Serial.println(version, HEX);
    Serial.print("Tarjetas autorizadas: ");
    Serial.println(NUM_AUTORIZADOS);
    Serial.println("Acerca una tarjeta...");
  }
}

void loop() {
  uint32_t ahora = millis();

  animarMatriz(ahora);       // el texto avanza a su ritmo
  actualizar(verde, ahora);  // cada LED cuenta su propio tiempo
  actualizar(rojo,  ahora);
  leerTarjeta(ahora);        // siempre atento, nunca bloqueado
}
