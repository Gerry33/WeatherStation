/*
 *
 D:\Daten\gsi\EigeneDaten\Programmierung\EclipseWorkspace.CDT\Wetterstation\Release\Wetterstation.bin

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
	- Simple WEB -GUI for configuration
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
// #include <bitset>

// Additional UI functions
#include "GfxUi.h"

// Fonts created by http://oleddisplay.squix.ch/
#include "ArialRoundedMTBold_14.h"
#include "ArialRoundedMTBold_36.h"

// Download helper
#include "WebResource.h"

#include <Common.h>
#include <NTPClient.h>
#include <TimeLib.h>		// https://github.com/PaulStoffregen/Time
#include <TimeAlarms.h>		// https://github.com/PaulStoffregen/TimeAlarms
#include <ESP8266WiFi.h>	// ???
#include <MQTTClient.h>
#include <EEPROM.h>			// https://github.com/esp8266/Arduino/tree/master/libraries/EEPROM

// check settings.h for adapting to your needs
#include "settings.h"
#include <JsonListener.h>
#include "WundergroundClient.h"

/*****************************
   Important: see settings.h to configure your settings!!!
 * ***************************/

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
GfxUi 	  ui = GfxUi(&tft);

WebResource webResource;

MQTTClient 		mqttClient;
WiFiClient 		wlan;

String 			myTempOut	="n/a";
String 			myHumOut 	="n/a";

String 			myTempIn	="n/a";
String 			myHumIn 	="n/a";

// flags for the main loop what to update.

boolean 	updateWeather 			= false;	// global flag set by timer
boolean 	updateWeatherCondition	= false;	//
boolean 	updateWeatherForecast	= false;	//
boolean 	updateDrawWeather		= false;	//
boolean 	updateOwnTemps 			= false;	// own temps come more often
boolean		initRessources 			= false;

enum UPDATE_FLAGS	// TODO
	{CurrentWeather,ForeCast, OwnTemps, downloadRessources, dateTime, idle}
updateFlags;

AlarmId  	timerId_UpdateWeather	= dtINVALID_ALARM_ID;
AlarmId  	timerId_UpdateTime		= dtINVALID_ALARM_ID;
int 		lastWifiState 		= WL_DISCONNECTED;

WundergroundClient wunderground(IS_METRIC);
String text_html;

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

	// TODO clear screen
	//erase SPIFFS and update all internet resources, this takes some time!
	// , 120, 200
	listFiles();

	// tft.drawRect(0, 141, 240, 320, TFT_BLACK);
	// tft.println("Formatting SPIFFS, \nplease wait!");
	// boolean rc = SPIFFS.format();

	Logger.info(F("Deleting all files\n"));
	deleteAllFile();
	listFiles();
	Logger.info(F("Deletion complete. Now downloading.\n"));

	// tft.drawRect(0, 141, 240, 320, TFT_BLACK);

	initRessources	= false;	// postpone to main loop
	// updateOwnTemps	= true;	// done on request
	updateWeather	= true;

}
void setup() {

	Serial.begin(115200);
	delay (10);

	text_html = PSTR("text/html");

	Logger.info("\nStarted:" __FILE__ ", ESP-Core:<%s>, ESP-SDK:<%s>, \ngetFlashChipRealSize: %d, FlashChipSize:%d\n",
			ESP.getCoreVersion().c_str(), ESP.getSdkVersion(), ESP.getFlashChipRealSize(), ESP.getFlashChipSize());

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

	tft.setRotation(2);	// 180 deg gedreht, kopf¸ber

	tft.fillScreen(TFT_BLACK);

	SPIFFS.begin();

	// http response handler
	server.on ("/reset", 	 	serverHandle_Reset	);		// if nothing goes, cleanup and restart
	server.on ("/updateIcons",	serverHandle_updateIcons);	// loads icons from server

	server.on ("/updateWeather", [](){
		updateWeather = true;
		server.send(200, "text/html", PSTR("Updating weather on request"));
		});

	server.on("/", 				serverHandle_Root	);
	server.on("/configRsp", 	serverHandle_ConfigRsp);
	server.on("/cmd", 			serverHandle_Cmd	);
	server.on("/test", 			serverHandle_Test	);

	server.begin();
	httpUpdater.setup(&server);

//	timerId_UpdateTime = Alarm.timerRepeat(60, timerDrawTime);
	timerId_UpdateTime = Alarm.timerRepeat(60, drawDateTime);	// controlled by ALarm lib
						 Alarm.timerRepeat(1 , drawAlivePixel);

	mqttClient.begin("server",  wlan);	//  before Wifi_Init(); --> wifi on connect, no can be started here

	drawDateTime();	//
	Wifi_Init();

	webPage		.reserve  (2000);

}
/*
 * test routine to write a string to time
 */
