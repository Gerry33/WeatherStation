/**The MIT License (MIT)

Copyright (c) 2015 by Daniel Eichhorn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at http://blog.squix.ch

	Adaptation by Gerry33:
	 - replaced WifiClient with HTTPClient
	 - Parser object local
	 - getForecastWeekDay(int period)
	 - forecast index linear 0 = today, 1= tommorow, 2 = ...
	 - pressure trend
	 - wind dir
	 - HTTP return code check
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "WundergroundClient.h"
#include <SimpleLog.h>

bool usePM = false; // Set to true if you want to use AM/PM time disaply
bool isPM = false; // JJG added ///////////

extern SimpleLog  Logger;	// not the  fine style, but pragmatic

WundergroundClient::WundergroundClient(boolean _isMetric) {
	isMetric = _isMetric;
	parser.setListener(this);
}

// Added by fowlerk, 12/22/16, as an option to change metric setting other than at instantiation
void WundergroundClient::initMetric(boolean _isMetric) {
	isMetric = _isMetric;
}
// end add fowlerk, 12/22/16

void WundergroundClient::updateConditions(String apiKey, String language, String country, String city) {
	isForecast = false;
	doUpdate(wuBaseUri + "/api/" + apiKey + "/conditions/lang:" + language + "/q/" + country + "/" + city + ".json");

}

void WundergroundClient::updateConditions(String host, int port, String url) {
	isForecast = false;
	doUpdate(host, port, url);
}

// wunderground change the API URL scheme:
// http://api.wunderground.com/api/<API-KEY>/conditions/lang:DL/q/zmw:00000.215.10348.json
void WundergroundClient::updateConditions(String apiKey, String language, String zmwCode) {
	isForecast = false;
	doUpdate(  wuBaseUri + apiKey + "/conditions/lang:" + language + "/q/zmw:" + zmwCode + ".json");
}

void WundergroundClient::updateConditions(String uri) {
	isForecast = false;
	doUpdate(uri);
}

void WundergroundClient::updateConditionsPWS(String apiKey, String language, String pws) {
	isForecast = false;
	doUpdate(wuBaseUri + "/api/" + apiKey + "/conditions/lang:" + language + "/q/pws:" + pws + ".json");
}

void WundergroundClient::updateForecast(String apiKey, String language, String country, String city) {
	isForecast = true;
	doUpdate(wuBaseUri + "/api/" + apiKey + "/forecast10day/lang:" + language + "/q/" + country + "/" + city + ".json");
}

void WundergroundClient::updateForecast(String uri) {
	isForecast = true;
	doUpdate (uri);
}

void WundergroundClient::updateForecast(String host, int port, String url) {
	isForecast = true;
	doUpdate(host, port, url);
}

void WundergroundClient::updateForecastPWS(String apiKey, String language, String pws) {
	isForecast = true;
	doUpdate(wuBaseUri + "/api/" + apiKey + "/forecast10day/lang:" + language + "/q/pws:" + pws + ".json");
}

void WundergroundClient::updateForecastZMW(String apiKey, String language, String zmwCode) {
	isForecast = true;
	doUpdate(wuBaseUri + "/api/" + apiKey + "/forecast10day/lang:" + language + "/q/zmw:" + zmwCode + ".json");
}

// JJG added ////////////////////////////////
void WundergroundClient::updateAstronomy(String apiKey, String language, String country, String city) {
	isForecast = true;
	doUpdate(wuBaseUri + "/api/" + apiKey + "/astronomy/lang:" + language + "/q/" + country + "/" + city + ".json");
}
// end JJG add  ////////////////////////////////////////////////////////////////////

void WundergroundClient::updateAstronomyPWS(String apiKey, String language, String pws) {
	isForecast = true;
	doUpdate(wuBaseUri + "/api/" + apiKey + "/astronomy/lang:" + language + "/q/pws:" + pws + ".json");
}
// fowlerk added
void WundergroundClient::updateAlerts(String apiKey, String language, String country, String city) {
	currentAlert = 0;
	activeAlertsCnt = 0;
	isForecast = false;
	isSimpleForecast = false;
	isCurrentObservation = false;
	isAlerts = true;
	if (country == "US") {
		isAlertUS = true;
		isAlertEU = false;
	} else {
		isAlertUS = false;
		isAlertEU = true;
	}
	doUpdate(wuBaseUri + "/api/" + apiKey + "/alerts/lang:" + language + "/q/" + country + "/" + city + ".json");
}
// end fowlerk add

void WundergroundClient::updateAlertsPWS(String apiKey, String language, String country, String pws) {
	currentAlert = 0;
	activeAlertsCnt = 0;
	isForecast = false;
	isSimpleForecast = false;
	isCurrentObservation = false;
	isAlerts = true;
	if (country == "US") {
		isAlertUS = true;
		isAlertEU = false;
	} else {
		isAlertUS = false;
		isAlertEU = true;
	}
	doUpdate( wuBaseUri + "/api/" + apiKey + "/alerts/lang:" + language + "/q/pws:" + pws + ".json");
}

void WundergroundClient::doUpdate(String host, int port, String url) {

	http.begin(host, port, url);
	doUpdateIntern();
}


void WundergroundClient::doUpdate(String fullUri) {

		http.begin(fullUri );
		doUpdateIntern();

}

void WundergroundClient::doUpdateIntern() {

		parser.reset();

		bool isBody = false;
		char c;
		int size;
		long start = millis();
		// http.setReuse(true);	// DO NOT USE gives hassle at least with TOMCAT server
		// start connection and send HTTP header
		int httpCode = http.GET();

		Logger.debug("[HTTP] GET end, code: %d, time:%ld\n", httpCode, millis() - start);

		if(httpCode > 0 && httpCode == 200) {

			start = millis();
			WiFiClient * client = http.getStreamPtr();

			while(client->connected()) {
				while((size = client->available()) > 0) {
					c = client->read();
					if (c == '{' || c == '[') {
						isBody = true;
					}
					if (isBody) {
						parser.parse(c);
					}
				}
			}
		}
		else
			Logger.error("[HTTP] GET error code:%d\n", httpCode);
		http.end();
		Logger.debug("Parsing end.Time:%ld\n", millis() - start);

	}


// Deprecated : do not use --- has hickup issues
/*
void WundergroundClient::doUpdate(String url) {

	parser.setListener(this);
	WiFiClient client;
	const int httpPort = 80;
	bool connected = false;

	for ( int i=0 ; i < 3 ; i++) {

		if (client.connect("api.wunderground.com", httpPort)) {
			connected = true;
			break;
		}
		else {
			Serial.println("Connection failed to api.wunderground.com. Retry no " + String (i));
			delay (2000);
		}
	}

	if ( !connected) {
		Serial.println("Unable to connect to <" + url + "> Giving up - for now.");
		return;
	}

	Serial.print("Requesting URL:<");
	Serial.println(url);

	// This will send the request to the server
	client.print(String("GET ") + url + " HTTP/1.1\r\n" +
			"Host: api.wunderground.com\r\n" +
			"Connection: close\r\n\r\n");
	int retryCounter = 0;
	while(!client.available()) {
		delay(1000);
		retryCounter++;
		if (retryCounter > 10) {
			return;
		}
	}

	int pos = 0;
	boolean isBody = false;
	char c;

	int size = 0;
	client.setNoDelay(false);
	while(client.connected()) {
		while((size = client.available()) > 0) {
			c = client.read();
			if (c == '{' || c == '[') {
				isBody = true;
			}
			if (isBody) {
				parser.parse(c);
			}
		}
	}
}

*/
void WundergroundClient::whitespace(char c) {
	// Serial.println("whitespace");
}

