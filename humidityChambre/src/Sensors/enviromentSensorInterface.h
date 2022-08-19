#ifndef ENVIROMENTSENSORINTERFACE_H
#define ENVIROMENTSENSORINTERFACE_H
#include <Wire.h>

class enviromentSensorInterface
{
public: 
    enum oversampling {Sample_X1=1,Sample_X2=2,Sample_X4=4};
    enviromentSensorInterface(){};
    virtual bool init(uint8_t address, TwoWire* i2c) = 0;
    virtual double getTemperature() =0 ;
    virtual double getPressure() = 0;
    virtual double getHumidity() = 0;
    virtual void setOversamplingTemperature (oversampling) =0;
    virtual void setOversamplingHumidity (oversampling) =0;
protected:
    oversampling _oversamplingTemperature = oversampling::Sample_X1;
    oversampling _oversamplingHumidity = oversampling::Sample_X1;
};

#endif