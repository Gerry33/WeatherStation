/*
sensorDataPoint[i]. * TempSensors.cpp
 *
 *  Created on: 15 Sep 2017
 *      Author: GSinne
 */

#include <TempSensors.h>
#include <SimpleLog.h>


/*
 * this file is compiled standalone without knowledge of the main procedures. Therefore all must be
 * defined before
  	 problem is that most of these sensors use static singletons. Defining them activates them, at least
 	 with oneWire
 */
// this  blocks the pin if just defined. Therefore we take it out by ifdefs
// putting it to class is difficult:
// https://stackoverflow.com/questions/32472265/how-do-i-write-a-proper-class-definition-declaration-when-a-member-is-a-user-d


 // #endif

const String EMPTY_STRING ="";
const String ERROR_STRING ="Err";


#ifdef ONE_WIRE_BUS_PIN		// nicht in *.h
#include <DallasTemperature.h>
	OneWire oneWire		(ONE_WIRE_BUS_PIN);
	DallasTemperature 	OneWireSensor(&oneWire);
#endif

// #if defined ONE_WIRE_BUS_PIN || SI7021ADDR || DHTPIN

TempSensors::TempSensors ( ){

	_dhtpin 		= -1;
// 	_dhttype 		= DHT::AUTO_DETECT;
	oneWirePin 		= 0;
	oneWireNoDevices= 0;

	memset(sensorDataPoint, 0, sizeof(sensorDataPoint));

	// these are fix, DHT is different

	// inisensorDataPoint
	for (int i =0; i < SENSOR_TYPE_OFS_LAST ; i++) {

			sensorDataPoint[i].max 		= -127.;	// max range of sensor
			sensorDataPoint[i].min 		= +127.;
			sensorDataPoint[i].index	= i;
			sensorDataPoint[i].description	.reserve(40);
			sensorDataPoint[i].MqttPubTopic	.reserve(40);
			sensorDataPoint[i].display		.reserve(10);
			sensorDataPoint[i].errMsg		.reserve(50);
			sensorDataPoint[i].isActive 	= false;
	}
}

SensorDataPoint  * TempSensors::getFreeDataPoint() {

	for ( int i =0; i < MAX_SENSORS_PT ; i++) {
		if (sensorDataPoint [i].isActive == false) {
			sensorDataPoint [i].isActive =true;
			return & sensorDataPoint [i];
		}
	}
	Logger.error ("No more free sensor data points");
	return nullptr;
}

// TODO : more than one of the same type  not supported
SensorDataPoint  * TempSensors::getSensor4Type (SENSOR_TYPE type ) {

	for ( int i =0; i < MAX_SENSORS_PT ; i++) {
		if (sensorDataPoint [i].isActive == true &&  sensorDataPoint [i].type == type) {
			return & sensorDataPoint [i];
		}
	}
	return nullptr;
}

//void  	TempSensors::setLogger ( SimpleLog logger){
//	this->Logger= logger;
//}

/*
 * DHTs must have a dedicated PIN on the ESP. so not possible to create an array per pin.
 */

void  TempSensors::init_DHT (char * devid) {

	#ifdef DHTPIN

	// if defined, it should be there. Otherwise a broken contact cannot be re-constructed without restart
	_dhtpin = DHTPIN;

	SensorDataPoint * pt = getFreeDataPoint();
	if (pt != nullptr) {
		pt->isActive 	= true;
		pt->type 		= SENSOR_TYPE_DHT_TEMP;
		pt->measure 	= MEASURE_TEMP_C;
		pt_DHT_TEMP	= pt;
		pt->description=  "DHT-TEMP: GPIO:" + String (_dhtpin)  ;
		pt->MqttPubTopic= String (devid) + "/OUT/DHT/TEMP";	// later extend if we have more than one

	}

	pt = getFreeDataPoint();
	if (pt != nullptr) {
		pt->isActive 	= true;
		pt->type 		= SENSOR_TYPE_DHT_HUM;
		pt->measure 	= MEASURE_HUM;
		pt_DHT_HUM		= pt;
		pt->description=  "DHT-HUM: GPIO:" + String (_dhtpin) ;
		pt->MqttPubTopic= String (devid) + "/OUT/DHT/HUM";	// later extend if we have more than one
	}


	dht.setup(DHTPIN);

	if ( dht.getStatus() == DHT::ERROR_NONE) {
		_dhttype = dht.getModel();
		Logger.info(PSTR("Found DHT Sensor at GPIO %d,  type:%d\n"), _dhtpin, _dhttype) ;
		pt_DHT_TEMP->description += ",type:" + String (_dhttype);	// type can only be determined if present
		pt_DHT_HUM ->description += ",type:" + String (_dhttype);
		delay (100);
		read_DHT();
	}
	else {
		Logger.info("No DHT Sensor found at GPIO %d\n", _dhtpin);
		return;
	}


#endif
}