long startParsingTime = -1L;

void WundergroundClient::startDocument() {
	// Serial.println("Parser:StartDocument");
	if ( startParsingTime > 0)
		Logger.error("ERROR:Parser:Start/End sequence failure.");
	startParsingTime = millis();

}

void WundergroundClient::endDocument() {
	// Logger.debug ("Parser:EndDocument:time:%ld\n", millis() - startParsingTime );
	startParsingTime = 0L;
}


void WundergroundClient::key(String key) {
	currentKey = String(key);
	//	Restructured following logic to accomodate the multiple types of JSON returns based on the API.  This was necessary since several
	//	keys are reused between various types of API calls, resulting in confusing returns in the original function.  Various booleans
	//	now indicate whether the JSON stream being processed is part of the text forecast (txt_forecast), the first section of the 10-day
	//	forecast API that contains detailed text for the forecast period; the simple forecast (simpleforecast), the second section of the
	//	10-day forecast API that contains such data as forecast highs/lows, conditions, precipitation / probabilities; the current
	//	observations (current_observation), from the observations API call; or alerts (alerts), for the future) weather alerts API call.
	//		Added by fowlerk...18-Dec-2016
	if (currentKey == "txt_forecast") {
		isForecast = true;
		isCurrentObservation = false;	// fowlerk
		isSimpleForecast = false;		// fowlerk
		isAlerts = false;				// fowlerk
		// Serial.println("UN-Detected simpleforecast");
	}
	if (currentKey == "simpleforecast") {
		isSimpleForecast 	= true;
		isCurrentObservation = false;	// fowlerk
		isForecast = false;				// fowlerk
		isAlerts = false;				// fowlerk
		// Serial.println("Detected simpleforecast");
	}
	//  Added by fowlerk...
	if (currentKey == "current_observation") {
		isCurrentObservation = true;
		isSimpleForecast = false;
		isForecast = false;
		isAlerts = false;
	}
	if (currentKey == "alerts") {
		isCurrentObservation = false;
		isSimpleForecast = false;
		isForecast = false;
		isAlerts = true;
	}
	// end fowlerk add
}

