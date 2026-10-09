#ifndef _LowPassFilter_hpp_
#define _LowPassFilter_hpp_

 
// Pasa bajos de primer orden parametrizado por constante de tiempo.
// tauSeconds >= 0 y deltaTime en segundos; tau=0 deja pasar la entrada.
// Cada senal/eje necesita su propia instancia (estado inicial cero).
class TimeConstantLowPassFilter {
public:
    explicit TimeConstantLowPassFilter(float tauSeconds);
    float update(float input, float deltaTime);

private:
    float _tau;
    float _output;
};

#endif //_LowPassFilter_hpp_
