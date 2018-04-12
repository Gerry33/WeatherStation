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

Adapted by Bodmer to use the faster TFT_ILI9341_ESP library:
https://github.com/Bodmer/TFT_ILI9341_ESP

*/

// gsi: works !
#pragma once

// Setup
const int UPDATE_INTERVAL_SECS = 10 * 60; // Update every 10 minutes

// Pins for the TFT interface are defined in the User_Config.h file inside the TFT_ILI9341_ESP library

// Wunderground Settings, EDIT TO SUIT YOUR LOCATION
const boolean IS_METRIC = true; // Temperature only? Wind speed units appear to stay in mph. To do: investigate <<<<<<<<<<<<<<<<<<<<<<<<<
const String WUNDERGRROUND_API_KEY   = "a87045dd69c1e7d6";
//const String WUNDERGRROUND_API_KEY = "1c265fajf48s0a82"; // Random key example showing how the above line should look

// For language codes see https://www.wunderground.com/weather/api/d/docs?d=language-support&_ga=1.55148395.1951311424.1484425551
const String WUNDERGRROUND_LANGUAGE = "DL"; // capital letters ! not ISO ! , Language EN = English

// For a list of countries, states and cities see https://www.wunderground.com/about/faq/international_cities.asp
const String WUNDERGROUND_COUNTRY = "de"; // UK, US etc
const String WUNDERGROUND_CITY = "salzkotten"; // City


#define WIND_SPEED_SCALING 1.60934  // mph to kph
#define WIND_SPEED_UNITS " km/h"

//Thingspeak Settings - not used, not trusted as an US based service
// const String THINGSPEAK_CHANNEL_ID = "<CHANNEL_ID_HERE>";
// const String THINGSPEAK_API_READ_KEY = "<API_READ_KEY_HERE>";


/***************************
 * End Settings
 **************************/