void TempSensors::read_DHT (){

	#ifdef DHTPIN

	float hum  	= dht.getHumidity	();
	float temp	= dht.getTemperature();

	DHT::DHT_ERROR_t status  = dht.getStatus(); 		// get DHT status

	if (status == DHT::ERROR_NONE) {

		pt_DHT_HUM ->value 	= hum;
		pt_DHT_TEMP->value 	= temp;
		pt_DHT_HUM->valid	= pt_DHT_TEMP->valid 	= true;
		pt_DHT_HUM->errCode = pt_DHT_TEMP->errCode 	= 0;
		pt_DHT_HUM->errCode = pt_DHT_TEMP->errCode 	= 0;

		pt_DHT_HUM ->display = String (hum, 0 );	// no fraction
		pt_DHT_TEMP->display = String (temp,1 );

//		Logger.debug ("DHT:<%s>, %s\n", pt_DHT_HUM ->MqttPubTopic, pt_DHT_HUM ->display);
//		Logger.debug ("DHT:<%s>, %s\n", pt_DHT_TEMP->MqttPubTopic, pt_DHT_TEMP->display);

	}
	else {

		pt_DHT_HUM->valid	= pt_DHT_TEMP->valid 	= false;
		pt_DHT_HUM->errCode = pt_DHT_TEMP->errCode  = status;

		pt_DHT_HUM->display  = pt_DHT_TEMP->display = ERROR_STRING;// also send as payload to MQTT
		pt_DHT_HUM->errMsg	 = pt_DHT_TEMP->errMsg	=  pt_DHT_TEMP->MqttPubTopic + ";Fehler:" + String(dht.getStatusString());
		Logger.error (pt_DHT_HUM->errMsg); // HUM and TEMP are equal

	}
#endif

}

//int		TempSensors::get_DHT_Pin (){
//	return 	_dhtpin;
//}

// maxsensors

void  TempSensors::init_1Wire( char * devid ) {

#ifdef ONE_WIRE_BUS_PIN

	OneWireSensor.begin();
	OneWireSensor.setWaitForConversion(true);	// very important, otherwise no sensors found
	OneWireSensor.setResolution(TEMP_12_BIT); // Genauigkeit auf 12-Bit setzen

	oneWireNoDevices = OneWireSensor.getDeviceCount();

	if ( oneWireNoDevices  == 0){
		Logger.info ("NO OneWire devices found.\n");
	}
	else {
		Logger.info("Found OneWire devices:%d.\n", oneWireNoDevices);

		if ( oneWireNoDevices > 2) {
			Logger.error("Found 1Wire devices:%d but only 2 supported\n", oneWireNoDevices);
			oneWireNoDevices =2;
		}

		SensorDataPoint * pt;

		for( int i=0; i < oneWireNoDevices; i++) {
			pt = getFreeDataPoint();
			if (pt != nullptr) {
				pt->isActive = true;
				pt->type 	= SENSOR_TYPE_1WIRE_TEMP;
				pt->measure = MEASURE_TEMP_C;
				pt->description  = "1Wire:GPIO:" + String(ONE_WIRE_BUS_PIN);
				pt->MqttPubTopic =  String(devid) + "/OUT/DS/TEMP/" + String (i);
				// can only accept int
				OneWireSensor.setUserDataByIndex(i, pt->index);	// store our index at the sensors struct
				// pt->readSensor = read_something;	// func ptr TODO with index to sensor
			}
		}
		delay (100);
		read_1Wire();	// all  proper
	}
#endif
}

// https://forum.arduino.cc/index.php?topic=403229.0

void TempSensors::read_1Wire( ){

#ifdef ONE_WIRE_BUS_PIN

	SensorDataPoint * pt;

	for( int i=0; i < oneWireNoDevices; i++ ) {

		OneWireSensor.requestTemperaturesByIndex(i); // Send the command to get temperatures

		pt = & sensorDataPoint[ OneWireSensor.getUserDataByIndex(i) ]; // ugly cast

		float t = OneWireSensor.getTempCByIndex(i);

		if (t <= -127.0) {
			pt->errCode = 1;
			pt->valid	= false;
			pt->display =  PSTR("Fehler");
			pt->errMsg  =  PSTR("1Wire Sensor Fehler");
			Logger.error ( pt->errMsg  + ", Pin:"+ String ( ONE_WIRE_BUS_PIN) );
		}
		else{
			pt->value	 	= t;	//	decimal delimiter must be '.' for openhab

		 // sprintf (pt->display, "%d.%d", (int) pt->value, (int)(pt->value * 10.0) % 10 ) ;
			pt->display = String (pt->value,1);
			pt->errCode  	= 0;
			pt->errMsg 		= EMPTY_STRING; 	// cpp string.clear) does not exist
			pt->valid		= true;
		 // Logger.debug ("1Wire:<%s>, %s\n", pt->MqttPubTopic, pt->display);
		}
	}
#endif
}

