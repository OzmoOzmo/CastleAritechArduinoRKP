/*
 * Log.h - Logs all to Hyterterminal or other via Arduino Pin 11
 *
 * 'Serial1'(Pin1&2) is used for Panel TX and the USB 'Serial' is really bad for debugging on Leonardo - so we use Pin 11.
 *
 * Note: Have QUIET Set during Normal use - some panels require full speed
 *
 * Created: 3/30/2014 11:35:06 PM
 *
 *   Aritech Alarm Panel Arduino Internet Enabled Keypad -  CS350 - CD34 - CD72 - CD91 and more
 *
 *   For Arduino (UNO or Leonardo) with added Ethernet Shield
 *
 *   See Circuit Diagram for wiring instructions
 *
 *   Author: Ozmo
 *
 *   See: http://www.boards.ie/vbulletin/showthread.php?p=88215184
 *
*/

#ifndef LOG_h
#define LOG_h

#include "Config.h"

#include <Arduino.h>

#ifndef QUIET

	#include "SoftwareSerial.h"

	//extern class SoftwareSerial;
	extern SoftwareSerial softwareSerial;

	#define nSerialBaudDbg 19200

	#define Log(x) softwareSerial.print(x)
	#define Log2(x,y) softwareSerial.print(x,y)
	#define LogLn(x) softwareSerial.println(x)
	#define LogLn2(x,y) softwareSerial.println(x,y)
	
#else
	#define Log(x) {}
	#define Log2(x,y) {}
	#define LogLn(x) {}
	#define LogLn2(x,y) {}
	
	
#endif


void LogHex(byte rx);
void LogHex(char* s);
void LogHex(byte* s , int len);

void Log_Init();

#endif