void serverHandle_Test (){

	for (int i = 0;  i < server.args(); i++) {
			Serial.println("Test:" + String (i) + ", argN:" + server.argName(i) + ", argV:" + server.arg(i) );
	}

	if ( server.arg ("tm").length() >0 ) {

		    tft.setFreeFont		(&Lato_Black_60);
		 // tft.setTextDatum	(BC_DATUM);	// bottom centre
			tft.setTextDatum	(TL_DATUM);	// top left: 	Datum = Bezugspunkt
			tft.setTextColor	(TFT_YELLOW, TFT_BLACK);
			tft.setTextPadding(tft.textWidth("00:00"));
			tft.drawString(server.arg ("tm"), 0, 0);		//hh:mm
	}
	server.send(200, text_html, "test?tm="+server.arg ("tm"));

}

void serverHandle_Root(){
	Logger.debug("Root called\n");
	WEBPageUpdate();
	server.send(200, text_html, webPage);
}
/*
 * these method are for just updating the flags. Most methods are updated on time as well as on event.
 * So instead of calling them in the callbacks, its better to call the drawing routines in the
 * main loop at one central place.
 */

void timerUpdateWeather(){
	updateWeather= true;
}

int weatherCount =0;

void loop() {

	Alarm.delay(0);	//  set timers, draw date time

	if ( WiFi.isConnected()) {

		server.handleClient();

		if (initRessources == false) {
			downloadResources();	// this cannot be done in WifoONConnected or server.handle, takes too long
			initRessources 	= true;
			return;
		}

		if (mqttClient.connected()) {
			mqttClient.loop();
			delay(10);
		}
		else
			mqtt_Connect();

		NTP.update();

		if (updateWeather) {	// main flag.
			updateWeatherCondition = true;
			updateWeather = false;
								// no return needed here
		}

		if (updateWeatherCondition) {
			updateWeatherData(updateWeatherCondition , updateWeatherForecast);
			updateWeatherCondition =  false;
			updateWeatherForecast  = true;
			return;
		}

		if (updateWeatherForecast) {
			updateWeatherData(updateWeatherCondition , updateWeatherForecast);
			updateWeatherForecast = false;
			updateDrawWeather= true;
			return;
		}

		if (updateDrawWeather) {
			drawAllWeather();
			updateDrawWeather = false;
			Logger.info("W-Ct:%d.FreeMem:%d\n", weatherCount++, ESP.getFreeHeap());
			return;
		}

		if ( updateOwnTemps){	// this comes more often than the weather. so we separate.
			drawOwnTemps();	// y- axis
			updateOwnTemps= false;
		}
	}
}

