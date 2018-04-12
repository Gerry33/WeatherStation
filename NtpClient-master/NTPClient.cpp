/*
   This file is part of the NTPClient library.

   Copyright 2016 - 2017 Sacha Telgenhof (stelgenhof@gmail.com). All rights reserved.

   For the full copyright and license information, please view the LICENSE
   file that was distributed with this source code.

   Re-Design by Gerd Sinne
   	   - integrated own time lib to be independend of external lib calls.
   	   - synched time-lib directly when a new sntp time is available - instead of waiting for a time call.

 */

#include "NTPClient.h"

// #define DEBUG_NTPCLIENT
#ifdef DEBUG_NTPCLIENT
// #define DEBUGLOG(...) os_printf(__VA_ARGS__)
#define DEBUGLOG(...) Serial.printf(__VA_ARGS__)
//extern SimpleLog Logger;
//#define DEBUGLOG(...) Logger.debug (const char *format, ...);


#else
#define DEBUGLOG(...)
#endif

NTPClient NTP;
onSyncEvent_t 	onNTPSyncEvent ;		// Event handler callback function

String weekdayChr 	  [] ={"??","So","Mo","Di","Mi","Do","Fr","Sa","nn"};
String monthName 	  [] ={"???","Jan","Feb","Mär","Apr","Mai","Jun","Jul,","Aug","Sep","Okt","Nov","Dez","nnn"};

NTPClient::NTPClient() {
	// Initialize class members
	syncIntervallCurrent = -1;		// do not start until someone calls init
	syncIntervallExtern 	= 86400L; 	// (long) Polling interval in secs for periodic time synchronization. set by user
	_timestamp 			= 0;
	timeStr.reserve  	(30);
	summerTimeChange 	= false;
	inSync 				= false;
	isSummertime 		= false;
	// change defaults here
	setNTPServer ((char *)"fritz.box",      0);
	setNTPServer ((char *)"0.de.pool.ntp.org",1);


}

bool NTPClient::setNTPServer(char *server, uint8_t idx) {
	// Supports 3 SNTP servers at most (0 ~ 2)
	if (idx > (NTP_SERVERS_MAXIMUM - 1)) {
		return false;
	}
	sntp_stop();
	sntp_setservername(idx, server);
	DEBUGLOG("[NTP] NTP Server #%d set to: %s.\n", idx, server);
	sntp_init();

	return true;
}
//
char *NTPClient::getNTPServer(uint8_t idx) {
  // Supports 3 SNTP servers at most (0 ~ 2)
  if (idx > (NTP_SERVERS_MAXIMUM - 1)) {
    return {};
  }

  return sntp_getservername(idx);
}

/* bool NTPClient::init(char *server, tz_utc_offsets_t utcOffset, int refreshIntervall) {
bool NTPClient::init(char *server, int  timezone, int refreshIntervall) {

	if (!setNTPServer(server)) {
		return false;
	}

	// Adjust for UTC Offset. Need to set timezone to 0 (UTC) since by default the
	// timezone is set to UTC+0800
	sntp_stop();
	if (sntp_set_timezone(timezone))
		sntp_init();
	else
		Serial.println("[NTP] Error:sntp_set_timezone\n");

	syncIntervallCurrent 	= NTP_SHORT_INTERVAL;
// 	syncIntervallExtern	= refreshIntervall;

	// instead of providing a callback for the time lib we set the time actively in syncTimefromSNTP.
	//
	// the time lib has its own sync intervall calling getTime() below.
	// this would lead to a double time sync. Disadv: Alarm libs have to be set actively to '0'
	// e.g. setTime ( 0,0,0.....);

	//  setSyncProvider	( getTime ); 	// time provider : time.cpp

//	DEBUGLOG("[NTP] Time synchronization init done.\n");
//	if (onNTPSyncEvent != NULL) {
//		onNTPSyncEvent(NTP_EVENT_INIT ,0, 0);
//	}

	return true;
}
*/
// indicates no internal NTP needed. sync must be done from outside.
bool NTPClient::initWithoutRefresh(int  utcOffset) {

	init(utcOffset, -1) ;
}
/*
 * the NTP switches the check time from NTP_SHORT_INTERVAL_DEFAULT to refreshIntervall once
 * the NTP is synced.
 */
