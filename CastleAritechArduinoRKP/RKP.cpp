/*
 * RKP.cpp - All the Remote Keypad comms and display commands
 *
 * Created: 3/30/2014 10:04:10 PM
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
 *
 * ref. Gen. Interrupts http://gammon.com.au/interrupts
 *      UART Interrupts http://www.electroons.com/blog/2013/02/interrupt-driven-uart-serial-communication-for-atmel-avr/
        Low Level UART  http://www.avrfreaks.net/forum/tut-soft-using-usart-serial-communications?name=PNphpBB2&file=viewtopic&t=45341
        Bits explained  http://maxembedded.com/2013/09/the-usart-of-the-avr/
*/

/*http://www.appelsiini.net/2011/simple-usart-with-avr-libc
UCSR0A Bit #	Name	Description
bit 7	RXC0	USART Receive Complete. Set when data is available and the data register has not be read yet.
bit 6	TXC0	USART Transmit Complete. Set when all data has transmitted.
bit 5	UDRE0	USART Data Register Empty. Set when the UDR0 register is empty and new data can be transmitted.
bit 4	FE0	Frame Error. Set when next byte in the UDR0 register has a framing error.
bit 3	DOR0	Data OverRun. Set when the UDR0 was not read before the next frame arrived.
bit 2	UPE0	USART Parity Error. Set when next frame in the UDR0 has a parity error.
bit 1	U2X0	USART Double Transmission Speed. When set decreases the bit time by half doubling the speed.
bit 0	MPCM0	Multi-processor Communication Mode. When set incoming data is ignored if no addressing information is provided.

UCSR0B Bit #	Name	Description
bit 7	RXCIE0	RX Complete Interrupt Enable. Set to allow receive complete interrupts.
bit 6	TXCIE0	TX Complete Interrupt Enable. Set to allow transmission complete interrupts.
bit 5	UDRIE0	USART Data Register Empty Interrupt Enable. Set to allow data register empty interrupts.
bit 4	RXEN0	Receiver Enable. Set to enable receiver.
bit 3	TXEN0	Transmitter enable. Set to enable transmitter.
bit 2	UCSZ20	USART Character Size 0. Used together with UCSZ01 and UCSZ00 to set data frame size. Available sizes are 5-bit (000), 6-bit (001), 7-bit (010), 8-bit (011) and 9-bit (111).
bit 1	RXB80	Receive Data Bit 8. When using 8 bit transmission the 8th bit received.
bit 0	TXB80	Transmit Data Bit 8. When using 8 bit transmission the 8th bit to be submitted.

UCSR0C Bit #	Name	Description
bit 7	UMSEL01  USART Mode Select 1 and 0. UMSEL01 and UMSEL00 combined select the operating mode. Available modes are asynchronous (00), synchronous (01) and master SPI (11).
bit 6	UMSEL00
bit 5	UPM01  USART Parity Mode 1 and 0. UPM01 and UPM00 select the parity. Available modes are none (00), even (10) and odd (11).
bit 4	UPM00
bit 3	USBS0	USART Stop Bit Select. Set to select 1 stop bit. Unset to select 2 stop bits.
bit 2   UCSZ01  USART Character Size 1 and 0. Used together with with UCSZ20 to set data frame size. Available sizes are 5-bit (000), 6-bit (001), 7-bit (010), 8-bit (011) and 9-bit (111).
bit 1	UCSZ00
bit 0	UCPOL0	USART Clock Polarity. Set to transmit on falling edge and sample on rising edge. Unset to transmit on rising edge and sample on falling edge.
*/


//change network settings to yours
#include <Arduino.h>
#include "RKP.h"
#include "LOG.h"
#include "Websocket.h"
#include "SMTP.h"
#include "Config.h"

#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define UART_BAUD_SELECT(baudRate,xtalCpu)  (((xtalCpu) + 8UL * (baudRate)) / (16UL * (baudRate)) -1UL)


#ifdef UCSR1A
/////////////Translations for Leonardo////////////
#define UCSRA UCSR1A
#define UCSRB UCSR1B
#define UCSRC UCSR1C //_SFR_MEM8(0xCA)

#define UBRRH UBRR1H
#define UBRRL UBRR1L

#define UCPOL UCPOL1
#define UCSZ0 UCSZ10
#define UCSZ1 UCSZ11
#define USBS USBS1
#define UPM0 UPM10
#define UPM1 UPM11
#define UMSEL0 UMSEL10
#define UMSEL1 UMSEL11

