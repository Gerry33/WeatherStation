/*
 *


  Fonts Generator: http://oleddisplay.squix.ch
  Adapted by Bodmer to use the faster TFT_eSPI library:
  https://github.com/Bodmer/TFT_eSPI

  Plus:
  Minor changes to text placement and auto-blanking out old text with background colour padding
  Moon phase text added
  Forecast text lines are automatically split onto two lines at a central space (some are long!)
  Time is printed with colons aligned to tidy display
  Min and max forecast temperatures spaced out
  The ` character has been changed to a degree symbol in the 36 point font
  Display does not need to be blanked between updates
  Icons nudged about slightly to add wind direction + speed

  Adaptations by Gerry33:

	- Wunderground client: Adapations using simplifications to new WG API:
				(winddir, pressure trend, icons, ...)
	- pressure trend icon
	- own icons for inside/outside
	- loop () reconstruction to avoid Wifi- disconnects caused by long lasting operations
	- MQTT support to display measures from other sensors in the network
	- HTTP OTA firmware update
	- Wifi username/password injected by compiler commandline
	- NTP- time
	- extended logging
	- SYSLOG
	- Timer alarm
	- WEB -GUI for configuration, tooltips, statistics, ressource monitoring
	- BMP display only as JPG has pixel issues
	- Arbitrary download of icons from any local source
	- Deleting  and reloading icons on request using REST Web service.
	- Reset/ format SPIFF from  REST Web service
	- Layout redesign.
	- Icon adaptations (icons not published due to legal fears).
	- Alive pixel on display
	- time supervision of the last observation time to recognize failures on pws

 */

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
// #include <bitset

// Additional UI functions
#include "GfxUi.h"

// Fonts created by http://oleddisplay.squix.ch/
#include "WebResource.h"

#include <Common.h>
#include <TimeLib.h>		// https://github.com/PaulStoffregen/Time
#include <TimeAlarms.h>		// https://github.com/PaulStoffregen/TimeAlarms
#include <ESP8266WiFi.h>
#include <MQTTClient.h>
#include <EEPROM.h>			// https://github.com/esp8266/Arduino/tree/master/libraries/EEPROM
#include <NTPClient.h>

// check settings.h for adapting to your needs
#include "settings.h"

#include "ArialRoundedMtBold_14.h"
#include "ArialRoundedMTBold_36.h"

// #define PSTR		// empty  redef.

#include "WundergroundClient.h"

#define SDA D1
#define SCL D2

// #define PSTR		// empty  redef.

#include <TempSensors.h> // contains the sensors defined
TempSensors tempSensors;

/*****************************
   Important: see settings.h to configure your settings!!!
 * ***************************/

TFT_eSPI 		tft = TFT_eSPI();       // Invoke custom library
GfxUi 	  		ui 	= GfxUi(&tft);

WebResource 	webResource;

MQTTClient 		mqttClient;
WiFiClient 		wlan;

String 			myTempOut	="n/a";
String 			myHumOut 	="n/a";

String 			myTempIn	="n/a";
String 			myHumIn 	="n/a";

// topics to be display as outside temps
const String topicTempExtern  = FPSTR("Aussensensor/OUT/SI/TEMP");	// PSTR does not w
const String topicHumExtern   = FPSTR("Aussensensor/OUT/SI/HUM");
const String topicSubscription= FPSTR("Aussensensor/OUT/#");

// edit this to your needs. it makes no sense to make this more configurable
String 			baseDownloadURL = "http://10.0.0.104:8081/icons/";

// putting these in settings.h produces weird results: in both arrays the second element is garbage
#define NO_ICONS_INTERN 5
String ourIcons [NO_ICONS_INTERN] = {"/TempDrinnen", "/TempDraussen", "/arrowDown", "/arrowEqual", "/arrowUp"};

// List, so that the downloader knows what to fetch. must coincide with WundergroundClient::getMeteoconIcon()
#define NO_ICONS_EXTERN 20
String iconsExtern [NO_ICONS_EXTERN] = {"chanceflurries","chancerain","chancesleet"
		,"chancesnow","clear","cloudy","flurries"
		,"fog","hazy","mostlycloudy","mostlysunny"
		,"partlycloudy","partlysunny","rain"
		, "sleet","snow","sunny","tstorms","chancetstorms", "unknown"};
/*
 * global text displayed on the date/time field in case of errors.
 */
String errorText="";

// flags for the main loop what to update.

boolean 	flagUpdateWeatherCondition	= false;
boolean 	flagUpdateWeatherForecast	= false;
boolean 	flagDrawCurrentWeather		= false;
boolean 	flagUpdateDrawForecast		= false;
boolean 	flagUpdateRemoteTemps 		= false;	// outside temps by MQTT
boolean 	flagUpdateInnerTemps 		= false;	// measured by own attached sensor
boolean		flagInitRessources 			= false;

//enum UPDATE_FLAGS	// TODO
//	{CurrentWeather,ForeCast, OwnTemps, downloadRessources, dateTime, idle}
//updateFlags;

AlarmId  	timerId_UpdateWeather	= dtINVALID_ALARM_ID;
AlarmId  	timerId_TempReading		= dtINVALID_ALARM_ID;
AlarmId  	timerId_NTP				= dtINVALID_ALARM_ID;


int 		lastWifiState 		= WL_DISCONNECTED;
String 		lastStart			= "";

WundergroundClient wunderground(IS_METRIC);
String 		text_html;

void serverHandle_Reset(){

	// std::bitset<5> ab;

	Logger.info(PSTR("Resetting and restarting ..."));
	server.send(200, "text/html", PSTR("Clearing EEProm and SPIFF. Restarting in 60 secs ..."));
	//erase SPIFFS and update all internet resources, this takes some time!
	tft.drawString("Formatting SPIFFS, please wait!", 120, 200);
	Logger.info(PSTR("Formatting SPIFFS ..."));
	boolean rc = SPIFFS.format();
	Logger.info("Formatting complete:rc:%d\n", rc);

	// memset  ( &config, 0, sizeof(config));
	tft.drawString("Done. Restarting.", 120, 200);
	ESP.restart();

}

void deleteAllFile(){
	String fileName ;
	fs::Dir dir = SPIFFS.openDir("/"); // Root directory
	uint32_t totalBytes = 0;

	Serial.println("Deleting all files");
	while (dir.next()) {
		fileName = dir.fileName();
		Serial.println("Deleting:" + fileName);
		SPIFFS.remove(fileName);
	}

}