void WundergroundClient::value(String value) {
	if (currentKey == "local_epoch") {
	// if (currentKey ==      "epoch") {
		localEpoc = value.toInt();
		localMillisAtUpdate = millis();
	}

	// JJG added ... //////////////////////// search for keys /////////////////////////
	if (currentKey == "percentIlluminated") {
		moonPctIlum = value;
	}

	if (currentKey == "ageOfMoon") {
		moonAge = value;
	}

	if (currentKey == "phaseofMoon") {
		moonPhase = value;
	}


	if (currentParent == "sunrise") {      // Has a Parent key and 2 sub-keys
		if (currentKey == "hour") {
			int tempHour = value.toInt();    // do this to concert to 12 hour time (make it a function!)
			if (usePM && tempHour > 12){
				tempHour -= 12;
				isPM = true;
			}
			else isPM = false;
			char tempHourBuff[3] = "";						// fowlerk add for formatting, 12/22/16
			sprintf(tempHourBuff, "%2d", tempHour);			// fowlerk add for formatting, 12/22/16
			sunriseTime = String(tempHourBuff);				// fowlerk add for formatting, 12/22/16
			//sunriseTime = value;
		}
		if (currentKey == "minute") {
			char tempMinBuff[3] = "";						// fowlerk add for formatting, 12/22/16
			sprintf(tempMinBuff, "%02d", value.toInt());	// fowlerk add for formatting, 12/22/16
			sunriseTime += ":" + String(tempMinBuff);		// fowlerk add for formatting, 12/22/16
			if (isPM) sunriseTime += "pm";
			else if (usePM) sunriseTime += "am";
		}
	}

	if (currentParent == "sunset") {      // Has a Parent key and 2 sub-keys
		if (currentKey == "hour") {
			int tempHour = value.toInt();   // do this to concert to 12 hour time (make it a function!)
			if (usePM && tempHour > 12){
				tempHour -= 12;
				isPM = true;
			}
			else isPM = false;
			char tempHourBuff[3] = "";						// fowlerk add for formatting, 12/22/16
			sprintf(tempHourBuff, "%2d", tempHour);			// fowlerk add for formatting, 12/22/16
			sunsetTime = String(tempHourBuff);				// fowlerk add for formatting, 12/22/16
			// sunsetTime = value;
		}
		if (currentKey == "minute") {
			char tempMinBuff[3] = "";						// fowlerk add for formatting, 12/22/16
			sprintf(tempMinBuff, "%02d", value.toInt());	// fowlerk add for formatting, 12/22/16
			sunsetTime += ":" + String(tempMinBuff);		// fowlerk add for formatting, 12/22/16
			if (isPM) sunsetTime += "pm";
			else if(usePM) sunsetTime += "am";
		}
	}

	if (currentParent == "moonrise") {      // Has a Parent key and 2 sub-keys
		if (currentKey == "hour") {
			int tempHour = value.toInt();   // do this to concert to 12 hour time (make it a function!)
			if (usePM && tempHour > 12){
				tempHour -= 12;
				isPM = true;
			}
			else isPM = false;
			char tempHourBuff[3] = "";						// fowlerk add for formatting, 12/22/16
			sprintf(tempHourBuff, "%2d", tempHour);			// fowlerk add for formatting, 12/22/16
			moonriseTime = String(tempHourBuff);			// fowlerk add for formatting, 12/22/16
			// moonriseTime = value;
		}
		if (currentKey == "minute") {
			char tempMinBuff[3] = "";						// fowlerk add for formatting, 12/22/16
			sprintf(tempMinBuff, "%02d", value.toInt());	// fowlerk add for formatting, 12/22/16
			moonriseTime += ":" + String(tempMinBuff);		// fowlerk add for formatting, 12/22/16
			if (isPM) moonriseTime += "pm";
			else if (usePM) moonriseTime += "am";
		}
	}

	if (currentParent == "moonset") {      // Not used - has a Parent key and 2 sub-keys
		if (currentKey == "hour") {
			char tempHourBuff[3] = "";						// fowlerk add for formatting, 12/22/16
			sprintf(tempHourBuff, "%2d", value.toInt());	// fowlerk add for formatting, 12/22/16
			moonsetTime = String(tempHourBuff);				// fowlerk add for formatting, 12/22/16
		}
		if (currentKey == "minute") {
			char tempMinBuff[3] = "";						// fowlerk add for formatting, 12/22/16
			sprintf(tempMinBuff, "%02d", value.toInt());	// fowlerk add for formatting, 12/22/16
			moonsetTime += ":" + String(tempMinBuff);		// fowlerk add for formatting, 12/22/16
		}
	}

	if (currentKey == "wind_mph" && !isMetric) {
		windSpeed = value + "mph";
	}

	if (currentKey == "wind_kph" && isMetric) {
		value.trim();	// has leading blanks
		windSpeed = value.substring(0, value.lastIndexOf(".")) + " km/h";	// no fraction
	}

	if (currentKey == "wind_dir") {
		windDir = value;
	}
	// gsi: always '0' ???
	if (currentKey == "pressure_trend") {
		pressure_trend = value;
	}

	if (currentKey == "wind_degrees") {
		wind_degrees = value;
	}

	// end JJG add  ////////////////////////////////////////////////////////////////////

	// gsi:this allows to check if a station is failed.
	// in epoch its easier to calculate the differences. the rfc times must be converted first.
	if (currentKey == "observation_epoch") {
		observation_epoch = value.toInt();
	}

	if (currentKey == "local_time_rfc822") {
		date = value.substring(0, 16);
	}

	if (currentKey == "observation_time_rfc822") {
		observationDate = value.substring(0, 16);
	}
	// Begin add, fowlerk...04-Dec-2016
	if (currentKey == "observation_time") {
		observationTime = value;
	}
	// end add, fowlerk

	if (currentKey == "temp_f" && !isMetric) {
		currentTemp = value;
	}
	if (currentKey == "temp_c" && isMetric) {
		currentTemp = value;
	}
	if (currentKey == "icon") {

//		Serial.println("Icon: period:"		+ String(currentForecastPeriod)
//				+ ", icon name:<" 			+ value
//				+ ">, currentParent:<" 		+ currentParent
//				+ ">, isSimpleForecast:" 	+ String ( isSimpleForecast)
//				+ ">, isForecast:" 			+ String ( isForecast) );

		// see above: if (currentKey == "simpleforecast") { ...

		if (isForecast== false && isSimpleForecast == true  && currentForecastPeriod < MAX_FORECAST_PERIODS) {
			// Serial.println("Forecast icon: idx:"+  String(currentForecastPeriod) + ", icon name:<" + value + ">");
			forecastIcon[currentForecastPeriod] = value;
		}
		// if (!isForecast) {													// Removed by fowlerk
		if (isCurrentObservation && !(isForecast || isSimpleForecast)) {		// Added by fowlerk
			weatherIcon = value;
		}
	}
	if (currentKey == "weather") {
		weatherText = value;
	}
	if (currentKey == "relative_humidity") {
		humidity = value;
	}
	if (currentKey == "pressure_mb" && isMetric) {
		pressure = value + "mb";
	}
	if (currentKey == "pressure_in" && !isMetric) {
		pressure = value + "in";
	}
	// fowlerk added...
	if (currentKey == "feelslike_f" && !isMetric) {
		feelslike = value;
	}

	if (currentKey == "feelslike_c" && isMetric) {
		feelslike = value;
	}

	if (currentKey == "UV") {
		UV = value;
	}

	// Active alerts...added 18-Dec-2016
	if (currentKey == "type" && isAlerts) {
		activeAlertsCnt++;
		currentAlert++;
		activeAlerts[currentAlert-1] = value;
		Serial.print("Alert type processed, value:  "); Serial.println(activeAlerts[currentAlert-1]);
	}
	if (currentKey == "description" && isAlerts && isAlertUS) {
		activeAlertsText[currentAlert-1] = value;
		Serial.print("Alert description processed, value:  "); Serial.println(activeAlertsText[currentAlert-1]);
	}
	if (currentKey == "wtype_meteoalarm_name" && isAlerts && isAlertEU) {
		activeAlertsText[currentAlert-1] = value;
		Serial.print("Alert description processed, value:  "); Serial.println(activeAlertsText[currentAlert-1]);
	}
	if (currentKey == "message" && isAlerts) {
		activeAlertsMessage[currentAlert-1] = value;
		Serial.print("Alert msg length:  "); Serial.println(activeAlertsMessage[currentAlert-1].length());
		if(activeAlertsMessage[currentAlert-1].length() >= 511) {
			activeAlertsMessageTrunc[currentAlert-1] = true;
		} else {
			activeAlertsMessageTrunc[currentAlert-1] = false;
		}
		Serial.print("Alert message processed, value:  "); Serial.println(activeAlertsMessage[currentAlert-1]);
	}
	if (currentKey == "date" && isAlerts) {
		activeAlertsStart[currentAlert-1] = value;
		// Check last char for a "/"; the returned value sometimes includes this; if so, strip it (47 is a "/" char)
		if (activeAlertsStart[currentAlert-1].charAt(activeAlertsStart[currentAlert-1].length()-1) == 47) {
			Serial.println("...last char is a slash...");
			activeAlertsStart[currentAlert-1] = activeAlertsStart[currentAlert-1].substring(0,(activeAlertsStart[currentAlert-1].length()-1));
		}
		// For meteoalarms, the start field is returned with the UTC=0 by default (not used?)
		if (isAlertEU && activeAlertsStart[currentAlert-1] == "1970-01-01 00:00:00 GMT") {
			activeAlertsStart[currentAlert-1] = "<Not specified>";
		}
		Serial.print("Alert start processed, value:  "); Serial.println(activeAlertsStart[currentAlert-1]);
	}
	if (currentKey == "expires" && isAlerts) {
		activeAlertsEnd[currentAlert-1] = value;
		Serial.print("Alert expiration processed, value:  "); Serial.println(activeAlertsEnd[currentAlert-1]);
	}
	if (currentKey == "phenomena" && isAlerts) {
		activeAlertsPhenomena[currentAlert-1] = value;
		Serial.print("Alert phenomena processed, value:  "); Serial.println(activeAlertsPhenomena[currentAlert-1]);
	}
	if (currentKey == "significance" && isAlerts && isAlertUS) {
		activeAlertsSignificance[currentAlert-1] = value;
		Serial.print("Alert significance processed, value:  "); Serial.println(activeAlertsSignificance[currentAlert-1]);
	}
	// Map meteoalarm level to the field for significance for consistency (used for European alerts)
	if (currentKey == "level_meteoalarm" && isAlerts && isAlertEU) {
		activeAlertsSignificance[currentAlert-1] = value;
		Serial.print("Meteo alert significance processed, value:  "); Serial.println(activeAlertsSignificance[currentAlert-1]);
	}
	// For meteoalarms only (European alerts); attribution must be displayed according to the T&C's of use
	if (currentKey == "attribution" && isAlerts) {
		activeAlertsAttribution[currentAlert-1] = value;
		// Remove some of the markup in the attribution
		activeAlertsAttribution[currentAlert-1].replace(" <a href='"," ");
		activeAlertsAttribution[currentAlert-1].replace("</a>","");
		activeAlertsAttribution[currentAlert-1].replace("/'>"," ");
	}

	// end fowlerk add

	if (currentKey == "dewpoint_f" && !isMetric) {
		dewPoint = value;
	}
	if (currentKey == "dewpoint_c" && isMetric) {
		dewPoint = value;
	}
	if (currentKey == "precip_today_metric" && isMetric) {
		precipitationToday = value + "mm";
	}
	if (currentKey == "precip_today_in" && !isMetric) {
		precipitationToday = value + "in";
	}
	if (currentKey == "period") {
		currentForecastPeriod = value.toInt();
	}
	// Modified below line to add check to ensure we are processing the 10-day forecast
	// before setting the forecastTitle (day of week of the current forecast day).
	// (The keyword title is used in both the current observation and the 10-day forecast.)
	//		Modified by fowlerk
	// if (currentKey == "title" && currentForecastPeriod < MAX_FORECAST_PERIODS) {				// Removed, fowlerk
	if (currentKey == "title" && isForecast && currentForecastPeriod < MAX_FORECAST_PERIODS) {
		// Serial.println(String(currentForecastPeriod) + ": " + value);
		forecastTitle[currentForecastPeriod] = value;
	}

	// Added forecastText key following...fowlerk, 12/3/16
	if (currentKey == "fcttext" && isForecast && !isMetric && currentForecastPeriod < MAX_FORECAST_PERIODS) {
		forecastText[currentForecastPeriod] = value;
	}
	// Added option for metric forecast following...fowlerk, 12/22/16
	if (currentKey == "fcttext_metric" && isForecast && isMetric && currentForecastPeriod < MAX_FORECAST_PERIODS) {
		forecastText[currentForecastPeriod] = value;
	}
	// end fowlerk add, 12/3/16

	// Added PoP (probability of precipitation) key following...fowlerk, 12/22/16
	if (currentKey == "pop" && isForecast && currentForecastPeriod < MAX_FORECAST_PERIODS) {
		PoP[currentForecastPeriod] = value;
	}
	// end fowlerk add, 12/22/16

	// The detailed forecast period has only one forecast per day with low/high for both
	// night and day, starting at index 1.
	// int dailyForecastPeriod = (currentForecastPeriod - 1) * 2 ;

//	if (currentKey == "fahrenheit" && !isMetric && dailyForecastPeriod < MAX_FORECAST_PERIODS) {
//
//		if (currentParent == "high") {
//			forecastHighTemp[dailyForecastPeriod] = value;
//		}
//		if (currentParent == "low") {
//			forecastLowTemp[dailyForecastPeriod] = value;
//		}
//	}

	if (currentKey == "celsius" && isMetric && currentForecastPeriod < MAX_FORECAST_PERIODS) {

		if (currentParent == "high") {
			// Serial.println("TempHigh:idx" + String(currentForecastPeriod)+ ": " + value);
		 // forecastHighTemp[dailyForecastPeriod] = value; gsi
			forecastHighTemp[currentForecastPeriod] = value;
		}
		if (currentParent == "low") {
		 // forecastLowTemp[dailyForecastPeriod] = value;
			forecastLowTemp[currentForecastPeriod] = value;
			// Serial.println("TempLow:idx" + String(currentForecastPeriod)+ ": " + value);
		}
	}
	// fowlerk added...to pull month/day from the forecast period
	if (currentKey == "month" && isSimpleForecast && currentForecastPeriod < MAX_FORECAST_PERIODS)  {
		//	Added by fowlerk to handle transition from txtforecast to simpleforecast, as
		//	the key "period" doesn't appear until after some of the key values needed and is
		//	used as an array index.
		if (isSimpleForecast && currentForecastPeriod == 19) {
			currentForecastPeriod = 0;
		}
		forecastMonth[currentForecastPeriod] = value;
	}

	if (currentKey == "day" && isSimpleForecast && currentForecastPeriod < MAX_FORECAST_PERIODS)  {
		//	Added by fowlerk to handle transition from txtforecast to simpleforecast, as
		//	the key "period" doesn't appear until after some of the key values needed and is
		//	used as an array index.
		if (isSimpleForecast && currentForecastPeriod == 19) {
			currentForecastPeriod = 0;
		}
		forecastDay[currentForecastPeriod] = value;
		// Serial.println("---currentForecastPeriod:" + String(currentForecastPeriod) + ",value:"+ value);
	}
	// gsi:new -----: ??? : currentForecastPeriod ist eins zu niedrig ???
	// if (currentKey == "weekday_short" && isSimpleForecast && currentForecastPeriod < MAX_FORECAST_PERIODS)  {
	  if (currentKey == "weekday_short" 					 && currentForecastPeriod < MAX_FORECAST_PERIODS)  {
		if (isSimpleForecast && currentForecastPeriod == 19) {
			currentForecastPeriod = 0;
		}
		forecastWeekDay[currentForecastPeriod + 1] = value;
		// Serial.println("Weekday:Idx:" + String(currentForecastPeriod+1) + ",value:"+ value);
	}
	// end fowlerk add
}

