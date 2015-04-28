/*
 * RKP.h
 *
 * Created: 3/30/2014 11:18:18 PM

 */ 

#ifndef RKP_h
#define RKP_h

#include "Arduino.h"



class RKPClass
{
	public:
		bool static loop_PanelMon();
		void static SendDisplayToClientIfChanged();
		static void DisplayScreen( byte* msgbuf, int msgbuflen);
		static char NextKeyPress();
		static char PopKeyPress();
		static void PushKey( char param1 );
		RKPClass();
		static void SendToPanel( int id, bool bAck);
		static void Init(byte nRKP_ID);
		static void SendDisplayToBrowser();
		static String msKeyToSend;
		
		static bool dateFlash;
		static byte dispBuffer[];
		static bool mbIsPanelWarning;
		static bool mbIsPanelAlarm;
		static byte mdevID;
		static bool bScreenHasUpdated;
		static unsigned long timeToSwitchOffLed;
};
//extern RKPClass rkp;

#endif
