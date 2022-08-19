#ifndef ENVIROMENT_SHT31_H
#define ENVIROMENT_SHT31_H

#include "enviromentSensorInterface.h"
#include "Adafruit_SHT31_mod.h"

class enviroment_SHT31 :public  enviromentSensorInterface,Adafruit_SHT31
{
public:
    
    enviroment_SHT31() :Adafruit_SHT31()
    {

    }
    inline bool init(uint8_t address, TwoWire* i2c= &Wire) 
    {
        
        _wire=i2c;
        begin(address);
        
        heater(false);
        return true ;
    };

    inline double getTemperature()  {
        return oversample(_oversamplingTemperature, &Adafruit_SHT31::readTemperature);};
    inline double getPressure()     {return 0;};
    inline double getHumidity()     {
        return oversample(_oversamplingTemperature, &Adafruit_SHT31::readHumidity);};        
    inline void setOversamplingTemperature (oversampling os) {this->_oversamplingTemperature =os;} ;
    inline void setOversamplingHumidity (oversampling os) {this->_oversamplingHumidity= os;};
private: 
    double oversample(oversampling , float (Adafruit_SHT31::*fun) () );
};

#endif