void serverHandle_updateIcons(){

	Logger.info(PSTR("Updating icons ..."));
	server.send(200, "text/html", PSTR("Updating icons ..."));

	listFiles();

	// tft.drawRect(0, 141, 240, 320, TFT_BLACK);
	// tft.println("Formatting SPIFFS, \nplease wait!");
	// boolean rc = SPIFFS.format();

	Logger.info(PSTR("Deleting all icon files\n"));
	deleteAllFile();
	listFiles();
	Logger.info(PSTR("Deletion complete. Now downloading.\n"));

	// tft.drawRect(0, 141, 240, 320, TFT_BLACK);

	flagInitRessources			= true;	// postpone to main loop
	flagUpdateWeatherCondition	= true;

}
void setup() {

	Serial.begin(115200);
	delay (10);

	text_html = PSTR("text/html");

	Logger.info("\nStarted:" __FILE__ ", ESP-Core:<%s>, ESP-SDK:<%s>, \ngetFlashChipREALSize: %d, FreeSketchSpace:%d\n",
			ESP.getCoreVersion().c_str(), ESP.getSdkVersion(), ESP.getFlashChipRealSize(), ESP.getFreeSketchSpace());

	EEPromReadConfig();

	if (strcmp(config.magicKey, magicEEKey)) {

		Logger.info(PSTR("Init EEProm"));
		memset(&config, 0, sizeof(config));
		config.readingIntervallTemp = 300;
		strcpy(config.magicKey, magicEEKey);
		config.debug = true;				// on very first init, debug is a good idea
		EEPromWriteConfig();

	}

	Logger.begin(config.debug ? LOGLEVEL_DEBUG : LOGLEVEL_INFO);

	tft.begin();

	tft.setRotation(2);	// 180 deg gedreht, kopfüber

	tft.fillScreen(TFT_BLACK);

	SPIFFS.begin();

	// http response handler
	server.on ("/reset", 	 	serverHandle_Reset	);		// if nothing goes, cleanup and restart
	server.on ("/updateIcons",	serverHandle_updateIcons);	// loads icons from server

	server.on ("/updateWeather", [](){
		flagUpdateWeatherCondition = true;
		server.send(200, "text/html", PSTR("Updating weather on request"));
	});

	server.on ("/", 			serverHandle_Root	);
	server.on ("/configRsp", 	serverHandle_ConfigRsp);
	server.on ("/debug_cmd", 	serverHandle_Debug	  	);
	server.on ("/test", 		serverHandle_Test	);

	server.begin();
	httpUpdater.setup(&server);

	mqttClient.begin("server",  wlan);	//  before Wifi_Init(); --> wifi on connect, no can be started here

	// dim the strings which are mainly constant
	lastStart	.reserve (20);	// date, time
	webPage		.reserve (2000);
	myTempOut	.reserve (5);
	myHumOut	.reserve (5);
	myTempIn	.reserve (5);
	myHumIn		.reserve (5);

	drawDateTime();

#ifdef SI7021ADDR
	tempSensors.init_SI7021( config.MqttId, SDA, SCL);
#endif
	timerInit();
	Alarm.timerRepeat(1,  drawAlivePixel);	// needs no special init method as independend and never changed.

	flagInitRessources			= true;

	//	for (int i = 0; i < NO_ICONS_INTERN; i++) {
	//		Serial.println("Init:idx:"+ String(i)+ ",file<" + ourIcons[i] +">");
	//	}

	Wifi_Init();
	flagUpdateWeatherCondition = true;	// very first init flag., not executed until wifi connected. see loop()
}

// #define PSTR		// empty  redef.

void timedSetLongIntervall(){
	Alarm.free(timerId_UpdateWeather);
	timerId_UpdateWeather = Alarm.timerRepeat(120 * 60 , timerUpdateWeather);
	Logger.debug (PSTR("Setting Long intervall.Next weather query:%s\n"), NTP.getTimeAsChr(Alarm.read(timerId_UpdateWeather)));
}

void timedSetShortIntervall(){
	Alarm.free(timerId_UpdateWeather);
	timerId_UpdateWeather = Alarm.timerRepeat(60 * 60 , timerUpdateWeather);	// every 60 mins is enough
	Logger.debug (PSTR("Setting Short intervall.Next weather query:%s\n"), NTP.getTimeAsChr(Alarm.read(timerId_UpdateWeather)));
}

/*
 * test routine to write a string to time
 */
void serverHandle_Test (){

	for (int i = 0;  i < server.args(); i++) {
		Logger.debug ("serverHandle_Test:argI:" + String (i) + ", argN:" + server.argName(i) + ", argV:" + server.arg(i));
	}

	if ( server.arg ("tm").length() >0 ) {

		tft.setFreeFont		(&Lato_Black_60);
		// tft.setTextDatum	(BC_DATUM);	// bottom centre
		tft.setTextDatum	(TL_DATUM);	// top left: 	Datum = Bezugspunkt
		tft.setTextColor	(TFT_YELLOW, TFT_BLACK);
		tft.setTextPadding(tft.textWidth("00:00"));
		tft.drawString(server.arg ("tm"), 0, 0);		//hh:mm
	}
	// http://wetterstation.fritz.box/test?text="WLAN Err"
	if ( server.arg ("text").length() >0 ) {
		errorText= server.arg ("text");
		errorText.replace("\"", "");	// make "" an empty string. empty parameters on a url a not allowed.
		Logger.debug("serverHandle_Test:<"+ errorText + ">:len:"+ errorText.length());
		drawDateTime();
	}
	else
		server.send(200, text_html, "<br><h2>Error:use argument 'text' </h2>");

	// server.send(200, text_html, "test?tm="+server.arg ("tm"));
	server.send(200, text_html, "OK");

}

void serverHandle_Root(){
	Logger.debug(PSTR("Root called"));
	WebPage_Update();
	server.send(200, text_html, webPage);
}
/*
 * these method are for just updating the flags. Most methods are updated on time as well as on event.
 * So instead of calling them in the callbacks, its better to call the drawing routines in the
 * main loop at one central place.
 */

void timerUpdateWeather(){
	flagUpdateWeatherCondition= true;
}

/*
 * timer procedure to start temp reading.
 * takes now 75 ms,
 */
void timedReadTemperatures() {

	// long start = millis();

	// Logger.debug(F("timedReadTemperatures: Reading tempertures\n"));

	SensorDataPoint * pt = tempSensors.sensorDataPoint;	// more official: &tempSensors.sensorDataPoint[0]

	for (int i= 0; i < MAX_SENSORS_PT; i++, pt++) {

		if (pt->isActive){	// only if used

			// TODO does not work. too less know-how to implement function pointers in C++...
			//			if (pt->readSensor != nullptr)
			//				pt->readSensor();

			// <workaround>
			switch (pt->type) {

			case SENSOR_TYPE_DHT_TEMP: {
				tempSensors.read_DHT();
				break;
			}

			case SENSOR_TYPE_1WIRE_TEMP: {
				tempSensors.read_1Wire();
				break;
			}

			case SENSOR_TYPE_SI702_TEMP: {
				tempSensors.read_SI7021();
				// update only if changed. avoids too much flicker.
				if (   myHumIn  != tempSensors.pt_SI7021_HUM -> display
					|| myTempIn != tempSensors.pt_SI7021_TEMP-> display)
				{
					myHumIn  = tempSensors.pt_SI7021_HUM-> display;
					myTempIn = tempSensors.pt_SI7021_TEMP->display;
					flagUpdateInnerTemps =  true;
				}
				break;
			}

			case SENSOR_TYPE_DHT_HUM:	// both of these are handled already in read_DHT resp  si7021
			case SENSOR_TYPE_SI702_HUM:
				break;					// do nothing.

			case SENSOR_TYPE_OFS_LAST:
				return;
			}
			// </workaround>

			// send to mqtt whatever is displayed
			// no comparison with previous values. e.g to save mqtt messages. Openhab does that for us.
			// otherwise the charts cannot be displayed correctly

			Logger.debug("Mqtt-Snd:" + pt->MqttPubTopic + ":"+ pt->display);
			mqttClient.publish	(pt->MqttPubTopic.c_str(), pt->display.c_str());

			if (pt->valid == false)	// send to global errro adr
				mqttClient.publish	("ERR",  pt->errMsg.c_str());

			flagUpdateInnerTemps= true;
		}
	}

	// Logger.debug(("timedReadTemperatures: duration:%d\n"), millis() - start);
}

/*
 * loop very large fragemented: after each action return to give ESP the chance to handle its own
 * task e.g. take care about WiFi.
 */
