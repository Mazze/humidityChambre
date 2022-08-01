#ifndef ENVIROMENT_BME280_H
#define ENVIROMENT_BME280_H

#include "enviromentSensorInterface.h"
#include <Adafruit_BME280.h>

class enviroment_BME280 :public  enviromentSensorInterface,Adafruit_BME280
{
public:
    enviroment_BME280(){}
    inline bool init(uint8_t address, TwoWire* i2c= &Wire)
    {
        begin(address,i2c);
        /* Default settings from datasheet. */
        setSampling(Adafruit_BME280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BME280::SAMPLING_X4,     /* Temp. oversampling */
                  Adafruit_BME280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BME280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BME280::FILTER_X16,      /* Filtering. */
                  Adafruit_BME280::STANDBY_MS_1000); /* Standby time. */
        return true ;
    };
    inline double getTemperature()  {return readTemperature();};
    inline double getPressure()     {return readPressure();};
    inline double getHumidity()     {return readHumidity();} ;

};

#endif