#define UCSRA UCSR1A //_SFR_MEM8(0xC8)
#define MPCM MPCM1 //0
#define U2X U2X1 //1
#define UPE UPE1 //2
#define DOR DOR1 //3
#define FE FE1 //4
#define UDRE UDRE1 //5
#define TXC TXC1 //6
#define RXC RXC1 //7

#define UCSRB UCSR1B //_SFR_MEM8(0xC9)
#define TXB8 TXB81 //0
#define RXB8 RXB81 //1
#define UCSZ2 UCSZ12 //2
#define TXEN TXEN1 //3
#define RXEN RXEN1 //4
#define UDRIE UDRIE1 //5
#define TXCIE TXCIE1 //6
#define RXCIE RXCIE1 //7

#define UDR UDR1 //_SFR_MEM8(0xCE)


#else
/////////////UNO etc///////////

#define UCSRA UCSR0A
#define UCSRB UCSR0B
#define UCSRC UCSR0C //_SFR_MEM8(0xCA)
#define UBRRH UBRR0H
#define UBRRL UBRR0L

#define UCPOL UCPOL0
#define UCSZ0 UCSZ00
#define UCSZ1 UCSZ01
#define USBS USBS0
#define UPM0 UPM00
#define UPM1 UPM01
#define UMSEL0 UMSEL00
#define UMSEL1 UMSEL01

#define UCSRA UCSR0A //_SFR_MEM8(0xC8)
#define MPCM MPCM0 //0
#define U2X U2X0 //1
#define UPE UPE0 //2
#define DOR DOR0 //3
#define FE FE0 //4
#define UDRE UDRE0 //5
#define TXC TXC0 //6
#define RXC RXC0 //7

#define UCSRB UCSR0B //_SFR_MEM8(0xC9)
#define TXB8 TXB80 //0
#define RXB8 RXB80 //1
#define UCSZ2 UCSZ02 //2
#define TXEN TXEN0 //3
#define RXEN RXEN0 //4
#define UDRIE UDRIE0 //5
#define TXCIE TXCIE0 //6
#define RXCIE RXCIE0 //7

#define UDR UDR0 //_SFR_MEM8(0xCE)

#endif

#define nSerialBaudKP_RX 1953 // 1953
#define ixMaxPanel 40	//40 bytes enough

const char PROGMEM allmonths[] = {"JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC"};
const char PROGMEM alldays[] = {"SUNMONTUEWEDTHUFRISAT"};

//String RKPClass::msKeyToSend="";
//bool RKPClass::dateFlash=true;
bool RKPClass::mbIsPanelWarning=false;
bool RKPClass::mbIsPanelAlarm=false;
volatile bool RKPClass::bScreenHasUpdated = false;


volatile int RKPClass::_nLen=0;
byte RKPClass::_r[4]; //the reply buffer

unsigned long RKPClass::timeToSwitchOffLed = 0;

FIFO RKPClass::fifo;
byte RKPClass::lastkey = 0xFF;

#define DISP_BUF_LEN 16+1+2		//16 characters - space - AW (will be followed by a Terminator 0)
/*volatile*/ byte RKPClass::dispBuffer[DISP_BUF_LEN + 1]="Not Connected";


RKPClass::RKPClass()
{//never runs
}

void uart_init()
{
	//disable
	DDRB |= _BV(0);      //sbi(DDRB, 0);
	PORTB &= ~(_BV(0));  //cbi(PORTB, 0);
	//configure
	UCSRA = 0;
	UCSRB = (1 << TXEN) | (1 << RXEN) | (1 << RXCIE);
	UCSRC = /*1<<URSEL |*/ 1 << UCSZ1 | 1 << UCSZ0 ; //1 stop bit 0 parity
	//  UCSRB |= (1 << UCSZ2); // Character Size = 9-bit
	UBRRL =  (unsigned char)UART_BAUD_SELECT(nSerialBaudKP_RX, F_CPU); //baud
	UBRRH =  (unsigned char)(UART_BAUD_SELECT(nSerialBaudKP_RX, F_CPU) >> 8);  //baud
	//enable
	sei();
}

void RKPClass::SendItems()
{
	if (_nLen!=0)
	{
		SendToPanelEx(_r, _nLen);
		_nLen=0;
	}
}

//Actually sends the message
void RKPClass::SendToPanelEx(byte* r, int len)
{
	digitalWrite(LED_Stat, HIGH);
	timeToSwitchOffLed = millis() + 50;

	//Interrupts Must be enabled
	for (int n = 0; n < len; n++)
	{
		loop_until_bit_is_set(UCSRA, UDRE); // Wait until data register empty.
		UDR = r[n];
	}
}