void loop() {

	if ( WiFi.isConnected()) {

		server.handleClient();

		if (flagInitRessources == true) {
			downloadResources();	// this cannot be done in WifoONConnected or server.handle, takes too long
			drawFixedElements();
			flagInitRessources 	= false;
			return;
		}

		if (mqttClient.connected()) {
			// this calls messageReceived ( ... ) on message
			mqttClient.loop();	// was : delay (50);
		}
		else {
			if (mqtt_Connect() == true){	// connected = ok
				return;						// then give back to esp
			}
		}

		// NTP.update(); // done by timer now

		if (flagUpdateWeatherCondition) {
			updateWeatherCondition();
			flagUpdateWeatherCondition	= false;
			flagUpdateWeatherForecast  	= true;
			flagDrawCurrentWeather 		= true;
			return;
		}

		if (flagUpdateWeatherForecast) {
			updateWeatherForecast();
			flagUpdateWeatherForecast 	= false;
			flagDrawCurrentWeather		= true;
			return;
		}

		if (flagDrawCurrentWeather) {
			long start = millis();
			drawCurrentWeather ( 200 );
			flagDrawCurrentWeather = false;
			flagUpdateDrawForecast = true;
			// Logger.debug("DrawCurrentWeather Ok:Counter:%d. FreeHeap:%d, time:%d\n", weatherCount++, ESP.getFreeHeap() , millis() - start) ;
			return;
		}

		if (flagUpdateDrawForecast) {
			long start = millis();
			drawForecast	   (  );
			flagUpdateDrawForecast = false;
			// Logger.debug("DrawWeather Ok:Counter:%d. FreeHeap:%d, time:%d\n", weatherCount++, ESP.getFreeHeap() , millis() - start) ;
			return;
		}

		if ( flagUpdateInnerTemps){	// this comes more often than the weather.
			drawTempsInside();
			flagUpdateInnerTemps= false;
			return;
		}
		if ( flagUpdateRemoteTemps){
			drawTempsOutside();
			flagUpdateRemoteTemps= false;
		}
	}

	Alarm.delay(0);	 // low priority: draws date time, blinking alive dot. Moved from top of loop() to here.-> no wifi disconnects
}

// Download the bitmaps
bool downloadResources() {

	Logger.info(PSTR("Attempting download of missing ressources from <%s>"), baseDownloadURL.c_str());

	WEB_RC rc; //

	for (int i = 0; i < NO_ICONS_EXTERN; i++) {

		// rc |= webResource.downloadFile("http://www.squix.org/blog/wunderground/" + wundergroundIcons[i] + ".bmp", wundergroundIcons[i] + ".bmp");// , _downloadCallback);
		// the leading slash is important as otherwise the list or delete methods do not find the files.
		//		Serial.println("DL1:idx:"+ String(i)+ ",file<" + iconsExtern[i] +">");
		rc = webResource.downloadFile (baseDownloadURL 			 + iconsExtern[i] + ".bmp", 	 "/" + iconsExtern[i] + ".bmp");// , _downloadCallback);
		logDownloadInf (rc);
		rc = webResource.downloadFile (baseDownloadURL  + "mini/" + iconsExtern[i] + ".bmp", "/mini/" + iconsExtern[i] + ".bmp");//  _downloadCallback);
		logDownloadInf (rc);
	}

	// Logger.debug("iconsIntern Size:%d\n",  iconsIntern[] );	// not easy to determine sizeof string array . STL is not available.

	for (int i = 0; i < NO_ICONS_INTERN; i++) {
		//		Serial.println("DL3:idx:"+ String(i)+ ",file<" + ourIcons[i] +">");
		rc = webResource.downloadFile( baseDownloadURL + ourIcons[i]+ ".bmp" , ourIcons[i] + ".bmp");
		logDownloadInf (rc);
	}
	fs::FSInfo  info;
	SPIFFS.info(info);
	Logger.info(PSTR("Download done. SPIFFS: totalBytes:%d, usedBytes %d\n"), info.totalBytes, info.usedBytes);

	return rc;
}

void logDownloadInf (WEB_RC rc) {

//	if (Logger.getLevel() == LOGLEVEL_DEBUG && rc == IGNORED)
//		Logger.debug("Ignored already present file from <" + webResource.currentURL +">");
//	else
		if ( rc != IGNORED)
			Logger.info("Download " +  (rc == SUCCESS ? String("Success") : String("Error")) + " from <" + webResource.currentURL +">");
}

// Update the internet based information and mark update screen.

time_t lastObsCheckTS=-1;		// last observation check time stamp:our timer to check
time_t lastObsTS 		=-1;	// last observation time stamp from WU

void updateWeatherCondition ( ) {

	// wunderground.updateConditions("10.0.0.104", 8081, "/wd/salzkotten.json");
	wunderground.updateConditions("http://api.wunderground.com/api/a87045dd69c1e7d6/conditions/lang:DL/q/de/salzkotten.json");

}

void updateWeatherForecast ( ) {

	// wunderground.updateConditionsPWS(WUNDERGRROUND_API_KEY, "DL", "IARGESEK6");	// Personal weather station
	// wunderground.updateForecast("10.0.0.104", 8081, "/wd/forecast.json");
	wunderground.updateForecast("http://api.wunderground.com/api/a87045dd69c1e7d6/forecast10day/lang:DL/q/de/salzkotten.json");

	Logger.debug("CurrentWeather:text:<"  + wunderground.getWeatherText()
			+ ">,todayIcon:<" 	+ wunderground.getTodayIconText ()	+">"
			+ ">,temp:" 		+ wunderground.getCurrentTemp()
			+ ",hum:"  		+ wunderground.getHumidity()
			+ "\nobsTime:" 	+ wunderground.getObservationTime()
	);

	/* check last observation epoche:
	 *
	 * The observation epoche on remote station is beyond our influence. we do not know timezone, update intervall, ...
	 * However, if there is no update on the epoche for xx mins, we consider it failed.
	 * We use our own millis() to determine xx mins this.
	 *
	 */

	if ( timeStatus() == timeSet) {
		time_t n = now(); 	// user very often, now() performs too many calcluation

		if (lastObsCheckTS < 0 || lastObsTS < 0 ){	// init
			lastObsCheckTS = n ;
			lastObsTS 	= wunderground.getObservationEpoch();
			return;
		}

		if ( n- lastObsCheckTS > 3600 ) {
			lastObsCheckTS = n;
			// time to check
			Logger.debug ("Checking Observation time:\n current ObsTS:" + wunderground.getObservationTime()
					+ ", curent obs epoch:" 	+ wunderground.getObservationEpoch()
					+ ", last obs epoch:" 		+ String(lastObsTS)
					+ ", diff:" + String(wunderground.getObservationEpoch() - lastObsTS));

			if (lastObsTS - wunderground.getObservationEpoch()  > 3600 )	// 60 min , getObservationEpoch = secs since 1970
				Logger.error("Observation time diff > 1h:\n last ObsTime:" + wunderground.getObservationTime()
						+ ", last obs epoch:" 		+ String(lastObsTS)
						+ ", current obs epoch:" 	+ String(wunderground.getObservationEpoch())
						+ ", diff:" + String(wunderground.getObservationEpoch() - lastObsTS));
		}
		lastObsTS=  	wunderground.getObservationEpoch() ;
	}
}


/*
 * draws the elements that do not change.
 */
void drawFixedElements(){

	drawHorizontalSeparator	( 62 );	// below date/time	// 61, immer noch kaputt

	//  vertical sepa line:x   y      len
	tft.drawFastVLine   (130, 62,  170, TFT_WHITE);	// leave a pixel empty
	tft.drawFastHLine	(  2, 149, 129, TFT_WHITE); // 149 ok

	drawHorizontalSeparator ( 244 );	// above forecast

	drawHorizontalSeparator	(319);	// bottom. max 320

	ui.drawBmp		("/TempDrinnen.bmp",  0, 61 );
	ui.drawBmp	    ("/TempDraussen.bmp", 1, 153);	// 30 *30 px
}


/*
 * we must notify if the error code vanished and then clear the rectangle as the text for
 * date time is different with the error code text.
 */
