# ESP8266 Weatherstation with TFT Display

This is another fork of the ESP8266 Weatherstation based on https://github.com/Bodmer/TFT_eSPI/tree/master/examples/320%20x%20240/weather-station-v8. All credits go to the persons referred in the code.

This fork has the following additional features:

	- Wunderground client:
		- Adpations using the new simplifications of the new WG API:  winddir, pressure trend, language specifics, ...
		- HTTClient (instead of Wificlient) 
		(Tested with ~ 10k simulated queries to an own WU simulation server. No heap or performance issues 
		any more.
		
	- pressure trend icon
	- own icons for inside/outside temperature
	- loop () reconstruction to avoid Wifi- disconnects caused by long lasting operations
	- MQTT support to display measures from other sensors in the network
	- HTTP OTA firmware update
	- Wifi username/password injected by compiler commandline
	- NTP- time with timezone and Summer-/Wintertime detection and callback event
	- extended logging
	- SYSLOG
	- Timer alarm
	- WEB -GUI for basic configuration with tooltips and systemstatistics and ressources overview. See screenshot in WiKi.
	- BMP- display only as JPG has pixel issues
	- Arbitrary download of icons from any local/remote source
	- Deleting  and reloading icons on request using REST Web service without restart
	- Reset/ format SPIFF from  REST Web service
	- Layout redesign
	- Icon adaptations (icons not published here due to legal fears).
	- Alive pixel on display
	- time supervision of the last remote observation time to recognize outage  on remote weather stations
	- removed all unnecessary screen gimmicks for very low screen flickering