void WundergroundClient::endArray() {

}


void WundergroundClient::startObject() {
	currentParent = currentKey;
}

void WundergroundClient::endObject() {
	currentParent = "";
}


void WundergroundClient::startArray() {

}


String WundergroundClient::getHours() {
	if (localEpoc == 0) {
		return "--";
	}
	int hours = (getCurrentEpoch()  % 86400L) / 3600 + gmtOffset;
	if (hours < 10) {
		return "0" + String(hours);
	}
	return String(hours); // print the hour (86400 equals secs per day)

}
String WundergroundClient::getMinutes() {
	if (localEpoc == 0) {
		return "--";
	}
	int minutes = ((getCurrentEpoch() % 3600) / 60);
	if (minutes < 10 ) {
		// In the first 10 minutes of each hour, we'll want a leading '0'
		return "0" + String(minutes);
	}
	return String(minutes);
}
String WundergroundClient::getSeconds() {
	if (localEpoc == 0) {
		return "--";
	}
	int seconds = getCurrentEpoch() % 60;
	if ( seconds < 10 ) {
		// In the first 10 seconds of each minute, we'll want a leading '0'
		return "0" + String(seconds);
	}
	return String(seconds);
}
String WundergroundClient::getDate() {
	return date;
}
String WundergroundClient::getObservationDate() {
	return observationDate;
}
long WundergroundClient::getCurrentEpoch() {
	return localEpoc + ((millis() - localMillisAtUpdate) / 1000);
}