boolean isError =false;
boolean isErrorBlinkDisplay =false;

// draws the clock
void drawDateTime( ) {

	// hh:mm
	char buf [20];
	os_sprintf  ( buf, "%02d:%02d", 	 hour() ,  minute() );
	// tft.drawRect(0, 0, 209, yAxis+62, TFT_BLACK);	// y = 63 is line, 210 is

	tft.setFreeFont		( &Lato_Black_60);
	tft.setTextDatum	( TL_DATUM);	// top left: 	Datum = Bezugspunkt
	tft.setTextColor	( TFT_YELLOW, TFT_BLACK);
	tft.setTextPadding	( tft.textWidth("00:00"));
	tft.drawString		( buf, 0, 0);		//hh:mm
	//Serial.printf("TimeDisplay:%s",buf);

	tft.setFreeFont(&Lato_Black_21);

	if ( errorText.length()==0) { 	// weekday
		if ( isError == true ) {	// do this only once on a change
			// clear display area with a black rectangle before writing new text
			Logger.debug (PSTR("Clearing date time field\n"));
			isError = false;
			tft.fillRect(160, 0, 80, 61, TFT_BLACK);	//  xmax= 160 + 80 = 240
		}
		// weekday
		tft.setTextColor(TFT_WHITE, TFT_BLACK);
		tft.setTextDatum(TC_DATUM);	// left
		tft.setTextPadding(tft.textWidth("DD"));  // String width + margin
		tft.drawString(NTP.getWeekdayAsStr(), 200, 5);	// was: 204

		// day-mm
		tft.setTextDatum(TL_DATUM);	// left
		tft.setTextPadding(tft.textWidth("DD-MMM"));  // String width + margin
		sprintf  ( buf, "%d-%s", day(),  NTP.getMonthAsStr().c_str());
		// tft.drawString(buf, 			210, 30);
		tft.drawString(buf, 			161, 30);
	}
	else {// Display Error text
		isError = true;
		// tft.fillRect	 (160, 0, 80, 61, TFT_WHITE);	//  xmax= 240
		tft.setTextColor (TFT_RED, TFT_WHITE);
		tft.setTextDatum (TL_DATUM);			// top left
		int splitIdx= splitIndex(errorText);
		tft.drawString (errorText.substring (0, 		     splitIdx), 		 160,  5);
		tft.drawString (errorText.substring (splitIdx+1, errorText.length()), 160, 30);
	}

	// Logger.debug("drawDateTime:%s\n", NTP.getTimeAsChr(now()));
	// drawHorizontalSeparator		(yAxis + 57 );	// hh:mm is font 60
	// tft.setTextPadding(0);
}

/*
 * we separate drawint to have least flicker in case of updates
 * supplied by si7021 sensor
 */
void drawTempsInside( ) {

	// ui.drawBmp		("/TempDrinnen.bmp", 0, 60 );

	tft.setFreeFont	( &FreeSans12pt7b);
	// tft.setFreeFont		( &FreeSans18pt7b); überlappt


	tft.setTextDatum	( TC_DATUM	);				// top center
	tft.setTextColor	( TFT_CYAN, TFT_BLACK	); 	//blue zu dunkel
	tft.setTextPadding	( tft.textWidth("111%"	));
	tft.drawString		( myHumIn + "%",  70, 	65);

	// TEMPS:

	//unlikley that inner temps drop < 0. so no need to adapt font size
	tft.setFreeFont(&Lato_Black_60);
	tft.setTextDatum(TL_DATUM);	// top left
	tft.setTextColor(TFT_ORANGE, TFT_BLACK);
	tft.setTextPadding(tft.textWidth("12.4"));
	tft.drawString( myTempIn,  0, 88);

	// tft.drawFastHLine(2, 149 , 129 , TFT_WHITE); // 149 ok

}

// humidity & Temps OUT----------------------------------------------------
void drawTempsOutside (){

	// ui.drawBmp		    ("/TempDraussen.bmp", 1, 153 );	// 30 *30 px
	//hum
	tft.setFreeFont		(&FreeSans12pt7b);
	tft.setTextDatum	( TC_DATUM	);	// top center
	tft.setTextColor	( TFT_CYAN, TFT_BLACK	); //blue zu dunkel
	tft.setTextPadding	( tft.textWidth("111%"	));
	tft.drawString		( myHumOut + "%",  70, 155 );

	// temp outside
	tft.setFreeFont(&Lato_Black_60);
	tft.setTextDatum(TL_DATUM);	// top left
	tft.setTextColor(TFT_ORANGE, TFT_BLACK);
	tft.setTextPadding(tft.textWidth("12.4"));
	// if awfull, lousy, rotten, cold outside, e.g. '-10.1', lower font size
	if  (myTempOut.length() >=5 ){
		tft.setFreeFont(&Lato_Bold_56);
		tft.setTextPadding(tft.textWidth("-12.4"));
	}
	tft.drawString( myTempOut,  0, 183);

}

// draws current weather information

void  drawCurrentWeather(int y) {

	y = 62;			// offset from top

	//  vertical sepa line:x   y      len
	// tft.drawFastVLine   (130, y  , 170, TFT_WHITE);	// leave a pixel empty

	// Weather Text
	String weatherText = wunderground.getWeatherText();

	if ( weatherText.length() == 0 ) {
		Logger.error(PSTR("CurrentWeather text not set\n"));
		return ;
	}
	// Weather Icon
	String weatherIcon  = "/";
	weatherIcon  +=  wunderground.getTodayIconText () + ".bmp";

	// check if exists, and if not, show the unknown symbol
	bool exists = SPIFFS.exists( weatherIcon);
	if ( !exists ) {
		Logger.error ("File does not exists:<" + weatherIcon +">\n");
		weatherIcon +="unknown.bmp";
	}

	ui.drawBmp	(weatherIcon , 135,  y +2 );	// 30 *30 px

	// tft.setFreeFont	( &ArialRoundedMTBold_14);
	// tft.setFreeFont	( &FreeSansBold12pt7b); zu gross
	tft.setFreeFont		( &FreeSansBold9pt7b);

	tft.setTextDatum	( TR_DATUM);	// top, bottom right
	// tft.setTextColor	( TFT_ORANGE, TFT_BLACK);
	tft.setTextColor	( TFT_WHITE, TFT_BLACK);

	// removed : considered unnecessary cause the icon already tells the current weather
	//	int splitPoint = 0;
	//	int xpos = 230;
	//	splitPoint =  splitIndex(weatherText);
	//	if (splitPoint > 16)
	//		xpos = 235;

	//	tft.setTextPadding(tft.textWidth("Heavy"));  // Max anticipated string width
	//	// tft.setTextPadding(tft.textWidth("Heavy"));  // Max anticipated string width
	//	if (splitPoint)
	//		tft.drawString(weatherText.substring(0, splitPoint), xpos, y + 88);
	//	tft.setTextPadding(tft.textWidth("Small"));  // Max anticipated string width + margin
	//	tft.drawString(weatherText.substring(splitPoint), xpos, 	   y + 102);

	// Serial.println ("Remote temp <"+ wunderground.getCurrentTemp() +">,own:" +myTempOut);

	// wind dir ----------------------------------------------------------------------------
	// weatherText 	= wunderground.getWindDir(); / not needed any more: we use degree instead
	int degree_i = wunderground.getWindDegrees().toInt();

	// Wind rose
	int center = y + 112;
	int left   = 180;

	//	// Erase old plot, radius + 1 to delete stray pixels
	tft.fillCircle(left,  center,  27, TFT_BLACK);

	// double outer ring
	tft.drawCircle(left,  center,  25, TFT_WHITE);    //
	tft.drawCircle(left,  center , 26, TFT_WHITE);

	if ( degree_i >= 0 )
		fillSegment(left, center, degree_i - 15, 30, 25, TFT_GREEN);
	else
		Logger.error (PSTR("No wind angle <%d>\n"), degree_i);
	// inner double red circle
	tft.drawCircle(left,  center, 8, TFT_RED);
	tft.drawCircle(left,  center, 9, TFT_RED);

	// wind speed --------------------------------------------------------------------

	tft.setTextDatum	(TC_DATUM);	// for both
	tft.setTextPadding	(tft.textWidth("88 kmh")); // Max string length?
	tft.drawString		(wunderground.getWindSpeed(), 185, y + 143);

	// pressure ----------------------------------------------------------------------------------------

	tft.setTextPadding	(tft.textWidth("8888mb")); 	// Max string length?
	tft.drawString(wunderground.getPressure(), 174, y + 163);

	weatherText = wunderground.getPressureTrend();
	int ofs = -1;
	if (weatherText == "-")
		ofs = 2; 			// /arrowDown.bmp";
	else if (weatherText == "0")
		ofs = 3 ; 	// equal
	else if (weatherText == "+")
		ofs = 4 ; 	// up
	else if (ofs ==-1) {
		Logger.error(PSTR("Unknown pressure trend symbol:<") + wunderground.getPressureTrend() +">");
	}

	Logger.debug ("wind:degree:"+ String(degree_i) + ",speed <" +  wunderground.getWindSpeed()
			+">, pressure:" + wunderground.getPressure	()
			+">, trend:"	+ wunderground.getPressureTrend()
			+">, iconOfs:"	+ String(ofs));
	if ( ofs > 0)
		ui.drawBmp	( ourIcons [ofs] +".bmp", 212,  y + 164);
}
// --------------------------------------------------------------------------------------------

