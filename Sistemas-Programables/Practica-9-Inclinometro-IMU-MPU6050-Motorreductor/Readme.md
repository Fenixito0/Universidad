# Nombre del proyecto
Inclinómetro con control de motorreductor (IMU MPU-6050)

## Descripción
El Arduino UNO R4 WiFi lee la inclinación frontal (pitch) de un MPU-6050 por
I2C y con ella controla un motorreductor a través de un puente H L298N: el
signo del ángulo decide la dirección y su magnitud la velocidad. El motor
acelera con rampa, se queda quieto en una zona muerta de ±5° y se detiene al
instante si se pierde el sensor. La matriz LED integrada muestra un punto que
sube o baja con la inclinación.

## Objetivos
- Leer acelerómetro y giroscopio del MPU-6050 por registros I2C y verificar el
  sensor con `WHO_AM_I` (0x68).
- Combinar ambos con un filtro complementario para obtener un ángulo estable.
- Controlar dirección y velocidad (PWM) de un motor DC con el L298N, con zona
  muerta, velocidad mínima útil y rampa de aceleración.
- Detener el motor de inmediato ante una falla del sensor (failsafe).
- Correr cuatro tareas concurrentes con `millis()`, sin `delay()`.

## Herramientas y material utilizado
- Arduino UNO R4 WiFi (con su matriz LED 12×8 integrada)
- Módulo MPU-6050 (GY-521)
- Driver L298N y motorreductor DC
- Batería LiPo 3S (11.1 V) para el motor
- Cables y protoboard
- Librerías `Wire` y `Arduino_LED_Matrix` (vienen con el core de la R4; el
  MPU-6050 se lee por registros, sin librería externa)
- Arduino IDE y Monitor Serie (115200 baudios)

## Diagrama
MPU-6050: VCC→5V, GND→GND, SDA→A4, SCL→A5. L298N: ENA→D9 (PWM, sin el
jumper), IN1→D8, IN2→D7, GND común; la batería va a 12V/GND del L298N y el
motor a OUT1/OUT2.

![Diagrama de conexión](Diagrama/diagrama-conexion-imu-motor.png)

## Código
Arranca con el motor apagado, verifica `WHO_AM_I`, configura ±2 g / ±250 °/s,
calibra el giroscopio con 500 lecturas y toma el ángulo inicial del
acelerómetro. En el `loop()` corren: sensor y filtro (10 ms), rampa y motor
(20 ms), Monitor Serie solo si algo cambia (500 ms) y matriz LED (50 ms).
Detalles en [Codigo/Readme.txt](Codigo/Readme.txt).

[Ver código](Codigo/)

## Reporte
Metodología (arranque seguro, filtro complementario, zona muerta, rampa,
failsafe y tareas con `millis()`), conexiones, diagrama, respuestas a las
preguntas de análisis y conclusiones.

[Ver Reporte](Reporte/Reporte-Inclinometro-IMU-Motorreductor.pdf)

## Resultados
PENDIENTE: probar en el circuito armado. Lo que se debe observar: motor quieto
dentro de ±5°, adelante al inclinar hacia adelante y en reversa hacia atrás,
más rápido mientras más inclinado; al invertir, frena antes de cambiar de
sentido; al desconectar SDA o SCL, "PARO DE SEGURIDAD" y X en la matriz.

## Video
PENDIENTE: video del funcionamiento.

[Ver carpeta Video](Video/)

## Conclusiones
Ni el acelerómetro (ruidoso con las sacudidas) ni el giroscopio (con deriva)
bastan solos; el filtro complementario combina lo mejor de ambos. La zona
muerta, la velocidad mínima de 90 y la rampa hacen que el motor responda
suave, y el failsafe aplica el principio de que un actuador sin información
del sensor debe detenerse de inmediato.
