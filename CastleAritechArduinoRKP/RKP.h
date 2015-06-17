/*
 * RKP.h
 *
 * Created: 3/30/2014 11:18:18 PM
 */ 

#ifndef RKP_h
#define RKP_h

#include "Arduino.h"

#define maxkeybufsize 6
#define LED_Stat 13  //displays packets using LED 13

class FIFO
{
	public:
		FIFO();
		byte pop();
		void push( byte b );
	private:
		int nextIn;
		int nextOut;
		int count;
		static byte raw[maxkeybufsize];
};


class RKPClass
{
		static FIFO fifo;
		static volatile int _nLen;
		static byte _r[4];

	public:
		void static SendDisplayToClientIfChanged();
		void static SendItems();
		//static char NextKeyPress();
		static char PopKeyPress();
		static void PushKey( char param1 );
		RKPClass();
		static void Init();
		static void SendToPanel(byte* r, int nLen);  //send a message to the panel
		static void SendToPanelEx(byte* r, int len);
		static void SendToPanel( bool bAck );
		static volatile bool bScreenHasUpdated; //When true there is a display buffer ready to g
		static /*volatile*/ byte dispBuffer[];

		static void DecodeScreen( byte* msgbuf, int msgbufLen ); //Renders the screen from compressed bytes to Text
                
		//static bool dateFlash;
		
		static bool mbIsPanelWarning;
		static bool mbIsPanelAlarm;
		static unsigned long timeToSwitchOffLed;
                static byte lastkey;
      private:

};

#endif