// Download the bitmaps
bool downloadResources() {

	Logger.debug("Downloading ressources - if needed\n");

	// download images from the net. If images already exist don't download
	// tft.drawString("Downloading to SPIFFS...", 120, 200);
	//	tft.drawString(" ", 120, 240);  // Clear line
	//	tft.drawString(" ", 120, 260);  // Clear line

	// tft.fillScreen(TFT_BLACK);
	// tft.setFreeFont(&ArialRoundedMTBold_14);


	// TODO clear  half screen and write onto it
	bool rc = false; //
	for (int i = 0; i < 19; i++) {

 	 // rc |= webResource.downloadFile("http://www.squix.org/blog/wunderground/" + wundergroundIcons[i] + ".bmp", wundergroundIcons[i] + ".bmp");// , _downloadCallback);
		// the leading slash is important as otherwise the list or delete methods do not find the files.
		rc |= webResource.downloadFile("http://10.0.0.104:8081/icons/" + iconsExtern[i] + ".bmp", "/"+ iconsExtern[i] + ".bmp");// , _downloadCallback);
		// tft.drawString("Downloading to SPIFFS:" + iconsExtern[i] + ".bmp", 0, 150);
	}

	for (int i = 0; i < NO_ICONS_EXTERN; i++) {

	 // rc |= webResource.downloadFile("http://www.squix.org/blog/wunderground/mini/" + wundergroundIcons[i] + ".bmp", "/mini/" + wundergroundIcons[i] + ".bmp");//  _downloadCallback);
		rc |= webResource.downloadFile("http://10.0.0.104:8081/icons/mini/" + iconsExtern[i] + ".bmp", "/mini/" + iconsExtern[i] + ".bmp");//  _downloadCallback);
		// tft.drawString("Downloading to SPIFFS:" + iconsExtern[i] + ".jpg", 0, 150);
	}

	// Logger.debug("iconsIntern Size:%d\n",  iconsIntern[] );	// not easy to determine sizeof string array . STL is not available.

	for (int i = 0; i < NO_ICONS_INTERN; i++) {
		rc |= webResource.downloadFile("http://10.0.0.104:8081/icons/" + iconsIntern[i] ,  iconsIntern[i]);
		// tft.drawString("Downloading to SPIFFS:" + iconsIntern[i] , 0, 150);
	}
	fs::FSInfo  info;
	SPIFFS.info(info);
	Logger.debug("SPIFFS: totalBytes:%d, usedBytes %d\n", info.totalBytes, info.usedBytes);

	return rc;


}

// Update the internet based information and mark update screen.

time_t lastObsCheckTS=-1;		// last observation check time stamp:our timer to check
time_t lastObsTS 		=-1;	// last observation time stamp from WU


