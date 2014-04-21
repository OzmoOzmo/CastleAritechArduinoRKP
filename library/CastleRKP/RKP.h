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
		void static loop_PanelMon();
		static void DisplayScreen( byte* msgbuf, int msgbuflen);
		static char NextKeyPress();
		static char PopKeyPress();
		static void PushKey( char param1 );
		RKPClass();
		static void SendToPanel( int id, bool bAck);
static void Init();
static void SendDisplayToBrowser();
		static String msKeyToSend;
		
		static bool dateFlash;
		static byte dispBuffer[];
		static bool mbIsPanelWarning;
		static bool mbIsPanelAlarm;
};
//extern RKPClass rkp;

#endif