// draws the three forecast columns

void drawForecast () {// X   Y >

	int y  =  261;
	// drawHorizontalSeparator ( 244 );

	//  0=  today, 1= tomorrow / JSON tag: simpleforecast/.../period
	drawForecastDetail(10,  y,  1);
	drawForecastDetail(95,  y,  2);
	drawForecastDetail(180, y,  3);

	// drawHorizontalSeparator(319);		// bottom. max 320
}

// forecast columns

void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex) {

	if ( dayIndex > MAX_FORECAST_PERIODS) {	// could cause overflows
		Logger.error(PSTR("ForecastDetail: index too high:") + String (dayIndex));
		return;
	}

	String weekdayName = wunderground.getForecastWeekDay(dayIndex).substring(0, 2);
	weekdayName .toUpperCase();

	Logger.debug ("drawForecastDetail:idx:" + String ( dayIndex)
			// + ", wkday_i:" 	+ String ( wkday_i)
			+ ", weekday:"	+ weekdayName
			+ ", high:"		+ wunderground.getForecastHighTemp	(dayIndex)
			+ ", low:" 		+ wunderground.getForecastLowTemp	(dayIndex)
			+ ", icon:" 	+ wunderground.getForecastIcon		(dayIndex));

	String weatherIcon  = wunderground.getForecastIcon(dayIndex);

	if (weatherIcon.length() > 0 ) {
		weatherIcon = "/mini/" + weatherIcon +".bmp";
		// weatherIcon += wunderground.getForecastIcon(dayIndex) +".bmp";
	}
	else {
		Logger.error (PSTR("drawForecastDetail: No valid weatherIcon at idx %d\n"), dayIndex);
		weatherIcon = "/mini/unknown.bmp";
	}


	tft.setFreeFont(&ArialRoundedMTBold_14);
	// weekday
	tft.setTextDatum	( BC_DATUM);
	tft.setTextColor	( TFT_ORANGE, TFT_BLACK);
	tft.setTextPadding	( tft.textWidth("WW"));
	tft.drawString		( weekdayName, x + 25, y);
	// high|low temp
	tft.setTextColor	( TFT_WHITE, TFT_BLACK);
	tft.setTextPadding	( tft.textWidth("-88|88"));
	tft.drawString		( wunderground.getForecastLowTemp(dayIndex) + " | " + wunderground.getForecastHighTemp(dayIndex)  , x + 25, y + 15);
	if ( SPIFFS.exists( weatherIcon))
		ui.drawBmp 	( weatherIcon, x, y + 16);
	else
		Logger.error (PSTR("drawForecastDetail: File does not exist <") + weatherIcon +">\n");

	// ui.drawJpeg	( weatherIcon, x, y + 16);
	// tft.setTextPadding(0); // Reset padding width to none
	// Serial.println("drawForecastDetail done for idx:" + String (dayIndex));
}

/*
 *  a secondly toggle.
 *  Also abused to find second 0 in  the minute to update date/time
 *  There is no method in the time alarm lib to adjust to a minutely second '0'.
 */

boolean toggle;
int 	lastMinuteDraw = -1L;
void drawAlivePixel(){

	toggle= ~toggle;
	tft.drawPixel(1, 2, toggle ? TFT_BLACK : TFT_WHITE);

	//	int sec = second();
	// that's rubbish
	// tft.drawPixel(sec, 1, TFT_WHITE);	//  draw an increasing line
	// tft.drawLine (1, 1,  sec * 2, 1, TFT_WHITE);
	//	tft.drawLine(0, 0, 0, 120, TFT_BLACK);	// clear line

	if ( minute () != lastMinuteDraw) {	// draw the minute if it changes
		drawDateTime();
		lastMinuteDraw = minute ();
	}
}


// if you want separators, uncomment the tft-line
void drawHorizontalSeparator(uint16_t y) {

	tft.drawFastHLine(10, y, 240 - 2 * 10, TFT_WHITE);
}

// determine the "space" split point in a long string
int splitIndex(String text)
{
	uint index = 0;
	while ( (text.indexOf(' ', index) >= 0) && ( index <= text.length() / 2 ) ) {
		index = text.indexOf(' ', index) + 1;
	}
	if (index) index--;
	return index;
}

// Calculate coord delta from start of text String to start of sub String contained within that text
// Can be used to vertically right align text so for example a colon ":" in the time value is always
// plotted at same point on the screen irrespective of different proportional character widths,
// could also be used to align decimal points for neat formatting
int rightOffset(String text, String sub)
{
	int index = text.indexOf(sub);
	return tft.textWidth(text.substring(index));
}

// Calculate coord delta from start of text String to start of sub String contained within that text
// Can be used to vertically left align text so for example a colon ":" in the time value is always
// plotted at same point on the screen irrespective of different proportional character widths,
// could also be used to align decimal points for neat formatting
int leftOffset(String text, String sub)
{
	int index = text.indexOf(sub);
	return tft.textWidth(text.substring(0, index));
}

// Draw a segment of a circle, centred on x,y with defined start_angle and subtended sub_angle
// Angles are defined in a clockwise direction with 0 at top
// Segment has radius r and it is plotted in defined colour
// Can be used for pie charts etc, in this sketch it is used for wind direction
#define DEG2RAD 0.0174532925 // Degrees to Radians conversion factor
#define INC 2 // Minimum segment subtended angle and plotting angle increment (in degrees)
void fillSegment(int x, int y, int start_angle, int sub_angle, int r, unsigned int colour)
{
	// Calculate first pair of coordinates for segment start
	float sx = cos((start_angle - 90) * DEG2RAD);
	float sy = sin((start_angle - 90) * DEG2RAD);
	uint16_t x1 = sx * r + x;
	uint16_t y1 = sy * r + y;

	// Draw colour blocks every INC degrees
	for (int i = start_angle; i < start_angle + sub_angle; i += INC) {

		// Calculate pair of coordinates for segment end
		int x2 = cos((i + 1 - 90) * DEG2RAD) * r + x;
		int y2 = sin((i + 1 - 90) * DEG2RAD) * r + y;

		tft.fillTriangle(x1, y1, x2, y2, x, y, colour);

		// Copy segment end to sgement start for next segment
		x1 = x2;
		y1 = y2;
	}
}