#if defined(UART1_RX_vect)
ISR(UART1_RX_vect)
#elif defined(USART1_RX_vect)
ISR(USART1_RX_vect)
#elif defined(USART_RX_vect)
ISR(USART_RX_vect) //UNO has no Serial1
#else
#error "No Serial Port?"
#endif
{
	static int msgbufLen = 0;
	static byte msgbuf[ixMaxPanel];
	static bool bReceivingPacketNow = false;
	static byte lastByte=-1;

	//Note - nothing here is to be blocking
	//- so sending emails must not block receiving serial logs
	byte rx = UDR;
	if (bReceivingPacketNow == false)
	{
		if(lastByte == 0)
		{//last char was a zero - this was the end of the last message
			if (rx==0)
				return; //CD34 sends 0 at start And 0 at End - different to CD72 and 92 - so ignore extra zero

			//It may be for us or not - but get whole message before testing so we stay in sync
			bReceivingPacketNow = true;
			msgbufLen=0;
			//not necessary to blank - but good for debug
			for(int n=0;n<ixMaxPanel;n++)
				msgbuf[n]=0xFF;
			msgbuf[msgbufLen++]=rx;
		}
		lastByte = rx;
		return; //wait for byte#3 (0,id,..)
	}
	lastByte = rx;	//needed in case cs error

	msgbuf[msgbufLen++]=rx;
	if (rx!=0)
	{//wasn't end of packet - is buffer full?
		if (msgbufLen>=ixMaxPanel)
		{//packet never terminated - bytes lost :(
			Log(F("Buf overflow"));
			bReceivingPacketNow = false;
			return;
		}
	}
	else
	{//End of the packet
		bReceivingPacketNow = false;

		byte idDev = (msgbuf[0] & 0xf0)>>4; //ID of remote that message is for 0x0-0xf are valid ids

		//Uncomment to Display all packets
		#ifdef DISPLAY_ALL_PACKETS
		Log(idDev);Log(')');LogHex(msgbuf,msgbufLen);
		#endif

		byte cs = 0;
		for(int n=0;n<msgbufLen;n++)
		{
			byte rx = msgbuf[n];
			if (n<msgbufLen-2) //dont sum cs or terminator
				cs+=rx;
		}

		if (cs == 0)
			cs++; //protocol avoids 0 except for end marker- so will send cs 00 as 01
		if (cs != msgbuf[msgbufLen-2])
		{
			LogLn(F("CS Fail :( "));
			LogHex(msgbuf,msgbufLen);
			//Log(F("Dev:"));LogLn(idDev);LogLn(F("!"));
			return;
		}

		//For CD34 this is ok- but not for 72/92 as there is no start 0. lastByte=-1; //good message - so waiting again for next zero

		//for us?
		if (idDev==RKP_ID)
		{
			bool bAck = (msgbuf[0] & 0x04) != 0;	//required for good comms
			RKPClass::SendToPanel(bAck);
			RKPClass::DecodeScreen(msgbuf, msgbufLen); //Queue up send to browser the screen as text
			//LogLn(F("-OK-"));
		}
		else
		{
			//LogLn("-Not for us-");
		}

		{//Send Email if alarm lights come on
			bool bIsPanelWarning = (msgbuf[1] & 0x04) != 0;
			if (bIsPanelWarning == true && RKPClass::mbIsPanelWarning == false)
				SMTP::QueueEmail(WARNING);
			RKPClass::mbIsPanelWarning = bIsPanelWarning;

			bool bIsPanelAlarm = (msgbuf[1] & 0x02) != 0;
			if (bIsPanelAlarm == true && RKPClass::mbIsPanelAlarm == false)
				SMTP::QueueEmail(ALARM);
			RKPClass::mbIsPanelAlarm = bIsPanelAlarm;
		}
	}
}