// SI7021 TEMP Sensor routines -----------------------------------------------------------------------------------------
// onyl one SI 1021 can be driven at one line at a time: http://community.silabs.com/t5/Optical-RH-Temp-Sensor/Can-we-use-2-or-more-pcs-SI7021-on-IIC/td-p/116970

void TempSensors::init_SI7021( String devid, int sda, int scl ) {

#ifdef SI7021ADDR
	SensorDataPoint * pt  = getFreeDataPoint();

	if (pt != nullptr) {
					  pt->isActive 		= true;
					  pt->type 			= SENSOR_TYPE_SI702_TEMP;
					  pt->measure 		= MEASURE_HUM;
					  pt->description	= "SI7021-TEMP:I2C:SCL:" + String( sda ) + ",SDA:"+ String (scl);
					  pt->MqttPubTopic 	= devid + "/OUT/SI/TEMP";
			pt_SI7021_TEMP= pt;
	}

	pt = getFreeDataPoint();
	if (pt != nullptr) {
				  pt->isActive 	= true;
				  pt ->type 	= SENSOR_TYPE_SI702_HUM;
				  pt->measure 	= MEASURE_TEMP_C;

		  pt->description	= "SI7021-HUM: I2C:SCL:" + String( sda ) + ",SDA:"+ String (scl);
		  pt->MqttPubTopic 	= devid + "/OUT/SI/HUM";
		  pt_SI7021_HUM=pt;
	  // pt->readSensor = read_SI7021;	// func ptr TODO with index to sensor

	}
	else {
		Logger.error (PSTR("No free SensorDataPoint slot\n"));
		return ;
	}

	// https://forum.mongoose-os.com/discussion/1675/si7021-on-esp8266-using-arduino-wire
	// https://github.com/ControlEverythingCommunity/SI7021/blob/master/Arduino/SI7021.ino
	Wire.begin(sda, scl);		// wichtig
	delay (300);
	Wire.beginTransmission(SI7021ADDR); //Send humidity measurement command
	int rc = Wire.endTransmission();
	if (rc == 0) {
//			read_SI7021SerialNumber();
//			Logger.info(PSTR("SI7021 init OK. SDA:%d, SCL:%d, I2C-status:%d, SerialA:0x%x, B:0x%x \n"),  sda, scl, rc, sernum_a, sernum_b);
			Logger.info(PSTR("SI7021 init OK. SDA:%d, SCL:%d, I2C-status:%d\n"),  sda, scl, rc);
	}
		else
			Logger.error(PSTR("SI7021 ERROR: adr 0x%02x, InitCode:%d\n"), SI7021ADDR, rc);
	// read_SI7021();

#endif
}