bool NTPClient::init(int  timezone, int refreshIntervall) {

	// Adjust for UTC Offset. Need to set timezone to 0 (UTC) since by default the
	// timezone is set to UTC+0800
	sntp_stop();
	if (sntp_set_timezone(timezone))
		sntp_init();
	else
		Serial.println("[NTP] Error:sntp_set_timezone\n");

	syncIntervallCurrent = NTP_SHORT_INTERVAL_DEFAULT;
	syncIntervallExtern	 = refreshIntervall;

	// !! we donn't do this anymore. the NTP sets the time actively in syncTimefromSNTP
	// time lib sync; the time lib has its own sync intervall calling getTime() below.
	// this would lead to a double time sync. Disadv: Alarm libs have to be set actively to '0'
	// e.g. setTime ( 0,0,0.....);

	//  setSyncInterval 	( refreshIntervall) ;	// !!! this must be first, then the sync provider must be set cause it calls the getSync
	//  setSyncProvider	( getTime ); 	// time provider : time.cpp

	DEBUGLOG("[NTP] Time synchronization init done.\n");

	if (onNTPSyncEvent != NULL) {
		onNTPSyncEvent(NTP_EVENT_INIT ,0, 0);
	}

	return true;
}

bool NTPClient::stop() {
//	setSyncProvider(NULL);
	sntp_stop();

	DEBUGLOG("[NTP] Time synchronization stopped.\n");

//	if (onNTPSyncEvent != NULL) {
//		onNTPSyncEvent(NTP_EVENT_STOP,0,0);
//	}
	syncIntervallCurrent = 0;
	inSync 				 = false;
	return true;
}

void NTPClient::setOnSyncEvent_cb (onSyncEvent_t cb)
{
	onNTPSyncEvent = cb;
}


// this is a bug in the original implementation: the buffer must be allocated new every time on heap.
// but then it does not clean it and crashes sooner or later.
// therefore second impl. of getTimeDateChr is done where the buffer is provided e.g. on stack.

char dateTimeBuf[30];