// JJG added ... /////////////////////////////////////////////////////////////////////////////////////////
String WundergroundClient::getMoonPctIlum() {
	return moonPctIlum;
}

String WundergroundClient::getMoonAge() {
	return moonAge;
}

String WundergroundClient::getMoonPhase() {
	return moonPhase;
}

String WundergroundClient::getSunriseTime() {
	return sunriseTime;
}

String WundergroundClient::getSunsetTime() {
	return sunsetTime;
}

String WundergroundClient::getMoonriseTime() {
	return moonriseTime;
}

String WundergroundClient::getMoonsetTime() {
	return moonsetTime;
}

String WundergroundClient::getWindSpeed() {
	return windSpeed;
}

String WundergroundClient::getWindDir() {
	return windDir;
}

String WundergroundClient::getWindDegrees(){
	return wind_degrees;
}
// end JJG add ////////////////////////////////////////////////////////////////////////////////////////////

String WundergroundClient::getCurrentTemp() {
	return currentTemp;
}

String WundergroundClient::getWeatherText() {
	return weatherText;
}

String WundergroundClient::getHumidity() {
	return humidity;
}

String WundergroundClient::getPressure() {
	return pressure;
}

String WundergroundClient::getDewPoint() {
	return dewPoint;
}
// fowlerk added...
String WundergroundClient::getFeelsLike() {
	return feelslike;
}