void RKPClass::DecodeScreen( byte* msgbuf, int msgbufLen )
{
	byte bufix=0;

	//Checksum so can see if changes and need update client
	static int previousCS =-1;

	int ixMsgbuf=2;//skip 2 header bytes
	msgbufLen-=2; //remove checksum and zero terminator

	while(ixMsgbuf<msgbufLen)
	{
		byte rx = msgbuf[ixMsgbuf++];
		if (rx>=0 && rx < 0x0f)
		{//not implemented
		}
		else if (rx == 0x13)
		{//not implemented
		}
		else if (rx == 0x1b)
		{//to do with foreign character set - not implemented
		}
		else if (rx>= 0x20 && rx <= 0x7F)
		{//Normal ASCII
			if (bufix==0)
				//Force Screen clear at start of each message
				for(int m=0;m<DISP_BUF_LEN;m++)
					dispBuffer[m]=' ';

			if (bufix < DISP_BUF_LEN)
				dispBuffer[bufix++]=(char)rx;
		}
		else if (rx>= 0x80 && rx <= 0x8F)
		{//Date in encoded format
			int b0=rx;
			int b1=msgbuf[ixMsgbuf++];
			int b2=msgbuf[ixMsgbuf++];
			int b3=msgbuf[ixMsgbuf++];

			byte nMonth= (b0 & 0x0f)-1;
			byte day = (b1 & (128+64+32))>> 5;
			byte date = (b1 & (31));
			byte h1=(b2 & 0xf0)>>4; if(h1==0x0A) h1=0;
			byte h2=(b2 & 0x0f); if(h2==0x0A) h2=0;
			byte m1=(b3 & 0xf0)>>4; if(m1==0x0A) m1=0;
			byte m2=(b3 & 0x0f); if(m2==0x0A) m2=0;

			memcpy_P(dispBuffer+0,alldays+(day*3),3);
			dispBuffer[3]=' ';
			dispBuffer[4]=('0'+(int)(date/10));
			dispBuffer[5]=('0'+(date%10));
			dispBuffer[6]=' ';

			memcpy_P(dispBuffer+7,allmonths+(nMonth*3),3);
			dispBuffer[10]=' ';
			dispBuffer[11]='0'+h1;
			dispBuffer[12]='0'+h2;
			//if (dateFlash)
			dispBuffer[13]= ':';
			//else
			//	buffer[13]= F(' ');
			//dateFlash=!dateFlash;
			//buffer[13]= ((millis()/500)&1) ==0? ':':' ';
			dispBuffer[14]='0'+m1;
			dispBuffer[15]='0'+m2;
			bufix=0;
		}
		else if (rx == 0x90)
		{//CLS
			bufix=0;
			for(int m=0;m<DISP_BUF_LEN;m++)
				dispBuffer[m]=' ';
		}
		else if (rx == 0x91)
		{//HOME
			bufix=0;
		}
		else if (rx >= 0xA0 && rx <= 0xAf)
		{//MOVE cursor to position x
			bufix = (rx & 0x0f); //-1 gives us 2 *'s  but without -1 we go off screen at Login ***
		}
		else if (rx >= 0xB0 && rx <= 0xBF)
		{//{BLINK_N}"	Bxh Blink x chars starting at current cursor position
		 //not implementing this as it will cause unnecessary traffic sending display each second
			//int nChars = (rx & 0x0f)-1;
			//if (dateFlash)
			//	buffer[i]= ':';
			//else
			//	buffer[i]= ' ';
			//dateFlash=!dateFlash;
		}
		else if (rx >= 0xC0 && rx <= 0xCf)
		{// Set position to x and clear all chars to right
			int i = (rx & 0x0f);
			if (i < DISP_BUF_LEN)
				bufix = i;
			for(int n=bufix;n<DISP_BUF_LEN;n++)
				dispBuffer[bufix++]=' ';
		}
		else if (rx>= 0xE0 && rx <= 0xFF)
		{// Special Characters Arrows and foreign chars
			int i = (rx & 0x0f);

			char c=0;
			if (i==4)	c= '*';
			else if (i==5)	c= '#';
			else if (i==7)	c= '>';

			if (c>0)
				if (bufix < DISP_BUF_LEN)
					dispBuffer[bufix++]=c;
		}
		else
		{//unknown command
			Log(F("{"));LogHex(rx);Log(F("}"));
		}

		//Note: there are quite a few codes in Engineer menu to deal with flashing cursors and characters - cannot do easily in html
	}

	dispBuffer[16]='|';	//this may overwrite a char sometimes...ok.
	dispBuffer[17]=(RKPClass::mbIsPanelAlarm)?'A':' ';
	dispBuffer[18]=(RKPClass::mbIsPanelWarning)?'W':' ';

	int cs=0;
	for(int n=0;n<DISP_BUF_LEN;n++)
		cs+=(char)dispBuffer[n];

	if (cs != previousCS)
	{
		////Display as hex
		//LogHex(msgbuf, msgbufLen);
		////Display as characters
		//for(int n=0;n<DISP_BUF_LEN;n++)	Log((char)dispBuffer[n]); LogLn(".");
		previousCS=cs;

		//Signal new screen to send but dont send immediately- SendDisplayToBrowser can take time
		bScreenHasUpdated = true;
	}

	//LogHex(msgbuf,msgbufLen);  LogHex((byte*)dispBuffer,DISP_BUF_LEN);LogLn(".");
}

