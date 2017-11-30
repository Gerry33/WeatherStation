/*
 * Common.c

 *
 *  Created on: 14 Sep 2017
 *      Author: Gerry33
 */

#include <Arduino.h>


const char *  bool2CharOnOff (bool t) {
	return (t ? "ON":"OFF" );
}

const char *  bool2ChrTrueFalse (bool  t) {
	return (t ? "true":"false" );
}

// https://stackoverflow.com/questions/791982/determine-if-a-string-is-a-valid-ip-address-in-c
/**
 * @return 1= isValidIP
 * @return 0= is NOT ValidIP
 */
int isIp_v4( char* ip){

	int num;
	int flag = 1;
	int counter=0;
	char buf [20];

	strncpy (buf,  ip, sizeof (buf));

	//char* p = strtok(ip,".");
	char* p = strtok(buf,".");

	while (p && flag ){
		num = atoi(p);
		if (num>=0 && num<=255 && (counter++<4)){
			flag=1;
			p=strtok(NULL,".");
		}
		else{
			flag=0;
			break;
		}
	}
	return flag && (counter==4);	// !!! see comment
}

void ICACHE_FLASH_ATTR printFloat(float val, char *buff) {
	char smallBuff[16];
	int val1 = (int) val;
	unsigned int val2;
	if (val < 0) {
		val2 = (int) (-100.0 * val) % 100;
	} else {
		val2 = (int) (100.0 * val) % 100;
	}
	os_sprintf(smallBuff, "%i.%02u", val1, val2);

	strcat(buff, smallBuff);
}

// extern Config_t config;
