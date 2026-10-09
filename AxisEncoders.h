/*
  AxisEncoders.h
  ------------------------------------------------------------
  ARCHIVO NUEVO. Envuelve la librería "Encoder.h" (PJRC, la misma
  que usaba el proyecto original) para roll y pitch.

  Pines, signos y limites se configuran en mis_defines.h.
  Solo necesita A/B por eje; no utiliza indice Z.
------------------------------------------------------------
*/

#ifndef AXIS_ENCODERS_H
#define AXIS_ENCODERS_H

#include <Arduino.h>
#if defined(ARDUINO_ARCH_RP2040)
#include "PicoEncoder.h"
using YokeEncoder = PicoEncoder;
#else
#include <Encoder.h>
using YokeEncoder = Encoder;
#endif
#include "mis_defines.h"

class AxisEncoders {
public:
    YokeEncoder roll;
    YokeEncoder pitch;

    AxisEncoders()
        : roll(ROLL_ENC_A, ROLL_ENC_B),
          pitch(PITCH_ENC_A, PITCH_ENC_B)
    {}

    void begin() {
#if defined(ARDUINO_ARCH_RP2040)
        roll.begin();
        pitch.begin();
#endif
        roll.write(0);
        pitch.write(0);
    }

    // Cuentas relativas al encendido: iniciar siempre con el yugo centrado.
    // El signo se ajusta en mis_defines.h, independientemente del motor.
    long readRollRaw()  { return ROLL_ENCODER_DIRECTION * roll.read(); }
    long readPitchRaw() { return PITCH_ENCODER_DIRECTION * pitch.read(); }
};

#endif
