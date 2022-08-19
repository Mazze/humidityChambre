#include "enviroment_SHT31.h"

double enviroment_SHT31::oversample(oversampling os, float (Adafruit_SHT31::*fun) () )
{
    float avr=0;
    for (int i =0;i< os;++i )
        avr+=(*this.*fun)();
    return double (avr/((float)os));
}