String WundergroundClient::getUV() {
	return UV;
}

// Added by fowlerk, 04-Dec-2016
String WundergroundClient::getObservationTime() {
	return observationTime;
}

// Active alerts...added 18-Dec-2016
String WundergroundClient::getActiveAlerts(int alertIndex) {
	return activeAlerts[alertIndex];
}

String WundergroundClient::getActiveAlertsText(int alertIndex) {
	return activeAlertsText[alertIndex];
}

String WundergroundClient::getActiveAlertsMessage(int alertIndex) {
	return activeAlertsMessage[alertIndex];
}

bool WundergroundClient::getActiveAlertsMessageTrunc(int alertIndex) {
	return activeAlertsMessageTrunc[alertIndex];
}

String WundergroundClient::getActiveAlertsStart(int alertIndex) {
	return activeAlertsStart[alertIndex];
}

String WundergroundClient::getActiveAlertsEnd(int alertIndex) {
	return activeAlertsEnd[alertIndex];
}

String WundergroundClient::getActiveAlertsPhenomena(int alertIndex) {
	return activeAlertsPhenomena[alertIndex];
}

String WundergroundClient::getActiveAlertsSignificance(int alertIndex) {
	return activeAlertsSignificance[alertIndex];
}

String WundergroundClient::getActiveAlertsAttribution(int alertIndex) {
	return activeAlertsAttribution[alertIndex];
}

