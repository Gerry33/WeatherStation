/*
 * Temperatures.h
 *
 *  Created on: 15 Sep 2017
 *      Author: GSinne
 */

#pragma once
#include <SimpleLog.h>

// moved to command line:

// #define ONE_WIRE_BUS_PIN   13	// D7, DS18...  Sensor : D7, gpio 13  // no need to define any More
// #define SI7021ADDR  	 0x40  	// SI7021 I2C; 	address is 0x40(64)
// #define DHTPIN  		   12	// D6, gpio 12: own protocol

// wiring:http://iot-playground.com/blog/2-uncategorised/41-esp8266-ds18b20-temperature-sensor-arduino-ide
#ifdef ONE_WIRE_BUS_PIN
	#include <OneWire.h> // https://github.com/milesburton/Arduino-Temperature-Control-Library
	#include <DallasTemperature.h>
#endif

#ifdef DHTPIN
#include <DHT.h>
#endif

#include <Wire.h>			// I2C

enum SENSOR_TYPE {
	SENSOR_TYPE_DHT_TEMP   =0,
	SENSOR_TYPE_DHT_HUM    =1,
	SENSOR_TYPE_SI702_TEMP =2,
	SENSOR_TYPE_SI702_HUM  =3,
	SENSOR_TYPE_1WIRE_TEMP =4,
	SENSOR_TYPE_OFS_LAST   =5			// dummy to fill loops and dim fields
} ;

enum MEASURE {
	MEASURE_TEMP_C,
	MEASURE_HUM,
	MEASURE_PRESSURE,
};

#define MAX_SENSORS_PT	4		// sensor data point. one sensor can have more than one data point

typedef struct SensorDataPoint_t {

	// index in sensorDataPoint[] ; used to identify between sensor no and this index
	// currently used only for 1Wire, cause the 1wire struct can only accept int
	int 		index;
	bool  		isActive;
	String  	description ;	// for display
	String  	MqttPubTopic; // the last part of the topic for MQTT publishing. mqtt devid is added in front
	float 		value;
	bool  		valid;
	float 		max, min;
	String  	display;	// value to display
	int   		errCode;	// 0 =ok
	String  	errMsg ;
	SENSOR_TYPE type;
	MEASURE 	measure;
	// void 		(TempSensors::)readSensorFunc ( );
	// std::function<void(int)> readSensor;	// TODO : YODI

}SensorDataPoint;

class TempSensors {

public:

	TempSensors ( );
	SensorDataPoint sensorDataPoint [ MAX_SENSORS_PT ];	// sensor data point

	// SensorDataPoint_N sensorDataPointn;

	void  		init_DHT( char * devid);
    void 		read_DHT ();
    int			get_DHT_Type();

    void 		init_1Wire(char * devid);
    void 		read_1Wire ();

    void 		init_SI7021(String , int sda, int scl);
    void 		read_SI7021();
    /*
    uint8_t  	read_SI7021Register (uint8_t reg);
    void 		read_SI7021SerialNumber(void);
    uint32_t 	sernum_a, sernum_b;		// 7021 serial numbers. no functional usage.
    */
    float 		computeHeatIndex(float temperature, float percentHumidity );

    SensorDataPoint * getFreeDataPoint	( 	);
    SensorDataPoint * getSensor4Type 	(SENSOR_TYPE type);

    SensorDataPoint * 	pt_SI7021_TEMP;
    SensorDataPoint * 	pt_SI7021_HUM;

    SensorDataPoint * 	pt_DHT_TEMP;
    SensorDataPoint * 	pt_DHT_HUM;

  private:
    int 				oneWirePin;
    int 				oneWireNoDevices;
	int 				_dhtpin;
#ifdef DHTPIN
	DHT::DHT_MODEL_t 	_dhttype;
	DHT dht;
#endif
};

// #endif
