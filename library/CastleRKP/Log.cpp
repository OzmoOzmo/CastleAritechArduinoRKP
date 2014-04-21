/*
 * Log.cpp - Logs all to Hyterterminal or other via Arduino Pin 11
 *
 * 'Serial1'(Pin1&2) is used for Panel TX and the USB 'Serial' is really bad for debugging on Leonardo - so we use Pin 11.
 *
 * Note: Have QUIET Set during Normal use - some panels require full speed
 *
 *   Created: 3/30/2014 11:35:06 PM
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


#include "Log.h"


//Workaround if needed for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))
#endif

#ifndef QUIET
//---Serial (for debugging)
//#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SoftwareSerial.cpp>

SoftwareSerial softwareSerial(9, 11,false); // RX, TX
#endif


#ifdef QUIET
void LogBuf(char* t){}
void LogHex(byte rx){}
void LogHex(char* s){}
void LogHex(byte* s , int len){}
#else
void LogBuf(char* t)
{
	int ix=0;
	while(t[ix]!=0)
	Log((char)t[ix++]);
	//LogLn(".");
	LogLn("..");
}
void LogHex(byte rx)
{//Show one two digit hex number
	if (rx<16) Log('0');
	Log2(rx,HEX);
	Log(' ');
}
void LogHex(char* s)
{//Show a buffer as hex
	int n=0;
	while(s[n] != 0)
	{
		Log2(s[n], HEX);
		Log(' ');
	}
	//LogLn("{end}");
}
void LogHex(byte* s , int len)
{
	const int l = 8;
	for(int col=0;;col++)
	{
		if ((col*l) >= len)		//len=10 = 1*8
			break;
		for(int r=0;r<l;r++)
		{
			if (r!=0) Log(' ');
			int x = col*l+r;
			byte c = (x<=len) ? s[col*l+r] : 0xFE;
			if (c<16) Log('0'); Log2(c, HEX);
		}
		Log(':');
		for(int r=0;r<l;r++)
		{
			int x = col*l+r;
			byte c = (x<len) ? s[col*l+r] : ',';
			if (c <= ' ' || c>= 128)
			{Log('.');}		//brackets important
			else
			{Log((char)c);} //brackets important
		}
		LogLn("");
		//len-=l;
		//if (len<=0)
		//break;
	}
	//LogLn("{end}");
}
#endif


void Log_Init()
{
#ifndef QUIET
	softwareSerial.begin(nSerialBaudDbg);
#endif
}