void updateWeatherData( bool updateWeatherCondition, bool  updateWeatherForecast) {

	// wunderground.updateConditions(WUNDERGRROUND_API_KEY, "DL", WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
	if (updateWeatherCondition) {
	 // wunderground.updateConditions("10.0.0.104", 8081, "/wd/condition.json");
		wunderground.updateConditions("http://api.wunderground.com/api/YourApiKey/conditions/lang:DL/q/de/yourcity.json");
		return;
	}

	// wunderground.updateConditionsPWS(WUNDERGRROUND_API_KEY, "DL", "YourStation");	// Personal weather station

	if ( updateWeatherForecast) {
// 		wunderground.updateForecast("10.0.0.104", 8081, "/wd/forecast.json");	// test server
		wunderground.updateForecast("http://api.wunderground.com/api/YourApiKey/forecast10day/lang:DL/q/de/yourcity.json");

		Logger.debug("CurrentWeather:text:<"  + wunderground.getWeatherText()
				+ ">,todayIcon:<" + wunderground.getTodayIconText () +">"
				+ "\n obsTime:" + wunderground.getObservationTime()
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
}

void drawAllWeather(){

	drawCurrentWeather ( 200 );
	drawForecast	   ( 280 );

}

// draws the clock

void drawDateTime( ) {

	int yAxis =0;

    // hh:mm

	char buf [20];
	os_sprintf  ( buf, "%02d:%02d", hour() ,  minute() );

	tft.drawRect(0, 0, 209, yAxis+62, TFT_BLACK);	// y = 63 is line, 210 is

 // tft.setFreeFont(&ArialRoundedMTBold_36);
    tft.setFreeFont		(&Lato_Black_60);
 // tft.setTextDatum	(BC_DATUM);	// bottom centre
	tft.setTextDatum	(TL_DATUM);	// top left: 	Datum = Bezugspunkt
	tft.setTextColor	(TFT_YELLOW, TFT_BLACK);
	tft.setTextPadding(tft.textWidth("00:00"));
	tft.drawString(buf, 0, yAxis);		//hh:mm

	// weekday
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
 // tft.setFreeFont(&ArialRoundedMTBold_14); //  50 % so groﬂ
	tft.setFreeFont(&Lato_Black_21);
	tft.setTextDatum(TC_DATUM);	// left
	tft.setTextPadding(tft.textWidth("DD"));  // String width + margin
	tft.drawString(NTP.getWeekdayAsStr(), 204, yAxis + 5);

	// day-mm
	tft.setTextDatum(TL_DATUM);	// left
	tft.setTextPadding(tft.textWidth("dd-mmm"));  // String width + margin
	sprintf  ( buf, "%d-%s", day(),  NTP.getMonthAsStr().c_str());
	tft.drawString(buf, 				  210, yAxis + 30);

	drawHorizontalSeparator		(yAxis + 57 );	// hh:mm is font 60
	tft.setTextPadding(0);
}

/*
 * from y:
 */
int drawOwnTemps( ) {

	ui.drawBmp		("/TempDrinnen.bmp", 0, 60 );
	// humidity IN /OUT----------------------------------------------------
	// tft.setFreeFont 		(&ArialRoundedMTBold_14); viel zu klein
	tft.setFreeFont			(&FreeSans12pt7b);

	tft.setTextDatum	( TC_DATUM	);	// top center
	tft.setTextColor	( TFT_CYAN, TFT_BLACK	); //blue zu dunkel
	tft.setTextPadding	( tft.textWidth("111%"	));
	tft.drawString		( myHumIn + "%",  70, 	 64  );
	tft.drawString		( myHumOut+ "%",  70, 	 154 );

	// TEMPS:
	tft.drawFastHLine(2, 149 , 129 , TFT_WHITE); // 149 ok

	//unlikley that inner temps drop < 0. so no need to adapt font size
	tft.setFreeFont(&Lato_Black_60);
	tft.setTextDatum(TL_DATUM);	// top left
	tft.setTextColor(TFT_ORANGE, TFT_BLACK);
	tft.setTextPadding(tft.textWidth("12.4"));

// IN----------------------------------------------------
	tft.drawString( myTempIn,    0, 87);
//	// OUT----------------------------------------------------
	ui.drawBmp		    ("/TempDraussen.bmp", 1, 153 );	// 30 *30 px
//
//	// if awfull, lousy, rotten, cold outside, i.e. '-10.1', lower font size
//
	if  (myTempOut.length() >=5 ){
			tft.setFreeFont(&Lato_Bold_56);
			tft.setTextPadding(tft.textWidth("-12.4"));
	}
	tft.drawString( myTempOut,  0, 183);

	return 180;		// 152
}


// draws current weather information

void  drawCurrentWeather(int y) {

	y = 62;			// offset from top

	//  vertical sepa line:x   y      len
	tft.drawFastVLine   (130, y  , 170, TFT_WHITE);	// leave a pixel empty

   // Weather Text
	String weatherText = wunderground.getWeatherText();

	if ( weatherText.length() == 0 ) {
		Logger.error("CurrentWeather text not set\n");
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
	tft.drawString		(wunderground.getWindSpeed(), 185, y + 144);

	// pressure ----------------------------------------------------------------------------------------

	tft.setTextPadding	(tft.textWidth("8888mb")); 	// Max string length?
	tft.drawString(wunderground.getPressure(), 175, y + 166);

	weatherText = wunderground.getPressureTrend();
	int ofs = -1;
	if (weatherText == "-")
		ofs = 2; 			// weather"/arrowDown.bmp";
	else if (weatherText == "0")
		ofs = 3 ; 	// equal
	else if (weatherText == "+")
		ofs = 4 ; 	// up
	else if (ofs ==-1) {
		Logger.error("Unknown pressure trend symbol:" + wunderground.getPressureTrend());
	}

	Logger.debug ("wind:degree:"+ String(degree_i) + ",speed <" +  wunderground.getWindSpeed()
				+">, pressure:" + wunderground.getPressure	()
				+">, trend:"	+ wunderground.getPressureTrend());
	if ( ofs > 0)
		ui.drawBmp	(iconsIntern [ofs], 213,  y + 166 );
}
// --------------------------------------------------------------------------------------------

// draws the three forecast columns

void drawForecast (int y  ) {// X   Y >

	y = 261;
	drawHorizontalSeparator ( 244 );

	//  1=  today, 2= tomorrow / JSON tag: simpleforecast/.../tomorrow
	drawForecastDetail(10,  y,  2);
	drawForecastDetail(95,  y,  3);		//
	drawForecastDetail(180, y,  4);		//

	drawHorizontalSeparator(319);		// bottom. max 320
}

// forecast columns

void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex) {

	if ( dayIndex > MAX_FORECAST_PERIODS) {	// could cause overflows
		Logger.error("ForecastDetail: index too high:" + String (dayIndex));
		return;
	}

	String weekdayName = wunderground.getForecastWeekDay(dayIndex).substring(0, 2);
	weekdayName .toUpperCase();

	String weatherIcon = "/mini/";
	weatherIcon       += wunderground.getForecastIcon(dayIndex) +".bmp";

	Logger.debug ("drawForecastDetail:idx:" + String ( dayIndex)
				 // + ", wkday_i:" 	+ String ( wkday_i)
					+ ", weekday:"	+ weekdayName
					+ ", H:"		+ wunderground.getForecastHighTemp	(dayIndex)
					+ ", L:" 		+ wunderground.getForecastLowTemp	(dayIndex)
					+ ", icon:" 	+  wunderground.getForecastIcon		(dayIndex));

	if ( !SPIFFS.exists( weatherIcon)) {
		Logger.error ("drawForecastDetail: File does not exist <" + weatherIcon +">\n");
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

    ui.drawBmp 	( weatherIcon, x, y + 16);

}

boolean toggle;
void drawAlivePixel(){

	toggle= ~toggle;
	if (toggle)
		//tft.drawPixel(239, 319, TFT_BLACK); rechts unten
		tft.drawPixel(1, 1, TFT_BLACK);
	else
		tft.drawPixel(1, 1, TFT_WHITE);
}

// if you want separators, uncomment the tft-line
void drawHorizontalSeparator(uint16_t y) {
 // tft.drawFastHLine(10, y, 240 - 2 * 10, 0x4228);
	tft.drawFastHLine(10, y, 240 - 2 * 10, TFT_WHITE);
}

// determine the "space" split point in a long string
int splitIndex(String text)
{
	int index = 0;
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

// -------------------------------------------------------------------------------
const int NTP_UPDATE_INTERVALL=86400;

void NTP_Init(){

	NTP.setOnSyncEvent_cb ( NTP_onEvent_cb ); // ntp callback
	NTP.setNTPServer ((char *)"pool.ntp.org",0);
	NTP.init		 (1, NTP_UPDATE_INTERVALL);
}

void NTP_onEvent_cb( NTPSyncEvent_t ntpEvent, time_t last,  time_t t2) {

	switch (ntpEvent) {
	case NTP_EVENT_INIT:
		//Serial.printf("NTP ini.\n");
		break;
	case NTP_EVENT_STOP:
		Serial.println("NTP_EVENT_STOP");
		break;
	case NTP_EVENT_NO_RESPONSE:
		Serial.printf("NTP_EVENT_NO_RESPONSE\n");
		break;
	case NTP_EVENT_SYNCHRONIZED:
		// th NTP_EVENT_SYNCHRONIZED: last:%d, now:%d\r\n", lastTime, timestamp);
		// Serial.printf("onSyncEvent.NTP_EVENT_SYNCHRONIZED: time set: from %s to %s\n", NTP.getTimeAsChr( last), NTP.getTimeAsChr( t2 ) );
		char  buf1 [20];	// getTimeDateChr needs a buffer to write to
		char  buf2 [20];

		Logger.info("NTP_EVENT_SYNCHRONIZED: time set: from %s to %s. diff:%lu\n",
				NTP.getTimeDateChr ( last, buf1),  NTP.getTimeDateChr ( t2, buf2) , t2 -last);

		drawDateTime();
		if (Alarm.isAllocated(timerId_UpdateTime ) ) { // align to second '0'. simply nicer ...
			Alarm.free(timerId_UpdateTime);
			timerId_UpdateTime = Alarm.timerRepeat(0, 1, 0, drawDateTime);
		}

		break;

	case NTP_EVENT_SUMMERTIME_CHANGE:
		Serial.printf("NTP_EVENT_SUMMERTIME_CHANGE\n");
		break;
	}
}

long mqttLastReconnectAttempt = -1L;

void mqtt_Connect() {

	bool rc = false;

	// timer not yet started, or timer expired
	if ( mqttLastReconnectAttempt < 0 || ((millis() - mqttLastReconnectAttempt)  > 10000L )) {

		// Logger.debug ("MQTT trying connect to server\n");
		rc = mqttClient.connect(config.MqttId);	// our id
		if ( rc) {
			Logger.info("MQTT connected rc:%d\n",rc);

			// subscribe to our sensors:change
			rc = mqttClient.subscribe  ("ESP12Buero/OUT/#");		// NO PSTR (...) here. crash
			rc = mqttClient.subscribe  ("Aussensensor/OUT/#");

			// send a command to the sensors to send their data immediately

			// this is unique to my environment: with this command the sensor report their state immediately.
			rc = mqttClient.publish		("ESP12Buero/IN/" ,   "INIT");
			rc = mqttClient.publish		("Aussensensor/IN/" , "INIT");
			mqttLastReconnectAttempt = -1L;	// timer off
		}
		else {
				// cannot reconnect :add a timer every 10 secs to try to connect. Otherwise we flood the network
				mqttLastReconnectAttempt = millis();
				// Logger.error(PSTR("CANNOT connect to MQTT server. Starting 10 secs reconnect timer\n")); TODO
		}
	}
}

/*
 * the topic we want to listen and display
 */
void messageReceived(String topic, String payload, char * bytes, unsigned int length) {

	Logger.debug (PSTR ("Mqtt-Rcv:TP<%s>, PL:%s\n"), topic.c_str() , payload.c_str() );

	if ( topic ==  		"ESP12Buero/OUT/SI/TEMP"){	//
		if ( myTempIn == payload){
			//Serial.println("Found same SI TEmp:" + myTempIn + ":" + payload);
			return;
		}
		myTempIn = payload;
	}
	else if ( topic ==  "ESP12Buero/OUT/SI/HUM") {
		if (myHumIn == payload)
			return;
		myHumIn= payload;
	}
	else  if ( topic == "Aussensensor/OUT/SI/TEMP") {
		if ( myTempOut == payload)
			return;
		myTempOut = payload;

	}
	else if  ( topic ==  "Aussensensor/OUT/SI/HUM") {
		if ( myHumOut == payload)
			return;
		myHumOut= payload;
	}
	else{
		Logger.error (PSTR  ("Unhandled topic:<%s>, payload:<%s>\n"),  topic.c_str(), payload.c_str());
		return;
	}

	updateOwnTemps = true;
}

void Wifi_Init(){

	WiFiConnectHandler 		= WiFi.onStationModeGotIP		 ( Wifi_onConnectedGotIP);// As soon WiFi is connected, start NTP Client
	WiFiDisConnectHandler 	= WiFi.onStationModeDisconnected ( Wifi_onDisconnected);

	WiFi.hostname("Weatherstation");

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
	Logger.info (("WLAN Connected: hostname:%s, IP:%s, localMac:%s, SYSLOG: %s, WifiState:%d, Ch:%d, RSSI:%d dBm\n"),
			WiFi.hostname().c_str(), ipInfo.ip. toString().c_str(),  WiFi.macAddress().c_str(), syslogServerIP.toString().c_str(),
			WiFi.status(), WiFi.channel(), 	WiFi.RSSI() );

	delay (100);

	server.begin();	// only used for OTA update herein

	httpUpdater.setup(&server);

	NTP_Init();

	if (!Alarm.isAllocated(timerId_UpdateWeather ) ) { 	// if !already scheduled
		timerId_UpdateWeather = Alarm.timerRepeat(15 * 60 , timerUpdateWeather);
	}

	Logger.info  (PSTR("Wifi_onConnectedGotIP finished\n"));

	// mqtt_Connect();	// moved to main loop as it crashes herein.

}

/*
 	 Manage network disconnection. We need not care about reconnection, ESP SDK does it on its own

 */
void Wifi_onDisconnected(WiFiEventStationModeDisconnected event_info) {

	// this is called very often. only a change is really interesting.
	if ( lastWifiState == WiFi.status())
		return;	//nothing to do

	lastWifiState = WiFi.status();
	Logger.stopSyslog(); // makes no sense any more
	Logger.error("WLAN disconnected from SSID " + event_info.ssid +", Reason:" + String ( event_info.reason ));

	if (Alarm.isAllocated(timerId_UpdateWeather ) )	// makes no sense anymore
		Alarm.free (timerId_UpdateWeather);

	server.close();
	NTP.stop();
}

void sysLogInit(){

	// set the syslog server in logging
	int rc =0;
	// if the string is a valid ip4 adr, use it directly. otherwise try to resolve host by name

	 if (syslogServerIP.fromString(config.syslogServer) == true){
		Logger.setSysLogPrefix(WiFi.hostname().c_str());
		rc = Logger.startSyslog(syslogServerIP);
		Logger.info("Started SYSLOG to server:<%s>,rc:%d\n", syslogServerIP.toString().c_str(), rc);
	}
	else
		Logger.warn("No valid SYSLOG server address:<%s>", config.syslogServer);
}


// https://gist.github.com/sticilface/e54016485fcccd10950e93ddcd4461a3
// https://github.com/esp8266/Arduino/issues/1143

void WEBPageUpdate (){

	webPage   	= ("<html><head><h2>Weatherstation</h2></head> <body>");

	webPage.concat (("<br><h3>IP: ")   + WiFi.localIP()		.toString()
			+ ", Gateway:"+ WiFi.gatewayIP()	.toString()
			+ ", DNS:"    + WiFi.dnsIP(0)		.toString());

	webPage.concat (("<br>Hostname:") 		 + String (WiFi.hostname()));
	webPage.concat (("<br>MQTT: Id:")   	 + String (config.MqttId)  +", State:" + String (mqttClient.connected() ? "Connected" : "DISconnected"  ) + "</h3>");
	webPage.concat (("<br>Time:")     		 + NTP.getTimeDateStr (now())) ;
	webPage.concat (("<br>Compile DateTime:")+ String(version) + "</h3>");
	webPage.concat ("<br>FreeHeap:" 		 + String (ESP.getFreeHeap())
			+ ", ChipID:"  			 + String(ESP.getChipId())
			+ ", ESP-Core version:"  + ESP.getCoreVersion()
			+ ", ESP-SdkVersion:"	 + String(ESP.getSdkVersion())
	);

	webPage.concat (("<br>WLAN: Channel:") 	+ String (WiFi.channel())
			+  ", Signal:" + String (WiFi.RSSI())
			+" dBm, localMacAdr:"	+ WiFi.macAddress());

	// FORM
		webPage  .concat(("<h2>Configuration</h2><form action=configRsp method=post>"\
				"<p><label for=hostNameId>Hostname (change requires reboot)</label>"\
				"<input id=hostNameId name=hostNameId type=text  maxlength=30  value=") + String(config.hostname) + ">"\
				"</p>");

		// MQTT Device ID
		webPage.concat(("<label for=devid>Device ID (MQTT DevID)</label>"\
				"<input id=devName name=devid type=text  maxlength=19  value=") + String(config.MqttId) + "><br>");

		// MQTT Server
		webPage  .concat(("<label for=mqttServerName>MQTT Server addr</label>"\
				"<input id=mqttServer name=mqttServerName type=text  maxlength=19  value=") + String(config.mqttServer) + ">"\
				"<br>");

		// syslog Server
		webPage.concat("<label for=syslogServer>SYSLOG Server IP Addr (w.x.y.z)</label>"\
				"<input id=syslogServer name=syslogServer type=text  maxlength=19  value=" + String(config.syslogServer) + ">"\
				"<br>");


	// https://wiki.selfhtml.org/wiki/HTML/Formulare/input/Radio-Buttons_und_Checkboxen
	// for bezieht sich auf id in der n‰chsten Zeile
	webPage.concat(("<p><label for=DebugMode>Debug:</label>"\
			"<input type=checkbox  id=DebugMode name=DebugMode value=DebugMode ") + String (config.debug ? "checked" : "")  + "><br></p>");

	webPage .concat(("<button style=width:100px name=task value=save>Save</button>"\
			"</form>"\
			"<form action=cmd method=post>"\
			"<button style=width:100px name=button value=reboot>Reboot</button><br>"\
			"<button style=width:100px name=button value=reset>Reset</button><br>"\
			"<button style=width:100px name=button value=readIcons>Re-Read Icons</button><br>"\
			"<button style=width:100px name=button value=updateWeather>Update Weather</button><br>"\
			"</form>"\
			"</body></html>"));

	// Serial.println("WEB page size:" + String(webPage.length()));
}

void serverHandle_Cmd(){

	if(server.arg("button").equals("reboot")){
		Logger.info("Reboot requested.");
		server.send(200, text_html, PSTR("Reboot ok."));
		ESP.restart();
	}
	else
		if(server.arg("button").equals("reset")){
			serverHandle_Reset();
		}
		else
			if(server.arg("button").equals("readIcons")){
				serverHandle_updateIcons();
			}
			else if(server.arg("button").equals("updateWeather")){
				updateWeather= true;
			}
			else
				server.send(200, text_html, "CMD not found for 'button':" + server.arg("button") );
}


void serverHandle_ConfigRsp (){

	char tmp [ sizeof (config.hostname) ];
	bool    reboot = false;

	int l  = server.args();
	for (int i = 0; i< l ; i++) {
		Logger.info("ConfigRsp:" + String (i) + ", argN:" + server.argName(i) + ", argV:" + server.arg(i) );
	}
	strcpy ( tmp, server.arg("devid").c_str());

	if ( strcmp(config.MqttId, tmp)  != 0 ){
		Logger.info("MQTT DevId changed from <" + String (config.MqttId) + " > to " + server.arg("devid"));
		strcpy (config.MqttId, tmp);	// changes also the topics
		reboot = true;
	}

	strcpy ( tmp, server.arg("mqttServerName").c_str());

	if ( strcmp(config.mqttServer, tmp)  != 0  ){
		Logger.info("MQTT Server name changed from <" + String (config.mqttServer) + "> to <" + server.arg("mqttServerName") +">");
		strcpy (config.mqttServer, tmp);
		reboot = true;
	}

	strcpy ( tmp, server.arg("hostNameId").c_str());
	if (strcmp(config.hostname, tmp)  != 0 ){
		Logger.info("Hostname changed from:<" + String (config.hostname) + " > to " +  String ( tmp));
		strcpy (config.hostname, tmp);
		reboot = true;
	}

	strcpy ( tmp, server.arg("syslogServer").c_str());
	if (strcmp(config.syslogServer, tmp)  != 0 ){
		Logger.info("syslogServer changed from:<" + String (config.syslogServer) + "> to <" +  String ( tmp) +">");
		strcpy (config.syslogServer, tmp);
		sysLogInit();
	}
	// debug
	config.debug =  (server.arg  ("DebugMode").length() != 0) ?  true : false;
	Logger.info("Debug:%s, LogLevel:%d\n",  bool2ChrTrueFalse(config.debug), Logger.getLevel());
	Logger.setLevel( config.debug ? LOGLEVEL_DEBUG :  LOGLEVEL_INFO);

	EEPromWriteConfig();

	if ( reboot) {
		server.send(200, text_html, "Restarting...");
		ESP.restart();
	}
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
