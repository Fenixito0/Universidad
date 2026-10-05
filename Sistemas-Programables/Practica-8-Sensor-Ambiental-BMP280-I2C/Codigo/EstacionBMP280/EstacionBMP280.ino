/*
 * ============================================================
 *  Estación Barométrica BMP280 — Arduino UNO R4 WiFi
 *  Sistemas Programables (SCC-1023) — Práctica P3.3
 * ============================================================
 *  Lee temperatura, presión y altitud del BMP280 por I2C,
 *  sin delay() (temporización con millis()).
 *  - Monitor/Serial Plotter a 115200 baudios
 *  - Matriz LED 12x8: alterna temperatura (°C) y altitud (m)
 *
 *  Librerías (Gestor de librerías):
 *    - Adafruit BMP280 Library
 *    - Adafruit Unified Sensor
 *    - ArduinoGraphics
 *
 *  CONEXIÓN:
 *    BMP280 VCC -> 3.3V     BMP280 GND -> GND
 *    BMP280 SDA -> SDA      BMP280 SCL -> SCL
 *
 *  ¿Conector Qwiic? Cambia USE_QWIIC a 1 (usa Wire1).
 * ============================================================
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "ArduinoGraphics.h"       // SIEMPRE antes de Arduino_LED_Matrix.h
#include "Arduino_LED_Matrix.h"

// ---------------- Configuración ----------------
#define USE_QWIIC 0                // 0 = pines SDA/SCL, 1 = conector Qwiic

#if USE_QWIIC
  #define BUS_I2C Wire1
#else
  #define BUS_I2C Wire
#endif

const float SEALEVEL_HPA = 1005.0;   // presión a nivel del mar del día
const unsigned long T_LECTURA    = 1000;    // ms entre lecturas
const unsigned long T_MATRIZ     = 2500;    // ms entre cambios en la matriz
const unsigned long T_REINTENTO  = 5000;    // ms entre reintentos si no hay sensor

const uint8_t REG_CHIP_ID = 0xD0;
const uint8_t ID_BMP280   = 0x58;
const uint8_t ID_BME280   = 0x60;

// ---------------- Objetos y estado ----------------
Adafruit_BMP280 bmp(&BUS_I2C);
ArduinoLEDMatrix matrix;

bool sensorOK     = false;
bool datosValidos = false;
uint8_t direccion = 0;

float tempC = 0, presionHpa = 0, altitudM = 0;

unsigned long tLectura = 0, tMatriz = 0, tReintento = 0;
uint8_t pantalla = 0;   // 0 = temperatura, 1 = altitud

// ---------------- Funciones auxiliares ----------------
uint8_t leerChipId(uint8_t addr) {
  BUS_I2C.beginTransmission(addr);
  BUS_I2C.write(REG_CHIP_ID);
  if (BUS_I2C.endTransmission(false) != 0) return 0;
  if (BUS_I2C.requestFrom((int)addr, 1) != 1) return 0;
  return BUS_I2C.read();
}

bool iniciarSensor() {
  const uint8_t candidatos[] = {0x76, 0x77};

  for (uint8_t addr : candidatos) {
    uint8_t id = leerChipId(addr);
    if (id == 0) continue;

    Serial.print("Dispositivo en 0x");
    Serial.print(addr, HEX);
    Serial.print(" | Chip ID: 0x");
    Serial.println(id, HEX);

    if (id == ID_BME280) {
      Serial.println("ATENCION: es un BME280. Usa el sketch con Adafruit_BME280.");
      return false;
    }
    if (id == ID_BMP280 && bmp.begin(addr, ID_BMP280)) {
      direccion = addr;
      // Modo normal recomendado por Bosch para estación meteorológica de interior:
      // temp x2, presión x16, filtro IIR x16, espera 500 ms entre mediciones
      bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                      Adafruit_BMP280::SAMPLING_X2,     // temperatura
                      Adafruit_BMP280::SAMPLING_X16,    // presión
                      Adafruit_BMP280::FILTER_X16,
                      Adafruit_BMP280::STANDBY_MS_500);
      return true;
    }
  }
  return false;
}

// Muestra hasta 3 caracteres en la matriz (fuente 4x6 -> 12 columnas)
void mostrarMatriz(const String &txt) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(txt);
  matrix.endText();
  matrix.endDraw();
}

void leerSensor() {
  tempC      = bmp.readTemperature();
  presionHpa = bmp.readPressure() / 100.0F;
  altitudM   = bmp.readAltitude(SEALEVEL_HPA);

  // Si se desconecta el sensor, la librería devuelve NaN o valores absurdos
  if (isnan(tempC) || isnan(presionHpa) || presionHpa < 300 || presionHpa > 1100) {
    Serial.println("Error: lectura invalida. Revisa el cableado.");
    datosValidos = false;
    sensorOK = false;
    mostrarMatriz("ERR");
    return;
  }
  datosValidos = true;

  // Formato etiqueta:valor -> compatible con Serial Plotter
  Serial.print("Temp_C:");               Serial.print(tempC, 2);
  Serial.print(",Presion_Atm_hPa:");     Serial.print(presionHpa, 2);
  Serial.print(",Altura_nivel_mar_m:");  Serial.println(altitudM, 1);
}

// ---------------- Programa principal ----------------
void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 3000) { }   // espera USB máx. 3 s

  matrix.begin();
  BUS_I2C.begin();

  Serial.println("\n=== Estacion BMP280 - UNO R4 WiFi ===");
  Serial.println(USE_QWIIC ? "Bus: Wire1 (Qwiic)" : "Bus: Wire (SDA/SCL)");

  sensorOK = iniciarSensor();
  if (sensorOK) {
    Serial.print("BMP280 listo en 0x");
    Serial.println(direccion, HEX);
  } else {
    Serial.println("No se encontro el BMP280. Reintentando cada 5 s...");
    mostrarMatriz("ERR");
  }
}

void loop() {
  unsigned long ahora = millis();

  // Reintento de conexión si el sensor no está disponible
  if (!sensorOK) {
    if (ahora - tReintento >= T_REINTENTO) {
      tReintento = ahora;
      sensorOK = iniciarSensor();
      if (!sensorOK) mostrarMatriz("ERR");
    }
    return;
  }

  // Tarea 1: lectura del sensor
  if (ahora - tLectura >= T_LECTURA) {
    tLectura = ahora;
    leerSensor();
  }

  // Tarea 2: actualización de la matriz LED
  if (datosValidos && ahora - tMatriz >= T_MATRIZ) {
    tMatriz = ahora;
    if (pantalla == 0) {
      mostrarMatriz(String((int)round(tempC)) + "C");
    } else {
      mostrarMatriz(String((int)round(altitudM)) + "m");
    }
    pantalla = (pantalla + 1) % 2;
  }
}
