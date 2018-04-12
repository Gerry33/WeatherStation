
#include "SimpleLog.h"

/* NEW:
 * Log hierarchy: (not bit combinations any more)
 *
 * 		QUIET	0 	-lowest
 * 		ERROR	1
 * 		WARNING	2
 * 		INFO	3
 * 		DEBUG	4	-highest
 *
 * lower level include always higher levels:  e.g. DEBUG includes I, W, E
 *
 */
const char * prefix [ LOGLEVEL_MAX ] = {"", "E:", "W:", "I:", "D:"};

SimpleLog Logger;

int  SimpleLog::begin(LOGLEVEL_t level, IPAddress syslogServer)
{
	int rc;
	_syslogPrefix[0] = '\0';

	_syslogServer = syslogServer;
	// _udp.begin(2390); not necessary: cause not listening
	// only once here: cause otherwise always a new udp context is generated
	rc = _udp.beginPacket(syslogServer, 514);

	begin(level);
	return( rc );
}

int  SimpleLog::begin(LOGLEVEL_t level)
{
	setLevel( level );	// errors must always be logged

	return( SIMPLE_LOG_SUCCESS);
}

int  SimpleLog::begin()
{
	setLevel( LOGLEVEL_INFO);	// default

	return( SIMPLE_LOG_SUCCESS);
}

/**
 *1: ok, 0: failure
 */
int  SimpleLog::startSyslog (IPAddress syslogServer)
{
	_syslogServer = syslogServer;
	_state = _udp.beginPacket(syslogServer, 514);	// Q: needed every send time ? A: no
	if ( _state == 0 )
		Serial.println ( "Unable to connect to syslog server:" + syslogServer);
	return( _state);
}

int  SimpleLog::startSyslog (const char * hostname)
{
	 _state = _udp.beginPacket(hostname, 514);

	return( _state);
}

void SimpleLog::setSysLogPrefix(const char *pf) {

	memset  (_syslogPrefix, 0,0);
	strncpy ( _syslogPrefix, pf, sizeof (_syslogPrefix) - 2 );
	strcat  ( _syslogPrefix, ":");

//			 _syslogPrefix 	  [ sizeof (_syslogPrefix) - 2] = ':';
//			 _syslogPrefix    [ sizeof (_syslogPrefix)    ] = '\0';

// vsnprintf_P (&_syslogPrefix, sizeof(_syslogPrefix)-2 , "", pf);
	// Serial.printf ( "_syslogPrefix:<%s>\n", pf);

	// strcat ( _syslogPrefix , ":");
}

int  SimpleLog::stopSyslog () {

	_state = 0;
	_udp.stop();

	return( _state);
}

int SimpleLog::setLevel(LOGLEVEL_t newLevel)
{
	int retVal = SIMPLE_LOG_SUCCESS;

	if( newLevel >= LOGLEVEL_QUIET &&  newLevel < LOGLEVEL_MAX) {
		_level = newLevel;
	}
	else
		retVal = SIMPLE_LOG_LOGLEVEL_INVAL;
	return( retVal );
}


LOGLEVEL_t SimpleLog::getLevel()
{
	return (_level);
}



void  SimpleLog::logIntern(LOGLEVEL_t logLevel, const char* msg, va_list args)
{
	// char *  bufl = new  char [40];	//this would be doubled as the udp also buffers
	// memset 		(buf,0,sizeof (buf));
	// char * 	buf  	 = new  char [100];

 	strcpy 			(&logBuffer[0],  prefix [logLevel]);
 	strcpy 			(&logBuffer[2],  _syslogPrefix);
    vsnprintf_P 	(&logBuffer [strlen(logBuffer)], sizeof(logBuffer) - strlen(logBuffer) , msg, args );	// prefix in first 2 bytes
	Serial.printf 	( logBuffer );
	sendUdpSyslog 	( logBuffer );

	// free (buf);
}

void  SimpleLog::logIntern(LOGLEVEL_t logLevel, String msg)
{
		msg = String(prefix [logLevel]) + String(_syslogPrefix)  + msg ;
		Serial.println(msg);
		sendUdpSyslog (msg.c_str());

}

//  ------------------------
void SimpleLog::debug (const char * msg, ...) {

	if( _level >= LOGLEVEL_DEBUG ) {
		va_list args;
		va_start(args, msg); // store all argumenet after 'msg'
		logIntern ( LOGLEVEL_DEBUG, msg, args);
		va_end(args );
	}
}


void  SimpleLog::debug (String msg){
	if( _level >= LOGLEVEL_DEBUG ) {
		logIntern( LOGLEVEL_DEBUG, msg);
	}
}

//  --------------------------------------------------------------

void SimpleLog::info (const char * msg, ...) {

	if( _level >= LOGLEVEL_INFO) {
		va_list args;
		va_start(args, msg); 	// store all argument after 'msg'
		logIntern ( LOGLEVEL_INFO, msg, args);
		va_end(args );
	}

}

void SimpleLog::info (String msg){
	if( _level >= LOGLEVEL_INFO) {
		logIntern( LOGLEVEL_INFO, msg);
	}
}
// --------------------------------------------------------------------------
void SimpleLog::error (String msg){
	logIntern( LOGLEVEL_ERROR, msg);
}


void SimpleLog::warn (String msg){
	logIntern( LOGLEVEL_WARNING, msg);
}

void SimpleLog::warn ( const char * msg, ...){
		va_list args;
		va_start(args, msg); 	// store all argument after 'msg'
		logIntern( LOGLEVEL_WARNING, msg, args);
		va_end(args );
}

// error kommt immer durch
void SimpleLog::error (const char * msg, ...) {

		va_list args;
		va_start(args, msg); 	// store all argument after 'msg'
		logIntern ( LOGLEVEL_ERROR, msg, args);
		va_end(args );

}
//  --------------------------------------------------------------

int  SimpleLog::sendUdpSyslog(const  char * msgtosend)
{
	if  (_state) {		// != 0

		// no udp.begin here as we always write to the same server.
		//_udp.write(_syslogPrefix,strlen(_syslogPrefix));
		unsigned int charSend = _udp.write(( unsigned char *) msgtosend, 	(unsigned int) strlen(msgtosend	));
		int rc 		 = _udp.endPacket();
 		delay (10);		// The ESP8266 needs a delay of at least 2ms between consecutive UDP packages
		if (rc   == 0 ){ // nothing send
			// Serial.println("UdpSyslog not send.");
			return rc;
		}
		if (charSend != strlen(msgtosend))
			Serial.printf("sendUdpSyslog incomplete:%d vs. %d\n", charSend, strlen(msgtosend));
		return charSend;
	}
	return -1;
}