/*
 * We take over the NTP timing. otherwise we cannot determine an exact NTP sync point per day.
 */

void NTP_Init(){

	NTP.setOnSyncEvent_cb ( NTP_onEvent_cb ); 	// ntp callback
	NTP.initWithoutRefresh(1);					// timezone only. no update by NTP we do it on our own
	if (timerId_NTP == dtINVALID_ALARM_ID){ 	// could be set already by repeatedly onDisconnect () calls
		timerId_NTP 	= Alarm.timerRepeat(2, NTP_timedUpdate);	// start a forced NTP sync every 2 secs
		Logger.debug(PSTR("NTP short timer started.\n"));
	}
}

void NTP_onEvent_cb( NTPSyncEvent_t ntpEvent, time_t oldTime,  time_t newTime) {

	switch (ntpEvent) {
	case NTP_EVENT_INIT:
		Logger.debug(PSTR("NTP init.\n"));
		break;
	case NTP_EVENT_STOP:
		//Serial.println(PSTR("NTP_EVENT_STOP\n"));
		break;
	case NTP_EVENT_NO_RESPONSE:
		Logger.error(PSTR("NTP_NO_RESPONSE\n"));
		break;
	case NTP_EVENT_SYNCHRONIZED:
		// th NTP_EVENT_SYNCHRONIZED: last:%d, now:%d\r\n", lastTime, timestamp);
		// Serial.printf("onSyncEvent.NTP_EVENT_SYNCHRONIZED: time set: from %s to %s\n", NTP.getTimeAsChr( last), NTP.getTimeAsChr( t2 ) );
		char  buf1 [20];	// getTimeDateChr needs a buffer to write to
		char  buf2 [20];

		Logger.info(PSTR("NTP_SYNCHRONIZED: time set from %s to %s:%s. Diff:%lu"),
				NTP.getTimeDateChr ( oldTime, buf1),  NTP.getTimeDateChr ( newTime, buf2) ,
				NTP.getTimeZoneAsChr(), newTime -oldTime);
		drawDateTime();

		// cancel short reconnect timer
		if (Alarm.isAllocated(timerId_NTP) ){
			Alarm.free (timerId_NTP);
			timerId_NTP = dtINVALID_ALARM_ID;
			Logger.debug(PSTR("NTP short timer cancelled.\n"));
		}
		// this block is executed only once after startup and after first NTPsync
		if (lastStart.length()==0) {// this is our flag
			lastStart= NTP.getTimeDateStr(now());

			/* Trying to adjust to second '0':
			 It is not possible to adjust to second '0' with this lib. Therefore we abuse the secondly timer.
				timerId_UpdateTime = Alarm.timerRepeat(0, 1, 0, drawDateTime);
			 */

			// set to 30  sec not to collide with second '0' of the minute
			Alarm.alarmRepeat(23,00, 30,  timedSetLongIntervall);  // 23:00  every day, query every  120 mins
			Alarm.alarmRepeat( 6, 0, 30,  timedSetShortIntervall);  // 6:00 every day,  query every  60  mins

			randomSeed	(millis());		// random enough cause now the time is set

			// random second so not all devices raise their ntp request at the same second
			// NTP sync once per 24hrs at 3:00 to get summer/wintertime change as early as possible

			// this timer remains on all the time. In case of failure, e.g. wifi disconnect, this is just another
			// NTP sync which will fail then, but who cares. The short NTP sync is now independend of this
			AlarmId  timerId_NTP_long = Alarm.alarmRepeat( 3, 2, random (newTime) %60,  NTP_timedUpdate);
			Logger.info(PSTR("NTP daily sync time set to %s"), NTP.getTimeAsChr(Alarm.read(timerId_NTP_long)));

		}

		break;
	case NTP_EVENT_SUMMERTIME_CHANGE:
		Logger.info(PSTR("NTP_SUMMER/WINTERTIME_CHANGE\n"));
		break;
	}
}

void NTP_timedUpdate(){
	bool rc = NTP.updateForced() ;
	Logger.debug(PSTR("NTP_timedUpdate:%d\n"), rc);
}

long mqttLastReconnectAttempt = -1L;

/**
 * @returns : true : connected, false: not connected
 */
boolean mqtt_Connect() {

	bool rc = false;

	// if (WiFi.status() == WL_CONNECTED) {	// not needed as the main loop does this already

	// timer not yet started, or timer expired. Check every 10 secs
	if ( mqttLastReconnectAttempt < 0 || ((millis() - mqttLastReconnectAttempt)  > 10000L )) {

		// Logger.debug ("MQTT trying connect to server\n");
		rc = mqttClient.connect(config.MqttId);	// our id
		if ( rc) {	// true
			Logger.debug(PSTR("MQTT connected rc:%d\n"),rc);

			// 			for ( unsigned int  i =0 ; i <= 1 ; i++) {
			//				Logger.debug ("Subscribing to topic <%s>\n",  subscribeTopic[i]);
			// rc = mqttClient.subscribe  (subscribeTopic[i]); // crashes
			// get all from our sensors
			// rc = mqttClient.subscribe  ("ESP12Buero/OUT/#");		// NO PSTR (...) here. crash
			rc = mqttClient.subscribe  (topicSubscription);

			// send a command to the sensors to send their data immediately
			// weird: mqtt server send last subscribed msg automatically. So not needed.
			//			rc = mqttClient.publish		("ESP12Buero/IN/" ,   "INIT");
			//			rc = mqttClient.publish		("Aussensensor/IN/" , "INIT");
			mqttLastReconnectAttempt = -1L;	// timer off
			errorText="";
		}
		else {
			// cannot reconnect :add a timer every 10 secs to try to connect. Otherwise we flood the network
			mqttLastReconnectAttempt = millis();

			//				    switch (rc) {
			//				      case 1: Serial.println("Wrong protocol"); 					break;
			//				      case 2: Serial.println("ID rejected"); 						break;
			//				      case 3: Serial.println("Server unavailable"); 				break;
			//				      case 4: Serial.println("Bad user/password"); 					break;
			//				      case 5: Serial.println("Not authenticated"); 					break;
			//				      case 6: Serial.println("Failed to subscribe"); 				break;
			//				      default: Serial.printf("Couldn't connect to server, code:%d"); 	break;
			//				}
			Logger.error(PSTR("CANNOT connect to MQTT server.rc:%d\n"),rc);
			errorText =PSTR("MQTT Err");
			drawDateTime();
		}
	}
	// Serial.printf("MQTT connected finished, rc:%d\n", rc );

	return rc;
}
/**
 * this method name is not changeable. it belongs to the MQTT lib.
 * 'async-mqtt-client'-lib  did not work. It caused constant unpredicable crashes.
 */
void messageReceived(String topic, String payload, char * bytes, unsigned int length) {

	Logger.debug (PSTR ("Mqtt-Rcv:TP<%s>, PL:%s\n"), topic.c_str() , payload.c_str() );

	//	if ( topic ==  		"ESP12Buero/OUT/SI/TEMP"){	//  2do: make nicer ...
	//		if ( myTempIn == payload){
	//			//Serial.println("Found same SI TEmp:" + myTempIn + ":" + payload);
	//			return;
	//		}
	//		myTempIn = payload;
	//	}
	//	else if ( topic ==  "ESP12Buero/OUT/SI/HUM") {
	//		if (myHumIn == payload)
	//			return;
	//		myHumIn= payload;
	//	}
	//	else

// if ( topic == "Aussensensor/OUT/SI/TEMP") {
	if ( topic == topicTempExtern) {
		if ( myTempOut == payload)	// do not update if the same
			return;
		myTempOut = payload;
		flagUpdateRemoteTemps = true;
	}
	else if  ( topic ==  topicHumExtern) {
		if ( myHumOut == payload)
			return;
		myHumOut= payload;
		flagUpdateRemoteTemps = true;
	}
	else{
		Logger.error (PSTR  ("Unhandled topic:<%s>, payload:<%s>\n"),  topic.c_str(), payload.c_str());
		return;
	}

}

