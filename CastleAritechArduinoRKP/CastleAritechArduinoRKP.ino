/*
*   Castle KeyPad
*
*   Aritech Alarm Panel Arduino Internet Enabled Keypad -  CS350 - CD34 - CD72 - CD91 and more 
*   
*   For Arduino (UNO or Leonardo) with added Ethernet Shield
*   
*   See Circuit Diagram for wiring instructions
*
*   V1.04  March  2014: Arm Disarm Complete and tested as working
*   V1.2	  April  2014: Updated to use ABCD Keypad bus
*   V1.3	  April  2015: Updated to current Arduino IDE
*
*   Author: Ozmo
*
*   See: http://www.boards.ie/vbulletin/showthread.php?p=88215184
*
*/

//--------Configuration Start----------
//This is the Keypad id of the Arduinio - recommended 1 or 2 (0 is first keypad)
#define RKP_ID 1

//The Arduino IP address and Port (192.168.1.205:8383)
#define IP_A 192
#define IP_B 168
#define IP_C 1
#define IP_D 205

//eg. this is the IP for "smtp.upcmail.ie" (will only work if you are a UPC customer)
#define SMTP_IP_A 213
#define SMTP_IP_B 46
#define SMTP_IP_C 255
#define SMTP_IP_D 2


/*---To Send emails You NEED get the IP address of the SMTP server from the list below that matches your Internet Provider
  No DNS lookup as Ive removed that from my libs to save space...

smtp.mysmart.ie							Smart Telecom Outgoing SMTP Server
smtp.irishbroadband.ie.					Irish Broadband Outgoing SMTP Server
mail1.eircom.net OR mail2.eircom.net	Eircom Outgoing SMTP Server
smtp.magnet.ie							Magnet Outgoing SMTP Server
smtp.upcmail.ie							NTL and UPC Outgoing SMTP Server
mail.icecomms.net						Ice Broadband Outgoing SMTP Server
mail.vodafone.ie						Vodafone Outgoing SMTP Server
smtp.o2.ie								O2 Outgoing SMTP Server
smtp.clearwire.ie / mail.clearwire.ie	Clearwire Outgoing SMTP Server
smtp.digiweb.ie							Digiweb Outgoing SMTP Server
mail.imagine.ie OR mail.gaelic.ie		Imagine Broadband Outgoing SMTP Server
mail.perlico.ie							Perlico Outgoing SMTP Server
mail-relay.3ireland.ie					3 Outgoing SMTP Server: Mobile broadband with 3 mobile Ireland
*/


//--------Configuration End----------

#ifdef AtmelStudio
  #include <Arduino.h>
  #include "RKP.h"
  #include "LOG.h"
  #include "WebSocket.h"
  #include "SMTP.h"
#else
  #include "Base64.h"
  #include "Config.h"
  #include "Log.h"
  #include "RKP.h"
  #include "sha1.h"
  #include "SMTP.h"
  #include "WebSocket.h"

  #include <SPI.h>
  #include <Ethernet.h>
  #include <EthernetClient.h>
  #include <EthernetServer.h>
#endif


//change this to your email address - set to NULL to Not send emails at all
const char* sEmail = NULL; //"ozmo@example.com";	

//Flashing Led we can use to show activity
int ledFeedback = 12;
int tiFlip = 0;
int tiLast = 0;


void setup()
{
	Log_Init();
	
	RKPClass::Init(RKP_ID);
	
	LogLn(F("\r\n-----[Start]-----"));
	WebSocket::WebSocket_EtherInit(
		IPAddress( IP_A, IP_B, IP_C, IP_D ),	//Give the device a unique IP
		IPAddress( IP_A, IP_B, IP_C, 1 )				//Gateway (your Router)
	);

	SMTP::Init(IPAddress( SMTP_IP_A, SMTP_IP_B, SMTP_IP_C, SMTP_IP_D ), sEmail);

	//Flashing led on Pin 12 show us we are still working ok
	pinMode(ledFeedback,OUTPUT); digitalWrite(ledFeedback,LOW);

	Log_ShowMem();

}

void loop()
{
	if (RKPClass::loop_PanelMon())
		RKPClass::SendDisplayToClientIfChanged();

	//Any browser activity?	Set up new connections and deliver web pages
	WebSocket::EtherPoll();
	
	//Send Email if Alarm
	if(SMTP::nEmailStage >= 0)
		SMTP::SendEmailProcess();
	
	//Flash status led
	int tiNow = millis();
	if (tiLast < tiNow - 500)
	{
		tiLast = tiNow;
		digitalWrite(ledFeedback,!digitalRead(ledFeedback));
		//Test comms: RKPClass::SendToPanel( 0 );
	}
}


#ifdef AtmelStudio
int main(void)
{
	init();

	#if defined(USBCON)
	USBDevice.attach();
	#endif
	
	setup();
	
	for (;;) {
		loop();
		if (serialEventRun) serialEventRun();
	}
	
	return 0;
}
#endif
