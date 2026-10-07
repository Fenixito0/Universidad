Diagrama del circuito.

  diagrama-conexion-imu-motor.png - Vista superior: Arduino UNO, IMU MPU-6050
    (GY-521) por I2C, driver L298N, motorreductor y bateria LiPo 3S.   [LISTO]

Conexiones:
  MPU-6050  VCC -> 5V, GND -> GND, SDA -> A4, SCL -> A5 (AD0 libre: 0x68)
  L298N     ENA -> D9 (PWM, jumper de ENA quitado), IN1 -> D8, IN2 -> D7,
            GND -> GND del Arduino (tierra comun), jumper de 5V puesto
            12V / GND -> bateria LiPo 3S 11.1 V (+ / -)
            OUT1 / OUT2 -> motorreductor (el orden solo invierte el sentido)
  El Arduino se alimenta por USB.
