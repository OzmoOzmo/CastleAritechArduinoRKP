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

//See Config.h for all the configuration you may need

#ifdef AtmelStudio
  #include <Arduino.h>
  #include "RKP.h"
  #include "LOG.h"
  #include "WebSocket.h"
  #include "SMTP.h"
#else
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


//Flashing Led we can use to show activity
#define ledFeedback 12
int tiLast = 0;


void setup()
{
  	//--debug logging start
  	Log_Init();
   	//while (!Serial) {;} // wait for serial port to connect. Needed for Leonardo only
	LogLn(F("\r\n-----[Start]-----"));

	RKPClass::Init();
	
   	//--Create Web Server
   	WebSocket::WebSocket_EtherInit();

   	SMTP::Init();

   	SMTP::QueueEmail(START);

	//Flashing led on Pin 12 show us we are still working ok
	pinMode(ledFeedback,OUTPUT); digitalWrite(ledFeedback,LOW);
}

void loop()
{
  while (true)
  {  
	//Flash status led
	int tiNow = millis();
	if (tiLast < tiNow - 500)
	{
		tiLast = tiNow;
		digitalWrite(ledFeedback, !digitalRead(ledFeedback));
	}

	//Knock off the "We Sent To Panel" Led not less than 100ms after we turn it on
	if (RKPClass::timeToSwitchOffLed != 0 && tiNow > RKPClass::timeToSwitchOffLed)
	{
		RKPClass::timeToSwitchOffLed = 0;
		digitalWrite(LED_Stat, LOW);
	}
    
	RKPClass::SendItems();

	if (RKPClass::bScreenHasUpdated)
		RKPClass::SendDisplayToClientIfChanged();

	//Any browser activity?	Set up new connections and deliver web pages
	WebSocket::EtherPoll();

	//Send Email if Alarm
	if (SMTP::nEmailStage >= 0)
		SMTP::SendEmailProcess();

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