int WundergroundClient::getActiveAlertsCnt() {
	return activeAlertsCnt;
}

// end fowlerk add


String WundergroundClient::getPrecipitationToday() {
	return precipitationToday;
}
//
//String WundergroundClient::getTodayIcon() {
//
// 	return getMeteoconIcon(weatherIcon);
//
//}

String WundergroundClient::getTodayIconText() {
	return weatherIcon;
}

String WundergroundClient::getForecastIcon(int period) {
 // return getMeteoconIcon(forecastIcon[period]);	// gsi : removed
		return forecastIcon[period];
}

String WundergroundClient::getForecastTitle(int period) {
	return forecastTitle[period];
}

String WundergroundClient::getForecastLowTemp(int period) {
	return forecastLowTemp[period];
}

String WundergroundClient::getForecastHighTemp(int period) {
	return forecastHighTemp[period];
}
// fowlerk added...
String WundergroundClient::getForecastDay(int period) {
	//  Serial.print("Day period:  "); Serial.println(period);
	return forecastDay[period];
}

// gsi added...
String WundergroundClient::getForecastWeekDay(int period) {
	//  Serial.print("Day period:  "); Serial.println(period);
	return forecastWeekDay[period];
}

String WundergroundClient::getForecastMonth(int period) {
	//  Serial.print("Month period:  "); Serial.println(period);
	return forecastMonth[period];
}

