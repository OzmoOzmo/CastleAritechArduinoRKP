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
*
*   Author: Ozmo
*
*   See: http://www.boards.ie/vbulletin/showthread.php?p=88215184
*
*/

#ifdef AtmelStudio
  #include <Arduino.h>
  #include "RKP.h"
  #include "LOG.h"
  #include "WebSocket.h"
  #include "SMTP.h"
#else
  #include <Base64.h>
  #include <Config.h>
  #include <Log.h>
  #include <RKP.h>
  #include <sha1.h>
  #include <SMTP.h>
  #include <WebSocket.h>
  
  #include <SPI.h>
  #include <Ethernet.h>
  #include <EthernetClient.h>
  #include <EthernetServer.h>
  #include <util.h>
#endif

//Build Version (displayed on webpage)
const String sVersion="V2.0";

#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))
#endif

//change network settings to yours
//--------Configuration Start----------


//Flashing Led we can use to show activity
int ledFeedback = 13;
int ledFeedbackState = HIGH;
int tiFlip = 0;
int tiLast = 0;





//Allows us to see home much ram we have left to play with
//static int freeRam () {extern int __heap_start, *__brkval;int v;return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);}
//#include <MemoryFree.h>

extern unsigned int __data_start;
extern unsigned int __data_end;
extern unsigned int __bss_start;
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;
int16_t ramSize=0;   // total amount of ram available for partitioning
int16_t dataSize=0;  // partition size for .data section
int16_t bssSize=0;   // partition size for .bss section
int16_t heapSize=0;  // partition size for current snapshot of the heap section
int16_t stackSize=0; // partition size for current snapshot of the stack section
int16_t freeMem1=0;  // available ram calculation #1
int16_t freeMem2=0;  // available ram calculation #2

//* This function places the current value of the heap and stack pointers in the
// * variables. You can call it from any place in your code and save the data for
// * outputting or displaying later. This allows you to check at different parts of
// * your program flow.
// * The stack pointer starts at the top of RAM and grows downwards. The heap pointer
// * starts just above the static variables etc. and grows upwards. SP should always
// * be larger than HP or you'll be in big trouble! The smaller the gap, the more
// * careful you need to be. Julian Gall 6-Feb-2009.
// *
uint8_t *heapptr, *stackptr;
//uint16_t diff=0;
void check_mem() {
  stackptr = (uint8_t *)malloc(4);          // use stackptr temporarily
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);      // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);           // save value of stack pointer
}
	
void memrep()                     // run over and over again
{
	
	Log("\r\n--------------------------------------------\r\n");
	Log("Freemem: "); Log(freeRam()); Log(" (bytes)\r\n");
	//Log(" which must be > 0 for no heap/stack collision. SP should always be larger than HP or you'll be in big trouble!");
	check_mem();
	Log("\r\nheapptr=[0x"); Log2( (int) heapptr, HEX); Log("]UP ");
	Log("  stackptr=[0x"); Log2( (int) stackptr, HEX); Log("]DWN "); // (growing downward, Log2( (int) stackptr, DEC); Log(" decimal)");
	Log("  sp-hp=[");Log2((int) (stackptr-heapptr), DEC); LogLn("]bytes\r\n");  //heapptr need be +ve
	//Log("Freemem:"); LogLn(freeRam()); Log("]\r\n");
	//Log("] (bytes) which must be > 0 for no heap/stack collision");
	// ---------------- Print memory profile -----------------
	Log("data_start=0x"); Log2( (int) &__data_start, HEX );	Log("   data_end=0x"); Log2((int) &__data_end, HEX );
	Log("   bss_start=0x"); Log2((int) & __bss_start, HEX ); Log("   bss_end=0x"); Log2( (int) &__bss_end, HEX ); 
	Log("\r\nheap_start=0x"); Log2( (int) &__heap_start, HEX ); Log("   malloc_heap_start=0x"); Log2( (int) __malloc_heap_start, HEX ); 
	Log("   malloc_margin=0x"); Log2( (int) &__malloc_margin, HEX ); Log("   brkval=0x"); Log2( (int) __brkval, HEX ); 
	Log("\r\nSP=0x"); Log2( (int) SP, HEX ); Log("   RAMEND=0x"); Log2( (int) RAMEND, HEX ); 

	// summaries:
	Log("\r\nSizes In Dec Bytes\r\n");
	Log("ram  =["); Log2( (int)RAMEND - (int)&__data_start, DEC); Log("] ");
	Log(".data=["); Log2( (int)&__data_end-(int) &__data_start, DEC ); Log("] ");
	Log(".bss =["); Log2( (int) &__bss_end - (int) &__bss_start, DEC ); Log("]\r\n");
	Log("heap =["); Log2( (int) __brkval - (int) &__heap_start, DEC ); Log("] ");
	Log("stack=["); Log2( (int) RAMEND - (int) SP, DEC ); Log("] ");
	Log("free =["); Log2( (int) SP - (int) __brkval, DEC ); Log("] ");
	Log("free =["); Log2( ramSize - stackSize - heapSize - bssSize - dataSize, DEC ); Log("]\r\n");
	LogLn("-----");
	/**/
}

void setup()
{
	Log_Init();
	
	RKPClass::Init();
	
	LogLn("");
	LogLn("-----[Start]-----");
	WebSocket::WebSocket_EtherInit();

	//Flashing led
	pinMode(ledFeedback,OUTPUT); digitalWrite(ledFeedback,ledFeedbackState);

	//Log("Ram:");int nRam = freeRam();LogLn(nRam);
	memrep();
	
	LogLn("Ready "+sVersion);
}

void loop()
{
	RKPClass::loop_PanelMon();

	//Any browser activity?	
	WebSocket::EtherPoll();
	
	//Send Email if Alarm
	if(SMTP::nEmailStage >= 0)	//TODO: put back
		SMTP::SendEmailProcess();
	
	//Flash status led
	int tiNow = millis();
	if (tiLast < tiNow - 500)
	{
		tiLast = tiNow;
		
		ledFeedbackState = ledFeedbackState == HIGH? LOW:HIGH;
		digitalWrite(ledFeedback,ledFeedbackState);
		//LogLn((unsigned long)tiNow);
		//Log(F("."));
		//Test comms: RKPClass::SendToPanel( 0 );
	}

	if (RKPClass::NextKeyPress() == 'X')
	{
		memrep();
		RKPClass::PopKeyPress();
	}

	//if (RKPClass::NextKeyPress() == 'A')
	//{
	//	SMTP::QueueEmail();
	//	RKPClass::PopKeyPress();
	//}
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
