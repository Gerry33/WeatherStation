/*
 * Common.h
 *
 *  Created on: 21.03.2017
 *      Author: gsi
 *      some common code
 */

#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h> 	// https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPUpdateServer
#include <SimpleLog.h>

const char version[] =  __DATE__ " " __TIME__;

char magicEEKey[] ={"1Q3bJ"}; 	// no magic: just the sign, that we have initialised the EE. Its very unlikely that the EE is delivered with this char seq.

//https://stackoverflow.com/questions/195975/how-to-make-a-char-string-from-a-c-macros-value#196093
#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

#define STR_SSID STR(MY_SSID)
#define STR_PSK  STR(MY_PSK)	// ' MY_PSK' from Ecl. settings:Arduino/compiler options/ add to C/ c++: "-D=...

struct Config {

	char 	magicKey 			[  6 ] ;
	char 	localIP 			[ 16 ] ;
	char 	gatewayIP 			[ 16 ] ;
	char 	dnsIP 				[ 16 ] ;
	char 	syslogServer		[ 30 ] ;
	int 	readingIntervallTemp;		// in secs, not #ifdef 'ed to keep config common
	char 	MqttId				[ 20 ] ;
	char 	mqttServer			[ 20 ] ;
	int     resetTimeMaxMinHr		   ; 	// hh:mm:ss. easier to display
	int     resetTimeMaxMinMin		   ;
	time_t 	lastSwitchTime 		[ 2 ]  ; 	// offset 0 = OFF , offset 1 = ON  (= state of OUT_PIN)
	boolean allowLocalMode			   ;
	char    hostname 			[30 ]  ;
	boolean forcedLocalMode			   ;
	boolean debug 						;

};

Config config;

static WiFiEventHandler WiFiConnectHandler, WiFiDisConnectHandler;

// TODO these must be put outta here into a separate file, but INO files cannot work on extern ...
// https://github.com/Sloeber/arduino-eclipse-plugin/issues/579
// https://github.com/Sloeber/arduino-eclipse-plugin/issues/762

IPAddress ip;
IPAddress gatewayIP;
IPAddress subnet	(255, 255, 255, 0);
IPAddress dnsServerIP;
IPAddress syslogServerIP;

ESP8266HTTPUpdateServer httpUpdater;	// http://www.esp8266.com/wiki/doku.php?id=ota-over-the-air-esp8266
ESP8266WebServer 		server(80);
String 					webPage ;
SimpleLog 				Logger;

extern "C" {
	const char *  bool2CharOnOff 	( bool  t );
	const char *  bool2ChrTrueFalse ( bool  t );
		   int 	  isIp_v4			( char* ip );
		   void   ICACHE_FLASH_ATTR printFloat(float val, char *buff);
}
