/*
  Practica 9 - Inclinometro con control de motorreductor (3.2.3 Unidad de
  Medicion Inercial)

  Arduino UNO R4 WiFi + MPU-6050 (I2C, 0x68) + driver L298N + motorreductor.

  El angulo frontal (pitch, giro sobre el eje Y del sensor) decide el motor:
    - su SIGNO decide la direccion (+ adelante, - reversa)
    - su MAGNITUD decide la velocidad (zona muerta de +-5 grados, de 5 a 45
      grados la velocidad crece de 90 a 255 en PWM)

  Cuatro tareas con millis(), sin delay():
    1. cada 10 ms   sensor + filtro complementario + velocidad objetivo
    2. cada 20 ms   rampa: la velocidad real se acerca a la objetivo
    3. cada 500 ms  Monitor Serie, solo si algo cambio
    4. cada 50 ms   matriz LED: punto 2x2 que sube o baja con la inclinacion

  Failsafe: si falla la lectura I2C el motor se detiene AL INSTANTE (sin
  rampa) y se anuncia PARO DE SEGURIDAD; cuando el sensor vuelve a responder
  se reconfigura y el sistema se recupera solo.
*/
#include <Wire.h>
#include "Arduino_LED_Matrix.h"

// ---------------- Pines del L298N ----------------
const int ENA = 9;    // PWM: velocidad
const int IN1 = 8;    // direccion
const int IN2 = 7;

// ---------------- MPU-6050 ----------------
const uint8_t DIR_MPU = 0x68;      // "MPU" a secas choca con una macro del nucleo ARM de la R4
const uint8_t REG_PWR_MGMT_1 = 0x6B;
const uint8_t REG_GYRO_CONFIG = 0x1B;   // 0x00 -> +-250 grados/s
const uint8_t REG_ACCEL_CONFIG = 0x1C;  // 0x00 -> +-2 g
const uint8_t REG_ACCEL_XOUT_H = 0x3B;  // 14 bytes: accel, temp, gyro
const uint8_t REG_WHO_AM_I = 0x75;
const float LSB_ACC = 16384.0;          // cuentas por g en +-2 g
const float LSB_GYR = 131.0;            // cuentas por grado/s en +-250

// ---------------- Control ----------------
const float ZONA_MUERTA = 5.0;     // grados
const float ANG_MAX = 45.0;        // a partir de aqui, velocidad maxima
const int PWM_MIN = 90;            // minimo que vence la friccion de los engranes
const int PWM_MAX = 255;
const int PASO_RAMPA = 9;          // 255 / 9 = 29 pasos x 20 ms = ~0.6 s
const float ALFA = 0.98;           // peso del giroscopio en el filtro
const float CENTRO = 2.0;          // +-2 grados: marco completo en la matriz

// ---------------- Periodos (ms) ----------------
const unsigned long T_SENSOR = 10;
const unsigned long T_RAMPA = 20;
const unsigned long T_SERIE = 500;
const unsigned long T_MATRIZ = 50;

ArduinoLEDMatrix matriz;
uint8_t cuadro[8][12];

float offsetGy = 0;          // error del giroscopio en reposo
float angulo = 0;            // pitch filtrado, en grados
int velObjetivo = 0;         // -255..255 (signo = direccion)
int velReal = 0;             // la que se aplica, sigue a la objetivo con rampa
bool sensorOk = true;
bool paroAnunciado = false;
bool enCentro = false;
bool centroAnunciado = false;

unsigned long tSensor = 0, tRampa = 0, tSerie = 0, tMatriz = 0;
unsigned long tAnterior = 0; // para el dt del giroscopio
String ultimoInforme = "";