void Wifi_Init(){

	WiFiConnectHandler 		= WiFi.onStationModeGotIP		 ( Wifi_onConnectedGotIP);// As soon WiFi is connected, start NTP Client
	WiFiDisConnectHandler 	= WiFi.onStationModeDisconnected ( Wifi_onDisconnected);

	WiFi.hostname(config.hostname);	// needs no mDNS

	WiFi.mode	( WIFI_STA);	  	// important !! station only
	WiFi.begin	( STR_SSID, STR_PSK);

	Logger.debug (PSTR("Wifi_Init done.\n"));

}

/*
 * this method is very sensitive. no large calculations to do here
 */
void Wifi_onConnectedGotIP(WiFiEventStationModeGotIP ipInfo) {

	lastWifiState = WiFi.status();

	sysLogInit();	// must be here, not on

	delay (100);
	Logger.info (PSTR("WLAN Connected: hostname:%s, IP:%s, localMac:%s,\n SYSLOG: %s, WifiState:%d, Ch:%d, RSSI:%d dBm\n"),
			WiFi.hostname().c_str(), ipInfo.ip. toString().c_str(),  WiFi.macAddress().c_str(), syslogServerIP.toString().c_str(),
			WiFi.status(), WiFi.channel(), 	WiFi.RSSI() );

	server.begin();
	httpUpdater.setup(&server);

	delay (100);

	NTP_Init();
	timedSetShortIntervall();

	//	if (!Alarm.isAllocated(timerId_UpdateWeather ) ) { 	// if !already scheduled
	//		timerId_UpdateWeather = Alarm.timerRepeat(30 * 60 , timerUpdateWeather);
	//	}

	errorText ="";
	// Logger.debug  (PSTR("Wifi_onConnectedGotIP finished\n"));
	// mqtt_Connect();	// moved to main loop as it crashes herein.

}

/*
 Manage network disconnection. We need not care about reconnection, ESP SDK does it on its own
 diese funktion darf nur ganz kurz aufgerufen werden. keine langen Aktionen.
 */
void Wifi_onDisconnected(WiFiEventStationModeDisconnected event_info) {

	// this is called very often. only a change is really interesting.
	if ( lastWifiState == WiFi.status())
		return;	//nothing to do

	lastWifiState = WiFi.status();
	Logger.stopSyslog(); // makes no sense any more
	Logger.error("WLAN disconnected from SSID " + event_info.ssid +", Reason:" + String ( event_info.reason ));

	// not needed as the main loop checks if wifi connected
	//	if (Alarm.isAllocated(timerId_UpdateWeather ) )	// makes no sense anymore
	//		Alarm.free (timerId_UpdateWeather);

	server.close();
	// NTP.stop();	// makes no sense any more
	// mark on screen
	errorText ="WLAN Err:"+ String ( event_info.reason );
	drawDateTime();
}

void sysLogInit(){

	// Logger.stopSyslog();
	// set the syslog server in logging
	int rc =0;
	// if the string is a valid ip4 adr, use it directly. otherwise try to resolve host by name

	if (syslogServerIP.fromString(config.syslogServer) == true){
		Logger.setSysLogPrefix(WiFi.hostname().c_str());
		rc = Logger.startSyslog(syslogServerIP);
		Logger.info(PSTR("Started SYSLOG to server:<%s>,rc:%d\n"), syslogServerIP.toString().c_str(), rc);
	}
	else
		Logger.warn(PSTR("No valid SYSLOG server address:<%s>"), config.syslogServer);
}


// https://gist.github.com/sticilface/e54016485fcccd10950e93ddcd4461a3
// https://github.com/esp8266/Arduino/issues/1143
/*
 * the string must be build with these lots of concat due to restrictions in PSTR
 * they cannot be combined in one
 */