char RKPClass::PopKeyPress()
{
  return (char)fifo.pop();
}

void RKPClass::PushKey( char key )
{
  fifo.push(key);
}

void RKPClass::SendDisplayToClientIfChanged()
{
  bScreenHasUpdated = false;    //possible issue if interrupt has changed this since we checked it - might miss one screen update
  //LogHex((byte*)dispBuffer,18);
  WebSocket::WebSocket_send((char*)dispBuffer, DISP_BUF_LEN);
  //Log("Sent:"); LogLn((char*)dispBuffer);
}


void RKPClass::Init()
{
  uart_init();

  dispBuffer[DISP_BUF_LEN] = 0;
  pinMode(LED_Stat, OUTPUT);
}

//a 6 character keyboard buffer
byte FIFO::raw[maxkeybufsize];
FIFO::FIFO()
{
  nextIn = nextOut = count = 0;
}
void FIFO::push( byte element )
{
  if ( count >= maxkeybufsize )
  {
    Log("Too Full. Count=");LogLn(count);
    return; //lost
  }
  count++;
  raw[nextIn++] = element;
  nextIn %= maxkeybufsize;
  Log("Added Item. Count=");LogLn(count);
}

byte FIFO::pop()
{
  if (count>0)
  {
    count--;
    byte c=raw[ nextOut++];
    nextOut %= maxkeybufsize;
    
    //Log("Popped Item. Count=");Log(count);Log(" c=");LogLn(c);
    return c;
  }
  return 0xFF;
}


void RKPClass::SendToPanel( bool bAck )
{
	//Log(F("Send: "));
	#define H1_LIDTAMP 1
	#define H1_BASE 2 //always set - probably to make sure its not zero(start message)
	#define H1_CD3008 0 //keypad with buzzer  - works on cd34 and cd72
	#define H1_CD9038 4 //Keypad with 4 zones - works on cd34 (TOTEST: if cd72 supported)
	#define H1_CD9041 8 //Glassbreak Expander with no keypad - works on cd34 but not cd72(Keypad displays Error)
	static byte nH2Previous=-1;

	byte h1 = H1_BASE + H1_CD3008 + (RKP_ID<<4); //kKKK (Capital K is an Extender enabled keypad)
	byte h2 = 0;

	if (bAck)	//change to if(true) to remove retries
	{//LogLn("Ack");
		char nBrowserKeyPress = PopKeyPress();
		if (nBrowserKeyPress!=-1)
		{
			//nKeyToSend &= 0x20; //tolower
			int nNumb=0;
			if (nBrowserKeyPress >='1' && nBrowserKeyPress <= '9')
			nNumb = (nBrowserKeyPress-'0');
			else if (nBrowserKeyPress == '0')	nNumb = 0x0a;
			else if (nBrowserKeyPress == 'f'||nBrowserKeyPress == '*')	nNumb = 0x0b;	//UP  (* for IPhone)
			else if (nBrowserKeyPress == 'v'||nBrowserKeyPress == '#')	nNumb = 0x0c;	//DOWN (# for IPhone)
			else if (nBrowserKeyPress == 'p')	nNumb = 0x0d;	//UP + DOWN (Panic)
			else if (nBrowserKeyPress == 'x'||nBrowserKeyPress == ';'||nBrowserKeyPress == 'n'||nBrowserKeyPress == 'N')	nNumb = 0x0e;	//UP + 0 or X(reject) (WAIT on IPhone numpad)
			else if (nBrowserKeyPress == 13||nBrowserKeyPress == '+'||nBrowserKeyPress == 'y'||nBrowserKeyPress == 'Y')	nNumb = 0x0f;	//UP + 0 or *(accept) (+ on IPhone numpad)

			Log(F("Sent: ")); LogHex(nNumb); LogHex(nBrowserKeyPress); LogLn(F("."));

			if(nNumb!=0)
				h2 = nNumb<<4;
		}
		nH2Previous = h2;
	}
	else
	{//If panel didnt get last kepress, resend it - if 0xFF or 0 then its a resend but the default normal we would have sent anyway.
		h2 = (nH2Previous == 0xFF)? 0 : nH2Previous;
		{
			Log(F("Resend: ("));
			LogHex(h2);
			LogLn(F(")"));
		}
	}

	byte h3=0; byte h4=(byte)(h1+h2+h3);

	digitalWrite(LED_Stat, HIGH);
        _r[0]=h1;
        _r[1]=h2;
        _r[2]=h3;
        _r[3]=h4;        
	_nLen=4;

	//Log("Send>");LogHex(h1); LogHex(h2); LogHex(h3); LogHex(h4); LogLn(".");
}