void TempSensors::read_SI7021() {
#ifdef SI7021ADDR
	unsigned int 	data[2];
	float humidity	=	-1;
	float temp 		= -127.;

	// Serial.println("Started reading SI Sensor");

	Wire.beginTransmission(SI7021ADDR); //Send humidity measurement command
	Wire.write(0xF5);
	Wire.endTransmission();
 // delay(500);
	delay(25);	// changed based on adafruit lib

	// Request 2 bytes of data
	Wire.requestFrom(SI7021ADDR, 2);
	int a;
	if( (a = Wire.available()) == 2) 	// Read 2 bytes of data to get humidity
	{
		data[0] = Wire.read();
		data[1] = Wire.read();
		// Convert the data
		humidity   = ((data[0] * 256.0) + data[1]);

		pt_SI7021_HUM->value 	= ((125 * humidity) / 65536.0) - 6;
		pt_SI7021_HUM->display	=  String (pt_SI7021_HUM->value,0)  ;
		pt_SI7021_HUM->errCode 	= 0;
		pt_SI7021_HUM->errMsg	= EMPTY_STRING;
		pt_SI7021_HUM->valid 	= true;

		Wire.beginTransmission(SI7021ADDR); // Send temperature measurement command
		Wire.write(0xF3);					// SI7021_MEASTEMP_NOHOLD_CMD
		Wire.endTransmission();
		delay(25);	// was: 500

		// Request 2 bytes of data
		Wire.requestFrom(SI7021ADDR, 2);

		if(Wire.available() == 2) {

			data[0] = Wire.read(); 	// Read 2 bytes of data for temperature
			data[1] = Wire.read();

			// Convert the data
			temp  		= ((data[0] * 256.0) + data[1]);
			pt_SI7021_TEMP->value   = ((175.72 * temp) / 65536.0) - 46.85;
			pt_SI7021_TEMP->value	=  roundf(pt_SI7021_TEMP->value * 10) / 10; // http://stackoverflow.com/questions/1343890/rounding-number-to-2-decimal-places-in-c

			pt_SI7021_TEMP->errCode	= 0;
			pt_SI7021_TEMP->errMsg 	= EMPTY_STRING;
			pt_SI7021_TEMP->valid 	= true;
			pt_SI7021_TEMP->display = String ( pt_SI7021_TEMP->value, 1);

//			Logger.debug("SI:<"+ pt_SI7021_HUM ->MqttPubTopic + "; HUM:" + pt_SI7021_HUM-> display );
//			Logger.debug("SI:<"+ pt_SI7021_TEMP->MqttPubTopic + "; HUM:" + pt_SI7021_TEMP->display );

		}
	}
	else {

		pt_SI7021_HUM->valid   	= pt_SI7021_TEMP->valid 	= false;
		pt_SI7021_HUM->errCode 	= pt_SI7021_TEMP->errCode  =-1;	// no more details
		pt_SI7021_TEMP ->display= pt_SI7021_HUM ->display = ERROR_STRING;

		Logger.error ("SI7021 Sensor: Err Reading at adr 0x%02x, ErrCode:%d\n", SI7021ADDR, a);
	}
#endif
}
/*
uint8_t  TempSensors::read_SI7021Register (uint8_t reg) {

  uint8_t value;
  Wire.beginTransmission(SI7021ADDR);
  Wire.write((uint8_t)reg);
  Wire.endTransmission(false);

  Wire.requestFrom(SI7021ADDR, 1);
  value = Wire.read();

  //Serial.print("Read $"); Serial.print(reg, HEX); Serial.print(": 0x"); Serial.println(value, HEX);
  return value;
}

// https://github.com/adafruit/Adafruit_Si7021/blob/master/Adafruit_Si7021.h
#define SI7021_ID1_CMD 0xFA0F
#define SI7021_ID2_CMD 0xFCC9

void TempSensors::read_SI7021SerialNumber (void ) {

  Wire.beginTransmission(SI7021ADDR);
  Wire.write((uint8_t)(SI7021_ID1_CMD>>8));
  Wire.write((uint8_t)(SI7021_ID1_CMD&0xFF));
  Wire.endTransmission();

  Wire.requestFrom(SI7021ADDR, 8);
  sernum_a = Wire.read();
  Wire.read();
  sernum_a <<= 8;
  sernum_a |= Wire.read();
  Wire.read();
  sernum_a <<= 8;
  sernum_a |= Wire.read();
  Wire.read();
  sernum_a <<= 8;
  sernum_a |= Wire.read();
  Wire.read();

  Wire.beginTransmission(SI7021ADDR);
  Wire.write((uint8_t)(SI7021_ID2_CMD>>8));
  Wire.write((uint8_t)(SI7021_ID2_CMD&0xFF));
  Wire.endTransmission();

  Wire.requestFrom(SI7021ADDR, 8);
  sernum_b = Wire.read();
  Wire.read();
  sernum_b <<= 8;
  sernum_b |= Wire.read();
  Wire.read();
  sernum_b <<= 8;
  sernum_b |= Wire.read();
  Wire.read();
  sernum_b <<= 8;
  sernum_b |= Wire.read();
  Wire.read();
}
*/

float TempSensors::computeHeatIndex(float temperature, float percentHumidity ) {
  // Using both Rothfusz and Steadman's equations
  // http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
  float hi;

  // if (!isFahrenheit)
    temperature = temperature * 1.8 + 32;

  hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

  if (hi > 79) {
    hi = 	-42.379 +							// C1
             2.04901523 * temperature +			// C2
            10.14333127 * percentHumidity +		// C3
            -0.22475541 * temperature*percentHumidity +	// C4
            -0.00683783 * pow(temperature, 2) +			// C5
            -0.05481717 * pow(percentHumidity, 2) +		// C6
             0.00122874 * pow(temperature, 2) * percentHumidity + // C7
             0.00085282 * temperature*pow(percentHumidity, 2) +		// c8
            -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2); 	//C9

    if((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
      hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

    else if((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
      hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
  }

  // return isFahrenheit ? hi : convertFtoC(hi);
  return (hi - 32) * 0.55555;
}
//#endif
