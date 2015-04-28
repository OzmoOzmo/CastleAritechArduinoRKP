/*
 * Log.cpp - Logs all to Hyterterminal or other via Arduino Pin 11
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


#include "Log.h"


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
	Serial.begin(nSerialBaudDbg);
#endif
}


#ifndef QUIET
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

static int freeRam () {int v;return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);}
//Log("Ram:");int nRam = freeRam();LogLn(nRam);
void Log_ShowMem()
{
/*	Log(F("\r\n------\r\n"));
	Log(F("Freemem: ")); Log(freeRam()); Log(F(" (bytes)\r\n"));
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
	Log(F("\r\nSizes In Dec Bytes\r\n"));
	Log("ram  =["); Log2( (int)RAMEND - (int)&__data_start, DEC); Log("] ");
	Log(".data=["); Log2( (int)&__data_end-(int) &__data_start, DEC ); Log("] ");
	Log(".bss =["); Log2( (int) &__bss_end - (int) &__bss_start, DEC ); Log("]\r\n");
	Log("heap =["); Log2( (int) __brkval - (int) &__heap_start, DEC ); Log("] ");
	Log("stack=["); Log2( (int) RAMEND - (int) SP, DEC ); Log("] ");
	Log("free =["); Log2( (int) SP - (int) __brkval, DEC ); Log("] ");
	Log("free =["); Log2( ramSize - stackSize - heapSize - bssSize - dataSize, DEC ); Log("]\r\n");
	LogLn(F("-----"));
*/
	LogLn("Ready "sVersion);

}
#else
void Log_ShowMem()
{//do nothing
}
#endif