// ---------------- Motor ----------------
void aplicaMotor(int v) {
  if (v > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else if (v < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
  analogWrite(ENA, abs(v));
}

void paroInmediato() {      // sin rampa
  velObjetivo = 0;
  velReal = 0;
  aplicaMotor(0);
}

// ---------------- I2C ----------------
bool escribe(uint8_t reg, uint8_t valor) {
  Wire.beginTransmission(DIR_MPU);
  Wire.write(reg);
  Wire.write(valor);
  return Wire.endTransmission() == 0;
}

bool lee(uint8_t reg, uint8_t *buf, uint8_t n) {
  Wire.beginTransmission(DIR_MPU);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(DIR_MPU, n) != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

bool configuraSensor() {
  return escribe(REG_PWR_MGMT_1, 0x00)      // despertar (arranca dormido)
      && escribe(REG_ACCEL_CONFIG, 0x00)    // +-2 g
      && escribe(REG_GYRO_CONFIG, 0x00);    // +-250 grados/s
}

// Lee aceleracion (g) y giro en Y (grados/s, sin offset)
bool leeSensor(float &ax, float &ay, float &az, float &gy) {
  uint8_t b[14];
  if (!lee(REG_ACCEL_XOUT_H, b, 14)) return false;
  int16_t rax = (b[0] << 8) | b[1];
  int16_t ray = (b[2] << 8) | b[3];
  int16_t raz = (b[4] << 8) | b[5];
  int16_t rgy = (b[10] << 8) | b[11];
  ax = rax / LSB_ACC;
  ay = ray / LSB_ACC;
  az = raz / LSB_ACC;
  gy = rgy / LSB_GYR;
  // sensor recien reconectado y aun dormido: todo en cero
  if (rax == 0 && ray == 0 && raz == 0) return false;
  return true;
}

float pitchAcelerometro(float ax, float ay, float az) {
  return atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
}

// Espera sin delay() (solo se usa en setup)
void espera(unsigned long ms) {
  unsigned long t0 = millis();
  while (millis() - t0 < ms) {}
}

// ---------------- Matriz ----------------
void dibujaX() {
  memset(cuadro, 0, sizeof(cuadro));
  for (int i = 0; i < 8; i++) {
    cuadro[i][i + 2] = 1;
    cuadro[i][9 - i] = 1;
  }
  matriz.renderBitmap(cuadro, 8, 12);
}

// ---------------- Tareas ----------------
void tareaSensor() {
  unsigned long ahora = millis();
  float dt = (ahora - tAnterior) / 1000.0;
  tAnterior = ahora;

  float ax, ay, az, gy;
  if (!leeSensor(ax, ay, az, gy)) {
    if (sensorOk) {
      paroInmediato();
      sensorOk = false;
      paroAnunciado = false;
    }
    configuraSensor();          // por si volvio y esta dormido
    return;
  }
  if (!sensorOk) {              // se recupero: reinicia el angulo
    sensorOk = true;
    angulo = pitchAcelerometro(ax, ay, az);
    Serial.println(">> Sensor recuperado, el sistema vuelve a operar");
    ultimoInforme = "";
    return;
  }

  // filtro complementario: giroscopio (rapido, deriva) + acelerometro
  // (lento, ruidoso, pero sin deriva)
  float acc = pitchAcelerometro(ax, ay, az);
  angulo = ALFA * (angulo + (gy - offsetGy) * dt) + (1.0 - ALFA) * acc;

  float mag = fabs(angulo);
  if (mag < ZONA_MUERTA) {
    velObjetivo = 0;
  } else {
    float m = constrain(mag, ZONA_MUERTA, ANG_MAX);
    int v = PWM_MIN + (int)((m - ZONA_MUERTA) * (PWM_MAX - PWM_MIN) / (ANG_MAX - ZONA_MUERTA));
    velObjetivo = angulo > 0 ? v : -v;
  }
  enCentro = mag <= CENTRO;
}

void tareaRampa() {
  if (!sensorOk) return;       // en paro el motor ya esta en 0
  if (velReal < velObjetivo) velReal = min(velReal + PASO_RAMPA, velObjetivo);
  else if (velReal > velObjetivo) velReal = max(velReal - PASO_RAMPA, velObjetivo);
  // si toca cruzar de adelante a reversa, pasa por 0: primero frena
  aplicaMotor(velReal);
}

void tareaSerie() {
  if (!sensorOk) {
    if (!paroAnunciado) {
      Serial.println("!! PARO DE SEGURIDAD: no hay lectura del sensor, motor detenido");
      paroAnunciado = true;
    }
    return;
  }
  if (enCentro != centroAnunciado) {
    Serial.println(enCentro ? "-- Entro al centro (nivelado)" : "-- Salio del centro");
    centroAnunciado = enCentro;
  }
  int grados = (int)round(angulo);
  int mag = abs(grados);
  String dir = grados > 0 ? "adelante" : (grados < 0 ? "atras" : "nivelado");
  String intensidad = mag < ZONA_MUERTA ? "nivelado"
                    : mag < 15 ? "leve" : mag < 30 ? "moderada" : "fuerte";
  String motor;
  if (velReal == 0) motor = "detenido";
  else motor = String(velReal > 0 ? "adelante " : "reversa ")
             + String(abs(velReal) * 100 / PWM_MAX) + "%";
  String informe = "Inclinacion: " + dir + " " + String(mag) + " grados ("
                 + intensidad + ") | Motor: " + motor;
  if (informe != ultimoInforme) {
    Serial.println(informe);
    ultimoInforme = informe;
  }
}

void tareaMatriz() {
  if (!sensorOk) {
    dibujaX();
    return;
  }
  memset(cuadro, 0, sizeof(cuadro));
  if (enCentro) {               // marco completo
    for (int c = 0; c < 12; c++) cuadro[0][c] = cuadro[7][c] = 1;
    for (int f = 0; f < 8; f++) cuadro[f][0] = cuadro[f][11] = 1;
  }
  // punto 2x2: +45 grados arriba (fila 0), -45 abajo (fila 6)
  float a = constrain(angulo, -ANG_MAX, ANG_MAX);
  int fila = (int)round((ANG_MAX - a) * 6.0 / (2 * ANG_MAX));
  for (int f = fila; f < fila + 2; f++)
    for (int c = 5; c < 7; c++) cuadro[f][c] = 1;
  matriz.renderBitmap(cuadro, 8, 12);
}

// ---------------- Arranque ----------------
void setup() {
  // 1. estado seguro: el motor apagado antes que cualquier otra cosa
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  aplicaMotor(0);

  Serial.begin(115200);
  espera(1500);
  matriz.begin();
  Wire.begin();
  Wire.setClock(400000);

  // 2. verificacion del sensor
  uint8_t id = 0;
  if (!lee(REG_WHO_AM_I, &id, 1) || id != 0x68) {
    Serial.print("ERROR: WHO_AM_I = 0x");
    Serial.print(id, HEX);
    Serial.println(" (se esperaba 0x68). Sistema detenido, motor apagado.");
    dibujaX();
    while (true) {}             // el motor queda en 0
  }
  Serial.println("MPU-6050 detectado (WHO_AM_I = 0x68)");

  // 3. configuracion
  configuraSensor();
  espera(100);

  // 4. calibracion del giroscopio: 500 lecturas con el sensor quieto
  Serial.println("Calibrando giroscopio, no muevas el sensor...");
  float suma = 0;
  int n = 0;
  for (int i = 0; i < 500; i++) {
    float ax, ay, az, gy;
    if (leeSensor(ax, ay, az, gy)) {
      suma += gy;
      n++;
    }
    espera(2);
  }
  offsetGy = n ? suma / n : 0;
  Serial.print("Offset del giroscopio Y: ");
  Serial.print(offsetGy, 3);
  Serial.println(" grados/s");

  // 5. angulo inicial con el acelerometro
  float ax, ay, az, gy;
  if (leeSensor(ax, ay, az, gy)) angulo = pitchAcelerometro(ax, ay, az);
  Serial.print("Angulo inicial: ");
  Serial.println(angulo, 1);

  tAnterior = millis();
  tSensor = tRampa = tSerie = tMatriz = millis();
}

// ---------------- Ciclo principal (sin delay) ----------------
void loop() {
  unsigned long ahora = millis();
  if (ahora - tSensor >= T_SENSOR) { tSensor += T_SENSOR; tareaSensor(); }
  if (ahora - tRampa >= T_RAMPA)   { tRampa += T_RAMPA;   tareaRampa(); }
  if (ahora - tSerie >= T_SERIE)   { tSerie += T_SERIE;   tareaSerie(); }
  if (ahora - tMatriz >= T_MATRIZ) { tMatriz += T_MATRIZ; tareaMatriz(); }
}
