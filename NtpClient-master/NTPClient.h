/*
   This file is part of the NTPClient library.

   Copyright 2016 - 2017 Sacha Telgenhof (stelgenhof@gmail.com). All rights reserved.

   For the full copyright and license information, please view the LICENSE
   file that was distributed with this source code.

   Re-design by Gerd Sinne.
 */

#pragma once
#include <sys/types.h>
#include <cstdint>

class String;

#ifndef ESP8266
#error This library only supports boards with the ESP8266 MCU
#endif

#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <functional>
extern "C" {
    #include "sntp.h"
}

using namespace std;

#define DEFAULT_NTP_SERVER "0.de.pool.ntp.org"  // Default NTP server. Recommended to use a NTP server closer to your location.
// #define DEFAULT_POLLING_INTERVAL 1800  // Default polling interval (seconds)
// #define NTP_MINIMUM_INTERVAL  15   // Minimum polling interval (seconds)
#define NTP_SHORT_INTERVAL_DEFAULT 10  // was: 5 polling intervall for NTP response; 1 sec too less,
#define NTP_SERVERS_MAXIMUM   	   3   // Maximum number of supported NTP servers

// Events used for the callback function
typedef enum {
        NTP_EVENT_INIT, // Start of NTP synchronization
        NTP_EVENT_STOP, // End of NTP synchronization
        NTP_EVENT_NO_RESPONSE, // No response from NTP server
        NTP_EVENT_SYNCHRONIZED, // Time successfully received from NTP server
		NTP_EVENT_SUMMERTIME_CHANGE	// summer/winter time change detected
} NTPSyncEvent_t;

typedef std::function<void (NTPSyncEvent_t, time_t, time_t)> onSyncEvent_t;

class NTPClient {

public:
        NTPClient();
        bool 		isSummertime;


        /**
         * @brief Sets the name of a NTP server to use.
         *
         * This function sets the name of a NTP Server to use for time synchronization.
         * By default the first (0) server is set.  The ESP8266 supports 3 SNTP servers at most (0 ~ 2),
         * whereby 0 is the main server, and servers identified at index 1 and 2 are backup NTP servers.
         *
         * @param  server the host name of the NTP server to use for time synchronization
         * @param  idx    idx NTP server index (0 ~ 2). If no index is given, the first server is set.
         * @return        true if successful
         */
        bool setNTPServer(char *server, uint8_t idx = 0);

        /**
         * @brief Gets the name of the configured NTP server.
         *
         * This function gets the name of the configured NTP Server set at index 'idx'.
         * By default the first (0) server is returned.  The ESP8266 supports 3 SNTP servers at most (0 ~ 2),
         * whereby 0 is the main server, and servers identified at index 1 and 2 are backup NTP servers.
         *
         * @param  idx NTP server index (0 ~ 2). If no index is given, the first server name is returned.
         * @return the name of the configured NTP Server set at index 'idx'.
         */
        char * getNTPServer(uint8_t idx = 0);

        /**
         * @brief Starts the NTP time synchronization.
         *
         * @param  server    the host name of the NTP server to use for time synchronization
         * @param  utcOffset the UTC Offset to use (i.e. timezone). If empty, UTC will be used.
         * @return           true if successful
         */
     // bool init(char *server = (char *)DEFAULT_NTP_SERVER, int utcOffset = 1, int refreshIntervall = 86400);
        // no two overlaoded methods with different number of same parameter type possible
        bool init( int utcOffset = 1, int refreshIntervall = 86400);
        bool initWithoutRefresh( int  utcOffset =1 );

        /**
         * @brief Stops the time synchronization
         * @return true if successful
         */
        bool stop();
        bool update();
        bool updateForced();


        /**
         * @brief Set a callback function that triggers after synchronization request.
         *
         * @param cb handle to callback function
         */
        void 		 setOnSyncEvent_cb ( onSyncEvent_t  cb) ;
        const char * getTimeDateChr	( time_t _time);
        const char * getTimeDateChr ( time_t tm, char * buf );
        const char * getTimeZoneAsChr();
        String 		 getTimeZoneAsStr();

        String 		 getTimeDateStr ( time_t  time);

        const char * getTimeAsChr	( time_t _time);
        String       getTimeAsStr	( time_t _time) ;
        String       getTimeAsStr	() ;

        String       getDateAsStr	() ;	// yyyy-mm-dd
        String  	 getWeekdayAsStr	() ;
        String  	 getMonthAsStr	() ;
        time_t 		 getTime();

//	bool isInSync() const {
//		return inSync;
//	}

time_t getLastNTPSyncTimestamp() const {
		return _timestamp;
	}

        String 		 timeStr;
        tmElements_t dateTime;
        time_t		 _timestamp;				// the current time

private:

        bool 			inSync; 	// false : not in sync or sntp stopped. true: succesfully synced.
        unsigned long 	previousMillis = 0;
        bool  			syncTimeFromSNTP	();
        unsigned int 	syncIntervallCurrent ; 	//
        unsigned int 	syncIntervallExtern; 	// (long) Polling interval in secs for periodic time synchronization. set by user

    // * @return time in UNIX time format.
    // static time_t 	getTime();	// static for timelib
	// Converts a unix time stamp to a strDateTime structure

        void 			convertUnixTimestamp ( unsigned long _tempTimeStamp) ;
        boolean 		summerTime			 ( time_t  _timeStamp );
        unsigned long 	adjustTimeZone		 ( unsigned long _timeStamp );
        bool			summerTimeChange ;
};

extern NTPClient NTP;
