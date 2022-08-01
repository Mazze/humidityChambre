#ifndef ENVIROMENTSENSORINTERFACE_H
#define ENVIROMENTSENSORINTERFACE_H
#include <Wire.h>

class enviromentSensorInterface
{
public: 
    enviromentSensorInterface(){};
    virtual bool init(uint8_t address, TwoWire* i2c) = 0;
    virtual double getTemperature() =0 ;
    virtual double getPressure() = 0;
    virtual double getHumidity() = 0;

};

#endif