const char *NTPClient::getTimeDateChr(time_t timeInput) {

	//  char *dt = new char[40]; // !!! bug in origina llib , cause not deallocated
	//  char buf[40];
	//snprintf(buf, sizeof(buf), "%d-%02d-%02dT%02d:%02d:%02d", year(tm), month(tm),
	//          day(tm), hour(tm), minute(tm), second(tm));
	// DEBUGLOG("[NTP] getTimeDate");
	//  sprintf(buf, "%d-%02d-%02dT%02d:%02d:%02d", year(tm), month(tm),day(tm), hour(tm), minute(tm), second(tm));
	//  strcpy(dt, buf);

	tmElements_t tm;
	breakTime	 ( timeInput, tm);
	// we must not use hour() , day() , ... here as it uses the internal cache of timelib
	sprintf		 ( dateTimeBuf, "%d-%02d-%02dT%02d:%02d:%02d", tm.Year + 1970, tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
	return 		   dateTimeBuf;

}

// same impl with buffer e.g. on stack. must be used if more than one call to getTimeDateChr in one function
const char *NTPClient::getTimeDateChr(time_t timeInput, char * buf ) {
	tmElements_t tm;
	breakTime(timeInput, tm);

	sprintf		 (buf, "%d-%02d-%02dT%02d:%02d:%02d", tm.Year + 1970, tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
	return 		 buf;
}

String NTPClient::getTimeDateStr (time_t tm) {
	timeStr = String (getTimeDateChr ( tm ));
	return timeStr ;

}


String  NTPClient::getDateAsStr	() {

	sprintf		 (dateTimeBuf, "%d.%02d.%02d",  day (now()) , month ( now() ), year(now()) );
	return 		 String(dateTimeBuf);

}


String  NTPClient::getWeekdayAsStr	() {
	return  weekdayChr [weekday()] ;
}

String  NTPClient::getMonthAsStr	() {
	return  monthName [ month ( ) ] ;
}
// --------------------------------------------------------------------------------------------------------

String NTPClient::getTimeAsStr(time_t tm) {
	timeStr = String (getTimeAsChr ( tm ));
	return timeStr ;
}

String  NTPClient::getTimeAsStr	() {
	return getTimeAsStr(now());	// now() -> time.cpp
}

// not really time zone ...
const char * NTPClient::getTimeZoneAsChr() {
	return (NTP.isSummertime ? "CEST" : "CET");
}

String   NTPClient::getTimeZoneAsStr(){
	return String(getTimeZoneAsChr());
}


const char *NTPClient::getTimeAsChr(time_t timeInput) {

	//  char *dt = new char[20];
	//  char buf[20];
	//  sprintf(buf, "%02d:%02d:%02d",  hour(tm), minute(tm), second(tm));
	//  strcpy(dt, buf);

	tmElements_t tm;
	breakTime(timeInput, tm);

	sprintf(dateTimeBuf, "%02d:%02d:%02d",  tm.Hour, tm.Minute, tm.Second);
	return dateTimeBuf;
}

/**
 * this is independent of any other mechanism. it runs fast if no smntp sync has been done
 * and long_interval slower, if done
 * this is a full syncron method, but yielded
 */

bool   NTPClient::syncTimeFromSNTP( ) {

	if (!WiFi.isConnected()) {
		DEBUGLOG("[NTP] syncTimeFromSNTP unable to sync. WiFi not connected.\n");
		return false;
	}

	DEBUGLOG("[NTP:syncTimeFromSNTP.Start]from:%s.\r\n", sntp_getservername(0));

	unsigned long 	newTimestamp = newTimestamp = sntp_get_current_timestamp();
	yield();	// this is essential. do not remove
	if (newTimestamp) {

		// sync time has changed from the initial NTP_SHORT_INTERVAL to normal set by user
		if (syncIntervallCurrent != syncIntervallExtern ) {
			DEBUGLOG("[NTP]:changed syncIntervall from %d to %d\n", syncIntervallCurrent, syncIntervallExtern);
			syncIntervallCurrent =  syncIntervallExtern;
		}

		newTimestamp 	  = adjustTimeZone (newTimestamp );
		uint32_t lastTime = now1();		// do not use now() as it call this function and recursives

		// gsi: added: this sets the timestatus to 'timeSet' (2) so that the callback called below has a proper time state

		// !!! this is the core. this adjusts the underlying time lib
		setTime( newTimestamp );	// time.cpp

#ifdef DEBUG_NTPCLIENT
		char buf1[30];	// time buffer for getTimeDateChr().
		char buf2[30];
		uint32_t diff  = newTimestamp - lastTime;
		Serial.printf("[NTP] Time synchronized from %s to:%s, diff:%d\r\n", NTP.getTimeDateChr ( lastTime, buf1 ), NTP.getTimeDateChr ( newTimestamp, buf2 ),  diff);
		// Serial.printf("[NTP] Time synchronized, diff:%d\r\n", diff);
#endif
		if (onNTPSyncEvent != NULL) {		// callback
			DEBUGLOG("[NTP] NTP callback with NTP_EVENT_SYNCHRONIZED: last:%d, now:%d\r\n", lastTime, newTimestamp);
			onNTPSyncEvent(NTP_EVENT_SYNCHRONIZED, lastTime, newTimestamp);

			if ( summerTimeChange == true) {
				if (_timestamp > 0 )	// only if we have a change within a valid time.
					onNTPSyncEvent(NTP_EVENT_SUMMERTIME_CHANGE, lastTime, newTimestamp);
				summerTimeChange  = false;
			}
		}
		// make it official
		_timestamp = newTimestamp;
		inSync = true;
		return true;
	} else {
		inSync = false;
		// Received no response from the NTP Server
		// leave interval as is.
		DEBUGLOG("[NTP.syncTimeFromSNTP] Error no SNTP response from <%s>.\r\n", sntp_getservername(0));

		if (onNTPSyncEvent != NULL) {
			DEBUGLOG("[NTP] calling NTP callback with NTP_EVENT_NO_RESPONSE.\n");
			onNTPSyncEvent(NTP_EVENT_NO_RESPONSE, 0, 0);
		}
		return false;
	}

}

// no static here: just in the header. this is called by time.cpp as callback if it needs a new time
//	not needed anymore;

time_t NTPClient::getTime() {

	DEBUGLOG("[NTP]:getTime\r\n");
	syncTimeFromSNTP();
	return _timestamp;

}

bool NTPClient::updateForced() {

	// Update NTP
		DEBUGLOG("[NTP]:update forced\r\n");
		yield();
		syncTimeFromSNTP();
		yield();
		now();	// must be called to update the background time in time.cpp
		return true;
}

// function to be put into main loop when no external timers are available.
// NTP sync can also be done using NTP.updateForced()
bool NTPClient::update() {

	// Update NTP
	if ( (syncIntervallCurrent > 0) &&  ((millis() - previousMillis) > (syncIntervallCurrent * 1000 )))  {
		DEBUGLOG("[NTP]:update\r\n");
		previousMillis = millis();
		yield();
		syncTimeFromSNTP();
		yield();
		now();	// updates must be called to update the background time in time.cpp
		return true;
	}
	return false;
}

// ----------------------------------------------------
// _timezone from this class
unsigned long NTPClient::adjustTimeZone(unsigned long _timeStamp) {

	bool st =   summerTime(_timeStamp);

	DEBUGLOG("[NTP]:Summer time check: isSummertime:%d, st:%d\n", isSummertime,st) ;
	if ( isSummertime != st) {
		isSummertime 		= st;
		summerTimeChange 	= true;	// set marker for callback to signify change
		DEBUGLOG("[NTP]:Summer time change detected. isSummertime:%d, st:%d\n", isSummertime,st) ;
	}

	if (st) {
		_timeStamp += 3600; 		// European Summer time
		DEBUGLOG("[NTP]:European Summer time\n");
	}
	return _timeStamp;
}

//
// Summertime calculates the daylight saving time for middle Europe. Input: Unixtime in UTC
// https://github.com/SensorsIot/NTPtimeESP
boolean NTPClient::summerTime ( time_t timeStamp ) {

	breakTime(timeStamp, dateTime);	// time.cpp
	dateTime.Year+=1970;			// http://forum.arduino.cc/index.php?topic=172044.msg1278536#msg1278536

	// DEBUGLOG("[NTP]:ST check: month:%d, day:%d, hr:%d, yr:%d\n", dateTime.Month, dateTime.Day, dateTime.Hour, dateTime.Year ) ;

	if (dateTime.Month < 3 || dateTime.Month > 10) {
	//	DEBUGLOG("[NTP]:ST check A: false\n");
		return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
	}
	if (dateTime.Month > 3 && dateTime.Month < 10) {
	//	DEBUGLOG("[NTP]:ST check B: true\n");
		return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
	}

	if (   ((dateTime.Month == 3)  && (dateTime.Hour + 24 * dateTime.Day) >= (3 +  24 * (31 - (5 * dateTime.Year / 4 + 4) % 7)))
	    || ((dateTime.Month == 10) && (dateTime.Hour + 24 * dateTime.Day) <  (3 +  24 * (31 - (5 * dateTime.Year / 4 + 1) % 7)))
	   )
	{
//		DEBUGLOG("[NTP]:ST check C: true\n");
		return true;
	}
	else{
//		DEBUGLOG("[NTP]:ST check D: false\n");
		return false;
	}
}