String WundergroundClient::getForecastText(int period) {
	//  Serial.print("Forecast period:  "); Serial.println(period);
	return forecastText[period];
}

// Added PoP...12/22/16
String WundergroundClient::getPoP(int period) {
	return PoP[period];
}
// end fowlerk add

/* gsi: obsolete: this method is complete garbage: first translate into a shortcut, then
	retranslate into a long icon name in weatherstation.getMeteoconIcon()

String WundergroundClient::getMeteoconIcon(String iconText) {
	if (iconText == "chanceflurries") 	return "F";
	if (iconText == "chancerain") 		return "Q";
	if (iconText == "chancesleet") 		return "W";
	if (iconText == "chancesnow") 		return "V";
	if (iconText == "chancetstorms") 	return "S";
	if (iconText == "clear") 			return "B";
	if (iconText == "cloudy") 			return "Y";
	if (iconText == "flurries") 		return "F";
	if (iconText == "fog") 				return "M";
	if (iconText == "hazy") 			return "E";
	if (iconText == "mostlycloudy") 	return "Y";
	if (iconText == "mostlysunny") 		return "H";
	if (iconText == "partlycloudy") 	return "H";
	if (iconText == "partlysunny") 		return "J";
	if (iconText == "sleet") 			return "W";
	if (iconText == "rain") return "R";
	if (iconText == "snow") return "W";
	if (iconText == "sunny") return "B";
	if (iconText == "tstorms") return "0";

	if (iconText == "nt_chanceflurries") return "F";
	if (iconText == "nt_chancerain") return "7";
	if (iconText == "nt_chancesleet") return "#";
	if (iconText == "nt_chancesnow") return "#";
	if (iconText == "nt_chancetstorms") return "&";
	if (iconText == "nt_clear") return "2";
	if (iconText == "nt_cloudy") return "Y";
	if (iconText == "nt_flurries") return "9";
	if (iconText == "nt_fog") return "M";
	if (iconText == "nt_hazy") return "E";
	if (iconText == "nt_mostlycloudy") return "5";
	if (iconText == "nt_mostlysunny") return "3";
	if (iconText == "nt_partlycloudy") return "4";
	if (iconText == "nt_partlysunny") return "4";
	if (iconText == "nt_sleet") return "9";
	if (iconText == "nt_rain") return "7";
	if (iconText == "nt_snow") return "#";
	if (iconText == "nt_sunny") return "4";
	if (iconText == "nt_tstorms") return "&";

	Serial.println("Unknown getMeteoconIcon:<" + iconText +">");
	return ")";
}
*/