void WebPage_Update (){

	webPage  = FPSTR("<html><head><h2>Wetterstation</h2></head> <body><br><h3>IP: ");
	webPage.concat ( WiFi.localIP().toString());
	webPage.concat ( FPSTR(", Gateway:"));
	webPage.concat  ( WiFi.gatewayIP().toString());
	webPage.concat  ( FPSTR(", DNS:"));
	webPage.concat  ( WiFi.dnsIP(0).toString());
	webPage.concat ( FPSTR("<br>Hostname:"));
	webPage.concat ( WiFi.hostname());
	webPage.concat ( FPSTR("<br>MQTT: Id:"));
	webPage.concat (String (config.MqttId));
	webPage.concat ( FPSTR(", State:"));
	webPage.concat ( String (mqttClient.connected() ? "Connected" : "DISconnected"  ) + "</h3>");
	webPage.concat ( FPSTR("</h3><br>Time:"));
	webPage.concat ( NTP.getTimeDateStr (now()));
	webPage.concat ( NTP.getTimeZoneAsStr   ()  );
	webPage.concat ( FPSTR ("; Last Start:"));
	webPage.concat ( lastStart);
	webPage.concat ( FPSTR("<br>NTP LastSync:"));
	webPage.concat ( NTP.getTimeDateStr (NTP.getLastNTPSyncTimestamp()) );
	webPage.concat ( FPSTR("; Compile DateTime:"));
	webPage.concat ( version);
	webPage.concat ( FPSTR("</h3><br>FreeHeap:"));
	webPage.concat ( String (ESP.getFreeHeap()));
	webPage.concat ( FPSTR(", FreeSketchSpace:"));
	webPage.concat ( String	(ESP.getFreeSketchSpace()));
	webPage.concat ( FPSTR(",<br>ChipID:"));
	webPage.concat ( String (ESP.getChipId()));
	webPage.concat ( FPSTR(", ESP-Core version:"));
	webPage.concat ( ESP.getCoreVersion());
	webPage.concat ( FPSTR(", ESP-SdkVersion:"));
	webPage.concat ( String(ESP.getSdkVersion()));
	webPage.concat ( FPSTR("<br>WLAN: Channel:"));
	webPage.concat ( String (WiFi.channel()));
	webPage.concat ( FPSTR(", Signal:"));
	webPage.concat ( String (WiFi.RSSI()));
	webPage.concat ( FPSTR(", dBm, localMacAdr:"));
	webPage.concat ( WiFi.macAddress());
	// ---------------------------------------------------------------------------------------------
	webPage.concat ( FPSTR("<br>Last weather observation date/time:"));
	webPage.concat ( wunderground.getObservationDate());
	webPage.concat ( FPSTR("<br><b>Sensors:<b>"));
	SensorDataPoint * pt = tempSensors.sensorDataPoint;

	for (int i= 0; i < MAX_SENSORS_PT; i++, pt++) {

		if (pt->isActive){	// only if used
			webPage  .concat (String (pt->description) +", topic:" + String (pt->MqttPubTopic) +";");
			if ( pt->valid)
				// webPage.concat("value:" + String (pt->value,1) );
				webPage.concat("value:" + String (pt->display) );
			else
				webPage.concat (("Err:") + String (pt->errCode));
			webPage.concat("<br>");
		}
	}

	webPage.concat ( FPSTR("<b>Remote sensors:</b><br>Topic:"));
	webPage.concat (topicTempExtern + ":" + myTempOut);
	webPage.concat ("<br>Topic:"+ topicHumExtern + ":" + myHumOut);

	// https://wiki.selfhtml.org/wiki/HTML/Formulare/input/Radio-Buttons_und_Checkboxen

	webPage.concat(FPSTR("<FORM action=debug_cmd method=post"\
			"<label for=DebugMode_cb>Debug</label>"\
			"<input type=checkbox  id=DebugMode_cb name=DebugMode value=1 onclick=submit(); "));
	webPage.concat (String (config.debug ? "checked" : ""));
	webPage.concat ( FPSTR("></FORM>"));

	//Config FORM --------------------------------------------------------------------------------------------------------
	webPage.concat(FPSTR("<h3>Configuration</h3>(change on servername/IPs require (manual) reboot)"));
	webPage.concat(FPSTR("<form action=configRsp method=post>"\
			"<table>"\
			"<tr>"\
			"<td align=right>Hostname:</td>"\
			"<td align=left> <input  title='DNS-name or IP' type=text name=hostNameId maxlength=20 value="));
	webPage.concat (String(config.hostname));
	webPage.concat(FPSTR("></td>"\
			"</tr>"\
			"<tr>"\
			"<td align=right>MQTT Server:</td>"\
			"<td align=left> <input title='IP adress only. Port 1589 default' type=text name=mqttServerName maxlength=20 value="));
	webPage.concat (String(config.mqttServer));
	webPage.concat(FPSTR("></td>"\
			"</tr>"\
			"<tr>"\
			"<td align=right>MQTT DevID:</td>"\
			"<td align=left><input title='Freely choosable. No whitespaces.' type=text name=devid  maxlength=19 value="));
	webPage.concat (String(config.MqttId));

	webPage.concat(FPSTR("></td>"\
			"</tr>"\
			"<tr>"\
			"<td align=right>SYSLOG Server:</td>"\
			"<td align=left><input title='IP adress only' type=text name=syslogServer maxlength=19 value="));
	webPage.concat (String(config.syslogServer));
	webPage.concat(FPSTR("></td>"\
			"</tr>"\
			"<tr>"\
			"<td align=right>Temperatur <br>Intervall(secs):</td>"\
			"<td align=left> <input title='The intervall in which temperatures are read from above sensors and transmitted' "\
			"type=text name=tempIntervall maxlength=3 value="));
	webPage .concat (String(config.readingIntervallTemp));

	webPage.concat (FPSTR("></td></tr>"\
			"</table>"));
	//--------------------------------------------------------------------------------------------------------
	webPage .concat(
			FPSTR  ("<button style=width:100px name=button value=save>Save</button>"\
					"<button style=width:100px name=button value=reboot>Reboot</button>"\
					"<button style=width:100px name=button title='Clears all configuration data and reboots' value=reset>Reset</button>"\
					"<button style=width:100px name=button title='re-reads icons from "));
	webPage .concat(baseDownloadURL);
	webPage .concat( FPSTR("' value=readIcons>Re-Read Icons</button>"\
					"<button style=width:100px name=button value=updateWeather>Update Weather</button>"\
					"</form>"\
					"</body></html>"));

	// Serial.println("WEB page size:" + String(webPage.length()));
}
void serverHandle_Debug () {

	config.debug =  (server.arg  ("DebugMode").length() != 0) ?  true : false;
	Logger.info(PSTR("Debug:%s, LogLevel:%d"),  bool2ChrTrueFalse(config.debug), Logger.getLevel());
	Logger.setLevel( config.debug ? LOGLEVEL_DEBUG :  LOGLEVEL_INFO);
	WebPage_Update();
	server.send(200, text_html, webPage);

}

void serverHandle_ConfigRsp (){

	char tmp [ sizeof (config.hostname) ];

	int l  = server.args();
	for (int i = 0; i< l ; i++) {
		Logger.info("ConfigRsp:" + String (i) + ", argN:" + server.argName(i) + ", argV:" + server.arg(i) );
	}

	String cmd = server.arg("button");
	Logger.info ("WEB Cmd:<" + cmd +">");

	if(cmd.equals("reboot")){
		Logger.info(PSTR("Reboot requested."));
		server.send(200, text_html, "Reboot ok.");
		ESP.restart();
	}
	else if(cmd.equals("reset")){
		serverHandle_Reset();
	}
	else if(cmd.equals("readIcons")){
		serverHandle_updateIcons();
	}
	else if(cmd.equals("updateWeather")){
		flagUpdateWeatherCondition = true;
	}
	else if(cmd.equals("save")){

		Logger.debug(PSTR("Saving config on button 'save."));

		strcpy ( tmp, server.arg("devid").c_str());

		if ( strcmp(config.MqttId, tmp)  != 0 ){
			Logger.info("MQTT DevId changed from <" + String (config.MqttId) + " > to " + server.arg("devid"));
			strcpy (config.MqttId, tmp);	// changes also the topics
		}

		strcpy ( tmp, server.arg("mqttServerName").c_str());

		if ( strcmp(config.mqttServer, tmp)  != 0  ){
			Logger.info("MQTT Server name changed from <" + String (config.mqttServer) + "> to <" + server.arg("mqttServerName") +">");
			strcpy (config.mqttServer, tmp);
		}

		Serial.println("Test:1");

		strcpy ( tmp, server.arg("hostNameId").c_str());
		if (strcmp(config.hostname, tmp)  != 0 ){
			Logger.info("Hostname changed from:<" + String (config.hostname) + " > to " +  String ( tmp));
			strcpy (config.hostname, tmp);
		}
		Serial.println("Test:2");
		strcpy ( tmp, server.arg("syslogServer").c_str());
		if (strcmp(config.syslogServer, tmp)  != 0 ){
			Logger.info("syslogServer changed from:<" + String (config.syslogServer) + "> to <" +  String ( tmp) +">");
			strcpy (config.syslogServer, tmp);
			sysLogInit();
		}
		Serial.println("Test:3");
		config.readingIntervallTemp = server.arg("tempIntervall").toInt();

		timerInit();
		Serial.println("Test:4");
		EEPromWriteConfig();

		WebPage_Update();
		Serial.println("Test:5");
		server.send(200, text_html, webPage);
	}
	else {
		String txt = "CMD not found for 'button':" + cmd;
		Logger.error ( txt);
		server.send  (200, text_html, txt);
	}
	Logger.debug("WEB page Update done\n");
}

void timerInit(){ 		// start the repeater timer. they need no ntp

	Alarm.free(timerId_TempReading);	// free
	timerId_TempReading = Alarm.timerRepeat(config.readingIntervallTemp, timedReadTemperatures );	// set new
	Logger.debug("Temp Timer init id:" + String (timerId_TempReading) +  ", next reading:" + NTP.getTimeDateStr(Alarm.read(timerId_TempReading)));

}
// --------------------------------------------------------------
void EEPromWriteConfig (){

	EEPROM.begin(sizeof (Config));
	Logger.debug("Writing EE " + String (sizeof (Config)) + " bytes");
	byte  * p = (byte *) & config;
	for (uint i  = 0; i < sizeof (Config); i++ )
		EEPROM.write(i, *p++);
	EEPROM.commit();
}

void EEPromReadConfig (){

	EEPROM.begin(sizeof (Config));

	byte  * p = (byte *) & config;
	Logger.debug("Reading EE " + String (sizeof (Config)) + " bytes");

	for (uint i  = 0; i < sizeof (Config); i++ )
		*p++ = EEPROM.read(i);
}

inline void set_bit(long *x, int bitNum) {
	*x |= (1L << bitNum);
}

inline void bit_unset (long *x, int bitNum) {
	*x ^= (1L << bitNum);
}

